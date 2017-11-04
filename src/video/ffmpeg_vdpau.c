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

#include <vdpau/vdpau_x11.h>
#include <libavutil/pixfmt.h>
#include <libavutil/imgutils.h>
#include <libavcodec/vdpau.h>

#define MAX_RENDER_STATES 32

static AVFrame* cpu_frame;

static VdpDevice vdp_device;
static VdpDecoder vdp_decoder;
static VdpVideoMixer vdp_mixer;
static VdpPresentationQueue vdp_queue;
static VdpPresentationQueueTarget vdp_queue_target;
static VdpOutputSurface vdp_output;
static struct vdpau_render_state* vdp_render_state[MAX_RENDER_STATES];
static int vdp_render_states = 0;

static VdpGetProcAddress* vdp_get_proc_address;
static VdpDeviceDestroy* vdp_device_destroy;
static VdpDecoderCreate* vdp_decoder_create;
static VdpDecoderRender* vdp_decoder_render;
static VdpVideoSurfaceGetBitsYCbCr* vdp_video_surface_get_bits_y_cb_cr;
static VdpVideoSurfaceCreate* vdp_video_surface_create;
static VdpVideoMixerCreate* vdp_video_mixer_create;
static VdpVideoMixerRender* vdp_video_mixer_render;
static VdpOutputSurfaceCreate* vdp_output_surface_create;
static VdpPresentationQueueCreate* vdp_presentation_queue_create;
static VdpPresentationQueueDisplay* vdp_presentation_queue_display;
static VdpPresentationQueueTargetCreateX11* vdp_presentation_queue_target_create_x11;

struct vdpau_render_state* vdp_get_free_render_state(int width, int height) {
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
  VdpStatus status = vdp_video_surface_create(vdp_device, VDP_CHROMA_TYPE_420, width, height, &render_state->surface);
  return render_state;
}

static void vdp_release_buffer(void* opaque, uint8_t *data) {
  struct vdpau_render_state *render_state = (struct vdpau_render_state *) data;
  render_state->state = 0;
}

static int vdp_get_buffer(AVCodecContext* context, AVFrame* frame, int flags) {
  struct vdpau_render_state* pRenderState = vdp_get_free_render_state(frame->width, frame->height);
  frame->data[0] = (uint8_t*) pRenderState;
  frame->buf[0] = av_buffer_create(frame->data[0], 0, vdp_release_buffer, NULL, 0);

  pRenderState->state |= FF_VDPAU_STATE_USED_FOR_RENDER;
  return 0;
}

static enum AVPixelFormat vdp_get_format(AVCodecContext* context, const enum AVPixelFormat* pixel_format) {
  return AV_PIX_FMT_VDPAU_H264;
}

static void vdp_draw_horiz_band(struct AVCodecContext* context, const AVFrame* frame, int offset[4], int y, int type, int height) {
  struct vdpau_render_state* render_state = (struct vdpau_render_state*)frame->data[0];

  vdp_decoder_render(vdp_decoder, render_state->surface, (VdpPictureInfo const*)&(render_state->info), render_state->bitstream_buffers_used, render_state->bitstream_buffers);
}

int vdpau_init_lib(Display* display) {
  VdpStatus status = vdp_device_create_x11(display, DefaultScreen(display), &vdp_device, &vdp_get_proc_address);
  if (status != VDP_STATUS_OK)
     return -1;

  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_DEVICE_DESTROY, (void**)&vdp_device_destroy);
  return 0;  
}

int vdpau_init(AVCodecContext* decoder_ctx, int width, int height) {
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR, (void**)&vdp_video_surface_get_bits_y_cb_cr);
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_VIDEO_SURFACE_CREATE, (void**)&vdp_video_surface_create);
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_OUTPUT_SURFACE_CREATE, (void**)&vdp_output_surface_create);
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_DECODER_RENDER, (void**)&vdp_decoder_render);
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_DECODER_CREATE, (void**)&vdp_decoder_create);
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_VIDEO_MIXER_CREATE, (void**)&vdp_video_mixer_create);
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_VIDEO_MIXER_RENDER, (void**)&vdp_video_mixer_render);
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE, (void**)&vdp_presentation_queue_create);
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11, (void**)&vdp_presentation_queue_target_create_x11);
  vdp_get_proc_address(vdp_device, VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY, (void**)&vdp_presentation_queue_display);

  decoder_ctx->get_buffer2 = vdp_get_buffer;
  decoder_ctx->draw_horiz_band = vdp_draw_horiz_band;
  decoder_ctx->get_format = vdp_get_format;

  cpu_frame = av_frame_alloc();
  if (cpu_frame == NULL) {
    printf("Couldn't allocate frame\n");
    return -1;
  }
  cpu_frame->format = AV_PIX_FMT_YUV420P;
  cpu_frame->width = width;
  cpu_frame->height = height;
  av_frame_get_buffer(cpu_frame, 32);

  if (!vdp_device) {
    printf("Can't get VDPAU device\n");
    return -1;
  }

  VdpStatus status = vdp_decoder_create(vdp_device, VDP_DECODER_PROFILE_H264_HIGH, width, height, 16, &vdp_decoder);
  if (status != VDP_STATUS_OK) {
    printf("Can't create VDPAU decoder\n");
    return -1;
  }

  return vdp_device;
}

void vdpau_destroy() {
  vdp_device_destroy(vdp_device);
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

  vdp_video_surface_get_bits_y_cb_cr(render_state->surface, VDP_YCBCR_FORMAT_YV12, dest, pitches);
  return cpu_frame;
}

int vdpau_init_presentation(Drawable win, int width, int height, int display_width, int display_height) {
  VdpVideoMixerParameter params[] = {
    VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH,
    VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT
  };
  void const* paramValues[] = { &width, &height };

  if (vdp_video_mixer_create(vdp_device, 0, NULL, 2, params, paramValues, &vdp_mixer) != VDP_STATUS_OK)
    return -1;

  if (vdp_output_surface_create(vdp_device, VDP_RGBA_FORMAT_B8G8R8A8, display_width, display_height, &vdp_output) != VDP_STATUS_OK)
    return -1;

  if(vdp_presentation_queue_target_create_x11(vdp_device, win, &vdp_queue_target) != VDP_STATUS_OK)
    return -1;

  if(vdp_presentation_queue_create(vdp_device, vdp_queue_target, &vdp_queue) != VDP_STATUS_OK)
    return -1;

  return 0;
}

void vdpau_queue(AVFrame* dec_frame) {
  struct vdpau_render_state *render_state = (struct vdpau_render_state *)dec_frame->data[0];
  vdp_video_mixer_render(vdp_mixer, VDP_INVALID_HANDLE, 0, VDP_VIDEO_MIXER_PICTURE_STRUCTURE_FRAME, 0, (VdpVideoSurface*)VDP_INVALID_HANDLE, render_state->surface, 0,(VdpVideoSurface*)VDP_INVALID_HANDLE, NULL, vdp_output, NULL, NULL, 0, NULL);  

  vdp_presentation_queue_display(vdp_queue, vdp_output, 0, 0, 0);
}
