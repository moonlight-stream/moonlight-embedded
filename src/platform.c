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

#define _GNU_SOURCE

#include "platform.h"
#include "video.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

enum platform platform_check(char* name) {
  bool std = strcmp(name, "default") == 0;
  #ifdef HAVE_SDL
  if (std || strcmp(name, "sdl") == 0)
    return SDL;
  #endif
  #ifdef HAVE_IMX
  if (std || strcmp(name, "imx") == 0) {
    if (dlsym(RTLD_DEFAULT, "vpu_Init") != NULL && video_imx_init())
      return IMX;
  }
  #endif
  #ifdef HAVE_OMX
  if (std || strcmp(name, "omx") == 0) {
    if (dlsym(RTLD_DEFAULT, "bcm_host_init") != NULL)
      return OMX;
  }
  #endif
  if (std || strcmp(name, "fake") == 0)
    return FAKE;
}

DECODER_RENDERER_CALLBACKS* platform_get_video(enum platform system) {
  switch (system) {
  #ifdef HAVE_SDL
  case SDL:
    return &decoder_callbacks_sdl;
  #endif
  #ifdef HAVE_IMX
  case IMX:
    return &decoder_callbacks_imx;
  #endif
  #ifdef HAVE_OMX
  case OMX:
    return &decoder_callbacks_omx;
  #endif
  case FAKE:
    return &decoder_callbacks_fake;
  }
  return NULL;
}
