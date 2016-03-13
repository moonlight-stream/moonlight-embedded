/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015 Iwan Timmer
 * Copyright (C) 2016 OtherCrashOverride, Daniel Mehrwald
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

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <amcodec/codec.h>

static codec_para_t codecParam = { 0 };
const size_t EXTERNAL_PTS = (1);
const size_t SYNC_OUTSIDE = (2);

fbdev_window window;

void SetupDisplay() {
  int ret = -1;

  int fd_fb0 = open("/dev/fb0", O_RDWR);
  fprintf(stdout, "file handle: %x\n", fd_fb0);

  struct fb_var_screeninfo info;
  ret = ioctl(fd_fb0, FBIOGET_VSCREENINFO, &info);
  if (ret < 0) {
    fprintf(stderr, "FBIOGET_VSCREENINFO failed.\n");
    exit(1);
  }

  window.width = info.xres;
  window.height = info.yres;

  fprintf(stdout, "screen info: width=%d, height=%d, bpp=%d\n", window.width, window.height, info.bits_per_pixel);

  // Get the EGL display (fb0)
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (display == EGL_NO_DISPLAY) {
    fprintf(stderr, "eglGetDisplay failed.\n");
    exit(1);
  }

  // Initialize EGL
  EGLint major;
  EGLint minor;

  EGLBoolean success = eglInitialize(display, &major, &minor);
  if (success != EGL_TRUE) {
    EGLint error = eglGetError();
    fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
    fprintf(stderr, "Failed eglInitialize at %s:%i\n", __FILE__, __LINE__);
    exit(1);
  }

  // Display info
  fprintf(stdout, "EGL: Initialized - major=%d minor=%d\n", major, minor);
  fprintf(stdout, "EGL: Vendor=%s\n", eglQueryString(display, EGL_VENDOR));
  fprintf(stdout, "EGL: Version=%s\n", eglQueryString(display, EGL_VERSION));
  fprintf(stdout, "EGL: ClientAPIs=%s\n", eglQueryString(display, EGL_CLIENT_APIS));
  fprintf(stdout, "EGL: Extensions=%s\n\n", eglQueryString(display, EGL_EXTENSIONS));

  // Find a config
  int redSize, greenSize, blueSize, alphaSize, depthSize = 24, stencilSize = 8;

  if (info.bits_per_pixel < 32) {
    redSize = 5;
    greenSize = 6;
    blueSize = 5;
    alphaSize = 0;
  }
  else {
    redSize = 8;
    greenSize = 8;
    blueSize = 8;
    alphaSize = 8;
  }

  EGLint configAttributes[] = {
    EGL_RED_SIZE,            redSize,
    EGL_GREEN_SIZE,          greenSize,
    EGL_BLUE_SIZE,           blueSize,
    EGL_ALPHA_SIZE,          alphaSize,

    EGL_DEPTH_SIZE,          depthSize,
    EGL_STENCIL_SIZE,        stencilSize,

    EGL_SURFACE_TYPE,        EGL_WINDOW_BIT ,

    EGL_NONE
  };

  int num_configs;
  success = eglChooseConfig(display, configAttributes, NULL, 0, &num_configs);
  if (success != EGL_TRUE) {
    EGLint error = eglGetError();
    fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
    fprintf(stderr, "Failed eglChooseConfig at %s:%i\n", __FILE__, __LINE__);
    exit(1);
  }

  EGLConfig* configs = (EGLConfig*)malloc(sizeof(EGLConfig) * num_configs);
  success = eglChooseConfig(display, configAttributes, configs, num_configs, &num_configs);
  if (success != EGL_TRUE) {
    EGLint error = eglGetError();
    fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
    fprintf(stderr, "Failed eglChooseConfig at %s:%i\n", __FILE__, __LINE__);
    exit(1);
  }

  EGLConfig match = 0;

  for (int i = 0; i < num_configs; ++i)
  {
    EGLint configRedSize;
    EGLint configGreenSize;
    EGLint configBlueSize;
    EGLint configAlphaSize;
    EGLint configDepthSize;
    EGLint configStencilSize;

    eglGetConfigAttrib(display, configs[i], EGL_RED_SIZE, &configRedSize);
    eglGetConfigAttrib(display, configs[i], EGL_GREEN_SIZE, &configGreenSize);
    eglGetConfigAttrib(display, configs[i], EGL_BLUE_SIZE, &configBlueSize);
    eglGetConfigAttrib(display, configs[i], EGL_ALPHA_SIZE, &configAlphaSize);
    eglGetConfigAttrib(display, configs[i], EGL_DEPTH_SIZE, &configDepthSize);
    eglGetConfigAttrib(display, configs[i], EGL_STENCIL_SIZE, &configStencilSize);

    if (configRedSize == redSize &&
      configBlueSize == blueSize &&
      configGreenSize == greenSize &&
      configAlphaSize == alphaSize &&
      configDepthSize == depthSize &&
      configStencilSize == stencilSize) {
      match = configs[i];
      break;
    }
  }

  free((void*)configs);

  if (match == 0) {
    fprintf(stderr, "No eglConfig match found.\n");
    exit(1);
  }

  fprintf(stdout, "EGLConfig match found: (%p)\n", match);
  fprintf(stdout, "\n");

  EGLint windowAttr[] = {
    EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
    EGL_NONE };

  EGLSurface surface = eglCreateWindowSurface(display, match, (NativeWindowType)&window, windowAttr);

  if (surface == EGL_NO_SURFACE) {
    EGLint error = eglGetError();
    fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
    fprintf(stderr, "Failed eglCreateWindowSurface at %s:%i\n", __FILE__, __LINE__);
    exit(1);
  }

  // Create a context
  eglBindAPI(EGL_OPENGL_ES_API);

  EGLint contextAttributes[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE };

  EGLContext context = eglCreateContext(display, match, EGL_NO_CONTEXT, contextAttributes);
  if (context == EGL_NO_CONTEXT) {
    EGLint error = eglGetError();
    fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
    fprintf(stderr, "Failed eglCreateContext at %s:%i\n", __FILE__, __LINE__);
    exit(1);
  }

  success = eglMakeCurrent(display, surface, surface, context);
  if (success != EGL_TRUE) {
    EGLint error = eglGetError();
    fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
    fprintf(stderr, "Failed eglMakeCurrent at %s:%i\n", __FILE__, __LINE__);
    exit(1);
  }

  // Display GLES info
  fprintf(stdout, "GL: Renderer=%s\n", glGetString(GL_RENDERER));
  fprintf(stdout, "GL: Vendor=%s\n", glGetString(GL_VENDOR));
  fprintf(stdout, "GL: Version=%s\n", glGetString(GL_VERSION));
  fprintf(stdout, "GL: Extensions=%s\n\n", glGetString(GL_EXTENSIONS));

  // VSYNC
  success = eglSwapInterval(display, 1);
  if (success != EGL_TRUE) {
    EGLint error = eglGetError();
    fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)error, (int)error);
    fprintf(stderr, "Failed eglSwapInterval at %s:%i\n", __FILE__, __LINE__);
    exit(1);
  }
}

void aml_setup(int videoFormat, int width, int height, int redrawRate, void* context, int drFlags) {
  fprintf(stderr, "\nvideoFormat=%d nwidth=%d, height=%d, redrawRate=%d, context=%p, drFlags=%x\n",
    videoFormat, width, height, redrawRate, context, drFlags);

  SetupDisplay();

  codecParam.stream_type = STREAM_TYPE_ES_VIDEO;
  codecParam.has_video = 1;
  codecParam.noblock = 0;
  
  switch (videoFormat) {
    case VIDEO_FORMAT_H264:	// 1
      if (width > 1920 || height > 1080) {
        codecParam.video_type = VFORMAT_H264_4K2K;
        codecParam.am_sysinfo.format = VIDEO_DEC_FORMAT_H264_4K2K; ///< video format, such as H264, MPEG2...
      } else {
        codecParam.video_type = VFORMAT_H264;
        codecParam.am_sysinfo.format = VIDEO_DEC_FORMAT_H264;  ///< video format, such as H264, MPEG2...
      }

      fprintf(stdout, "Decoding H264 video.\n");
	  break;
    case VIDEO_FORMAT_H265: // 2

      codecParam.video_type = VFORMAT_HEVC;
      codecParam.am_sysinfo.format = VIDEO_DEC_FORMAT_HEVC;  ///< video format, such as H264, MPEG2...

      fprintf(stdout, "Decoding HEVC video.\n");
	  break;
    default:
      printf("Unsupported video format.\n");
      exit(1);
  }

  codecParam.am_sysinfo.width = width;   //< video source width
  codecParam.am_sysinfo.height = height;  //< video source height
  codecParam.am_sysinfo.rate = (96000 / (redrawRate));    //< video source frame duration
  codecParam.am_sysinfo.param = (void *)(EXTERNAL_PTS | SYNC_OUTSIDE);   //< other parameters for video decoder
 
  int api = codec_init(&codecParam);
  fprintf(stdout, "codec_init=%x\n", api);

  if (api != 0) {
    fprintf(stderr, "codec_init failed.\n");
    exit(1);
  }
}

void aml_cleanup() {
  struct buf_status videoBufferStatus;

  while(1) {
    int ret = codec_get_vbuf_state(&codecParam, &videoBufferStatus);
    if (ret != 0) {
      fprintf(stderr, "codec_get_vbuf_state error: %x\n", -ret);
      break;
    }
    if (videoBufferStatus.data_len < 0x100)
      break;

    sleep(100);
  }

  int api = codec_close(&codecParam);
}

int aml_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  int result = DR_OK;
  PLENTRY entry = decodeUnit->bufferList;
  while (entry != NULL) {
    int api = codec_write(&codecParam, entry->data, entry->length);
    if (api != entry->length) {
      fprintf(stderr, "codec_write error: %x\n", api);
      codec_reset(&codecParam);
      result = DR_NEED_IDR;
    }

    entry = entry->next;
  }
  return result;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_aml = {
  .setup = aml_setup,
  .cleanup = aml_cleanup,
  .submitDecodeUnit = aml_submit_decode_unit,
  .capabilities = CAPABILITY_DIRECT_SUBMIT | CAPABILITY_SLICES_PER_FRAME(8),
};
