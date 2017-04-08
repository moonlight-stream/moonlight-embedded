/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015-2017 Iwan Timmer
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

#include "mapping.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mapping* mapping_load(char* fileName) {
  struct mapping* mappings = NULL;
  struct mapping* map = NULL;
  FILE* fd = fopen(fileName, "r");
  if (fd == NULL) {
    fprintf(stderr, "Can't open mapping file: %s\n", fileName);
    exit(EXIT_FAILURE);
  }

  char *line = NULL;
  size_t len = 0;
  while (getline(&line, &len, fd) != -1) {
    char* strpoint;
    char* guid = strtok_r(line, ",", &strpoint);
    char* name = strtok_r(NULL, ",", &strpoint);
    if (guid == NULL || name == NULL)
      continue;

    struct mapping* newmap = malloc(sizeof(struct mapping));
    if (newmap == NULL) {
      fprintf(stderr, "Not enough memory");
      exit(EXIT_FAILURE);
    } else if (mappings == NULL)
      mappings = newmap;
    else
      map->next = newmap;

    map = newmap;

    strncpy(map->guid, guid, sizeof(map->guid));
    strncpy(map->name, name, sizeof(map->name));
    
    char* option;
    while ((option = strtok_r(NULL, ",", &strpoint)) != NULL) {
      char *key = NULL, *value = NULL;
      int ret;
      if ((ret = sscanf(option, "%m[^:]:%ms", &key, &value)) == 2) {
        int int_value, direction_value;
        char flag = NULL;
        if (strcmp("platform", key) == 0)
          strncpy(map->platform, value, sizeof(map->platform));
        else if (sscanf(value, "b%d", &int_value) == 1) {
          if (strcmp("a", key) == 0)
            map->btn_a = int_value;
          else if (strcmp("y", key) == 0)
            map->btn_y = int_value;
          else if (strcmp("x", key) == 0)
            map->btn_x = int_value;
          else if (strcmp("b", key) == 0)
            map->btn_b = int_value;
          else if (strcmp("back", key) == 0)
            map->btn_back = int_value;
          else if (strcmp("start", key) == 0)
            map->btn_start = int_value;
          else if (strcmp("guide", key) == 0)
            map->btn_guide = int_value;
          else if (strcmp("dpup", key) == 0)
            map->btn_dpup = int_value;
          else if (strcmp("dpdown", key) == 0)
            map->btn_dpdown = int_value;
          else if (strcmp("dpleft", key) == 0)
            map->btn_dpleft = int_value;
          else if (strcmp("dpright", key) == 0)
            map->btn_dpright = int_value;
          else if (strcmp("leftstick", key) == 0)
            map->btn_leftstick = int_value;
          else if (strcmp("rightstick", key) == 0)
            map->btn_rightstick = int_value;
          else if (strcmp("leftshoulder", key) == 0)
            map->btn_leftshoulder = int_value;
          else if (strcmp("rightshoulder", key) == 0)
            map->btn_rightshoulder = int_value;
          else if (strcmp("lefttrigger", key) == 0)
            map->btn_lefttrigger = int_value;
          else if (strcmp("righttrigger", key) == 0)
            map->btn_righttrigger = int_value;
        } else if (sscanf(value, "a%d%c", &int_value, &flag) >= 1) {
          if (strcmp("leftx", key) == 0) {
            map->abs_leftx = int_value;
            map->reverse_leftx = flag == '~';
          } else if (strcmp("lefty", key) == 0) {
            map->abs_lefty = int_value;
            map->reverse_lefty = flag == '~';
          } else if (strcmp("rightx", key) == 0) {
            map->abs_rightx = int_value;
            map->reverse_rightx = flag == '~';
          } else if (strcmp("righty", key) == 0) {
            map->abs_righty = int_value;
            map->reverse_righty = flag == '~';
          } else if (strcmp("lefttrigger", key) == 0)
            map->abs_lefttrigger = int_value;
          else if (strcmp("righttrigger", key) == 0)
            map->abs_righttrigger = int_value;
        } else if (sscanf(value, "h%d.%d", &int_value, &direction_value) == 2) {
          if (strcmp("dpright", key) == 0) {
            map->hat_dpright = int_value;
            map->hat_dir_dpright = direction_value;
          } else if (strcmp("dpleft", key) == 0) {
            map->hat_dpleft = int_value;
            map->hat_dir_dpleft = direction_value;
          } else if (strcmp("dpup", key) == 0) {
            map->hat_dpup = int_value;
            map->hat_dir_dpup = direction_value;
          } else if (strcmp("dpdown", key) == 0) {
            map->hat_dpdown = int_value;
            map->hat_dir_dpdown = direction_value;
          }
        } else
          fprintf(stderr, "Can't map (%s)\n", option);
      } else if (ret == 0 && option[0] != '\n')
        fprintf(stderr, "Can't map (%s)\n", option);

      if (key != NULL)
        free(key);

      if (value != NULL)
        free(value);
    }
    map->guid[32] = '\0';
    map->name[256] = '\0';
    map->platform[32] = '\0';
  }
  free(line);

  return mappings;
}
