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
#include <dlfcn.h>

typedef bool(*ImxInit)();

enum platform platform_check(char* name) {
  bool std = strcmp(name, "auto") == 0;
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
  #ifdef HAVE_MMAL
  if (std || strcmp(name, "mmal") == 0) {
    void *handle = dlopen("libmoonlight-mmal.so", RTLD_NOW | RTLD_GLOBAL);
    if (handle != NULL && dlsym(RTLD_DEFAULT, "bcm_host_init") != NULL)
      return MMAL;
  }
  #endif
  #ifdef HAVE_AML
  if (std || strcmp(name, "aml") == 0) {
    void *handle = dlopen("libmoonlight-aml.so", RTLD_LAZY | RTLD_GLOBAL);
    if (handle != NULL && access("/dev/amvideo", F_OK) != -1)
      return AML;
  }
  #endif
  #ifdef HAVE_ROCKCHIP
  if (std || strcmp(name, "rk") == 0) {
    void *handle = dlopen("libmoonlight-rk.so", RTLD_NOW | RTLD_GLOBAL);
    if (handle != NULL && dlsym(RTLD_DEFAULT, "mpp_init") != NULL)
      return RK;
  }
  #endif
  #ifdef HAVE_X11
  bool x11 = strcmp(name, "x11") == 0;
  bool vdpau = strcmp(name, "x11_vdpau") == 0;
  bool vaapi = strcmp(name, "x11_vaapi") == 0;
  if (std || x11 || vdpau || vaapi) {
    int init = x11_init(std || vdpau, std || vaapi);
    #ifdef HAVE_VAAPI
    if (init == INIT_VAAPI)
      return X11_VAAPI;
    #endif
    #ifdef HAVE_VDPAU
    if (init == INIT_VDPAU)
      return X11_VDPAU;
    #endif
    return X11;
  }
  #endif
  #ifdef HAVE_SDL
  if (std || strcmp(name, "sdl") == 0)
    return SDL;
  #endif
  if (strcmp(name, "fake") == 0)
    return FAKE;

  return 0;
}

void platform_start(enum platform system) {
  switch (system) {
  #ifdef HAVE_AML
  case AML:
    write_bool("/sys/class/graphics/fb0/blank", true);
    write_bool("/sys/class/graphics/fb1/blank", true);
    write_bool("/sys/class/video/disable_video", false);
    break;
  #endif
  #if defined(HAVE_PI) | defined(HAVE_MMAL)
  case PI:
    write_bool("/sys/class/graphics/fb0/blank", true);
    break;
  #endif
  }
}

void platform_stop(enum platform system) {
  switch (system) {
  #ifdef HAVE_AML
  case AML:
    write_bool("/sys/class/graphics/fb0/blank", false);
    write_bool("/sys/class/graphics/fb1/blank", false);
    break;
  #endif
  #if defined(HAVE_PI) | defined(HAVE_MMAL)
  case PI:
    write_bool("/sys/class/graphics/fb0/blank", false);
    break;
  #endif
  }
}

DECODER_RENDERER_CALLBACKS* platform_get_video(enum platform system) {
  switch (system) {
  #ifdef HAVE_X11
  case X11:
    return &decoder_callbacks_x11;
  #ifdef HAVE_VAAPI
  case X11_VAAPI:
    return &decoder_callbacks_x11_vaapi;
  #endif
  #ifdef HAVE_VDPAU
  case X11_VDPAU:
    return &decoder_callbacks_x11_vdpau;
  #endif
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
  #ifdef HAVE_MMAL
  case MMAL:
    return (PDECODER_RENDERER_CALLBACKS) dlsym(RTLD_DEFAULT, "decoder_callbacks_mmal");
  #endif
  #ifdef HAVE_AML
  case AML:
    return (PDECODER_RENDERER_CALLBACKS) dlsym(RTLD_DEFAULT, "decoder_callbacks_aml");
  #endif
  #ifdef HAVE_ROCKCHIP
  case RK:
    return (PDECODER_RENDERER_CALLBACKS) dlsym(RTLD_DEFAULT, "decoder_callbacks_rk");
  #endif
  }
  return NULL;
}

AUDIO_RENDERER_CALLBACKS* platform_get_audio(enum platform system, char* audio_device) {
  switch (system) {
  case FAKE:
      return NULL;
  #ifdef HAVE_SDL
  case SDL:
    return &audio_callbacks_sdl;
  #endif
  #ifdef HAVE_PI
  case PI:
    if (audio_device == NULL || strcmp(audio_device, "local") == 0 || strcmp(audio_device, "hdmi") == 0)
      return (PAUDIO_RENDERER_CALLBACKS) dlsym(RTLD_DEFAULT, "audio_callbacks_omx");
    // fall-through
  #endif
  default:
    #ifdef HAVE_PULSE
    if (audio_pulse_init(audio_device))
      return &audio_callbacks_pulse;
    #endif
    #ifdef HAVE_ALSA
    return &audio_callbacks_alsa;
    #endif
  }
  return NULL;
}

bool platform_supports_hevc(enum platform system) {
  switch (system) {
  case AML:
  case RK:
    return true;
  }
  return false;
}

char* platform_name(enum platform system) {
  switch(system) {
  case PI:
    return "Raspberry Pi (Broadcom)";
  case MMAL:
    return "Raspberry Pi (Broadcom) MMAL";
  case IMX:
    return "i.MX6 (MXC Vivante)";
  case AML:
    return "AMLogic VPU";
  case RK:
    return "Rockchip VPU";
  case X11:
    return "X Window System (software decoding)";
  case X11_VAAPI:
    return "X Window System (VAAPI)";
  case X11_VDPAU:
    return "X Window System (VDPAU)";
  case SDL:
    return "SDL2 (software decoding)";
  case FAKE:
    return "Fake (no a/v output)";
  default:
    return "Unknown";
  }
}
