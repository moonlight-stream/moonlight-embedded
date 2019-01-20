/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015 Iwan Timmer
 *
 * Moonlight is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonlight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight; if not, see <http://www.gnu.org/licenses/>.
 */

#include "http.h"
#include "errors.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <switch.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

static const char *pCertFile = "./client.pem";
static const char *pKeyFile = "./key.pem";

SSL_CTX *ssl_ctx;

static bool debug = true;

/**
 * Workaround for issues stemming from SSL.
 * Implementation courtesy of @Noah on the ReSwitched Discord.
 *
 * Per @natinusala, this method is called prior to every SSL read.
 */
typedef struct {
    u32 is_valid;
    nvioctl_fence nv_fences[4];
} PACKED bufferProducerFence;

Result nvgfxEventWait(u32 syncpt_id, u32 threshold, s32 timeout)
{
    Result rc;
    do {
        u32 event_res;
        rc = nvioctlNvhostCtrl_EventWait(-1, syncpt_id, threshold, timeout, 0, &event_res);
    } while (rc == MAKERESULT(Module_LibnxNvidia, LibnxNvidiaError_Timeout)); // todo: Fix timeout error
    return rc;
}

void FIXSLAB() {
    bufferProducerFence tmp_fence;
    nvgfxEventWait(tmp_fence.nv_fences[0].id,tmp_fence.nv_fences[0].value,-1);
}
/**
 * END SSL workaround.
 */

#define BUFFER_LENGTH 4096

int http_init(const char* keyDirectory, int logLevel) {
  char certificateFilePath[4096];
  sprintf(certificateFilePath, "%s/%s", keyDirectory, CERTIFICATE_FILE_NAME);

  char keyFilePath[4096];
  sprintf(&keyFilePath[0], "%s/%s", keyDirectory, KEY_FILE_NAME);

  // Create the SSL context
  ssl_ctx = SSL_CTX_new(SSLv23_client_method());

  if (ssl_ctx == NULL) {
    if (debug) fprintf(stderr, "* ERROR: error creating SSL context: %s\n", ERR_error_string(ERR_get_error(), NULL));
    return GS_FAILED;
  }
  else {
    if (debug) fprintf(stderr, "* Successfully created SSL context\n");
  }

  SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);

  if (!SSL_CTX_use_certificate_file(ssl_ctx, certificateFilePath, SSL_FILETYPE_PEM)) {
    if (debug) fprintf(stderr, "* ERROR: error loading certificate file: %s\n", ERR_error_string(ERR_get_error(), NULL));
    gs_error = ERR_error_string(ERR_get_error(), NULL);
    return GS_FAILED;
  }

  if (!SSL_CTX_use_PrivateKey_file(ssl_ctx, keyFilePath, SSL_FILETYPE_PEM)) {
    if (debug) fprintf(stderr, "* ERROR: error loading private key file: %s\n", ERR_error_string(ERR_get_error(), NULL));
    gs_error = ERR_error_string(ERR_get_error(), NULL);
    return GS_FAILED;
  }

  return GS_OK;
}

int http_request(char* host, int port, char* path, PHTTP_DATA data) {
  if (debug) printf("Request %s:%d%s\n", host, port, path);

  if (data->memory_size > 0) {
    free(data->memory);
    data->memory = malloc(1);

    if(data->memory == NULL) {
      gs_error = "Could not malloc() data for HTTP response before request";
      return GS_OUT_OF_MEMORY;
    }

    data->memory_size = 0;
  }
  else {
    if (debug) printf("* Allocated some memory\n");
  }

  // Create a socket
  char *buffer = NULL;
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (!sock) {
    gs_error = "Could not create socket";
    return GS_FAILED;
  }
  else {
    if (debug) printf("* Created socket object\n");
  }

  // Connect to the server
  struct sockaddr_in server;
  server.sin_addr.s_addr = inet_addr(host);
  server.sin_port = htons(port);
  server.sin_family = AF_INET;

  if (connect(sock, &server, sizeof(server)) < 0) {
    gs_error = "Could not connect to server";
    goto failure;
  }
  else {
    if (debug) printf("* Connected to server\n");
  }

  // Create the HTTP contents
  buffer = calloc(BUFFER_LENGTH, sizeof(char));
  snprintf(buffer, BUFFER_LENGTH, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);

  if (debug) printf("* Sending data: %s\n", buffer);

  int bytes_sent = send(sock, buffer, strlen(buffer), 0);
  int bytes_to_send = strlen(buffer);
  if (debug) printf("* Sent %d bytes of %d bytes to server\n", bytes_sent, bytes_to_send);

  // Read the HTTP response
  while (1) {
      int bytes_received = recv(sock, buffer, BUFFER_LENGTH, 0);

      if (bytes_received == -1) {
        if (debug) printf("Errno: %d\n", errno);
        gs_error = "Received error when trying to read";
        goto failure;
      } else if (bytes_received == 0) {
        break;
      }

      if (bytes_received > 0) {
        char *temp = malloc(data->memory_size + bytes_received);
        if (!temp) {
          gs_error = "Could not allocate memory for response";
          goto failure;
        }
        memcpy(temp, data->memory, data->memory_size);
        memcpy(temp + data->memory_size, buffer, bytes_received);
        free(data->memory);
        data->memory_size += bytes_received;
        data->memory = temp;
      }
  }
  

  // Parse the body by looking for the separator between header and body
  const char *sep = "\r\n\r\n";
  char *before_sep = strstr(data->memory, sep);

  if (before_sep != NULL) {
    data->body = before_sep + strlen(sep);
    data->body_size = data->memory_size - (data->body - data->memory);
  }
  else {
    data->body = data->memory;
    data->body_size = data->memory_size;
  }

success:
  if (debug) {
      printf("* Received %d bytes from server\n", data->memory_size);
      printf("* Received:\n%s\n\n", data->memory);
  }

  free(buffer);

  close(sock);
  return GS_OK;

failure:
  if (buffer)
    free(buffer);

  close(sock);
  return GS_FAILED;
}


int https_request(char* host, int port, char* path, PHTTP_DATA data) {
  if (debug) printf("HTTPS request: %s:%d%s\n", host, port, path);

  if (data->memory_size > 0) {
    free(data->memory);
    data->memory = malloc(1);

    if(data->memory == NULL) {
      gs_error = "Could not malloc() data for HTTP response before request";
      return GS_OUT_OF_MEMORY;
    }

    data->memory_size = 0;
  }
  else {
    if (debug) printf("* Allocated some memory\n");
  }

  // Create a socket
  char *buffer = NULL;
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (!sock) {
    gs_error = "Could not create socket";
    return GS_FAILED;
  }
  else {
    if (debug) printf("* Created socket object\n");
  }

  // Connect to the server
  struct sockaddr_in server;
  server.sin_addr.s_addr = inet_addr(host);
  server.sin_port = htons(port);
  server.sin_family = AF_INET;

  if (connect(sock, &server, sizeof(server)) < 0) {
    gs_error = "Could not connect to server";
    goto failure;
  }
  else {
    if (debug) printf("* Connected to server\n");
  }

  // Create the new SSL object
  SSL *ssl = SSL_new(ssl_ctx);

  if (!ssl) {
    int e = ERR_get_error();
    if (debug) fprintf(stderr, "* ERROR: error creating new SSL context: %s\n", ERR_error_string(e, NULL));
    gs_error = ERR_error_string(e, NULL);
    goto failure;
  }
  else {
    if (debug) printf("* Created SSL object\n");
  }

  // Set the SSL's socket
  if (!SSL_set_fd(ssl, sock)) {
    int e = ERR_get_error();
    if (debug) fprintf(stderr, "* ERROR: error wiring SSL context to socket: %s\n", ERR_error_string(e, NULL));
    gs_error = ERR_error_string(e, NULL);
    goto failure;
  }
  else {
    if (debug) printf("* Wired SSL object to socket\n");
  }

  // Connect via SSL
  FIXSLAB();
  if (SSL_connect(ssl) <= 0) {
    int e = ERR_get_error();
    if (debug) fprintf(stderr, "* ERROR: error connecting to server via SSL/TLS: %s\n", ERR_error_string(e, NULL));
    gs_error = ERR_error_string(e, NULL);
    goto failure;
  }
  else {
    if (debug) printf("* Connected via SSL/TLS to server using cipher `%s`\n", SSL_get_cipher(ssl));
  }

  // Create the HTTP contents
  buffer = calloc(BUFFER_LENGTH, sizeof(char));
  snprintf(buffer, BUFFER_LENGTH, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);

  if (debug) printf("* Sending data: %s\n", buffer);

  int bytes_sent = SSL_write(ssl, buffer, strlen(buffer));
  int bytes_to_send = strlen(buffer);
  if (debug) printf("* Sent %d bytes of %d bytes to server\n", bytes_sent, bytes_to_send);

  // Read the HTTP response
  while (1) {
      FIXSLAB();
      int bytes_received = SSL_read(ssl, buffer, BUFFER_LENGTH);

      if (bytes_received < 0) {
        int err = SSL_get_error(ssl, bytes_received);

        if (err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_SYSCALL || err == SSL_ERROR_SSL) {
          if (debug) printf("Errno: %d\n", errno);
          gs_error = "Received SSL error when trying to read";
          goto failure;
        }
        else {
          break;
        }
      } else if (bytes_received == 0) {
        break;
      }

      if (bytes_received > 0) {
        char *temp = malloc(data->memory_size + bytes_received);
        if (!temp) {
          gs_error = "Could not allocate memory for response";
          goto failure;
        }
        memcpy(temp, data->memory, data->memory_size);
        memcpy(temp + data->memory_size, buffer, bytes_received);
        free(data->memory);
        data->memory_size += bytes_received;
        data->memory = temp;
      }
  }

  // Parse the body by looking for the separator between header and body
  const char *sep = "\r\n\r\n";
  char *before_sep = strstr(data->memory, sep);

  if (before_sep != NULL) {
    data->body = before_sep + strlen(sep);
    data->body_size = data->memory_size - (data->body - data->memory);
  }
  else {
    data->body = data->memory;
    data->body_size = data->memory_size;
  }

  if (debug) printf("* Received data: %s\n", data->body);

success:
  if (debug) {
      printf("* Received %d bytes from server\n", data->memory_size);
      printf("* Received:\n%s\n\n", data->memory);
  }

  SSL_shutdown(ssl);
  free(buffer);
  close(sock);
  return GS_OK;

failure:
  if (buffer)
    free(buffer);

  close(sock);
  return GS_FAILED;
}

void http_cleanup() {

}

PHTTP_DATA http_create_data() {
  PHTTP_DATA data = malloc(sizeof(HTTP_DATA));
  if (data == NULL)
    return NULL;

  data->memory = malloc(1);
  if(data->memory == NULL) {
    free(data);
    return NULL;
  }
  data->memory_size = 0;

  data->body = data->memory;
  data->body_size = data->memory_size;

  return data;
}

void http_free_data(PHTTP_DATA data) {
  if (data != NULL) {
    if (data->memory != NULL)
      free(data->memory);

    free(data);
  }
}
