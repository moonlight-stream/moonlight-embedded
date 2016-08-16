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

#include "../audio.h"

#include <stdio.h>
#include <opus/opus_multistream.h>
#include <psp2/audioout.h>

#define FRAME_SIZE 240
#define VITA_SAMPLES 64
#define BUFFER_SAMPLES 1024
#define BUFFER_SIZE (2 * BUFFER_SAMPLES)

static OpusMSDecoder* decoder;

// circular buffer
static short buffer[BUFFER_SIZE];
static size_t decode_offset;
static size_t output_offset;

static int port;

static void buffer_push(short *b, size_t samples) {
  for (int i = 0; i < 2 * samples; ++i)
    buffer[(decode_offset + i) % BUFFER_SIZE] = b[i];
  decode_offset = (decode_offset + 2 * samples) % BUFFER_SIZE;
}

static void buffer_pull(short *b, size_t samples) {
  for (int i = 0; i < 2 * samples; ++i)
    b[i] = buffer[(output_offset + i) % BUFFER_SIZE];
  output_offset = (output_offset + 2 * samples) % BUFFER_SIZE;
}

static size_t buffer_avail(void) {
  if (decode_offset >= output_offset)
    return (decode_offset - output_offset) / 2;
  return decode_offset / 2 + (BUFFER_SIZE - output_offset) / 2;
}

static void vita_renderer_init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig) {
  int rc;
  decoder = opus_multistream_decoder_create(opusConfig->sampleRate,
                                            opusConfig->channelCount,
                                            opusConfig->streams,
                                            opusConfig->coupledStreams,
                                            opusConfig->mapping,
                                            &rc);

  sceClibPrintf("decoder: 0x%x\n", decoder);

  port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, VITA_SAMPLES, 48000, SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO);
  sceClibPrintf("open port 0x%x\n", port);
}

static void vita_renderer_cleanup() {
  if (decoder != NULL)
    opus_multistream_decoder_destroy(decoder);
}

static void vita_renderer_decode_and_play_sample(char* data, int length) {
  short tmp_buffer[2 * FRAME_SIZE];

  if (!data)
    return;

  int decodeLen = opus_multistream_decode(decoder, data, length, tmp_buffer, FRAME_SIZE, 0);
  if (decodeLen > 0) {
    buffer_push(tmp_buffer, decodeLen);

    while (buffer_avail() >= VITA_SAMPLES) {
      buffer_pull(tmp_buffer, VITA_SAMPLES);
      sceAudioOutOutput(port, tmp_buffer);
    }
  } else {
    printf("Opus error from decode: %d\n", decodeLen);
  }
}

AUDIO_RENDERER_CALLBACKS audio_callbacks_vita = {
  .init = vita_renderer_init,
  .cleanup = vita_renderer_cleanup,
  .decodeAndPlaySample = vita_renderer_decode_and_play_sample,
  .capabilities = CAPABILITY_DIRECT_SUBMIT,
};
