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

void mapping_load(char* fileName, struct mapping* map) {
  FILE* fd = fopen(fileName, "r");
  if (fd == NULL) {
    fprintf(stderr, "Can't open mapping file: %s\n", fileName);
    exit(EXIT_FAILURE);
  }

  char *line = NULL;
  size_t len = 0;
  while (getline(&line, &len, fd) != -1) {
    char *key = NULL, *value = NULL;
    if (sscanf(line, "%ms = %ms", &key, &value) == 2) {
      long int_value = strtol(value, NULL, 10);
      if (strcmp("abs_x", key) == 0)
        map->abs_x = int_value;
      else if (strcmp("abs_y", key) == 0)
        map->abs_y = int_value;
      else if (strcmp("abs_z", key) == 0)
        map->abs_z = int_value;
      else if (strcmp("abs_rx", key) == 0)
        map->abs_rx = int_value;
      else if (strcmp("abs_ry", key) == 0)
        map->abs_ry = int_value;
      else if (strcmp("abs_rz", key) == 0)
        map->abs_rz = int_value;
      else if (strcmp("abs_deadzone", key) == 0)
        map->abs_deadzone = int_value;
      else if (strcmp("hat_dpad_right", key) == 0)
        map->hat_dpad_right = int_value;
      else if (strcmp("hat_dpad_left", key) == 0)
        map->hat_dpad_left = int_value;
      else if (strcmp("hat_dpad_up", key) == 0)
        map->hat_dpad_up = int_value;
      else if (strcmp("hat_dpad_down", key) == 0)
        map->hat_dpad_down = int_value;
      else if (strcmp("hat_dpad_dir_right", key) == 0)
        map->hat_dpad_dir_right = int_value;
      else if (strcmp("hat_dpad_dir_left", key) == 0)
        map->hat_dpad_dir_left = int_value;
      else if (strcmp("hat_dpad_dir_up", key) == 0)
        map->hat_dpad_dir_up = int_value;
      else if (strcmp("hat_dpad_dir_down", key) == 0)
        map->hat_dpad_dir_down = int_value;
      else if (strcmp("btn_south", key) == 0)
        map->btn_south = int_value;
      else if (strcmp("btn_north", key) == 0)
        map->btn_north = int_value;
      else if (strcmp("btn_east", key) == 0)
        map->btn_east = int_value;
      else if (strcmp("btn_west", key) == 0)
        map->btn_west = int_value;
      else if (strcmp("btn_select", key) == 0)
        map->btn_select = int_value;
      else if (strcmp("btn_start", key) == 0)
        map->btn_start = int_value;
      else if (strcmp("btn_mode", key) == 0)
        map->btn_mode = int_value;
      else if (strcmp("btn_thumbl", key) == 0)
        map->btn_thumbl = int_value;
      else if (strcmp("btn_thumbr", key) == 0)
        map->btn_thumbr = int_value;
      else if (strcmp("btn_tl", key) == 0)
        map->btn_tl = int_value;
      else if (strcmp("btn_tr", key) == 0)
        map->btn_tr = int_value;
      else if (strcmp("btn_tl2", key) == 0)
        map->btn_tl2 = int_value;
      else if (strcmp("btn_tr2", key) == 0)
        map->btn_tr2 = int_value;
      else if (strcmp("btn_dpad_up", key) == 0)
        map->btn_dpad_up = int_value;
      else if (strcmp("btn_dpad_down", key) == 0)
        map->btn_dpad_down = int_value;
      else if (strcmp("btn_dpad_left", key) == 0)
        map->btn_dpad_left = int_value;
      else if (strcmp("btn_dpad_right", key) == 0)
        map->btn_dpad_right = int_value;
      else if (strcmp("reverse_x", key) == 0)
        map->reverse_x = strcmp("true", value) == 0;
      else if (strcmp("reverse_y", key) == 0)
        map->reverse_y = strcmp("true", value) == 0;
      else if (strcmp("reverse_rx", key) == 0)
        map->reverse_rx = strcmp("true", value) == 0;
      else if (strcmp("reverse_ry", key) == 0)
        map->reverse_ry = strcmp("true", value) == 0;
      else
        fprintf(stderr, "Can't map (%s)\n", key);
    }
    if (key != NULL)
      free(key);

    if (value != NULL)
      free(value);
  }
  free(line);
}
