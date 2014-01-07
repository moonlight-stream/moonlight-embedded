/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.
	* Neither the name of the copyright holder nor the
	names of its contributors may be used to endorse or promote products
	derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Video deocode demo using OpenMAX IL though the ilcient helper library

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bcm_host.h"
#include "ilclient.h"

TUNNEL_T tunnel[2];
COMPONENT_T *list[3];
ILCLIENT_T *client;

COMPONENT_T *video_decode = NULL, *video_scheduler = NULL, *video_render = NULL;

OMX_BUFFERHEADERTYPE *buf;
unsigned char *dest;

int port_settings_changed;
int first_packet;
int last_packet;

int nv_omx_init(void) {
	bcm_host_init();
	
	OMX_VIDEO_PARAM_PORTFORMATTYPE format;
	OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
	COMPONENT_T *clock = NULL;
	int status = 0;
	unsigned int data_len = 0;
	int packet_size = 80<<10;

	memset(list, 0, sizeof(list));
	memset(tunnel, 0, sizeof(tunnel));

	if((client = ilclient_init()) == NULL) 
		return -3;

	if(OMX_Init() != OMX_ErrorNone) {
		ilclient_destroy(client);
		return -4;
	}

	// create video_decode
	if(ilclient_create_component(client, &video_decode, "video_decode", ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS) != 0)
		status = -14;
	list[0] = video_decode;

	// create video_render
	if(status == 0 && ilclient_create_component(client, &video_render, "video_render", ILCLIENT_DISABLE_ALL_PORTS) != 0)
		status = -14;
	list[1] = video_render;

/*
	// create clock
	if(status == 0 && ilclient_create_component(client, &clock, "clock", ILCLIENT_DISABLE_ALL_PORTS) != 0)
		status = -14;
	list[2] = clock;
*/

//	memset(&cstate, 0, sizeof(cstate));
//	cstate.nSize = sizeof(cstate);
//	cstate.nVersion.nVersion = OMX_VERSION;
//	cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
//	cstate.nWaitMask = 1;
//	if(clock != NULL && OMX_SetParameter(ILC_GET_HANDLE(clock), OMX_IndexConfigTimeClockState, &cstate) != OMX_ErrorNone)
//		status = -13;

	// create video_scheduler
//	if(status == 0 && ilclient_create_component(client, &video_scheduler, "video_scheduler", ILCLIENT_DISABLE_ALL_PORTS) != 0)
//		status = -14;
//	list[3] = video_scheduler;

//	set_tunnel(tunnel, video_decode, 131, video_scheduler, 10);
//	set_tunnel(tunnel+1, video_scheduler, 11, video_render, 90);
//	set_tunnel(tunnel+2, clock, 80, video_scheduler, 12);
	set_tunnel(tunnel, video_decode, 131, video_render, 90);

	// setup clock tunnel first
/*
	if(status == 0 && ilclient_setup_tunnel(tunnel+2, 0, 0) != 0)
		status = -15;
	else
		ilclient_change_component_state(clock, OMX_StateExecuting);
*/

	if(status == 0)
		ilclient_change_component_state(video_decode, OMX_StateIdle);

	memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
	format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
	format.nVersion.nVersion = OMX_VERSION;
	format.nPortIndex = 130;
	format.eCompressionFormat = OMX_VIDEO_CodingAVC;

        OMX_PARAM_DATAUNITTYPE unit;

        memset(&unit, 0, sizeof(OMX_PARAM_DATAUNITTYPE));
        unit.nSize = sizeof(OMX_PARAM_DATAUNITTYPE);
        unit.nVersion.nVersion = OMX_VERSION;
        unit.nPortIndex = 130;
        unit.eUnitType = OMX_DataUnitCodedPicture;
        unit.eEncapsulationType = OMX_DataEncapsulationElementaryStream;

	if(status == 0 &&
		OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone &&
		OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamBrcmDataUnit, &unit) == OMX_ErrorNone &&
/*		OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamBrcmExtraBuffers, 2) == OMX_ErrorNone && */
		ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0) {

		port_settings_changed = 0;
		first_packet = 1;
		last_packet = 1;

		ilclient_change_component_state(video_decode, OMX_StateExecuting);
	} else
		status = -14;

	return status;
}

int nv_omx_decode(const unsigned char* indata, int data_len, int last) {
	if (last_packet) {
		last_packet = 0;

		if((buf = ilclient_get_input_buffer(video_decode, 130, 1)) == NULL)
			return -22;
	
		// feed data and wait until we get port settings changed
	        dest = buf->pBuffer;

        	buf->nFilledLen = 0;

       		buf->nOffset = 0;

	        buf->nFlags = OMX_BUFFERFLAG_ENDOFFRAME | OMX_BUFFERFLAG_EOS;

	        if(first_packet) {
	                buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
	                first_packet = 0;
	        }
	}

        memcpy(dest, indata, data_len);
	buf->nFilledLen += data_len;
	dest += data_len;

	if (last) {
		if (buf->nFilledLen == 26) {
			buf->nFilledLen += 2;
			buf->pBuffer[24] = 0x11;
			buf->pBuffer[25] = 0xe3;
			buf->pBuffer[26] = 0x06;
			buf->pBuffer[27] = 0x50;
		}

		if (buf->nFilledLen <=28) {
			int i;
		        for (i=0; i<buf->nFilledLen; i++) {
	       	 	        printf("%02x ", buf->pBuffer[i]);
		        }
	       	 	printf("\n");
		}

		if(port_settings_changed == 0 &&
			((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
			(data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
													ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0))) {
			port_settings_changed = 1;

			//OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamBrcmDataUnit, OMX_DataUnitCodedPicture);

			if(ilclient_setup_tunnel(tunnel, 0, 0) != 0)
				return -7;

			//ilclient_change_component_state(video_scheduler, OMX_StateExecuting);

			// now setup tunnel to video_render
			//if(ilclient_setup_tunnel(tunnel+1, 0, 1000) != 0)
			//	return -12;
		
			ilclient_change_component_state(video_render, OMX_StateExecuting);
			printf("Port changed\n");
		}

		last_packet = last;
		
		if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
			return -6;
	}
}

void nv_omx_stop(void) {
	int status = 0;
	
	buf->nFilledLen = 0;
	buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

	if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(list[0]), buf) != OMX_ErrorNone)
		status = -20;

	// wait for EOS from render
//	ilclient_wait_for_event(list[1], OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0,
//							ILCLIENT_BUFFER_FLAG_EOS, 10000);

	// need to flush the renderer to allow video_decode to disable its input port
	ilclient_flush_tunnels(tunnel, 0);

	ilclient_disable_port_buffers(list[0], 130, NULL, NULL, NULL);
}

void nv_omx_destroy(void) {
	ilclient_disable_tunnel(tunnel);
//	ilclient_disable_tunnel(tunnel+1);
//	ilclient_disable_tunnel(tunnel+2);
	ilclient_teardown_tunnels(tunnel);

	ilclient_state_transition(list, OMX_StateIdle);
	ilclient_state_transition(list, OMX_StateLoaded);

	ilclient_cleanup_components(list);

	OMX_Deinit();

	ilclient_destroy(client);
}
