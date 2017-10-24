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
#include <stdbool.h>
#include <string.h>

struct mapping* mapping_parse(char* mapping) {
  char* strpoint;
  char* guid = strtok_r(mapping, ",", &strpoint);
  char* name = strtok_r(NULL, ",", &strpoint);
  if (guid == NULL || name == NULL)
    return NULL;

  struct mapping* map = malloc(sizeof(struct mapping));
  if (map == NULL) {
    fprintf(stderr, "Not enough memory");
    exit(EXIT_FAILURE);
  }

  strncpy(map->guid, guid, sizeof(map->guid));
  strncpy(map->name, name, sizeof(map->name));
  memset(&map->abs_leftx, -1, sizeof(short) * 31);

  char* option;
  while ((option = strtok_r(NULL, ",", &strpoint)) != NULL) {
    char *key = NULL, *value = NULL;
    int ret;
    if ((ret = sscanf(option, "%m[^:]:%ms", &key, &value)) == 2) {
      int int_value, direction_value;
      char flag = 0;
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
  return map;
}

struct mapping* mapping_load(char* fileName, bool verbose) {
  struct mapping* mappings = NULL;
  FILE* fd = fopen(fileName, "r");
  if (fd == NULL) {
    fprintf(stderr, "Can't open mapping file: %s\n", fileName);
    exit(EXIT_FAILURE);
  } else if (verbose)
    printf("Loading mappingfile %s\n", fileName);

  char *line = NULL;
  size_t len = 0;
  while (getline(&line, &len, fd) != -1) {
    struct mapping* map = mapping_parse(line);
    if (map) {
      map->next = mappings;
      mappings = map;
    }
  }
  free(line);

  return mappings;
}

#define print_btn(btn, code) if (code > -1) printf("%s:b%d,", btn, code)
#define print_abs(abs, code) if (code > -1) printf("%s:a%d,", abs, code)
#define print_hat(hat, code, dir) if (code > -1) printf("%s:h%d.%d,", hat, code, dir)

void mapping_print(struct mapping* map) {
  printf("%s,%s,", map->guid, map->name);
  print_btn("a", map->btn_a);
  print_btn("b", map->btn_b);
  print_btn("x", map->btn_x);
  print_btn("y", map->btn_y);
  print_btn("start", map->btn_start);
  print_btn("guide", map->btn_guide);
  print_btn("back", map->btn_back);
  print_btn("leftstick", map->btn_leftstick);
  print_btn("rightstick", map->btn_rightstick);
  print_btn("leftshoulder", map->btn_leftshoulder);
  print_btn("rightshoulder", map->btn_rightshoulder);
  print_btn("dpup", map->btn_dpup);
  print_btn("dpleft", map->btn_dpleft);
  print_btn("dpdown", map->btn_dpdown);
  print_btn("dpright", map->btn_dpright);
  print_hat("dpup", map->hat_dpup, map->hat_dir_dpup);
  print_hat("dpleft", map->hat_dpleft, map->hat_dir_dpleft);
  print_hat("dpdown", map->hat_dpdown, map->hat_dir_dpdown);
  print_hat("dpright", map->hat_dpright, map->hat_dir_dpright);
  print_abs("leftx", map->abs_leftx);
  print_abs("lefty", map->abs_lefty);
  print_abs("rightx", map->abs_rightx);
  print_abs("righty", map->abs_righty);
  print_abs("lefttrigger", map->abs_lefttrigger);
  print_abs("righttrigger", map->abs_righttrigger);
  print_btn("lefttrigger", map->btn_lefttrigger);
  print_btn("righttrigger", map->btn_righttrigger);
  printf("platform:Linux\n");
}
