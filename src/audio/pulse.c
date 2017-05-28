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

#include "audio.h"

#include <stdio.h>
#include <stdlib.h>

#include <opus_multistream.h>
#include <pulse/simple.h>
#include <pulse/error.h>

static OpusMSDecoder* decoder;
static pa_simple *dev = NULL;
static short pcmBuffer[FRAME_SIZE * MAX_CHANNEL_COUNT];
static int channelCount;

bool audio_pulse_init(char* audio_device) {
  pa_sample_spec spec = {
    .format = PA_SAMPLE_S16LE,
    .rate = 44000,
    .channels = 2
  };

  int error;
  dev = pa_simple_new(audio_device, "Moonlight Embedded", PA_STREAM_PLAYBACK, NULL, "Streaming", &spec, NULL, NULL, &error);

  if (dev)
    pa_simple_free(dev);

  return (bool) dev;
}

static int pulse_renderer_init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig, void* context, int arFlags) {
  int rc, error;
  unsigned char alsaMapping[MAX_CHANNEL_COUNT];

  channelCount = opusConfig->channelCount;

  /* The supplied mapping array has order: FL-FR-C-LFE-RL-RR
   * ALSA expects the order: FL-FR-RL-RR-C-LFE
   * We need copy the mapping locally and swap the channels around.
   */
  alsaMapping[0] = opusConfig->mapping[0];
  alsaMapping[1] = opusConfig->mapping[1];
  if (opusConfig->channelCount == 6) {
    alsaMapping[2] = opusConfig->mapping[4];
    alsaMapping[3] = opusConfig->mapping[5];
    alsaMapping[4] = opusConfig->mapping[2];
    alsaMapping[5] = opusConfig->mapping[3];
  }

  decoder = opus_multistream_decoder_create(opusConfig->sampleRate, opusConfig->channelCount, opusConfig->streams, opusConfig->coupledStreams, alsaMapping, &rc);

  pa_sample_spec spec = {
    .format = PA_SAMPLE_S16LE,
    .rate = opusConfig->sampleRate,
    .channels = opusConfig->channelCount
  };

  char* audio_device = (char*) context;
  dev = pa_simple_new(audio_device, "Moonlight Embedded", PA_STREAM_PLAYBACK, NULL, "Streaming", &spec, NULL, NULL, &error);

  if (!dev) {
    printf("Pulseaudio error: %s\n", pa_strerror(error));
    return -1;
  }

  return 0;
}

static void pulse_renderer_decode_and_play_sample(char* data, int length) {
  int decodeLen = opus_multistream_decode(decoder, data, length, pcmBuffer, FRAME_SIZE, 0);
  if (decodeLen > 0) {
    int error;
    int rc = pa_simple_write(dev, pcmBuffer, decodeLen * sizeof(short) * channelCount, &error);

    if (rc<0)
      printf("Pulseaudio error: %s\n", pa_strerror(error));
  } else {
    printf("Opus error from decode: %d\n", decodeLen);
  }
}

static void pulse_renderer_cleanup() {
  pa_simple_free(dev);
}

AUDIO_RENDERER_CALLBACKS audio_callbacks_pulse = {
  .init = pulse_renderer_init,
  .cleanup = pulse_renderer_cleanup,
  .decodeAndPlaySample = pulse_renderer_decode_and_play_sample,
  .capabilities = CAPABILITY_DIRECT_SUBMIT,
};
