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

#include "xml.h"
#include "errors.h"

#include <expat.h>
#include <string.h>

struct xml_query {
  char *memory;
  size_t size;
  int start;
  void* data;
};

static void XMLCALL _xml_start_element(void *userData, const char *name, const char **atts) {
  struct xml_query *search = (struct xml_query*) userData;
  if (strcmp(search->data, name) == 0)
    search->start++;
}

static void XMLCALL _xml_end_element(void *userData, const char *name) {
  struct xml_query *search = (struct xml_query*) userData;
  if (strcmp(search->data, name) == 0)
    search->start--;
}

static void XMLCALL _xml_start_applist_element(void *userData, const char *name, const char **atts) {
  struct xml_query *search = (struct xml_query*) userData;
  if (strcmp("App", name) == 0) {
    PAPP_LIST app = malloc(sizeof(APP_LIST));
    if (app == NULL)
      return;

    app->next = (PAPP_LIST) search->data;
    search->data = app;
  } else if (strcmp("ID", name) == 0 || strcmp("AppTitle", name) == 0) {
    search->memory = malloc(1);
    search->size = 0;
    search->start = 1;
  }
}

static void XMLCALL _xml_end_applist_element(void *userData, const char *name) {
  struct xml_query *search = (struct xml_query*) userData;
  if (search->start) {
    PAPP_LIST list = (PAPP_LIST) search->data;
    if (list == NULL)
      return;

    if (strcmp("ID", name) == 0) {
        list->id = atoi(search->memory);
        free(search->memory);
    } else if (strcmp("AppTitle", name) == 0) {
        list->name = search->memory;
    }
    search->start = 0;
  }
}

static void XMLCALL _xml_write_data(void *userData, const XML_Char *s, int len) {
  struct xml_query *search = (struct xml_query*) userData;
  if (search->start > 0) {
    search->memory = realloc(search->memory, search->size + len + 1);
    if(search->memory == NULL)
      return;
  
    memcpy(&(search->memory[search->size]), s, len);
    search->size += len;
    search->memory[search->size] = 0;
  }
}

int xml_search(char* data, size_t len, char* node, char** result) {
  struct xml_query search;
  search.data = node;
  search.start = 0;
  search.memory = calloc(1, 1);
  search.size = 0;
  XML_Parser parser = XML_ParserCreate("UTF-8");
  XML_SetUserData(parser, &search);
  XML_SetElementHandler(parser, _xml_start_element, _xml_end_element);
  XML_SetCharacterDataHandler(parser, _xml_write_data);
  if (! XML_Parse(parser, data, len, 1)) {
    int code = XML_GetErrorCode(parser);
    gs_error = XML_ErrorString(code);
    free(search.memory);
    return GS_INVALID;
  } else if (search.memory == NULL)
    return GS_OUT_OF_MEMORY;

  *result = search.memory;
  
  return GS_OK;
}

int xml_applist(char* data, size_t len, PAPP_LIST app_list) {
  struct xml_query query;
  query.memory = calloc(1, 1);
  query.size = 0;
  query.start = 0;
  query.data = NULL;
  XML_Parser parser = XML_ParserCreate("UTF-8");
  XML_SetUserData(parser, &query);
  XML_SetElementHandler(parser, _xml_start_applist_element, _xml_end_applist_element);
  XML_SetCharacterDataHandler(parser, _xml_write_data);
  if (! XML_Parse(parser, data, len, 1)) {
    int code = XML_GetErrorCode(parser);
    gs_error = XML_ErrorString(code);
    return GS_INVALID;
  }
  app_list = (PAPP_LIST) query.data;

  return GS_OK;
}
