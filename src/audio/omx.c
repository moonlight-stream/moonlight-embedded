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

#include <opus_multistream.h>
#include "bcm_host.h"
#include "ilclient.h"

#define MAX_CHANNEL_COUNT 6
#define FRAME_SIZE 240

static OpusMSDecoder* decoder;
ILCLIENT_T* handle;
COMPONENT_T* component;
static OMX_BUFFERHEADERTYPE *buf;
static short pcmBuffer[FRAME_SIZE * MAX_CHANNEL_COUNT];
static int channelCount;
const char* omx_device = "hdmi";

static void omx_renderer_init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig) {
  int rc, error;
  OMX_ERRORTYPE err;
  unsigned char omxMapping[6];
  char* componentName = "audio_render";

  channelCount = opusConfig->channelCount;
  /* The supplied mapping array has order: FL-FR-C-LFE-RL-RR
   * OMX expects the order: FL-FR-LFE-C-RL-RR
   * We need copy the mapping locally and swap the channels around.
   */
  memcpy(omxMapping, opusConfig->mapping, sizeof(omxMapping));
  if (opusConfig->channelCount > 2) {
    omxMapping[2] = opusConfig->mapping[3];
    omxMapping[3] = opusConfig->mapping[2];
  }

  decoder = opus_multistream_decoder_create(opusConfig->sampleRate,
                                            opusConfig->channelCount,
                                            opusConfig->streams,
                                            opusConfig->coupledStreams,
                                            omxMapping,
                                            &rc);

  handle = ilclient_init();
  if (handle == NULL) {
  	fprintf(stderr, "IL client init failed\n");
  	exit(1);
  }

  if (ilclient_create_component(handle, &component, componentName, ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS) != 0) {
    fprintf(stderr, "Component create failed\n");
    exit(1);
  }

  if (ilclient_change_component_state(component, OMX_StateIdle)!= 0) {
    fprintf(stderr, "Couldn't change state to Idle\n");
    exit(1);
  }

  // must be before we enable buffers
  OMX_AUDIO_PARAM_PORTFORMATTYPE audioPortFormat;
  memset(&audioPortFormat, 0, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
  audioPortFormat.nSize = sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE);
  audioPortFormat.nVersion.nVersion = OMX_VERSION;

  audioPortFormat.nPortIndex = 100;

  OMX_GetParameter(ilclient_get_handle(component), OMX_IndexParamAudioPortFormat, &audioPortFormat);

  audioPortFormat.eEncoding = OMX_AUDIO_CodingPCM;
  OMX_SetParameter(ilclient_get_handle(component), OMX_IndexParamAudioPortFormat, &audioPortFormat);

  OMX_AUDIO_PARAM_PCMMODETYPE sPCMMode;

  memset(&sPCMMode, 0, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
  sPCMMode.nSize = sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
  sPCMMode.nVersion.nVersion = OMX_VERSION;
  sPCMMode.nPortIndex = 100;
  sPCMMode.nChannels = channelCount;
  sPCMMode.eNumData = OMX_NumericalDataSigned;
  sPCMMode.eEndian = OMX_EndianLittle;
  sPCMMode.nSamplingRate = opusConfig->sampleRate;
  sPCMMode.bInterleaved = OMX_TRUE;
  sPCMMode.nBitPerSample = 16;
  sPCMMode.ePCMMode = OMX_AUDIO_PCMModeLinear;

  switch(channelCount) {
  case 1:
     sPCMMode.eChannelMapping[0] = OMX_AUDIO_ChannelCF;
     break;
  case 8:
     sPCMMode.eChannelMapping[7] = OMX_AUDIO_ChannelRS;
  case 7:
     sPCMMode.eChannelMapping[6] = OMX_AUDIO_ChannelLS;
  case 6:
     sPCMMode.eChannelMapping[5] = OMX_AUDIO_ChannelRR;
  case 5:
     sPCMMode.eChannelMapping[4] = OMX_AUDIO_ChannelLR;
  case 4:
     sPCMMode.eChannelMapping[3] = OMX_AUDIO_ChannelLFE;
  case 3:
     sPCMMode.eChannelMapping[2] = OMX_AUDIO_ChannelCF;
  case 2:
     sPCMMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
     sPCMMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
     break;
  }

  err = OMX_SetParameter(ilclient_get_handle(component), OMX_IndexParamAudioPcm, &sPCMMode);
  if(err != OMX_ErrorNone){
  	fprintf(stderr, "PCM mode unsupported\n");
  	return;
  }
  OMX_CONFIG_BRCMAUDIODESTINATIONTYPE arDest;

  if (omx_device && strlen(omx_device) < sizeof(arDest.sName)) {
    memset(&arDest, 0, sizeof(OMX_CONFIG_BRCMAUDIODESTINATIONTYPE));
    arDest.nSize = sizeof(OMX_CONFIG_BRCMAUDIODESTINATIONTYPE);
    arDest.nVersion.nVersion = OMX_VERSION;

    strcpy((char *)arDest.sName, omx_device);

    err = OMX_SetParameter(ilclient_get_handle(component), OMX_IndexConfigBrcmAudioDestination, &arDest);
    if (err != OMX_ErrorNone) {
      fprintf(stderr, "Error on setting audio destination\nomx option must be set to hdmi or local\n");
      exit(1);
    }
  }

  // input port
  ilclient_enable_port_buffers(component, 100, NULL, NULL, NULL);
  ilclient_enable_port(component, 100);

  err = ilclient_change_component_state(component, OMX_StateExecuting);
  if (err < 0) {
  	fprintf(stderr, "Couldn't change state to Executing\n");
  	exit(1);
  }
}

static void omx_renderer_cleanup() {
  if (decoder != NULL)
    opus_multistream_decoder_destroy(decoder);
  if (handle != NULL) {
    if((buf = ilclient_get_input_buffer(component, 100, 1)) == NULL){
      fprintf(stderr, "Can't get audio buffer\n");
      exit(EXIT_FAILURE);
    }

    buf->nFilledLen = 0;
    buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

    if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(component), buf) != OMX_ErrorNone){
      fprintf(stderr, "Can't empty audio buffer\n");
      return;
    }

    ilclient_disable_port_buffers(component, 100, NULL, NULL, NULL);
    ilclient_change_component_state(component, OMX_StateIdle);
    ilclient_change_component_state(component, OMX_StateLoaded);
  }
}

static void omx_renderer_decode_and_play_sample(char* data, int length) {
  int decodeLen = opus_multistream_decode(decoder, data, length, pcmBuffer, FRAME_SIZE, 0);
  if (decodeLen > 0) {
	buf = ilclient_get_input_buffer(component, 100, 1);
    buf->nOffset = 0;
    buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;
	int bufLength = decodeLen * sizeof(short) * channelCount;
    memcpy(buf->pBuffer, pcmBuffer, bufLength);
    buf->nFilledLen = bufLength;
    int r = OMX_EmptyThisBuffer(ilclient_get_handle(component), buf);
    if (r != OMX_ErrorNone) {
    fprintf(stderr, "Empty buffer error\n");
    } 
  } else {
    printf("Opus error from decode: %d\n", decodeLen);
  }
}

AUDIO_RENDERER_CALLBACKS audio_callbacks_omx = {
  .init = omx_renderer_init,
  .cleanup = omx_renderer_cleanup,
  .decodeAndPlaySample = omx_renderer_decode_and_play_sample,
  .capabilities = CAPABILITY_DIRECT_SUBMIT,
};
