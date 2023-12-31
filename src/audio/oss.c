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

#ifdef __FreeBSD__
#include <sys/soundcard.h>
#include "audio.h"

#include <opus_multistream.h>
#include <stdio.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

static OpusMSDecoder* decoder;
static short* pcmBuffer;
static int samplesPerFrame;
static int channelCount;
static int fd;

static int oss_renderer_init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig, void* context, int arFlags) {
  int rc;
  decoder = opus_multistream_decoder_create(opusConfig->sampleRate, opusConfig->channelCount, opusConfig->streams, opusConfig->coupledStreams, opusConfig->mapping, &rc);

  channelCount = opusConfig->channelCount;
  samplesPerFrame = opusConfig->samplesPerFrame;
  pcmBuffer = malloc(sizeof(short) * channelCount * samplesPerFrame);
  if (pcmBuffer == NULL)
    return -1;

  const char* oss_name = "/dev/dsp";
  fd = open(oss_name, O_WRONLY);
  // buffer size for fragment ,selector 12 is 4096;11 is 2048;10 is 1024; 13is 8192
  if (fd == -1) {
    close(fd);
    printf("Open audio device /dev/dsp faild!!!");
    return -1;
  }
  int frag = 12;
  if (ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &frag) == -1)
    printf("Set framgment for /dev/dsp faild.");

  int format = AFMT_S16_LE;
  int channels = opusConfig->channelCount;
  int rate = opusConfig->sampleRate;
  if (ioctl(fd, SNDCTL_DSP_SETFMT, &format) == -1)
    printf("Set framgment for /dev/dsp faild.");
  if (ioctl(fd, SNDCTL_DSP_CHANNELS, &channels) == -1)
    printf("Set channels for /dev/dsp faild.");
  if (ioctl(fd, SNDCTL_DSP_SPEED, &rate) == -1)
    printf("Set sameple rate for /dev/dsp faild.");

  return 0;
}

static void oss_renderer_cleanup() {
  if (decoder != NULL) {
    opus_multistream_decoder_destroy(decoder);
    decoder = NULL;
  }

  if (pcmBuffer != NULL) {
    free(pcmBuffer);
    pcmBuffer = NULL;
  }

  if (fd != 0) {
    close(fd);
    fd = 0;
  }
}

static void oss_renderer_decode_and_play_sample(char* data, int length) {
  int decodeLen = opus_multistream_decode(decoder, data, length, pcmBuffer, samplesPerFrame, 0);
  if (decodeLen > 0) {
    write(fd, pcmBuffer, decodeLen * channelCount * sizeof(short));
  } else {
    printf("Opus error from decode: %d\n", decodeLen);
  }
}

AUDIO_RENDERER_CALLBACKS audio_callbacks_oss = {
  .init = oss_renderer_init,
  .cleanup = oss_renderer_cleanup,
  .decodeAndPlaySample = oss_renderer_decode_and_play_sample,
  .capabilities = CAPABILITY_DIRECT_SUBMIT | CAPABILITY_SUPPORTS_ARBITRARY_AUDIO_DURATION,
};
#endif
