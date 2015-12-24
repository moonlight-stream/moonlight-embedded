/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015 Iwan Timmer
 * Copyright (C) 2003-2014 Ulrich von Zadow
 *
 * Based on Libavg VDPAU implementation
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

#include "ffmpeg_vdpau.h"

#include <vdpau/vdpau.h>
#include <libavutil/pixfmt.h>
#include <libavutil/imgutils.h>
#include <libavcodec/vdpau.h>

#define MAX_RENDER_STATES 32

static AVFrame* cpu_frame;

static VdpDevice vdp_device;
static VdpDecoder vdp_decoder;
static VdpVideoMixer vdp_mixer;
static struct vdpau_render_state* vdp_render_state[MAX_RENDER_STATES];
static int vdp_render_states = 0;

static VdpGetProcAddress* vdp_get_proc_address;
static VdpDecoderCreate* vdp_decoder_create;
static VdpDecoderRender* vdp_decoder_render;
static VdpVideoSurfaceGetBitsYCbCr* vdp_video_surface_get_bits_y_cb_cr;
static VdpVideoSurfaceCreate* vdp_video_surface_create;
static VdpVideoMixerCreate* vdp_video_mixer_create;

struct vdpau_render_state* vdp_get_free_render_state() {
  for (unsigned i = 0; i < vdp_render_states; i++) {
    struct  vdpau_render_state* render_state = vdp_render_state[i];
    if (!render_state->state)
      return vdp_render_state[i];
  }

  if (vdp_render_states == MAX_RENDER_STATES)
    return NULL;

  // No free surfaces available
  struct vdpau_render_state* render_state = malloc(sizeof(struct vdpau_render_state));
  vdp_render_state[vdp_render_states] = render_state;
  vdp_render_states++;
  memset(render_state, 0, sizeof(struct vdpau_render_state));
  render_state->surface = VDP_INVALID_HANDLE;
  VdpStatus status = vdp_video_surface_create(vdp_device, VDP_CHROMA_TYPE_420, 1280, 720, &render_state->surface);
  return render_state;
}

static int vdp_get_buffer(AVCodecContext* context, AVFrame* frame) {
  struct vdpau_render_state* pRenderState = vdp_get_free_render_state();
  frame->data[0] = (uint8_t*) pRenderState;
  frame->type = FF_BUFFER_TYPE_USER;

  pRenderState->state |= FF_VDPAU_STATE_USED_FOR_RENDER;
  return 0;
}

static void vdp_release_buffer(AVCodecContext* context, AVFrame* frame) {
  struct vdpau_render_state *render_state = (struct vdpau_render_state *)frame->data[0];
  render_state->state = 0;
  frame->data[0] = 0;
}

static enum AVPixelFormat vdp_get_format(AVCodecContext* context, const enum AVPixelFormat* pixel_format) {
  return PIX_FMT_VDPAU_H264;
}

static void vdp_draw_horiz_band(struct AVCodecContext* context, const AVFrame* frame, int offset[4], int y, int type, int height) {
  struct vdpau_render_state* render_state = (struct vdpau_render_state*)frame->data[0];

  VdpStatus status = vdp_decoder_render(vdp_decoder, render_state->surface, (VdpPictureInfo const*)&(render_state->info), render_state->bitstream_buffers_used, render_state->bitstream_buffers);
  status = vdp_decoder_render(vdp_decoder, render_state->surface, (VdpPictureInfo const*)&(render_state->info), render_state->bitstream_buffers_used, render_state->bitstream_buffers);
}

int vdpau_init(AVCodecContext* decoder_ctx, int width, int height) {
  if (vdp_device)
    return vdp_device;

  Display* xdisplay = XOpenDisplay(0);
  if (!xdisplay)
    return -1;

  VdpStatus status = vdp_device_create_x11(xdisplay, DefaultScreen(xdisplay), &vdp_device, &vdp_get_proc_address);
  if (status != VDP_STATUS_OK)
     return -1;

  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR, (void**)&vdp_video_surface_get_bits_y_cb_cr);
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_VIDEO_SURFACE_CREATE, (void**)&vdp_video_surface_create);
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_DECODER_RENDER, (void**)&vdp_decoder_render);
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_DECODER_CREATE, (void**)&vdp_decoder_create);
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_VIDEO_MIXER_CREATE, (void**)&vdp_video_mixer_create);

  decoder_ctx->get_buffer = vdp_get_buffer;
  decoder_ctx->release_buffer = vdp_release_buffer;
  decoder_ctx->draw_horiz_band = vdp_draw_horiz_band;
  decoder_ctx->get_format = vdp_get_format;
  decoder_ctx->slice_flags = SLICE_FLAG_CODED_ORDER | SLICE_FLAG_ALLOW_FIELD;

  cpu_frame = av_frame_alloc();
  if (cpu_frame == NULL) {
    printf("Couldn't allocate frame\n");
    return -1;
  }
  cpu_frame->format = PIX_FMT_YUV420P;
  cpu_frame->width = width;
  cpu_frame->height = height;
  av_frame_get_buffer(cpu_frame, 32);

  if (!vdp_device) {
    printf("Can't get VDPAU device\n");
    return -1;
  }

  status = vdp_decoder_create(vdp_device, VDP_DECODER_PROFILE_H264_HIGH, width, height, 16, &vdp_decoder);
  if (status != VDP_STATUS_OK) {
    printf("Can't create VDPAU decoder\n");
    return -1;
  }

  VdpVideoMixerFeature features[] = {
    VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL,
    VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL_SPATIAL,
  };
  VdpVideoMixerParameter params[] = {
    VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH,
    VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT,
    VDP_VIDEO_MIXER_PARAMETER_CHROMA_TYPE,
    VDP_VIDEO_MIXER_PARAMETER_LAYERS
  };
  VdpChromaType chroma = VDP_CHROMA_TYPE_420;
  int numLayers = 0;
  void const* paramValues[] = { &width, &height, &chroma, &numLayers };

  status = vdp_video_mixer_create(vdp_device, 0, features, 4, params, paramValues, &vdp_mixer);
  if (status != VDP_STATUS_OK) {
    printf("Can't create VDPAU mixer\n");
    return -1;
  }

  return vdp_device;
}

AVFrame* vdpau_get_frame(AVFrame* dec_frame) {
  struct vdpau_render_state *render_state = (struct vdpau_render_state *)dec_frame->data[0];
  void *dest[3] = {
    cpu_frame->data[0],
    cpu_frame->data[2],
    cpu_frame->data[1]
  };
  uint32_t pitches[3] = {
    cpu_frame->linesize[0],
    cpu_frame->linesize[2],
    cpu_frame->linesize[1]
  };

  VdpStatus status = vdp_video_surface_get_bits_y_cb_cr(render_state->surface, VDP_YCBCR_FORMAT_YV12, dest, pitches);
  return cpu_frame;
}
