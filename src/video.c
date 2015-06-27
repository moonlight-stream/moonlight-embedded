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

#include "video.h"

#include "limelight-common/Limelight.h"

#include <dlfcn.h>
#include <stdlib.h>

DECODER_RENDERER_CALLBACKS *decoder_callbacks;

static int decoder_level;

void video_init() {
  #ifdef HAVE_SDL
  decoder_callbacks = &decoder_callbacks_sdl;
  #else
  decoder_callbacks = &decoder_callbacks_fake;
  #endif
  #ifdef HAVE_IMX
  if (dlsym(RTLD_DEFAULT, "vpu_Init") != NULL && video_imx_init()) {
    decoder_callbacks = &decoder_callbacks_imx;
  }
  #endif
  #ifdef HAVE_OMX
  if (dlsym(RTLD_DEFAULT, "bcm_host_init") != NULL) {
    decoder_callbacks = &decoder_callbacks_omx;
  }
  #endif
}
