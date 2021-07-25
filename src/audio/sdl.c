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

#include <SDL.h>
#include <SDL_audio.h>

#include <stdio.h>
#include <opus_multistream.h>

static OpusMSDecoder* decoder;
static short* pcmBuffer;
static int samplesPerFrame;
static SDL_AudioDeviceID dev;
static int channelCount;

static int sdl_renderer_init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig, void* context, int arFlags) {
  int rc;
  decoder = opus_multistream_decoder_create(opusConfig->sampleRate, opusConfig->channelCount, opusConfig->streams, opusConfig->coupledStreams, opusConfig->mapping, &rc);

  channelCount = opusConfig->channelCount;
  samplesPerFrame = opusConfig->samplesPerFrame;
  pcmBuffer = malloc(sizeof(short) * channelCount * samplesPerFrame);
  if (pcmBuffer == NULL)
    return -1;

  SDL_InitSubSystem(SDL_INIT_AUDIO);

  SDL_AudioSpec want, have;
  SDL_zero(want);
  want.freq = opusConfig->sampleRate;
  want.format = AUDIO_S16LSB;
  want.channels = opusConfig->channelCount;
  want.samples = 4096;

  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if (dev == 0) {
    printf("Failed to open audio: %s\n", SDL_GetError());
    return -1;
  } else {
    SDL_PauseAudioDevice(dev, 0);  // start audio playing.
  }

  return 0;
}

static void sdl_renderer_cleanup() {
  if (decoder != NULL) {
    opus_multistream_decoder_destroy(decoder);
    decoder = NULL;
  }

  if (pcmBuffer != NULL) {
    free(pcmBuffer);
    pcmBuffer = NULL;
  }

  if (dev != 0) {
    SDL_CloseAudioDevice(dev);
    dev = 0;
  }
}

static void sdl_renderer_decode_and_play_sample(char* data, int length) {
  int decodeLen = opus_multistream_decode(decoder, data, length, pcmBuffer, samplesPerFrame, 0);
  if (decodeLen > 0) {
    SDL_QueueAudio(dev, pcmBuffer, decodeLen * channelCount * sizeof(short));
  } else {
    printf("Opus error from decode: %d\n", decodeLen);
  }
}

AUDIO_RENDERER_CALLBACKS audio_callbacks_sdl = {
  .init = sdl_renderer_init,
  .cleanup = sdl_renderer_cleanup,
  .decodeAndPlaySample = sdl_renderer_decode_and_play_sample,
  .capabilities = CAPABILITY_DIRECT_SUBMIT | CAPABILITY_SUPPORTS_ARBITRARY_AUDIO_DURATION,
};
