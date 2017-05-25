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

#include "../audio.h"

#include <SDL.h>
#include <SDL_audio.h>

#include <stdio.h>
#include <opus_multistream.h>

static OpusMSDecoder* decoder;
static short pcmBuffer[FRAME_SIZE * MAX_CHANNEL_COUNT];
static SDL_AudioDeviceID dev;
static int channelCount;

static void sdl_renderer_init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig) {
  int rc;
  decoder = opus_multistream_decoder_create(opusConfig->sampleRate, opusConfig->channelCount, opusConfig->streams, opusConfig->coupledStreams, opusConfig->mapping, &rc);

  channelCount = opusConfig->channelCount;

  SDL_InitSubSystem(SDL_INIT_AUDIO);

  SDL_AudioSpec want, have;
  SDL_zero(want);
  want.freq = opusConfig->sampleRate;
  want.format = AUDIO_S16LSB;
  want.channels = opusConfig->channelCount;
  want.samples = 4096;

  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if (dev == 0) {
    printf("Failed to open audio: %s\n", SDL_GetError());
  } else {
    if (have.format != want.format)  // we let this one thing change.
      printf("We didn't get requested audio format.\n");
    SDL_PauseAudioDevice(dev, 0);  // start audio playing.
  }
}

static void sdl_renderer_cleanup() {
  if (decoder != NULL)
    opus_multistream_decoder_destroy(decoder);

  SDL_CloseAudioDevice(dev);
}

static void sdl_renderer_decode_and_play_sample(char* data, int length) {
  int decodeLen = opus_multistream_decode(decoder, data, length, pcmBuffer, FRAME_SIZE, 0);
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
  .capabilities = CAPABILITY_DIRECT_SUBMIT,
};
