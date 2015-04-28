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

#include <string.h>
#include <curl/curl.h>

static CURL *curl;

static const char *pCertFile = "./client.pem";
static const char *pKeyFile = "./key.pem";

static size_t _write_curl(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct http_data *mem = (struct http_data *)userp;
 
  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(EXIT_FAILURE);
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

void http_init() {
  curl = curl_easy_init();
  
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
  curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE,"PEM");
  curl_easy_setopt(curl, CURLOPT_SSLCERT, pCertFile);
  curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
  curl_easy_setopt(curl, CURLOPT_SSLKEY, pKeyFile);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_curl);
}

int http_request(char* url, struct http_data* data) {
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
  curl_easy_setopt(curl, CURLOPT_URL, url);

  if (data->size > 0) {
    free(data->memory);
    data->memory = malloc(1);
    if(data->memory == NULL) {
      fprintf(stderr, "Not enough memory\n");
      exit(EXIT_FAILURE);
    }
    data->size = 0;
  }
  CURLcode res = curl_easy_perform(curl);
  
  if(res != CURLE_OK) {
    fprintf(stderr, "Connection failed: %s\n", curl_easy_strerror(res));
    exit(EXIT_FAILURE);
  }
  
  return 0;
}

void http_cleanup() {
  curl_easy_cleanup(curl);
}

struct http_data* http_create_data() {
  struct http_data* data = malloc(sizeof(struct http_data));
  data->memory = malloc(1);
  if(data->memory == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(EXIT_FAILURE);
  }
  data->size = 0;

  return data;
}

void http_free_data(struct http_data* data) {
  free(data->memory);
  free(data);
}
