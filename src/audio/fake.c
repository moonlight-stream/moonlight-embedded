/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2016 Iwan Timmer
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

#include "../audio.h"

#include <stdio.h>

static FILE* fd;
static const char* fileName = "fake.opus";

static void alsa_renderer_init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig) {
  fd = fopen(fileName, "w");
}

static void alsa_renderer_cleanup() {
  fclose(fd);
}

static void alsa_renderer_decode_and_play_sample(char* data, int length) {
  fwrite(data, length, 1, fd);
}

AUDIO_RENDERER_CALLBACKS audio_callbacks_fake = {
  .init = alsa_renderer_init,
  .cleanup = alsa_renderer_cleanup,
  .decodeAndPlaySample = alsa_renderer_decode_and_play_sample,
  .capabilities = CAPABILITY_DIRECT_SUBMIT,
};
