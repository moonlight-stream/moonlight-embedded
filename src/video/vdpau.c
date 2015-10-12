/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015 Roman Grechin
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

#include "limelight-common/Limelight.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdarg.h>

#include <X11/Xlib.h>
#include <vdpau/vdpau.h>
#include <vdpau/vdpau_x11.h>

#include <pthread.h>

VdpGetProcAddress *vdp_get_proc_address;

VdpDecoderCreate *vdp_decoder_create;
VdpDecoderDestroy *vdp_decoder_destroy;
VdpDecoderRender *vdp_decoder_render;

VdpVideoSurfaceCreate *vdp_video_surface_create;
VdpVideoSurfaceDestroy *vdp_video_surface_destroy;
VdpVideoSurfaceGetBitsYCbCr *vdp_video_surface_get_bits_ycbcr;

VdpOutputSurfaceCreate *vdp_output_surface_create;
VdpOutputSurfaceDestroy *vdp_output_surface_destroy;
VdpOutputSurfaceGetBitsNative *vdp_output_surface_get_bits_native;

VdpVideoMixerCreate *vdp_video_mixer_create;
VdpVideoMixerDestroy *vdp_video_mixer_destroy;
VdpVideoMixerRender *vdp_video_mixer_render;

VdpPresentationQueueCreate *vdp_presentation_queue_create;
VdpPresentationQueueDestroy *vdp_presentation_queue_destroy;
VdpPresentationQueueDisplay *vdp_presentation_queue_display;
VdpPresentationQueueBlockUntilSurfaceIdle *vdp_presentation_queue_block_until_surface_idle;
VdpPresentationQueueGetTime *vdp_presentation_queue_get_time;
VdpPresentationQueueTargetCreateX11 *vdp_presentation_queue_target_create_x11;
VdpGenerateCSCMatrix *vdp_generate_csc_matrix;
VdpPresentationQueueQuerySurfaceStatus *vdp_presentation_queue_query_surface_status;


#define INT_MAX 2147483647

static uint32_t width=0;
static uint32_t height=0;
static uint32_t current_size = 0;
static int field_order_cnt =0;
static Screen* screen = NULL;
static char *dest;
static int frame_size = 0;
static int vframe = 0;
static VdpTime t;
static VdpOutputSurface output;
static VdpVideoMixer mixer;
static VdpPresentationQueue queue;
static VdpDecoder dec;
static VdpVideoSurface video[16];
static VdpDevice dev;
static VdpStatus ret;
static VdpProcamp procamp;
static VdpPictureInfoH264 info = {
        .slice_count = 1,
        .field_order_cnt = { 0, 0 },
        .is_reference = 1,
        .frame_num = -1,
        .field_pic_flag = 0,
        .bottom_field_flag = 0,
        .num_ref_frames = 1,
        .mb_adaptive_frame_field_flag = 0,
        .constrained_intra_pred_flag = 0,
        .weighted_pred_flag = 0,
        .weighted_bipred_idc = 0,
        .frame_mbs_only_flag = 1,
        .transform_8x8_mode_flag = 1,
        .chroma_qp_index_offset = 0,
        .second_chroma_qp_index_offset = 0,
        .pic_init_qp_minus26 = 0,
        .num_ref_idx_l0_active_minus1 = 0,
        .num_ref_idx_l1_active_minus1 = 0,
        .log2_max_frame_num_minus4 = 4,
        .pic_order_cnt_type = 2,
        .log2_max_pic_order_cnt_lsb_minus4 = 0,
        .delta_pic_order_always_zero_flag = 0,
        .direct_8x8_inference_flag = 1,
        .entropy_coding_mode_flag = 1,
        .pic_order_present_flag = 0,
        .deblocking_filter_control_present_flag = 1,
        .redundant_pic_cnt_present_flag = 0,
};



static int32_t h264_foc(int foc)
{
    if (foc == INT_MAX)
        foc = 0;
    return foc;
}

void decoder_renderer_setup(int width_t, int height_t, int redrawRate, void* context, int drFlags) {

    width = width_t;
    height = height_t;
    Display *display = XOpenDisplay(NULL);
    screen = DefaultScreenOfDisplay(display);
    Window root = XDefaultRootWindow(display);
    Window window = XCreateSimpleWindow(
            display, root, 0, 0, screen->width, screen->height, 0, 0, 0);

    Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
    Atom fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

    XEvent xev;
    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = window;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1;
    xev.xclient.data.l[1] = fullscreen;
    xev.xclient.data.l[2] = 0;

    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);
    XSendEvent (display, DefaultRootWindow(display), False,
                SubstructureRedirectMask | SubstructureNotifyMask, &xev);
    XSync(display, 0);

    ret = vdp_device_create_x11(display, 0, &dev, &vdp_get_proc_address);
    assert(ret == VDP_STATUS_OK);

#define get(id, func) \
  ret = vdp_get_proc_address(dev, id, (void **)&func); \
  assert(ret == VDP_STATUS_OK);

    get(VDP_FUNC_ID_DECODER_CREATE, vdp_decoder_create);
    get(VDP_FUNC_ID_DECODER_DESTROY, vdp_decoder_destroy);
    get(VDP_FUNC_ID_DECODER_RENDER, vdp_decoder_render);

    get(VDP_FUNC_ID_VIDEO_MIXER_CREATE, vdp_video_mixer_create);
    get(VDP_FUNC_ID_VIDEO_MIXER_DESTROY, vdp_video_mixer_destroy);
    get(VDP_FUNC_ID_VIDEO_MIXER_RENDER, vdp_video_mixer_render);

    get(VDP_FUNC_ID_VIDEO_SURFACE_CREATE, vdp_video_surface_create);
    get(VDP_FUNC_ID_VIDEO_SURFACE_DESTROY, vdp_video_surface_destroy);
    get(VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR, vdp_video_surface_get_bits_ycbcr);

    get(VDP_FUNC_ID_OUTPUT_SURFACE_CREATE, vdp_output_surface_create);
    get(VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY, vdp_output_surface_destroy);
    get(VDP_FUNC_ID_OUTPUT_SURFACE_GET_BITS_NATIVE, vdp_output_surface_get_bits_native);

    get(VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE, vdp_presentation_queue_create);
    get(VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY, vdp_presentation_queue_destroy);
    get(VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY, vdp_presentation_queue_display);
    get(VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11, vdp_presentation_queue_target_create_x11);
    get(VDP_FUNC_ID_PRESENTATION_QUEUE_BLOCK_UNTIL_SURFACE_IDLE, vdp_presentation_queue_block_until_surface_idle);
    get(VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME, vdp_presentation_queue_get_time);
    get(VDP_FUNC_ID_GENERATE_CSC_MATRIX, vdp_generate_csc_matrix);
    get(VDP_FUNC_ID_PRESENTATION_QUEUE_QUERY_SURFACE_STATUS,vdp_presentation_queue_query_surface_status);

#undef get
    VdpPresentationQueueTarget target;

    ret = vdp_decoder_create(dev, VDP_DECODER_PROFILE_H264_HIGH, width, height, 16, &dec);
    assert(ret == VDP_STATUS_OK);

    ret = vdp_output_surface_create(dev, VDP_RGBA_FORMAT_B8G8R8A8, screen->width, screen->height, &output);
    assert(ret == VDP_STATUS_OK);

    ret = vdp_presentation_queue_target_create_x11(dev, window, &target);
    assert(ret == VDP_STATUS_OK);

    ret = vdp_presentation_queue_create(dev, target, &queue);
    assert(ret == VDP_STATUS_OK);

    printf("vdp_video_mixer_create\n");
    if ( mixer!=VDP_INVALID_HANDLE )
        vdp_video_mixer_destroy( mixer );
    VdpVideoMixerFeature mixer_features[] = {};

    VdpVideoMixerParameter mixer_params[] = {
            VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH,
            VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT,
            VDP_VIDEO_MIXER_PARAMETER_CHROMA_TYPE,
            VDP_VIDEO_MIXER_PARAMETER_LAYERS
    };
    int zero = 0;
    const void *mixer_param_vals[] = {
            &width,
            &height,
            &zero
    };

    ret = vdp_video_mixer_create(dev, sizeof(mixer_features)/sizeof(mixer_features[0]), mixer_features,
                                 sizeof(mixer_params)/sizeof(mixer_params[0]), mixer_params, mixer_param_vals, &mixer);
    assert(ret == VDP_STATUS_OK);

    int j;
    for (j = 0; j < 6; ++j) {
        int k;

        for (k = 0; k < 16; ++k)
            info.scaling_lists_4x4[j][k] = 16;
    }

    for (j = 0; j < 2; ++j) {
        int k;

        for (k = 0; k < 64; ++k)
            info.scaling_lists_8x8[j][k] = 16;
    }

    for (j = 0; j < 16; ++j){
        info.referenceFrames[j].surface = VDP_INVALID_HANDLE;
    }


    for (j = 0; j < 16; j++) {
        video[j] = VDP_INVALID_HANDLE;
        ret = vdp_video_surface_create(dev, VDP_CHROMA_TYPE_420, width, height, &video[j]);
        assert(ret == VDP_STATUS_OK);
    }

    ret = vdp_presentation_queue_get_time(queue, &t);
    assert(ret == VDP_STATUS_OK);
}

void decoder_renderer_cleanup() {

}

struct thread_info {
    int nal_unit_type;
    int size;
    char *addr;
};

void *threadFunc(void *arg)
{
    struct thread_info *tinfo = arg;
    char *addr = tinfo->addr;
    int size = tinfo->size;
    int nal_unit_type = tinfo->nal_unit_type;
    if (nal_unit_type==0x65 || nal_unit_type==0x61){
        VdpBitstreamBuffer buffer;
        buffer.struct_version = VDP_BITSTREAM_BUFFER_VERSION;
        buffer.bitstream = addr;
        buffer.bitstream_bytes = size;
        ret = vdp_decoder_render(dec, video[vframe], (void * const*)&info, 1, &buffer);
        assert(ret == VDP_STATUS_OK);
        VdpRect vid_source = { 0, 0, width, height };
        VdpRect out_dest = { 0, 0, screen->width, screen->height };
        ret = vdp_video_mixer_render(
                mixer,
                VDP_INVALID_HANDLE, NULL,
                VDP_VIDEO_MIXER_PICTURE_STRUCTURE_FRAME,
                0, NULL,
                video[vframe],
                0, NULL,
                NULL,
                output,
                NULL,
                &out_dest,
                0, NULL);
        assert(ret == VDP_STATUS_OK);
        t += 1000000000ULL;

        ret = vdp_presentation_queue_display(queue, output, screen->width, screen->height, t);
        assert(ret == VDP_STATUS_OK);
        int j;
        if (info.is_reference) {
            info.referenceFrames[0].surface = video[vframe];
            info.referenceFrames[0].field_order_cnt[0] = info.field_order_cnt[0];
            info.referenceFrames[0].field_order_cnt[0] = info.field_order_cnt[1];
            info.referenceFrames[0].frame_idx = info.frame_num;
            info.referenceFrames[0].top_is_reference = 1;
            info.referenceFrames[0].bottom_is_reference = 1;
        }
        vframe = (vframe + 1) % 16;
        field_order_cnt+=2;
        field_order_cnt =  h264_foc(field_order_cnt);
    }
    return NULL;
}



int decoder_renderer_submit_decode_unit(PDECODE_UNIT decodeUnit) {
    struct thread_info *tinfo;
    int num_threads;
    PLENTRY entry = decodeUnit->bufferList;
    int nal_unit_type = entry->data[4];
    info.field_order_cnt[0] = field_order_cnt;
    info.field_order_cnt[1] = field_order_cnt;
    int size = 0;
    char addr[decodeUnit->fullLength];
    dest = addr;
    while (entry != NULL) {
        memcpy(dest, entry->data, entry->length);
        size += entry->length;
        dest += entry->length;
        entry = entry->next;
    }
    tinfo = calloc(1, sizeof(struct thread_info));
    tinfo[0].nal_unit_type = nal_unit_type;
    tinfo[0].addr = addr;
    tinfo[0].size = size;
    pthread_t pth;
    pthread_create(&pth,NULL,&threadFunc,&tinfo[0]);
    pthread_join(pth, NULL);
    return DR_OK;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_vdpau = {
        .setup = decoder_renderer_setup,
        .cleanup = decoder_renderer_cleanup,
        .submitDecodeUnit = decoder_renderer_submit_decode_unit,
};
