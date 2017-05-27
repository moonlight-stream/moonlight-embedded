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
#include <dlfcn.h>

typedef bool(*ImxInit)();

enum platform platform_check(char* name) {
  bool std = strcmp(name, "default") == 0;
  #ifdef HAVE_IMX
  if (std || strcmp(name, "imx") == 0) {
    void *handle = dlopen("libmoonlight-imx.so", RTLD_NOW | RTLD_GLOBAL);
    ImxInit video_imx_init = (ImxInit) dlsym(RTLD_DEFAULT, "video_imx_init");
    if (handle != NULL) {
      if (video_imx_init())
        return IMX;
    }
  }
  #endif
  #ifdef HAVE_PI
  if (std || strcmp(name, "pi") == 0) {
    void *handle = dlopen("libmoonlight-pi.so", RTLD_NOW | RTLD_GLOBAL);
    if (handle != NULL && dlsym(RTLD_DEFAULT, "bcm_host_init") != NULL)
      return PI;
  }
  #endif
  #ifdef HAVE_AML
  if (std || strcmp(name, "aml") == 0) {
    void *handle = dlopen("libmoonlight-aml.so", RTLD_NOW | RTLD_GLOBAL);
    if (handle != NULL && access("/dev/amvideo", F_OK) != -1)
      return AML;
  }
  #endif
  #ifdef HAVE_X11
  if (std || strcmp(name, "x11") == 0)
    return X11;
  #endif
  #ifdef HAVE_SDL
  if (std || strcmp(name, "sdl") == 0)
    return SDL;
  #endif
  if (strcmp(name, "fake") == 0)
    return FAKE;

  return 0;
}

DECODER_RENDERER_CALLBACKS* platform_get_video(enum platform system) {
  switch (system) {
  #ifdef HAVE_X11
  case X11:
    return &decoder_callbacks_x11;
  #endif
  #ifdef HAVE_SDL
  case SDL:
    return &decoder_callbacks_sdl;
  #endif
  #ifdef HAVE_IMX
  case IMX:
    return (PDECODER_RENDERER_CALLBACKS) dlsym(RTLD_DEFAULT, "decoder_callbacks_imx");
  #endif
  #ifdef HAVE_PI
  case PI:
    return (PDECODER_RENDERER_CALLBACKS) dlsym(RTLD_DEFAULT, "decoder_callbacks_pi");
  #endif
  #ifdef HAVE_AML
  case AML:
    return (PDECODER_RENDERER_CALLBACKS) dlsym(RTLD_DEFAULT, "decoder_callbacks_aml");
  #endif
  }
  return NULL;
}

AUDIO_RENDERER_CALLBACKS* platform_get_audio(enum platform system) {
  switch (system) {
  #ifdef HAVE_SDL
  case SDL:
    return &audio_callbacks_sdl;
  #endif
  #ifdef HAVE_PI
  case PI:
    if (audio_device == NULL || strcmp(audio_device, "local") == 0 || strcmp(audio_device, "hdmi") == 0)
      return (PAUDIO_RENDERER_CALLBACKS) dlsym(RTLD_DEFAULT, "audio_callbacks_omx");
  #endif
  default:
    #ifdef HAVE_PULSE
    if (audio_pulse_init())
      return &audio_callbacks_pulse;
    #endif
    return &audio_callbacks_alsa;
  }
  return NULL;
}

bool platform_supports_hevc(enum platform system) {
  switch (system) {
  case AML:
    return true;
  }
  return false;
}
