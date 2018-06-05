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

#define _GNU_SOURCE

#include "platform.h"

#include "util.h"

#include "audio/audio.h"
#include "video/video.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef bool(*ImxInit)();

enum platform platform_check(char* name) {
  bool std = strcmp(name, "auto") == 0;

  if (std || strcmp(name, "switch") == 0)
    return SWITCH;

  return 0;
}

void platform_start(enum platform system) {

}

void platform_stop(enum platform system) {

}

DECODER_RENDERER_CALLBACKS* platform_get_video(enum platform system) {
  return NULL;
}

AUDIO_RENDERER_CALLBACKS* platform_get_audio(enum platform system, char* audio_device) {
  return NULL;
}

bool platform_supports_hevc(enum platform system) {
  return false;
}

char* platform_name(enum platform system) {
  switch(system) {
  default:
    return "Unknown";
  }
}
