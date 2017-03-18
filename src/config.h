/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015, 2016 Iwan Timmer
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

#include <Limelight.h>

#include <stdbool.h>

#define MAX_INPUTS 6

enum codecs { CODEC_UNSPECIFIED, CODEC_H264, CODEC_HEVC };

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
  char key_dir[4096];
  bool sops;
  bool localaudio;
  bool fullscreen;
  bool forcehw;
  bool unsupported_version;
  struct input_config inputs[MAX_INPUTS];
  int inputsCount;
  enum codecs codec;
} CONFIGURATION, *PCONFIGURATION;

bool inputAdded;

bool config_file_parse(char* filename, PCONFIGURATION config);
void config_parse(int argc, char* argv[], PCONFIGURATION config);
