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

#include "mapping.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define write_config(fd, key, value) fprintf(fd, "%s = %hd\n", key, value)
#define write_config_bool(fd, key, value) fprintf(fd, "%s = %s\n", key, value?"true":"false");

void mapping_load(char* fileName, struct mapping* map) {
  FILE* fd = fopen(fileName, "r");
  if (fd == NULL) {
    printf("Can't open mapping file: %s\n", fileName);
  }

  char *line = NULL;
  size_t len = 0;
  while (__getline(&line, &len, fd) != -1) {
    char key[256], value[256];
    if (sscanf(line, "%s = %s", &key, &value) == 2) {
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
      else if (strcmp("abs_dpad_x", key) == 0)
        map->abs_dpad_x = int_value;
      else if (strcmp("abs_dpad_y", key) == 0)
        map->abs_dpad_y = int_value;
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
      else if (strcmp("reverse_dpad_x", key) == 0)
        map->reverse_dpad_x = strcmp("true", value) == 0;
      else if (strcmp("reverse_dpad_y", key) == 0)
        map->reverse_dpad_y = strcmp("true", value) == 0;
      else
        fprintf(stderr, "Can't map (%s)\n", key);
    }
  }
  free(line);
}

void mapping_save(char* fileName, struct mapping* map) {
  FILE* fd = fopen(fileName, "w");
  if (fd == NULL) {
    fprintf(stderr, "Can't open mapping file: %s\n", fileName);
    exit(EXIT_FAILURE);
  }

  write_config(fd, "abs_x", map->abs_x);
  write_config(fd, "abs_y", map->abs_y);
  write_config(fd, "abs_z", map->abs_z);

  write_config_bool(fd, "reverse_x", map->reverse_x);
  write_config_bool(fd, "reverse_y", map->reverse_y);

  write_config(fd, "abs_rx", map->abs_rx);
  write_config(fd, "abs_ry", map->abs_ry);
  write_config(fd, "abs_rz", map->abs_rz);

  write_config_bool(fd, "reverse_rx", map->reverse_rx);
  write_config_bool(fd, "reverse_ry", map->reverse_ry);

  write_config(fd, "abs_deadzone", map->abs_deadzone);

  write_config(fd, "abs_dpad_x", map->abs_dpad_x);
  write_config(fd, "abs_dpad_y", map->abs_dpad_y);

  write_config_bool(fd, "reverse_dpad_x", map->reverse_dpad_x);
  write_config_bool(fd, "reverse_dpad_y", map->reverse_dpad_y);

  write_config(fd, "btn_north", map->btn_north);
  write_config(fd, "btn_east", map->btn_east);
  write_config(fd, "btn_south", map->btn_south);
  write_config(fd, "btn_west", map->btn_west);

  write_config(fd, "btn_select", map->btn_select);
  write_config(fd, "btn_start", map->btn_start);
  write_config(fd, "btn_mode", map->btn_mode);

  write_config(fd, "btn_thumbl", map->btn_thumbl);
  write_config(fd, "btn_thumbr", map->btn_thumbr);

  write_config(fd, "btn_tl", map->btn_tl);
  write_config(fd, "btn_tr", map->btn_tr);
  write_config(fd, "btn_tl2", map->btn_tl2);
  write_config(fd, "btn_tr2", map->btn_tr2);

  write_config(fd, "btn_dpad_up", map->btn_dpad_up);
  write_config(fd, "btn_dpad_down", map->btn_dpad_down);
  write_config(fd, "btn_dpad_left", map->btn_dpad_left);
  write_config(fd, "btn_dpad_right", map->btn_dpad_right);

  fclose(fd);
}
