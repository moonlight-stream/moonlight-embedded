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

#include "audio.h"

#include <stdio.h>

static FILE* fd;
static const char* fileName = "fake.opus";

void audio_renderer_init() {
  printf("audio_renderer_init\n");
  fd = fopen(fileName, "w");
}

void audio_renderer_start() {
  printf("audio_renderer_start\n");
}

void audio_renderer_stop() {
  printf("audio_renderer_stop\n");
}

void audio_renderer_release() {
  printf("audio_renderer_release\n");
  fclose(fd);
}

void audio_renderer_decode_and_play_sample(char* data, int length) {
  fwrite(data, length, 1, fd);
}

AUDIO_RENDERER_CALLBACKS audio_callbacks = {
  .init = audio_renderer_init,
  .start = audio_renderer_start,
  .stop = audio_renderer_stop,
  .release = audio_renderer_release,
  .decodeAndPlaySample = audio_renderer_decode_and_play_sample,
};
