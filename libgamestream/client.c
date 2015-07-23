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
#include "xml.h"
#include "mkcert.h"
#include "client.h"
#include "errors.h"

#include "limelight-common/Limelight.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>

static const char *uniqueFileName = "uniqueid.dat";
static const char *certificateFileName = "client.pem";
static const char *p12FileName = "client.p12";
static const char *keyFileName = "key.pem";

#define UNIQUEID_BYTES 8
#define UNIQUEID_CHARS (UNIQUEID_BYTES*2)

static char unique_id[UNIQUEID_CHARS+1];
static X509 *cert;
static char cert_hex[4096];
static EVP_PKEY *privateKey;

static void load_unique_id() {
  FILE *fd = fopen(uniqueFileName, "r");
  if (fd == NULL) {
    unsigned char unique_data[UNIQUEID_BYTES];
    RAND_bytes(unique_data, UNIQUEID_BYTES);
    for (int i = 0; i < UNIQUEID_BYTES; i++) {
      sprintf(unique_id + (i * 2), "%02x", unique_data[i]);
    }
    fd = fopen(uniqueFileName, "w");
    fwrite(unique_id, UNIQUEID_CHARS, 1, fd);
  } else {
    fread(unique_id, UNIQUEID_CHARS, 1, fd);
  }
  fclose(fd);
  unique_id[UNIQUEID_CHARS] = 0;
}

static void load_cert() {
  FILE *fd = fopen(certificateFileName, "r");
  if (fd == NULL) {
    printf("Generating certificate...");
    CERT_KEY_PAIR cert = mkcert_generate();
    printf("done\n");
    mkcert_save(certificateFileName, p12FileName, keyFileName, cert);
    mkcert_free(cert);
    fd = fopen(certificateFileName, "r");
  }

  if (fd == NULL) {
    fprintf(stderr, "Can't open certificate file\n");
    exit(-1);
  }

  if (!(cert = PEM_read_X509(fd, NULL, NULL, NULL))) {
    fprintf(stderr, "Error loading cert into memory.\n");
    exit(-1);
  }

  rewind(fd);

  int c;
  int length = 0;
  while ((c = fgetc(fd)) != EOF) {
    sprintf(cert_hex + length, "%02x", c);
    length += 2;
  }
  cert_hex[length] = 0;

  fclose(fd);

  fd = fopen(keyFileName, "r");
  PEM_read_PrivateKey(fd, &privateKey, NULL, NULL);
  fclose(fd);
}

static int load_server_status(const char *address, PSERVER_DATA server) {
  int ret = GS_INVALID;
  char url[4096];
  sprintf(url, "https://%s:47984/serverinfo?uniqueid=%s", address, unique_id);

  PHTTP_DATA data = http_create_data();
  if (data == NULL) {
    ret = GS_OUT_OF_MEMORY;
    goto cleanup;
  }
  if (http_request(url, data) != GS_OK) {
    ret = GS_IO_ERROR;
    goto cleanup;
  }

  char *pairedText = NULL;
  char *currentGameText = NULL;
  char *versionText = NULL;
  if (xml_search(data->memory, data->size, "currentgame", &currentGameText) != GS_OK) {
    goto cleanup;
  }

  if (xml_search(data->memory, data->size, "PairStatus", &pairedText) != GS_OK)
    goto cleanup;

  if (xml_search(data->memory, data->size, "appversion", &versionText) != GS_OK)
    goto cleanup;

  server->paired = pairedText != NULL && strcmp(pairedText, "1") == 0;
  server->currentGame = currentGameText == NULL ? 0 : atoi(currentGameText);
  char *versionSep = strstr(versionText, ".");
  if (versionSep != NULL) {
    *versionSep = 0;
  }
  server->serverMajorVersion = atoi(versionText);
  ret = GS_OK;

  cleanup:
  if (data != NULL)
    http_free_data(data);

  if (pairedText != NULL)
    free(pairedText);

  if (currentGameText != NULL)
    free(currentGameText);

  if (versionText != NULL)
    free(versionText);

  return ret;
}

static void bytes_to_hex(unsigned char *in, char *out, size_t len) {
  for (int i = 0; i < len; i++) {
    sprintf(out + i * 2, "%02x", in[i]);
  }
  out[len * 2] = 0;
}

static int sign_it(const char *msg, size_t mlen, unsigned char **sig, size_t *slen, EVP_PKEY *pkey) {
  int result = GS_FAILED;

  *sig = NULL;
  *slen = 0;

  EVP_MD_CTX *ctx = EVP_MD_CTX_create();
  if (ctx == NULL) {
    fprintf(stderr, "EVP_MD_CTX_create failed, error 0x%lx\n", ERR_get_error());
    return -1;
  }

  const EVP_MD *md = EVP_get_digestbyname("SHA256");
  if (md == NULL) {
    fprintf(stderr, "EVP_get_digestbyname failed, error 0x%lx\n", ERR_get_error());
    goto cleanup;
  }

  int rc = EVP_DigestInit_ex(ctx, md, NULL);
  if (rc != 1) {
    fprintf(stderr, "EVP_DigestInit_ex failed, error 0x%lx\n", ERR_get_error());
    goto cleanup;
  }

  rc = EVP_DigestSignInit(ctx, NULL, md, NULL, pkey);
  if (rc != 1) {
    fprintf(stderr, "EVP_DigestSignInit failed, error 0x%lx\n", ERR_get_error());
    goto cleanup;
  }

  rc = EVP_DigestSignUpdate(ctx, msg, mlen);
  if (rc != 1) {
    fprintf(stderr, "EVP_DigestSignUpdate failed, error 0x%lx\n", ERR_get_error());
    goto cleanup;
  }

  size_t req = 0;
  rc = EVP_DigestSignFinal(ctx, NULL, &req);
  if (rc != 1) {
    fprintf(stderr, "EVP_DigestSignFinal failed (1), error 0x%lx\n", ERR_get_error());
    goto cleanup;
  }

  if (!(req > 0)) {
    fprintf(stderr, "EVP_DigestSignFinal failed (2), error 0x%lx\n", ERR_get_error());
    goto cleanup;
  }

  *sig = OPENSSL_malloc(req);
  if (*sig == NULL) {
    fprintf(stderr, "OPENSSL_malloc failed, error 0x%lx\n", ERR_get_error());
    goto cleanup;
  }

  *slen = req;
  rc = EVP_DigestSignFinal(ctx, *sig, slen);
  if (rc != 1) {
    fprintf(stderr, "EVP_DigestSignFinal failed (3), return code %d, error 0x%lx\n", rc,
           ERR_get_error());
    goto cleanup;
  }

  if (req != *slen) {
    fprintf(stderr, "EVP_DigestSignFinal failed, mismatched signature sizes %ld, %ld\n",
           req, *slen);
    goto cleanup;
  }

  result = GS_OK;

  cleanup:
  EVP_MD_CTX_destroy(ctx);
  ctx = NULL;

  return result;
}

int gs_pair(PSERVER_DATA server, char* pin) {
  int ret = GS_OK;
  char url[4096];

  if (server->paired) {
    fprintf(stderr, "Already paired\n");
    return GS_WRONG_STATE;
  }

  if (server->currentGame != 0) {
    fprintf(stderr, "The computer is currently in a game. You must close the game before pairing.\n");
    return GS_WRONG_STATE;
  }

  printf("Please enter the following PIN on the target PC: %s\n", pin);

  unsigned char salt_data[16];
  char salt_hex[33];
  RAND_bytes(salt_data, 16);
  bytes_to_hex(salt_data, salt_hex, 16);

  sprintf(url, "https://%s:47984/pair?uniqueid=%s&devicename=roth&updateState=1&phrase=getservercert&salt=%s&clientcert=%s", server->address, unique_id, salt_hex, cert_hex);
  PHTTP_DATA data = http_create_data();
  if (data == NULL)
    return GS_OUT_OF_MEMORY;
  else if ((ret = http_request(url, data)) != GS_OK)
    goto cleanup;

  unsigned char salt_pin[20];
  unsigned char aes_key_hash[20];
  AES_KEY aes_key;
  memcpy(salt_pin, salt_data, 16);
  memcpy(salt_pin+16, salt_pin, 4);
  SHA1(salt_pin, 20, aes_key_hash);
  AES_set_encrypt_key((unsigned char *)aes_key_hash, 128, &aes_key);

  unsigned char challenge_data[16];
  unsigned char challenge_enc[16];
  char challenge_hex[33];
  RAND_bytes(challenge_data, 16);
  AES_encrypt(challenge_data, challenge_enc, &aes_key);
  bytes_to_hex(challenge_enc, challenge_hex, 16);

  sprintf(url, "https://%s:47984/pair?uniqueid=%s&devicename=roth&updateState=1&clientchallenge=%s", server->address, unique_id, challenge_hex);
  if ((ret = http_request(url, data)) != GS_OK)
    goto cleanup;

  char *result;
  if (xml_search(data->memory, data->size, "challengeresponse", &result) != GS_OK) {
    ret = GS_INVALID;
    goto cleanup;
  }

  char challenge_response_data_enc[48];
  char challenge_response_data[48];
  for (int count = 0; count < strlen(result); count++) {
    sscanf(&result[count], "%2hhx", &challenge_response_data_enc[count / 2]);
  }
  free(result);

  for (int i = 0; i < 48; i += 16) {
    AES_decrypt(&challenge_response_data_enc[i], &challenge_response_data[i], &aes_key);
  }

  char client_secret_data[16];
  RAND_bytes(client_secret_data, 16);

  char challenge_response[16 + 256 + 16];
  char challenge_response_hash[32];
  char challenge_response_hash_enc[32];
  char challenge_response_hex[33];
  memcpy(challenge_response, challenge_response_data + 20, 16);
  memcpy(challenge_response + 16, cert->signature->data, 256);
  memcpy(challenge_response + 16 + 256, client_secret_data, 16);
  SHA1(challenge_response, 16 + 256 + 16, challenge_response_hash);

  for (int i = 0; i < 32; i += 16) {
    AES_encrypt(&challenge_response_hash[i], &challenge_response_hash_enc[i], &aes_key);
  }
  bytes_to_hex(challenge_response_hash_enc, challenge_response_hex, 32);

  sprintf(url, "https://%s:47984/pair?uniqueid=%s&devicename=roth&updateState=1&serverchallengeresp=%s", server->address, unique_id, challenge_response_hex);
  if ((ret = http_request(url, data)) != GS_OK)
    goto cleanup;

  if (xml_search(data->memory, data->size, "pairingsecret", &result) != GS_OK) {
    ret = GS_INVALID;
    goto cleanup;
  }

  unsigned char *signature = NULL;
  size_t s_len;
  if (sign_it(client_secret_data, 16, &signature, &s_len, privateKey) != GS_OK) {
      fprintf(stderr, "Failed to sign data\n");
      ret = GS_FAILED;
      goto cleanup;
  }

  char client_pairing_secret[16 + 256];
  char client_pairing_secret_hex[(16 + 256) * 2 + 1];
  memcpy(client_pairing_secret, client_secret_data, 16);
  memcpy(client_pairing_secret + 16, signature, 256);
  bytes_to_hex(client_pairing_secret, client_pairing_secret_hex, 16 + 256);

  sprintf(url, "https://%s:47984/pair?uniqueid=%s&devicename=roth&updateState=1&clientpairingsecret=%s", server->address, unique_id, client_pairing_secret_hex);
  if ((ret = http_request(url, data)) != GS_OK)
    goto cleanup;

  sprintf(url, "https://%s:47984/pair?uniqueid=%s&devicename=roth&updateState=1&phrase=pairchallenge", server->address, unique_id);
  if ((ret = http_request(url, data)) != GS_OK)
    goto cleanup;

  printf("Paired\n");
  server->paired = true;

  cleanup:
  http_free_data(data);

  return ret;
}

int gs_applist(PSERVER_DATA server, PAPP_LIST list) {
  int ret = GS_OK;
  char url[4096];
  PHTTP_DATA data = http_create_data();
  if (data == NULL)
    return GS_OUT_OF_MEMORY;

  sprintf(url, "https://%s:47984/applist?uniqueid=%s", server->address, unique_id);
  if (http_request(url, data) != GS_OK)
    ret = GS_IO_ERROR;
  else if (xml_applist(data->memory, data->size, list) != GS_OK)
    ret = GS_INVALID;

  http_free_data(data);
  return ret;
}

int gs_start_app(PSERVER_DATA server, STREAM_CONFIGURATION *config, int appId, bool sops, bool localaudio) {
  RAND_bytes(config->remoteInputAesKey, 16);
  memset(config->remoteInputAesIv, 0, 16);

  srand(time(NULL));
  char url[4096];
  u_int32_t rikeyid = 1;
  char rikey_hex[33];
  bytes_to_hex(config->remoteInputAesKey, rikey_hex, 16);

  PHTTP_DATA data = http_create_data();
  if (data == NULL)
    return GS_OUT_OF_MEMORY;

  if (server->currentGame == 0)
    sprintf(url, "https://%s:47984/launch?uniqueid=%s&appid=%d&mode=%dx%dx%d&additionalStates=1&sops=%d&rikey=%s&rikeyid=%d&localAudioPlayMode=%d", server->address, unique_id, appId, config->width, config->height, config->fps, sops, rikey_hex, rikeyid, localaudio);
  else
    sprintf(url, "https://%s:47984/resume?uniqueid=%s&rikey=%s&rikeyid=%d", server->address, unique_id, rikey_hex, rikeyid);

  int ret = http_request(url, data);
  if (ret == GS_OK)
    server->currentGame = appId;

  http_free_data(data);
  return ret;
}

int gs_quit_app(PSERVER_DATA server) {
  char url[4096];
  PHTTP_DATA data = http_create_data();
  if (data == NULL)
    return GS_OUT_OF_MEMORY;

  sprintf(url, "https://%s:47984/cancel?uniqueid=%s", server->address, unique_id);
  int ret = http_request(url, data);

  http_free_data(data);
  return ret;
}

int gs_init(PSERVER_DATA server, const char *address) {
  http_init();
  load_unique_id();
  load_cert();

  return load_server_status(address, server);
}
