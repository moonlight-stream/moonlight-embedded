/*
 * This file is part of Moonlight Embedded.
 *
 * FFmpeg V4L2 Request hwaccel + DRM PRIME display backend for LibreELEC.
 * Hardware decode via rkvdec (V4L2 Request API), display via KMS atomic
 * commits on a dedicated display thread so vsync blocking never stalls
 * the decode/network pipeline.
 */

#include "video.h"
#include "../util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm_fourcc.h>

#include <libavcodec/avcodec.h>
#include <libavutil/hwcontext_drm.h>

/* ------------------------------------------------------------------ */
/* DRM state                                                            */
/* ------------------------------------------------------------------ */

static int      drm_fd  = -1;
static uint32_t crtc_id, plane_id;
static int      crtc_w, crtc_h;
static drmModeAtomicReqPtr atomic_req;

#define MAX_PROPS 64
static drmModePropertyPtr plane_props[MAX_PROPS];

static uint32_t prop_id_of(drmModePropertyPtr *props, const char *name) {
  for (int i = 0; i < MAX_PROPS && props[i]; i++)
    if (!strcmp(props[i]->name, name))
      return props[i]->prop_id;
  return 0;
}

static void load_obj_props(uint32_t obj_id, uint32_t type,
                           drmModePropertyPtr *out) {
  drmModeObjectPropertiesPtr p =
      drmModeObjectGetProperties(drm_fd, obj_id, type);
  if (!p) return;
  int n = p->count_props < MAX_PROPS - 1 ? p->count_props : MAX_PROPS - 1;
  for (int i = 0; i < n; i++)
    out[i] = drmModeGetProperty(drm_fd, p->props[i]);
  drmModeFreeObjectProperties(p);
}

/* ------------------------------------------------------------------ */
/* Display thread                                                        */
/* ------------------------------------------------------------------ */

/* One-slot queue: decoder drops a new AVFrame here; display thread        *
 * picks it up, imports as DRM FB, does a blocking atomic commit, then    *
 * releases the previous frame. Never races because we use a mutex+cond.  */

static pthread_t        disp_thread;
static pthread_mutex_t  disp_mtx  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   disp_cond = PTHREAD_COND_INITIALIZER;
static AVFrame         *disp_pending = NULL;  /* next frame to display   */
static bool             disp_eos = false;

/* Currently-displayed DRM framebuffer — released after next vsync commit */
static uint32_t cur_fb  = 0;
static uint32_t cur_gem = 0;

static void release_fb(uint32_t fb, uint32_t gem) {
  if (!fb) return;
  struct drm_gem_close gc = { .handle = gem };
  drmModeRmFB(drm_fd, fb);
  drmIoctl(drm_fd, DRM_IOCTL_GEM_CLOSE, &gc);
}

static void *display_loop(void *arg) {
  (void)arg;
  for (;;) {
    AVFrame *f = NULL;
    pthread_mutex_lock(&disp_mtx);
    while (!disp_pending && !disp_eos)
      pthread_cond_wait(&disp_cond, &disp_mtx);
    if (disp_eos && !disp_pending) {
      pthread_mutex_unlock(&disp_mtx);
      break;
    }
    f = disp_pending;
    disp_pending = NULL;
    pthread_mutex_unlock(&disp_mtx);

    if (f->format != AV_PIX_FMT_DRM_PRIME) {
      av_frame_free(&f);
      continue;
    }

    AVDRMFrameDescriptor *desc = (AVDRMFrameDescriptor *)f->data[0];
    AVDRMLayerDescriptor *layer = &desc->layers[0];

    uint32_t handles[4]={0}, pitches[4]={0}, offsets[4]={0};
    uint64_t mods[4]={0};
    for (int i = 0; i < layer->nb_planes && i < 4; i++) {
      int oi = layer->planes[i].object_index;
      if (drmPrimeFDToHandle(drm_fd, desc->objects[oi].fd, &handles[i]) < 0) {
        perror("ffmpeg_drm: drmPrimeFDToHandle");
        goto next;
      }
      pitches[i] = layer->planes[i].pitch;
      offsets[i] = layer->planes[i].offset;
      mods[i]    = desc->objects[oi].format_modifier;
    }

    uint32_t new_fb = 0;
    if (drmModeAddFB2WithModifiers(drm_fd, f->width, f->height, layer->format,
                                   handles, pitches, offsets, mods, &new_fb,
                                   mods[0] ? DRM_MODE_FB_MODIFIERS : 0) < 0) {
      if (drmModeAddFB2(drm_fd, f->width, f->height, layer->format,
                        handles, pitches, offsets, &new_fb, 0) < 0) {
        perror("ffmpeg_drm: drmModeAddFB2");
        goto next;
      }
    }

    /* Blocking atomic commit — waits for vsync before returning.
     * This is fine because we're on the display thread, not the decode thread. */
    drmModeAtomicSetCursor(atomic_req, 0);
    drmModeAtomicAddProperty(atomic_req, plane_id, prop_id_of(plane_props,"FB_ID"),   new_fb);
    drmModeAtomicAddProperty(atomic_req, plane_id, prop_id_of(plane_props,"CRTC_ID"), crtc_id);
    drmModeAtomicAddProperty(atomic_req, plane_id, prop_id_of(plane_props,"SRC_X"),   0);
    drmModeAtomicAddProperty(atomic_req, plane_id, prop_id_of(plane_props,"SRC_Y"),   0);
    drmModeAtomicAddProperty(atomic_req, plane_id, prop_id_of(plane_props,"SRC_W"),   (uint64_t)f->width  << 16);
    drmModeAtomicAddProperty(atomic_req, plane_id, prop_id_of(plane_props,"SRC_H"),   (uint64_t)f->height << 16);
    drmModeAtomicAddProperty(atomic_req, plane_id, prop_id_of(plane_props,"CRTC_X"),  0);
    drmModeAtomicAddProperty(atomic_req, plane_id, prop_id_of(plane_props,"CRTC_Y"),  0);
    drmModeAtomicAddProperty(atomic_req, plane_id, prop_id_of(plane_props,"CRTC_W"),  crtc_w);
    drmModeAtomicAddProperty(atomic_req, plane_id, prop_id_of(plane_props,"CRTC_H"),  crtc_h);
    drmModeAtomicCommit(drm_fd, atomic_req, 0, NULL);

    /* Previous FB is now definitely off-screen — safe to release */
    release_fb(cur_fb, cur_gem);
    cur_fb  = new_fb;
    cur_gem = handles[0];

next:
    av_frame_free(&f);
  }
  return NULL;
}

/* ------------------------------------------------------------------ */
/* FFmpeg state                                                          */
/* ------------------------------------------------------------------ */

static AVBufferRef    *hw_ctx;
static const AVCodec  *codec;
static AVCodecContext *codec_ctx;
static AVPacket       *pkt;
static void           *nal_buf;
static size_t          nal_size;

static enum AVPixelFormat get_hw_fmt(AVCodecContext *c,
                                     const enum AVPixelFormat *fmts) {
  (void)c;
  for (const enum AVPixelFormat *p = fmts; *p != AV_PIX_FMT_NONE; p++)
    if (*p == AV_PIX_FMT_DRM_PRIME)
      return AV_PIX_FMT_DRM_PRIME;
  fprintf(stderr, "ffmpeg_drm: DRM_PRIME not available\n");
  return fmts[0];
}

/* ------------------------------------------------------------------ */
/* DRM device init                                                       */
/* ------------------------------------------------------------------ */

static int drm_init(void) {
  const char *cards[] = { "/dev/dri/card1", "/dev/dri/card0", NULL };
  for (int i = 0; cards[i]; i++) {
    drm_fd = open(cards[i], O_RDWR | O_CLOEXEC);
    if (drm_fd >= 0) break;
  }
  if (drm_fd < 0) { perror("ffmpeg_drm: open DRM device"); return -1; }

  drmSetClientCap(drm_fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
  drmSetClientCap(drm_fd, DRM_CLIENT_CAP_ATOMIC, 1);

  drmModeRes *res = drmModeGetResources(drm_fd);
  if (!res) { perror("drmModeGetResources"); return -1; }

  drmModeConnector *conn = NULL;
  for (int i = 0; i < res->count_connectors && !conn; i++) {
    drmModeConnector *c = drmModeGetConnector(drm_fd, res->connectors[i]);
    if (c && c->connection == DRM_MODE_CONNECTED && c->count_modes > 0) conn = c;
    else if (c) drmModeFreeConnector(c);
  }
  if (!conn) { fprintf(stderr, "ffmpeg_drm: no connected display\n"); return -1; }

  drmModeEncoder *enc = drmModeGetEncoder(drm_fd, conn->encoder_id);
  if (!enc) { fprintf(stderr, "ffmpeg_drm: no encoder\n"); return -1; }
  drmModeFreeConnector(conn);

  uint32_t crtc_bit = 0;
  for (int i = 0; i < res->count_crtcs; i++) {
    if (res->crtcs[i] == enc->crtc_id) {
      drmModeCrtc *c = drmModeGetCrtc(drm_fd, res->crtcs[i]);
      if (c) { crtc_id = c->crtc_id; crtc_w = c->width; crtc_h = c->height; drmModeFreeCrtc(c); }
      crtc_bit = 1 << i; break;
    }
  }
  drmModeFreeEncoder(enc);
  drmModeFreeResources(res);
  if (!crtc_id) { fprintf(stderr, "ffmpeg_drm: no CRTC\n"); return -1; }

  drmModePlaneRes *pr = drmModeGetPlaneResources(drm_fd);
  if (!pr) { perror("drmModeGetPlaneResources"); return -1; }
  for (uint32_t i = 0; i < pr->count_planes && !plane_id; i++) {
    drmModePlane *p = drmModeGetPlane(drm_fd, pr->planes[i]);
    if (!p || !(p->possible_crtcs & crtc_bit) || p->crtc_id) { if (p) drmModeFreePlane(p); continue; }
    for (uint32_t f = 0; f < p->count_formats; f++) {
      if (p->formats[f] == DRM_FORMAT_NV12) {
        load_obj_props(pr->planes[i], DRM_MODE_OBJECT_PLANE, plane_props);
        plane_id = pr->planes[i]; break;
      }
    }
    drmModeFreePlane(p);
  }
  drmModeFreePlaneResources(pr);
  if (!plane_id) { fprintf(stderr, "ffmpeg_drm: no free NV12 overlay plane\n"); return -1; }

  atomic_req = drmModeAtomicAlloc();
  if (!atomic_req) return -1;

  /* Blank all other planes on our CRTC (fbcon, GUI, etc.) */
  drmModePlaneRes *pr2 = drmModeGetPlaneResources(drm_fd);
  if (pr2) {
    drmModeAtomicReqPtr blank = drmModeAtomicAlloc();
    bool dirty = false;
    for (uint32_t i = 0; i < pr2->count_planes; i++) {
      if (pr2->planes[i] == plane_id) continue;
      drmModePlane *p = drmModeGetPlane(drm_fd, pr2->planes[i]);
      if (!p || p->crtc_id != crtc_id) { if (p) drmModeFreePlane(p); continue; }
      drmModePropertyPtr pp[MAX_PROPS] = {0};
      load_obj_props(pr2->planes[i], DRM_MODE_OBJECT_PLANE, pp);
      uint32_t fb_p = prop_id_of(pp, "FB_ID"), ct_p = prop_id_of(pp, "CRTC_ID");
      if (fb_p)  drmModeAtomicAddProperty(blank, pr2->planes[i], fb_p, 0);
      if (ct_p)  drmModeAtomicAddProperty(blank, pr2->planes[i], ct_p, 0);
      if (fb_p || ct_p) dirty = true;
      for (int j = 0; j < MAX_PROPS && pp[j]; j++) drmModeFreeProperty(pp[j]);
      drmModeFreePlane(p);
    }
    if (dirty)
      drmModeAtomicCommit(drm_fd, blank, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    drmModeAtomicFree(blank);
    drmModeFreePlaneResources(pr2);
  }

  drmModeMoveCursor(drm_fd, crtc_id, 0, crtc_h);
  printf("ffmpeg_drm: crtc=%u plane=%u display=%dx%d\n",
         crtc_id, plane_id, crtc_w, crtc_h);
  return 0;
}

/* ------------------------------------------------------------------ */
/* Moonlight callbacks                                                   */
/* ------------------------------------------------------------------ */

static int ffmpeg_drm_setup(int videoFormat, int width, int height,
                             int redrawRate, void *context, int drFlags) {
  (void)redrawRate; (void)context; (void)drFlags;

  if (drm_init() < 0) return -1;

  if (av_hwdevice_ctx_create(&hw_ctx, AV_HWDEVICE_TYPE_DRM,
                              "/dev/dri/renderD128", NULL, 0) < 0) {
    fprintf(stderr, "ffmpeg_drm: hw device create failed\n"); return -1;
  }

  enum AVCodecID id = (videoFormat & VIDEO_FORMAT_MASK_H264) ? AV_CODEC_ID_H264 :
                      (videoFormat & VIDEO_FORMAT_MASK_H265) ? AV_CODEC_ID_HEVC :
                      AV_CODEC_ID_NONE;
  if (id == AV_CODEC_ID_NONE) return -1;

  codec = avcodec_find_decoder(id);
  if (!codec) { fprintf(stderr, "ffmpeg_drm: no decoder\n"); return -1; }

  codec_ctx = avcodec_alloc_context3(codec);
  if (!codec_ctx) return -1;
  codec_ctx->flags       |= AV_CODEC_FLAG_LOW_DELAY | AV_CODEC_FLAG_OUTPUT_CORRUPT;
  codec_ctx->flags2      |= AV_CODEC_FLAG2_SHOW_ALL;
  codec_ctx->thread_count = 1;
  codec_ctx->width        = width;
  codec_ctx->height       = height;
  codec_ctx->hw_device_ctx = av_buffer_ref(hw_ctx);
  codec_ctx->get_format    = get_hw_fmt;

  if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
    fprintf(stderr, "ffmpeg_drm: avcodec_open2 failed\n");
    avcodec_free_context(&codec_ctx); return -1;
  }

  pkt = av_packet_alloc();
  if (!pkt) return -1;

  disp_eos = false;
  pthread_create(&disp_thread, NULL, display_loop, NULL);

  printf("ffmpeg_drm: decoder=%s display=%dx%d\n", codec->name, crtc_w, crtc_h);
  return 0;
}

static void ffmpeg_drm_cleanup(void) {
  /* Signal display thread to exit */
  pthread_mutex_lock(&disp_mtx);
  disp_eos = true;
  if (disp_pending) { av_frame_free(&disp_pending); disp_pending = NULL; }
  pthread_cond_signal(&disp_cond);
  pthread_mutex_unlock(&disp_mtx);
  pthread_join(disp_thread, NULL);

  /* Disable our overlay plane */
  if (atomic_req && plane_id) {
    drmModeAtomicSetCursor(atomic_req, 0);
    drmModeAtomicAddProperty(atomic_req, plane_id, prop_id_of(plane_props,"FB_ID"),   0);
    drmModeAtomicAddProperty(atomic_req, plane_id, prop_id_of(plane_props,"CRTC_ID"), 0);
    drmModeAtomicCommit(drm_fd, atomic_req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    drmModeAtomicFree(atomic_req); atomic_req = NULL;
  }

  release_fb(cur_fb, cur_gem);
  cur_fb = cur_gem = 0;

  for (int i = 0; i < MAX_PROPS; i++) {
    if (plane_props[i]) { drmModeFreeProperty(plane_props[i]); plane_props[i] = NULL; }
  }

  if (pkt)       av_packet_free(&pkt);
  if (codec_ctx) avcodec_free_context(&codec_ctx);
  if (hw_ctx)    av_buffer_unref(&hw_ctx);
  if (nal_buf)   { free(nal_buf); nal_buf = NULL; nal_size = 0; }
  if (drm_fd >= 0) { close(drm_fd); drm_fd = -1; }
  plane_id = crtc_id = 0;
}

static int ffmpeg_drm_submit(PDECODE_UNIT du) {
  ensure_buf_size(&nal_buf, &nal_size,
                  du->fullLength + AV_INPUT_BUFFER_PADDING_SIZE);
  memset((uint8_t *)nal_buf + du->fullLength, 0, AV_INPUT_BUFFER_PADDING_SIZE);
  int len = 0;
  for (PLENTRY e = du->bufferList; e; e = e->next) {
    memcpy((uint8_t *)nal_buf + len, e->data, e->length);
    len += e->length;
  }
  pkt->data = (uint8_t *)nal_buf;
  pkt->size = len;

  if (avcodec_send_packet(codec_ctx, pkt) < 0)
    return DR_NEED_IDR;

  AVFrame *f = av_frame_alloc();
  if (!f) return DR_OK;

  int ret;
  while ((ret = avcodec_receive_frame(codec_ctx, f)) == 0) {
    if (f->format == AV_PIX_FMT_DRM_PRIME) {
      /* Clone for display thread — avcodec_receive_frame reuses f next call */
      AVFrame *clone = av_frame_clone(f);
      if (clone) {
        pthread_mutex_lock(&disp_mtx);
        if (disp_pending) av_frame_free(&disp_pending);  /* drop if display lagging */
        disp_pending = clone;
        pthread_cond_signal(&disp_cond);
        pthread_mutex_unlock(&disp_mtx);
      }
    }
    av_frame_unref(f);
  }
  av_frame_free(&f);

  return (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) ? DR_OK : DR_NEED_IDR;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_ffmpeg_drm = {
  .setup            = ffmpeg_drm_setup,
  .cleanup          = ffmpeg_drm_cleanup,
  .submitDecodeUnit = ffmpeg_drm_submit,
  .capabilities     = CAPABILITY_DIRECT_SUBMIT,
};
