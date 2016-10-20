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

#define _GNU_SOURCE

#include "platform.h"
#include "audio.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <dlfcn.h>

typedef bool(*ImxInit)();

enum platform platform_check(char* name) {
  bool std = strcmp(name, "default") == 0;
  if (strcmp(name, "vita") == 0)
    return VITA;
  return 0;
}

DECODER_RENDERER_CALLBACKS* platform_get_video(enum platform system) {
  return &decoder_callbacks_vita;
}

AUDIO_RENDERER_CALLBACKS* platform_get_audio(enum platform system) {
  return &audio_callbacks_vita;
}

bool platform_supports_hevc(enum platform system) {
  return false;
}

int chmod(const char *path, mode_t mode) {
  return 0;
}

uid_t getuid(void) {
  return 1;
}

uid_t geteuid(void) {
  return 1;
}

uid_t getgid(void) {
  return 1;
}

uid_t getegid(void) {
  return 1;
}
