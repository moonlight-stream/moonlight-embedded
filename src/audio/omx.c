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
#include <stdlib.h>
#include <sys/stat.h>

#include <OMX_Core.h>
#include <OMX_Component.h>

#include <opus_multistream.h>
#include <bcm_host.h>
#include "ilclient.h"

#define MAX_CHANNEL_COUNT 6
#define FRAME_SIZE 240

static OpusMSDecoder* decoder;
ILCLIENT_T  *handle;
COMPONENT_T *component;
static short pcmBuffer[FRAME_SIZE * MAX_CHANNEL_COUNT];
static int channelCount;

void setOutputDevice(OMX_HANDLETYPE hdl, const char *name) {
    OMX_ERRORTYPE err;
    OMX_CONFIG_BRCMAUDIODESTINATIONTYPE arDest;

    if (name && strlen(name) < sizeof(arDest.sName)) {
	memset(&arDest, 0, sizeof(OMX_CONFIG_BRCMAUDIODESTINATIONTYPE));
	arDest.nSize = sizeof(OMX_CONFIG_BRCMAUDIODESTINATIONTYPE);
	arDest.nVersion.nVersion = OMX_VERSION;

	strcpy((char *)arDest.sName, name);
       
	err = OMX_SetParameter(hdl, OMX_IndexConfigBrcmAudioDestination, &arDest);
	if (err != OMX_ErrorNone) {
	    fprintf(stderr, "Error on setting audio destination\n");
	    exit(1);
	}
    }
}

static OMX_STATETYPE GetOMXState(OMX_HANDLETYPE hdl) {
	OMX_STATETYPE state;
	OMX_ERRORTYPE err;
	
    err = OMX_GetState(handle, &state);
    if (err != OMX_ErrorNone) {
        fprintf(stderr, "Error on getting state\n");
        exit(1);
    } else {
    return state;
    }
}

void setPCMMode(OMX_HANDLETYPE hdl, int startPortNumber, int num_channels, int sampleRate) {
    OMX_AUDIO_PARAM_PCMMODETYPE sPCMMode;
    OMX_ERRORTYPE err;
 
    memset(&sPCMMode, 0, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
    sPCMMode.nSize = sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
    sPCMMode.nVersion.nVersion = OMX_VERSION;

    sPCMMode.nPortIndex = startPortNumber;

    sPCMMode.nPortIndex = 100;
    sPCMMode.nChannels = num_channels;
    sPCMMode.eNumData = OMX_NumericalDataSigned;
    sPCMMode.eEndian = OMX_EndianLittle;
    sPCMMode.nSamplingRate = sampleRate;
    sPCMMode.bInterleaved = OMX_TRUE;
    sPCMMode.nBitPerSample = 16;
    sPCMMode.ePCMMode = OMX_AUDIO_PCMModeLinear;

    switch(num_channels) {
    case 1:
       sPCMMode.eChannelMapping[0] = OMX_AUDIO_ChannelCF;
       break;
    case 3:
       sPCMMode.eChannelMapping[2] = OMX_AUDIO_ChannelCF;
       sPCMMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
       sPCMMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
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
       sPCMMode.eChannelMapping[2] = OMX_AUDIO_ChannelCF;
    case 2:
       sPCMMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
       sPCMMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
       break;
	}

    err = OMX_SetParameter(hdl, OMX_IndexParamAudioPcm, &sPCMMode);
    if(err != OMX_ErrorNone){
		fprintf(stderr, "PCM mode unsupported\n");
		return;
    }
}

char *err2str(int err) {
    switch (err) {
    case OMX_ErrorInsufficientResources: return "OMX_ErrorInsufficientResources";
    case OMX_ErrorUndefined: return "OMX_ErrorUndefined";
    case OMX_ErrorInvalidComponentName: return "OMX_ErrorInvalidComponentName";
    case OMX_ErrorComponentNotFound: return "OMX_ErrorComponentNotFound";
    case OMX_ErrorInvalidComponent: return "OMX_ErrorInvalidComponent";
    case OMX_ErrorBadParameter: return "OMX_ErrorBadParameter";
    case OMX_ErrorNotImplemented: return "OMX_ErrorNotImplemented";
    case OMX_ErrorUnderflow: return "OMX_ErrorUnderflow";
    case OMX_ErrorOverflow: return "OMX_ErrorOverflow";
    case OMX_ErrorHardware: return "OMX_ErrorHardware";
    case OMX_ErrorInvalidState: return "OMX_ErrorInvalidState";
    case OMX_ErrorStreamCorrupt: return "OMX_ErrorStreamCorrupt";
    case OMX_ErrorPortsNotCompatible: return "OMX_ErrorPortsNotCompatible";
    case OMX_ErrorResourcesLost: return "OMX_ErrorResourcesLost";
    case OMX_ErrorNoMore: return "OMX_ErrorNoMore";
    case OMX_ErrorVersionMismatch: return "OMX_ErrorVersionMismatch";
    case OMX_ErrorNotReady: return "OMX_ErrorNotReady";
    case OMX_ErrorTimeout: return "OMX_ErrorTimeout";
    case OMX_ErrorSameState: return "OMX_ErrorSameState";
    case OMX_ErrorResourcesPreempted: return "OMX_ErrorResourcesPreempted";
    case OMX_ErrorPortUnresponsiveDuringAllocation: return "OMX_ErrorPortUnresponsiveDuringAllocation";
    case OMX_ErrorPortUnresponsiveDuringDeallocation: return "OMX_ErrorPortUnresponsiveDuringDeallocation";
    case OMX_ErrorPortUnresponsiveDuringStop: return "OMX_ErrorPortUnresponsiveDuringStop";
    case OMX_ErrorIncorrectStateTransition: return "OMX_ErrorIncorrectStateTransition";
    case OMX_ErrorIncorrectStateOperation: return "OMX_ErrorIncorrectStateOperation";
    case OMX_ErrorUnsupportedSetting: return "OMX_ErrorUnsupportedSetting";
    case OMX_ErrorUnsupportedIndex: return "OMX_ErrorUnsupportedIndex";
    case OMX_ErrorBadPortIndex: return "OMX_ErrorBadPortIndex";
    case OMX_ErrorPortUnpopulated: return "OMX_ErrorPortUnpopulated";
    case OMX_ErrorComponentSuspended: return "OMX_ErrorComponentSuspended";
    case OMX_ErrorDynamicResourcesUnavailable: return "OMX_ErrorDynamicResourcesUnavailable";
    case OMX_ErrorMbErrorsInFrame: return "OMX_ErrorMbErrorsInFrame";
    case OMX_ErrorFormatNotDetected: return "OMX_ErrorFormatNotDetected";
    case OMX_ErrorContentPipeOpenFailed: return "OMX_ErrorContentPipeOpenFailed";
    case OMX_ErrorContentPipeCreationFailed: return "OMX_ErrorContentPipeCreationFailed";
    case OMX_ErrorSeperateTablesUsed: return "OMX_ErrorSeperateTablesUsed";
    case OMX_ErrorTunnelingUnsupported: return "OMX_ErrorTunnelingUnsupported";
    default: return "unknown error";
    }
}

void error_callback(void *userdata, COMPONENT_T *comp, OMX_U32 data) {
    fprintf(stderr, "OMX error %s\n", err2str(data));
}

static void set_audio_render_input_format(COMPONENT_T *cmpt, int num_channels, int sampleRate) {
    // set input audio format
    OMX_AUDIO_PARAM_PORTFORMATTYPE audioPortFormat;
    memset(&audioPortFormat, 0, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
    audioPortFormat.nSize = sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE);
    audioPortFormat.nVersion.nVersion = OMX_VERSION;

    audioPortFormat.nPortIndex = 100;


    OMX_GetParameter(ilclient_get_handle(cmpt),
                     OMX_IndexParamAudioPortFormat, &audioPortFormat);

    audioPortFormat.eEncoding = OMX_AUDIO_CodingPCM;
    OMX_SetParameter(ilclient_get_handle(cmpt),
                     OMX_IndexParamAudioPortFormat, &audioPortFormat);

    setPCMMode(ilclient_get_handle(cmpt), 100, num_channels, sampleRate);

}

static void omx_renderer_init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig) {
    int rc;
	unsigned char omxMapping[6];

	channelCount = opusConfig->channelCount;

    omxMapping[0] = opusConfig->mapping[0];
    omxMapping[1] = opusConfig->mapping[1];
    if (opusConfig->channelCount == 6) {
      omxMapping[2] = opusConfig->mapping[4];
      omxMapping[3] = opusConfig->mapping[5];
      omxMapping[4] = opusConfig->mapping[2];
      omxMapping[5] = opusConfig->mapping[3];
    }

    decoder = opus_multistream_decoder_create(opusConfig->sampleRate,
                                              opusConfig->channelCount,
                                              opusConfig->streams,
                                              opusConfig->coupledStreams,
                                              omxMapping,
                                              &rc);

    int i;
    char *componentName = "audio_render";
    int err;

    bcm_host_init();

    handle = ilclient_init();
    if (handle == NULL) {
		fprintf(stderr, "IL client init failed\n");
		exit(1);
    }

    if (OMX_Init() != OMX_ErrorNone) {
        ilclient_destroy(handle);
        fprintf(stderr, "OMX init failed\n");
		exit(1);
    }

    ilclient_set_error_callback(handle,
				error_callback,
				NULL);

    err = ilclient_create_component(handle,
				    &component,
				    componentName,
				    ILCLIENT_DISABLE_ALL_PORTS
				    |
				    ILCLIENT_ENABLE_INPUT_BUFFERS
				    );
    if (err == -1) {
	fprintf(stderr, "Component create failed\n");
	exit(1);
    }

    if (GetOMXState(handle) != OMX_StateIdle) {
		err = ilclient_change_component_state(component, OMX_StateIdle);
		if (err < 0) {
			fprintf(stderr, "Couldn't change state to Idle\n");
			exit(1);
		}
	}

    // must be before we enable buffers
    set_audio_render_input_format(component, opusConfig->channelCount, opusConfig->sampleRate);

    setOutputDevice(ilclient_get_handle(component), "hdmi");

    // input port
    ilclient_enable_port_buffers(component, 100, 
				 NULL, NULL, NULL);
    ilclient_enable_port(component, 100);

	if (GetOMXState(handle) != OMX_StateExecuting) {
		err = ilclient_change_component_state(component, OMX_StateExecuting);
		if (err < 0) {
			fprintf(stderr, "Couldn't change state to Executing\n");
			exit(1);
			}
	}
}

static void omx_renderer_cleanup() {
  if (decoder != NULL)
    opus_multistream_decoder_destroy(decoder);
  if (handle != NULL) {
    OMX_Deinit();
    ilclient_destroy(handle);
    bcm_host_deinit();
  }
}

static inline OMX_TICKS ToOmxTicks(int64_t value) {
    OMX_TICKS s;
    s.nLowPart = value;
    s.nHighPart = value >> 32;
    return s;
}

static OMX_ERRORTYPE omx_play_buffer(short *buffer, int length) {
    int r;
	OMX_BUFFERHEADERTYPE *buff_header = 
	ilclient_get_input_buffer(component, 100, 1);

	buff_header->nOffset = 0;
	buff_header->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;
	buff_header->nTimeStamp = ToOmxTicks(0);
	memcpy(buff_header->pBuffer, buffer, length);
	buff_header->nFilledLen = length;
    r = OMX_EmptyThisBuffer(ilclient_get_handle(component),
			    buff_header);
    if (r != OMX_ErrorNone) {
	fprintf(stderr, "Empty buffer error %s\n",
		err2str(r));
    }
    return r;
}

static void omx_renderer_decode_and_play_sample(char* data, int length) {
  int decodeLen = opus_multistream_decode(decoder, data, length, pcmBuffer, FRAME_SIZE, 0);
  if (decodeLen > 0) {
	OMX_ERRORTYPE err = omx_play_buffer(pcmBuffer, decodeLen * sizeof(short) * channelCount);
    if (err < 0)
      printf("OMX error from omx_play_buffer: %d\n", err);
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
