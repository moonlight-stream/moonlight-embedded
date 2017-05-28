/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2016 Ilya Zhuravlev
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
#include <opus/opus_multistream.h>
#include <psp2/audioout.h>

enum {
  VITA_AUDIO_INIT_OK        = 0,
  VITA_AUDIO_ERROR_BAD_OPUS = 0x80020001,
  VITA_AUDIO_ERROR_PORT     = 0x80020002,
};

#include "debug.h"

#define FRAME_SIZE 240
#define VITA_SAMPLES 960
#define BUFFER_SIZE (2 * VITA_SAMPLES)
static int decode_offset;
static int port;

static int active_audio_thread = true;
static OpusMSDecoder* decoder = NULL;

static short buffer[BUFFER_SIZE];

static void vita_renderer_cleanup() {
  if (decoder != NULL) {
    opus_multistream_decoder_destroy(decoder);
    decoder = NULL;
  }
}

static int vita_renderer_init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig) {
  int rc;
  decoder = opus_multistream_decoder_create(opusConfig->sampleRate,
                                            opusConfig->channelCount,
                                            opusConfig->streams,
                                            opusConfig->coupledStreams,
                                            opusConfig->mapping,
                                            &rc);

  if (rc < 0) {
      return VITA_AUDIO_ERROR_BAD_OPUS;
  }

  port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, VITA_SAMPLES, 48000, SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO);

  if (port < 0) {
      vita_renderer_cleanup();
      return VITA_AUDIO_ERROR_PORT;
  }

  DEBUG_PRINT("open port 0x%x\n", port);
  return VITA_AUDIO_INIT_OK;
}

static void vita_renderer_decode_and_play_sample(char* data, int length) {
  if (!data)
    return;

  int decodeLen = opus_multistream_decode(decoder, data, length, buffer + 2 * decode_offset, FRAME_SIZE, 0);
  if (decodeLen > 0) {
    if (decodeLen != FRAME_SIZE)
      return;
    decode_offset += decodeLen;

    if (decode_offset == VITA_SAMPLES) {
      decode_offset = 0;
      if (active_audio_thread) {
        sceAudioOutOutput(port, buffer);
      }
    }
  } else {
    DEBUG_PRINT("Opus error from decode: %d\n", decodeLen);
  }
}

AUDIO_RENDERER_CALLBACKS audio_callbacks_vita = {
  .init = vita_renderer_init,
  .cleanup = vita_renderer_cleanup,
  .decodeAndPlaySample = vita_renderer_decode_and_play_sample,
  .capabilities = CAPABILITY_DIRECT_SUBMIT,
};


void vitaaudio_start() {
  active_audio_thread = true;
}

void vitaaudio_stop() {
  active_audio_thread = false;
}
