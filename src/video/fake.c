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

#include "../video.h"

#include <stdio.h>

static FILE* fd;
static const char* fileName = "fake.h264";
static h264_stream_t* stream;

void decoder_renderer_setup(int width, int height, int redrawRate, void* context, int drFlags) {
  printf("decoder_renderer_setup %dx%d %dfps\n", width, height, redrawRate);
  fd = fopen(fileName, "w");

  stream = h264_new();
  if (stream == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(EXIT_FAILURE);
  }
}

void decoder_renderer_start() {
  printf("decoder_renderer_start\n");
}

void decoder_renderer_stop() {
  printf("decoder_renderer_stop\n");
}

void decoder_renderer_release() {
  printf("decoder_renderer_release\n");
  fclose(fd);
}

int decoder_renderer_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  while (entry != NULL) {
    fwrite(entry->data, entry->length, 1, fd);
    entry = entry->next;
  }
  return DR_OK;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks = {
  .setup = decoder_renderer_setup,
  .start = decoder_renderer_start,
  .stop = decoder_renderer_stop,
  .release = decoder_renderer_release,
  .submitDecodeUnit = decoder_renderer_submit_decode_unit,
};
