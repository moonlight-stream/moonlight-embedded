
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

// Video decode on Raspberry Pi using MMAL
// Based upon example code from the Raspberry Pi

#include "video.h"

#include <Limelight.h>

#include <sps.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <bcm_host.h>
#include <interface/mmal/mmal.h>
#include <interface/mmal/util/mmal_default_components.h>
#include <interface/mmal/util/mmal_util_params.h>
#include <interface/mmal/util/mmal_util.h>
#include <interface/mmal/vc/mmal_vc_api.h>
#include <interface/vcos/vcos.h>

#define ALIGN(x, a) (((x)+(a)-1)&~((a)-1))

static VCOS_SEMAPHORE_T semaphore;
static MMAL_COMPONENT_T *decoder = NULL, *renderer = NULL;
static MMAL_POOL_T *pool_in = NULL, *pool_out = NULL;

static void input_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buf) {
  mmal_buffer_header_release(buf);

  if (port == decoder->input[0])
    vcos_semaphore_post(&semaphore);
}

static void control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buf) {
 if (buf->cmd == MMAL_EVENT_ERROR) {
    MMAL_STATUS_T status = *(uint32_t *) buf->data;
    fprintf(stderr, "Video decode error MMAL_EVENT_ERROR:%d\n", status);
 }

 mmal_buffer_header_release(buf);
}

static void output_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buf) {
  if (mmal_port_send_buffer(renderer->input[0], buf) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't display decoded frame\n");
    mmal_buffer_header_release(buf);
  }
}

static int decoder_renderer_setup(int videoFormat, int width, int height, int redrawRate, void* context, int drFlags) {
  if (videoFormat != VIDEO_FORMAT_H264) {
    fprintf(stderr, "Video format not supported\n");
    return -1;
  }

  bcm_host_init();
  mmal_vc_init();
  gs_sps_init(width, height);

  vcos_semaphore_create(&semaphore, "video_decoder", 1);
  if (mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_DECODER, &decoder) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't create decoder\n");
    return -2;
  }

  MMAL_ES_FORMAT_T *format_in = decoder->input[0]->format;
  format_in->type = MMAL_ES_TYPE_VIDEO;
  format_in->encoding = MMAL_ENCODING_H264;
  format_in->es->video.width = ALIGN(width, 32);
  format_in->es->video.height = ALIGN(height, 16);
  format_in->es->video.crop.width = width;
  format_in->es->video.crop.height = height;
  format_in->es->video.frame_rate.num = redrawRate;
  format_in->es->video.frame_rate.den = 1;
  format_in->es->video.par.num = 1;
  format_in->es->video.par.den = 1;
  format_in->flags = MMAL_ES_FORMAT_FLAG_FRAMED;

  if (mmal_port_format_commit(decoder->input[0]) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't commit input format to decoder\n");
    return -3;
  }

  decoder->input[0]->buffer_num = 5;
  decoder->input[0]->buffer_size = INITIAL_DECODER_BUFFER_SIZE;
  pool_in = mmal_port_pool_create(decoder->input[0], decoder->input[0]->buffer_num, decoder->output[0]->buffer_size);

  MMAL_ES_FORMAT_T *format_out = decoder->output[0]->format;
  format_out->encoding = MMAL_ENCODING_OPAQUE;
  if (mmal_port_format_commit(decoder->output[0]) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't commit output format to decoder\n");
    return -3;
  }

  decoder->output[0]->buffer_num = 3;
  decoder->output[0]->buffer_size = decoder->output[0]->buffer_size_recommended;
  pool_out = mmal_port_pool_create(decoder->output[0], decoder->output[0]->buffer_num, decoder->output[0]->buffer_size);

  if (mmal_port_enable(decoder->control, control_callback) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't enable control port\n");
    return -4;
  }

  if (mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER, &renderer) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't create renderer\n");
    return -5;
  }

  format_in = renderer->input[0]->format;
  format_in->encoding = MMAL_ENCODING_OPAQUE;
  format_in->es->video.width = width;
  format_in->es->video.height = height;
  format_in->es->video.crop.x = 0;
  format_in->es->video.crop.y = 0;
  format_in->es->video.crop.width = width;
  format_in->es->video.crop.height = height;
  if (mmal_port_format_commit(renderer->input[0]) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't set output format\n");
    return -6;
  }

  MMAL_DISPLAYREGION_T param;
  param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
  param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);
  param.set = MMAL_DISPLAY_SET_LAYER | MMAL_DISPLAY_SET_NUM | MMAL_DISPLAY_SET_FULLSCREEN | MMAL_DISPLAY_SET_TRANSFORM;
  param.layer = 128;
  param.display_num = 0;
  param.fullscreen = true;
  int displayRotation = drFlags & DISPLAY_ROTATE_MASK;
  switch (displayRotation) {
  case DISPLAY_ROTATE_90:
    param.transform = MMAL_DISPLAY_ROT90;
    break;
  case DISPLAY_ROTATE_180:
    param.transform = MMAL_DISPLAY_ROT180;
    break;
  case DISPLAY_ROTATE_270:
    param.transform = MMAL_DISPLAY_ROT270;
    break;
  default:
    param.transform = MMAL_DISPLAY_ROT0;
    break;
  }

  if (mmal_port_parameter_set(renderer->input[0], &param.hdr) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't set parameters\n");
    return -7;
  }

  if (mmal_port_enable(renderer->control, control_callback) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't enable control port\n");
    return -8;
  }

  if (mmal_component_enable(renderer) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't enable renderer\n");
    return -9;
  }

  if (mmal_port_enable(renderer->input[0], input_callback) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't enable renderer input port\n");
    return -10;
  }

  if (mmal_port_enable(decoder->input[0], input_callback) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't enable decoder input port\n");
    return -11;
  }

  if (mmal_port_enable(decoder->output[0], output_callback) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't enable decoder output port\n");
    return -12;
  }

  if (mmal_component_enable(decoder) != MMAL_SUCCESS) {
    fprintf(stderr, "Can't enable decoder\n");
    return -13;
  }

  return 0;
}

static void decoder_renderer_cleanup() {
  if (decoder)
    mmal_component_destroy(decoder);

  if (renderer)
    mmal_component_destroy(renderer);

  if (pool_in)
    mmal_pool_destroy(pool_in);

  if (pool_out)
    mmal_pool_destroy(pool_out);

  vcos_semaphore_delete(&semaphore);
}

static int decoder_renderer_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  MMAL_STATUS_T status;
  MMAL_BUFFER_HEADER_T *buf = NULL;
  PLENTRY entry = decodeUnit->bufferList;
  bool first_entry = false;

  while (entry != NULL) {
    if (buf == NULL) {
      vcos_semaphore_wait(&semaphore);
      if ((buf = mmal_queue_get(pool_in->queue)) != NULL) {
        buf->flags = 0;
        buf->offset = 0;
        buf->pts = buf->dts = MMAL_TIME_UNKNOWN;
      } else {
        fprintf(stderr, "Video buffer full\n");
        return DR_NEED_IDR;
      }
    }

    if (entry->bufferType != BUFFER_TYPE_PICDATA)
      buf->flags |= MMAL_BUFFER_HEADER_FLAG_CONFIG;
    else if (!first_entry) {
      buf->flags |= MMAL_BUFFER_HEADER_FLAG_FRAME_START;
      first_entry = true;
    }

    if (entry->bufferType == BUFFER_TYPE_SPS)
      gs_sps_fix(entry, GS_SPS_BITSTREAM_FIXUP, buf->data, &buf->length);
    else {
      if (entry->length + buf->length > buf->alloc_size) {
        fprintf(stderr, "Video decoder buffer too small\n");
        mmal_buffer_header_release(buf);
        return DR_NEED_IDR;
      }
      memcpy(buf->data + buf->length, entry->data, entry->length);
      buf->length += entry->length;
    }

    if (entry->bufferType != BUFFER_TYPE_PICDATA || entry->next == NULL || entry->next->bufferType != BUFFER_TYPE_PICDATA) {
      buf->flags |= MMAL_BUFFER_HEADER_FLAG_FRAME_END;
      if ((status = mmal_port_send_buffer(decoder->input[0], buf)) != MMAL_SUCCESS) {
        mmal_buffer_header_release(buf);
        return DR_NEED_IDR;
      }
      buf = NULL;
    }

    entry = entry->next;
  }

  //Send available output buffers to decoder
  while ((buf = mmal_queue_get(pool_out->queue))) {
    if ((status = mmal_port_send_buffer(decoder->output[0], buf)) != MMAL_SUCCESS)
      mmal_buffer_header_release(buf);
  }

  return DR_OK;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_mmal = {
  .setup = decoder_renderer_setup,
  .cleanup = decoder_renderer_cleanup,
  .submitDecodeUnit = decoder_renderer_submit_decode_unit,
  .capabilities = CAPABILITY_DIRECT_SUBMIT,
};
