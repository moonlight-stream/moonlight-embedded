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

#include <SDL.h>
#include <SDL_audio.h>

#include <stdio.h>
#include <opus.h>

#define SAMPLE_RATE 48000
#define CHANNEL_COUNT 2
#define FRAME_SIZE 240

static OpusDecoder* decoder;
static short pcmBuffer[FRAME_SIZE * CHANNEL_COUNT];
static SDL_AudioDeviceID dev;

static void sdl_renderer_init() {
  int rc;
  decoder = opus_decoder_create(SAMPLE_RATE, CHANNEL_COUNT, &rc);

  SDL_InitSubSystem(SDL_INIT_AUDIO);

  SDL_AudioSpec want, have;
  SDL_zero(want);
  want.freq = SAMPLE_RATE;
  want.format = AUDIO_S16LSB;
  want.channels = CHANNEL_COUNT;
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
    opus_decoder_destroy(decoder);

  SDL_CloseAudioDevice(dev);
}

static void sdl_renderer_decode_and_play_sample(char* data, int length) {
  int decodeLen = opus_decode(decoder, data, length, pcmBuffer, FRAME_SIZE, 0);
  if (decodeLen > 0) {
    SDL_QueueAudio(dev, pcmBuffer, decodeLen * CHANNEL_COUNT * sizeof(short));
  } else {
    printf("Opus error from decode: %d\n", decodeLen);
  }
}

AUDIO_RENDERER_CALLBACKS audio_callbacks_sdl = {
  .init = sdl_renderer_init,
  .cleanup = sdl_renderer_cleanup,
  .decodeAndPlaySample = sdl_renderer_decode_and_play_sample,
};
