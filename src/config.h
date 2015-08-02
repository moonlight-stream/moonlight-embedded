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

#include "limelight-common/Limelight.h"

#include <stdbool.h>

#define MAX_INPUTS 6

struct input_config {
  char* path;
  char* mapping;
};

typedef struct _CONFIGURATION {
  STREAM_CONFIGURATION stream;
  char* app;
  char* action;
  char* address;
  char* mapping;
  char* platform;
  char* config_file;
  bool sops;
  bool localaudio;
  struct input_config inputs[MAX_INPUTS];
  int inputsCount;
} CONFIGURATION, *PCONFIGURATION;

bool inputAdded;

void config_file_parse(char* filename, PCONFIGURATION config);
void config_parse(int argc, char* argv[], PCONFIGURATION config);
