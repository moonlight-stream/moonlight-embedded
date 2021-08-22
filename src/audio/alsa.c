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
#include <string.h>

#include <opus_multistream.h>
#include <alsa/asoundlib.h>

#define CHECK_RETURN(f) if ((rc = f) < 0) { printf("Alsa error code %d\n", rc); return -1; }

static snd_pcm_t *handle;
static OpusMSDecoder* decoder;
static short* pcmBuffer;
static int samplesPerFrame;

static int alsa_renderer_init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig, void* context, int arFlags) {
  int rc;
  unsigned char alsaMapping[AUDIO_CONFIGURATION_MAX_CHANNEL_COUNT];

  samplesPerFrame = opusConfig->samplesPerFrame;
  pcmBuffer = malloc(sizeof(short) * opusConfig->channelCount * samplesPerFrame);
  if (pcmBuffer == NULL)
    return -1;

  snd_pcm_hw_params_t *hw_params;
  snd_pcm_sw_params_t *sw_params;
  snd_pcm_uframes_t period_size = samplesPerFrame * FRAME_BUFFER;
  snd_pcm_uframes_t buffer_size = 2 * period_size;
  unsigned int sampleRate = opusConfig->sampleRate;

  char* audio_device = (char*) context;
  if (audio_device == NULL)
    audio_device = "sysdefault";

  /* Open PCM device for playback. */
  CHECK_RETURN(snd_pcm_open(&handle, audio_device, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK))

  /* Set hardware parameters */
  CHECK_RETURN(snd_pcm_hw_params_malloc(&hw_params));
  CHECK_RETURN(snd_pcm_hw_params_any(handle, hw_params));
  CHECK_RETURN(snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED));
  CHECK_RETURN(snd_pcm_hw_params_set_format(handle, hw_params, SND_PCM_FORMAT_S16_LE));
  CHECK_RETURN(snd_pcm_hw_params_set_rate_near(handle, hw_params, &sampleRate, NULL));
  CHECK_RETURN(snd_pcm_hw_params_set_channels(handle, hw_params, opusConfig->channelCount));
  CHECK_RETURN(snd_pcm_hw_params_set_period_size_near(handle, hw_params, &period_size, NULL));
  CHECK_RETURN(snd_pcm_hw_params_set_buffer_size_near(handle, hw_params, &buffer_size));
  CHECK_RETURN(snd_pcm_hw_params(handle, hw_params));
  snd_pcm_hw_params_free(hw_params);

  /* Set software parameters */
  CHECK_RETURN(snd_pcm_sw_params_malloc(&sw_params));
  CHECK_RETURN(snd_pcm_sw_params_current(handle, sw_params));
  CHECK_RETURN(snd_pcm_sw_params_set_avail_min(handle, sw_params, period_size));
  CHECK_RETURN(snd_pcm_sw_params_set_start_threshold(handle, sw_params, 1));
  CHECK_RETURN(snd_pcm_sw_params(handle, sw_params));
  snd_pcm_sw_params_free(sw_params);

  /* Determine the ideal mapping for a given channel count */
  snd_pcm_chmap_t* chmap;
  switch (opusConfig->channelCount)
  {
    case 2:
      chmap = snd_pcm_chmap_parse_string("FL FR");
      break;
    case 6:
      chmap = snd_pcm_chmap_parse_string("FL FR RL RR FC LFE");
      break;
    case 8:
      chmap = snd_pcm_chmap_parse_string("FL FR RL RR FC LFE SL SR");
      break;
    default:
      fprintf(stderr, "Unsupported channel count: %d\n", opusConfig->channelCount);
      return -1;
  }

  /* If we have a channel map, try to apply it */
  if (snd_pcm_set_chmap(handle, chmap) != 0) {
    snd_pcm_chmap_t* native_chmap = snd_pcm_get_chmap(handle);
    if (native_chmap != NULL) {
      /* We couldn't apply our channel map, so we'll match the current map */
      free(chmap);
      chmap = native_chmap;
    }
    else {
      /* We couldn't even get the native layout. We'll just hope for the best */
      fprintf(stderr, "Failed to get ALSA channel map. Surround audio channels may be incorrect!\n");
    }
  }

  char chmap_str[512];
  snd_pcm_chmap_print(chmap, sizeof(chmap_str), chmap_str);
  printf("ALSA channel map: %s\n", chmap_str);

  /* The supplied mapping array has order: FL-FR-C-LFE-RL-RR-SL-SR
   * We need copy the mapping locally and swap the channels around.
   */
  memcpy(alsaMapping, opusConfig->mapping, sizeof(alsaMapping));
  for (int i = 0; i < chmap->channels && i < AUDIO_CONFIGURATION_MAX_CHANNEL_COUNT; i++) {
    switch (chmap->pos[i]) {
      case SND_CHMAP_FL:
        alsaMapping[i] = opusConfig->mapping[0];
        break;
      case SND_CHMAP_FR:
        alsaMapping[i] = opusConfig->mapping[1];
        break;
      case SND_CHMAP_FC:
        alsaMapping[i] = opusConfig->mapping[2];
        break;
      case SND_CHMAP_LFE:
        alsaMapping[i] = opusConfig->mapping[3];
        break;
      case SND_CHMAP_RL:
        alsaMapping[i] = opusConfig->mapping[4];
        break;
      case SND_CHMAP_RR:
        alsaMapping[i] = opusConfig->mapping[5];
        break;
      case SND_CHMAP_SL:
        alsaMapping[i] = opusConfig->mapping[opusConfig->channelCount >= 8 ? 6 : 4];
        break;
      case SND_CHMAP_SR:
        alsaMapping[i] = opusConfig->mapping[opusConfig->channelCount >= 8 ? 7 : 5];
        break;
      default:
        alsaMapping[i] = 0xFF; /* Silence */
        break;
    }
  }
  free(chmap);

  decoder = opus_multistream_decoder_create(opusConfig->sampleRate, opusConfig->channelCount, opusConfig->streams, opusConfig->coupledStreams, alsaMapping, &rc);

  CHECK_RETURN(snd_pcm_prepare(handle));

  return 0;
}

static void alsa_renderer_cleanup() {
  if (decoder != NULL) {
    opus_multistream_decoder_destroy(decoder);
    decoder = NULL;
  }

  if (handle != NULL) {
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    handle = NULL;
  }

  if (pcmBuffer != NULL) {
    free(pcmBuffer);
    pcmBuffer = NULL;
  }
}

static void alsa_renderer_decode_and_play_sample(char* data, int length) {
  int decodeLen = opus_multistream_decode(decoder, data, length, pcmBuffer, samplesPerFrame, 0);
  if (decodeLen > 0) {
    int rc = snd_pcm_writei(handle, pcmBuffer, decodeLen);
    if (rc == -EPIPE)
      snd_pcm_recover(handle, rc, 1);

    if (rc<0)
      printf("Alsa error from writei: %d\n", rc);
    else if (decodeLen != rc)
      printf("Alsa shortm write, write %d frames\n", rc);
  } else {
    printf("Opus error from decode: %d\n", decodeLen);
  }
}

AUDIO_RENDERER_CALLBACKS audio_callbacks_alsa = {
  .init = alsa_renderer_init,
  .cleanup = alsa_renderer_cleanup,
  .decodeAndPlaySample = alsa_renderer_decode_and_play_sample,
  .capabilities = CAPABILITY_DIRECT_SUBMIT | CAPABILITY_SUPPORTS_ARBITRARY_AUDIO_DURATION,
};
