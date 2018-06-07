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
#include <switch.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

static const char *pCertFile = "./client.pem";
static const char *pKeyFile = "./key.pem";

static bool debug = true;

#define BUFFER_LENGTH 4096

int http_init(const char* keyDirectory, int logLevel) {
  char certificateFilePath[4096];
  sprintf(certificateFilePath, "%s/%s", keyDirectory, CERTIFICATE_FILE_NAME);

  char keyFilePath[4096];
  sprintf(&keyFilePath[0], "%s/%s", keyDirectory, KEY_FILE_NAME);

  return GS_OK;
}

int http_request(char* host, int port, char* path, PHTTP_DATA data) {
  if (debug)
    printf("Request %s:%d%s\n", host, port, path);

  if (data->size > 0) {
    free(data->memory);
    data->memory = malloc(1);

    if(data->memory == NULL) {
      gs_error = "Could not malloc() data for HTTP response before request";
      return GS_OUT_OF_MEMORY;
    }

    data->size = 0;
  }
  else {
    printf("* Allocated some memory\n");
  }

  // Create a socket
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (!sock) {
    gs_error = "Could not create socket";
    return GS_FAILED;
  }
  else {
    printf("* Created socket object\n");
  }

  // Connect to the server
  struct sockaddr_in server;
  server.sin_addr.s_addr = inet_addr(host);
  server.sin_port = htons(port);
  server.sin_family = AF_INET;

  if (connect(sock, &server, sizeof(server)) < 0) {
    gs_error = "Could not connect to server";
    return GS_FAILED;
  }
  else {
    printf("* Connected to server\n");
  }

  // Create the HTTP contents
  char *buffer = calloc(BUFFER_LENGTH, sizeof(char));
  snprintf(buffer, BUFFER_LENGTH, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n", path, host);
  int bytes_sent = send(sock, buffer, strlen(buffer), 0);
  int bytes_to_send = strlen(buffer);
  printf("* Sent %d bytes of %d to server (%s)\n", bytes_sent, bytes_to_send, buffer);

  // Read the HTTP response
  while (1) {
      int bytes_received = recv(sock, buffer, BUFFER_LENGTH, 0);

      if (bytes_received == -1) {
        printf("Errno: %d\n", errno);
        gs_error = "Received error when trying to read";
        return GS_FAILED;
      } else if (bytes_received == 0) {
        break;
      }

      if (bytes_received > 0) {
        data->memory = realloc(data->memory, data->size + bytes_received);
        if (!data->memory) {
          gs_error = "Could not allocate memory for response";
          return GS_FAILED;
        }
        memcpy(data->memory + data->size, buffer, bytes_received);
        data->size += bytes_received;
      }
  }
  
  return GS_OK;
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
  data->size = 0;

  return data;
}

void http_free_data(PHTTP_DATA data) {
  if (data != NULL) {
    if (data->memory != NULL)
      free(data->memory);

    free(data);
  }
}
