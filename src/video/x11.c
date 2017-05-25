/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2017 Iwan Timmer
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

#include "../video.h"
#include "egl.h"
#include "ffmpeg.h"

#include <Limelight.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <stdbool.h>

#define DECODER_BUFFER_SIZE 92*1024

static char* ffmpeg_buffer = NULL;

static Display *display;

void x11_setup(int videoFormat, int width, int height, int redrawRate, void* context, int drFlags) {
  int avc_flags = SLICE_THREADING;
  if (drFlags & FORCE_HARDWARE_ACCELERATION)
    avc_flags |= HARDWARE_ACCELERATION;

  if (ffmpeg_init(videoFormat, width, height, avc_flags, 2, 2) < 0) {
    fprintf(stderr, "Couldn't initialize video decoding\n");
    exit(1);
  }

  ffmpeg_buffer = malloc(DECODER_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);
  if (ffmpeg_buffer == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(1);
  }

  display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Error: failed to open X display.\n");
    return;
  }

  Window root = DefaultRootWindow(display);
  XSetWindowAttributes winattr = {0};
  Window window = XCreateWindow(display, root, 0, 0, width, height, 0, CopyFromParent, InputOutput, CopyFromParent, CWEventMask, &winattr);
  XMapWindow(display, window);
  XStoreName(display, window, "Moonlight");

  egl_init(display, window, width, height);
}

void x11_cleanup() {
  ffmpeg_destroy();
  egl_destroy();
}

int x11_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  if (decodeUnit->fullLength < DECODER_BUFFER_SIZE) {
    PLENTRY entry = decodeUnit->bufferList;
    int length = 0;
    while (entry != NULL) {
      memcpy(ffmpeg_buffer+length, entry->data, entry->length);
      length += entry->length;
      entry = entry->next;
    }
    ffmpeg_decode(ffmpeg_buffer, length);
    AVFrame* frame = ffmpeg_get_frame();
    if (frame != NULL)
      egl_draw(frame->data);
  }

  return DR_OK;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_x11 = {
  .setup = x11_setup,
  .cleanup = x11_cleanup,
  .submitDecodeUnit = x11_submit_decode_unit,
  .capabilities = CAPABILITY_SLICES_PER_FRAME(4) | CAPABILITY_REFERENCE_FRAME_INVALIDATION | CAPABILITY_DIRECT_SUBMIT,
};
