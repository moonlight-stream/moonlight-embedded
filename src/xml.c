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

#include "expat.h"

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
    struct app_list* app = malloc(sizeof(struct app_list));
    if (app == NULL) {
      perror("Not enough memory");
      exit(EXIT_FAILURE);
    }
    app->next = (struct app_list*) search->data;
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
    struct app_list* list = (struct app_list*) search->data;
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
    if(search->memory == NULL) {
      fprintf(stderr, "Not enough memory\n");
      exit(EXIT_FAILURE);
    }
  
    memcpy(&(search->memory[search->size]), s, len);
    search->size += len;
    search->memory[search->size] = 0;
  }
}

int xml_search(char* data, size_t len, char* node, char** result) {
  struct xml_query search;
  search.data = node;
  search.start = 0;
  search.memory = malloc(1);
  search.size = 0;
  XML_Parser parser = XML_ParserCreate("UTF-8");
  XML_SetUserData(parser, &search);
  XML_SetElementHandler(parser, _xml_start_element, _xml_end_element);
  XML_SetCharacterDataHandler(parser, _xml_write_data);
  if (! XML_Parse(parser, data, len, 1)) {
    int code = XML_GetErrorCode(parser);
    fprintf(stderr, "XML Error: %s\n", XML_ErrorString(code));
    return 1;
  }
  *result = search.memory;
  
  return 0;
}

struct app_list* xml_applist(char* data, size_t len) {
  struct xml_query query;
  query.memory = malloc(1);
  query.size = 0;
  query.start = 0;
  query.data = NULL;
  XML_Parser parser = XML_ParserCreate("UTF-8");
  XML_SetUserData(parser, &query);
  XML_SetElementHandler(parser, _xml_start_applist_element, _xml_end_applist_element);
  XML_SetCharacterDataHandler(parser, _xml_write_data);
  if (! XML_Parse(parser, data, len, 1)) {
    int code = XML_GetErrorCode(parser);
    fprintf(stderr, "XML Error %s\n", XML_ErrorString(code));
    exit(-1);
  }

  return (struct app_list*) query.data;
}
