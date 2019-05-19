/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2016 Ilya Zhuravlev
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

#include "../video.h"
#include "../config.h"
#include "../debug.h"
#include "sps.h"

#include <Limelight.h>

#include <stdbool.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/display.h>
#include <psp2/videodec.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdarg.h>

#if 0
#define printf vita_debug_log
#define DRAW_FPS 1
#endif

void display_message(int gX, int gY, char *format, ...);
void draw_indicators();

enum {
  VITA_VIDEO_INIT_OK                    = 0,
  VITA_VIDEO_ERROR_NO_MEM               = 0x80010001,
  VITA_VIDEO_ERROR_INIT_LIB             = 0x80010002,
  VITA_VIDEO_ERROR_QUERY_DEC_MEMSIZE    = 0x80010003,
  VITA_VIDEO_ERROR_ALLOC_MEM            = 0x80010004,
  VITA_VIDEO_ERROR_GET_MEMBASE          = 0x80010005,
  VITA_VIDEO_ERROR_CREATE_DEC           = 0x80010006,
  VITA_VIDEO_ERROR_CREATE_PACER_THREAD  = 0x80010007,
};

#define DECODER_BUFFER_SIZE 92*1024

static char* decoder_buffer = NULL;

enum {
  SCREEN_WIDTH = 960,
  SCREEN_HEIGHT = 544,
  LINE_SIZE = 960,
  FRAMEBUFFER_SIZE = 2 * 1024 * 1024,
  FRAMEBUFFER_ALIGNMENT = 256 * 1024
};

enum VideoStatus {
  NOT_INIT,
  INIT_GS,
  INIT_DISPLAY_MEMBLOCK,
  INIT_FRAMEBUFFER,
  INIT_AVC_LIB,
  INIT_DECODER_MEMBLOCK,
  INIT_AVC_DEC,
  INIT_FRAME_PACER_THREAD,
};

int backbuffer;
void *framebuffer[2];
enum VideoStatus video_status = NOT_INIT;

SceAvcdecCtrl *decoder = NULL;
SceUID displayblock = -1;
SceUID decoderblock = -1;
SceUID pacer_thread = -1;
SceVideodecQueryInitInfoHwAvcdec *init = NULL;
SceAvcdecQueryDecoderInfo *decoder_info = NULL;

static unsigned numframes;
static bool active_video_thread = true;
static bool active_pacer_thread = false;
static bool active_poor_net_indicator = false;

typedef struct frame_elem frame_elem;
struct frame_elem {
  unsigned long long time;
  uint8_t framebuffer[FRAMEBUFFER_SIZE];
  frame_elem *next;
};

uint32_t frame_count = 0;
uint32_t need_drop = 0;
uint32_t curr_fps[2] = {0, 0};
float carry = 0;

static int vita_pacer_thread_main(SceSize args, void *argp) {
  // 1s
  int wait = 1000000;
  //float max_fps = 0;
  //sceDisplayGetRefreshRate(&max_fps);
  //if (config.stream.fps == 30) {
  //  max_fps /= 2;
  //}
  int max_fps = config.stream.fps;
  uint64_t last_vblank_count = sceDisplayGetVcount();
  uint64_t last_check_time = sceKernelGetSystemTimeWide();
  //float carry = 0;
  need_drop = 0;
  frame_count = 0;
  while (active_pacer_thread) {
    uint64_t curr_vblank_count = sceDisplayGetVcount();
    uint32_t vblank_fps = curr_vblank_count - last_vblank_count;
    uint32_t curr_frame_count = frame_count;
    frame_count = 0;

    if (!active_video_thread) {
    //  carry = 0;
    } else {
      if (config.enable_frame_pacer && curr_frame_count > max_fps) {
        //carry += curr_frame_count - max_fps;
        //if (carry > 1) {
        //  need_drop += (int)carry;
        //  carry -= (int)carry;
        //}
        need_drop += curr_frame_count - max_fps;
      }
      //vita_debug_log("fps0/fps1/carry/need_drop: %u/%u/%f/%u\n",
      //               curr_frame_count, vblank_fps, carry, need_drop);
    }

    curr_fps[0] = curr_frame_count;
    curr_fps[1] = vblank_fps;

    last_vblank_count = curr_vblank_count;
    uint64_t curr_check_time = sceKernelGetSystemTimeWide();
    uint32_t lapse = curr_check_time - last_check_time;
    last_check_time = curr_check_time;
    if (lapse > wait && (lapse - wait) < wait) {
      //vita_debug_log("sleep: %d", wait * 2 - lapse);
      sceKernelDelayThread(wait * 2 - lapse);
    } else {
      sceKernelDelayThread(wait);
    }
  }
  return 0;
}

static void vita_cleanup() {
  int ret;
  if (video_status == INIT_FRAME_PACER_THREAD) {
    active_pacer_thread = false;
    // wait 10sec
    SceUInt timeout = 10000000;
    sceKernelWaitThreadEnd(pacer_thread, &ret, &timeout);
    sceKernelDeleteThread(pacer_thread);
    video_status--;
  }
  if (video_status == INIT_AVC_DEC) {
    sceAvcdecDeleteDecoder(decoder);
    video_status--;
  }
  if (video_status == INIT_DECODER_MEMBLOCK) {
    if (decoderblock >= 0) {
      sceKernelFreeMemBlock(decoderblock);
      decoderblock = -1;
    }
    if (decoder != NULL) {
      free(decoder);
      decoder = NULL;
    }
    if (decoder_info != NULL) {
      free(decoder_info);
      decoder_info = NULL;
    }
    video_status--;
  }
  if (video_status == INIT_AVC_LIB) {
    sceVideodecTermLibrary(0x1001);

    if (init != NULL) {
      free(init);
      init = NULL;
    }
    video_status--;
  }

  if (video_status == INIT_FRAMEBUFFER) {
    if (decoder_buffer != NULL) {
      free(decoder_buffer);
      decoder_buffer = NULL;
    }
    video_status--;
  }

  if (video_status == INIT_DISPLAY_MEMBLOCK) {
    if (displayblock >= 0) {
      sceKernelFreeMemBlock(displayblock);
      displayblock = -1;
    }
    video_status--;
  }

  if (video_status == INIT_GS) {
    gs_sps_stop();
    video_status--;
  }
}

static int vita_setup(int videoFormat, int width, int height, int redrawRate, void* context, int drFlags) {
  int ret;
  printf("vita video setup\n");

  if (video_status == NOT_INIT) {
    // INIT_GS
    gs_sps_init(width, height);
    video_status++;
  }

  if (video_status == INIT_GS) {
    // INIT_DISPLAY_MEMBLOCK
    SceKernelAllocMemBlockOpt opt = { 0 };
    opt.size = sizeof(opt);
    opt.attr = 0x00000004;
    opt.alignment = FRAMEBUFFER_ALIGNMENT;
    displayblock = sceKernelAllocMemBlock(
      "display",
      SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, FRAMEBUFFER_SIZE * 2,
      &opt
    );
    if (displayblock < 0) {
      printf("not enough memory\n");
      ret = VITA_VIDEO_ERROR_NO_MEM;
      goto cleanup;
    }
    printf("displayblock: 0x%08x\n", displayblock);
    video_status++;
  }

  if (video_status == INIT_DISPLAY_MEMBLOCK) {
    // INIT_FRAMEBUFFER
    void *base;
    ret = sceKernelGetMemBlockBase(displayblock, &base);
    if (ret < 0) {
      printf("sceKernelGetMemBlockBase: 0x%x\n", ret);
      ret = VITA_VIDEO_ERROR_GET_MEMBASE;
      goto cleanup;
    }
    printf("base: 0x%08x\n", base);
    framebuffer[0] = base;
    framebuffer[1] = (char*)base + FRAMEBUFFER_SIZE;
    backbuffer = 1;

    SceDisplayFrameBuf framebuf = { 0 };
    framebuf.size = sizeof(framebuf);
    framebuf.base = base;
    framebuf.pitch = SCREEN_WIDTH;
    framebuf.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
    framebuf.width = SCREEN_WIDTH;
    framebuf.height = SCREEN_HEIGHT;

    ret = sceDisplaySetFrameBuf(&framebuf, SCE_DISPLAY_SETBUF_NEXTFRAME);
    printf("SetFrameBuf: 0x%x\n", ret);

    decoder_buffer = malloc(DECODER_BUFFER_SIZE);
    if (decoder_buffer == NULL) {
      printf("not enough memory\n");
      ret = VITA_VIDEO_ERROR_NO_MEM;
      goto cleanup;
    }
    video_status++;
  }

  if (video_status == INIT_FRAMEBUFFER) {
    // INIT_AVC_LIB
    if (init == NULL) {
      init = calloc(1, sizeof(SceVideodecQueryInitInfoHwAvcdec));
      if (init == NULL) {
        printf("not enough memory\n");
        ret = VITA_VIDEO_ERROR_NO_MEM;
        goto cleanup;
      }
    }
    init->size = sizeof(SceVideodecQueryInitInfoHwAvcdec);
    init->horizontal = width;
    init->vertical = height;
    init->numOfRefFrames = 5;
    init->numOfStreams = 1;

    ret = sceVideodecInitLibrary(0x1001, init);
    if (ret < 0) {
      printf("sceVideodecInitLibrary 0x%x\n", ret);
      ret = VITA_VIDEO_ERROR_INIT_LIB;
      goto cleanup;
    }
    video_status++;
  }

  if (video_status == INIT_AVC_LIB) {
    // INIT_DECODER_MEMBLOCK
    if (decoder_info == NULL) {
      decoder_info = calloc(1, sizeof(SceAvcdecQueryDecoderInfo));
      if (decoder_info == NULL) {
        printf("not enough memory\n");
        ret = VITA_VIDEO_ERROR_NO_MEM;
        goto cleanup;
      }
    }
    decoder_info->horizontal = init->horizontal;
    decoder_info->vertical = init->vertical;
    decoder_info->numOfRefFrames = init->numOfRefFrames;

    SceAvcdecDecoderInfo decoder_info_out = {0};

    ret = sceAvcdecQueryDecoderMemSize(0x1001, decoder_info, &decoder_info_out);
    if (ret < 0) {
      printf("sceAvcdecQueryDecoderMemSize 0x%x size 0x%x\n", ret, decoder_info_out.frameMemSize);
      ret = VITA_VIDEO_ERROR_QUERY_DEC_MEMSIZE;
      goto cleanup;
    }

    decoder = calloc(1, sizeof(SceAvcdecCtrl));
    if (decoder == NULL) {
      printf("not enough memory\n");
      ret = VITA_VIDEO_ERROR_ALLOC_MEM;
      goto cleanup;
    }

    size_t sz = (decoder_info_out.frameMemSize + 0xFFFFF) & ~0xFFFFF;
    decoder->frameBuf.size = sz;
    printf("allocating size 0x%x\n", sz);

    decoderblock = sceKernelAllocMemBlock("decoder", SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW, sz, NULL);
    if (decoderblock < 0) {
      printf("decoderblock: 0x%08x\n", decoderblock);
      ret = VITA_VIDEO_ERROR_ALLOC_MEM;
      goto cleanup;
    }

    ret = sceKernelGetMemBlockBase(decoderblock, &decoder->frameBuf.pBuf);
    if (ret < 0) {
      printf("sceKernelGetMemBlockBase: 0x%x\n", ret);
      ret = VITA_VIDEO_ERROR_GET_MEMBASE;
      goto cleanup;
    }
    video_status++;
  }

  if (video_status == INIT_DECODER_MEMBLOCK) {
    // INIT_AVC_DEC
    printf("base: 0x%08x\n", decoder->frameBuf.pBuf);

    ret = sceAvcdecCreateDecoder(0x1001, decoder, decoder_info);
    if (ret < 0) {
      printf("sceAvcdecCreateDecoder 0x%x\n", ret);
      ret = VITA_VIDEO_ERROR_CREATE_DEC;
      goto cleanup;
    }
    video_status++;
  }

  if (video_status == INIT_AVC_DEC) {
    // INIT_FRAME_PACER_THREAD
    ret = sceKernelCreateThread("frame_pacer", vita_pacer_thread_main, 0, 0x10000, 0, 0, NULL);
    if (ret < 0) {
      printf("sceKernelCreateThread 0x%x\n", ret);
      ret = VITA_VIDEO_ERROR_CREATE_PACER_THREAD;
      goto cleanup;
    }
    pacer_thread = ret;
    active_pacer_thread = true;
    sceKernelStartThread(pacer_thread, 0, NULL);
    video_status++;
  }

  return VITA_VIDEO_INIT_OK;

cleanup:
  vita_cleanup();
  return ret;
}

static int vita_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  SceAvcdecAu au = {0};
  SceAvcdecArrayPicture array_picture = {0};
  struct SceAvcdecPicture picture = {0};
  struct SceAvcdecPicture *pictures = { &picture };
  array_picture.numOfElm = 1;
  array_picture.pPicture = &pictures;

  //frame->time = decodeUnit->receiveTimeMs;

  picture.size = sizeof(picture);
  picture.frame.pixelType = 0;
  picture.frame.framePitch = LINE_SIZE;
  picture.frame.frameWidth = SCREEN_WIDTH;
  picture.frame.frameHeight = SCREEN_HEIGHT;
  picture.frame.pPicture[0] = framebuffer[backbuffer];

  if (decodeUnit->fullLength >= DECODER_BUFFER_SIZE) {
    printf("Video decode buffer too small\n");
    exit(1);
  }

  PLENTRY entry = decodeUnit->bufferList;
  uint32_t length = 0;
  while (entry != NULL) {
    if (entry->bufferType == BUFFER_TYPE_SPS) {
      gs_sps_fix(entry, GS_SPS_BITSTREAM_FIXUP, decoder_buffer, &length);
    } else {
      memcpy(decoder_buffer+length, entry->data, entry->length);
      length += entry->length;
    }
    entry = entry->next;
  }

  au.es.pBuf = decoder_buffer;
  au.es.size = decodeUnit->fullLength;
  au.dts.lower = 0xFFFFFFFF;
  au.dts.upper = 0xFFFFFFFF;
  au.pts.lower = 0xFFFFFFFF;
  au.pts.upper = 0xFFFFFFFF;

  int ret = 0;
  ret = sceAvcdecDecode(decoder, &au, &array_picture);
  if (ret < 0) {
    printf("sceAvcdecDecode (len=0x%x): 0x%x numOfOutput %d\n", decodeUnit->fullLength, ret, array_picture.numOfOutput);
    return DR_NEED_IDR;
  }

  if (array_picture.numOfOutput != 1) {
    //printf("numOfOutput %d\n", array_picture.numOfOutput);
    return DR_OK;
  }

  if (active_video_thread) {
    if (need_drop > 0) {
      vita_debug_log("remain frameskip: %d\n", need_drop);
      // skip
      need_drop--;
    } else {
      SceDisplayFrameBuf framebuf = { 0 };
      framebuf.size = sizeof(framebuf);

      framebuf.base = framebuffer[backbuffer];
      framebuf.pitch = SCREEN_WIDTH;
      framebuf.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
      framebuf.width = SCREEN_WIDTH;
      framebuf.height = SCREEN_HEIGHT;

      int ret = sceDisplaySetFrameBuf(&framebuf, SCE_DISPLAY_SETBUF_NEXTFRAME);
#ifdef DRAW_FPS
      display_message(20, 40, "fps: %u / %u", curr_fps[0], curr_fps[1]);
#endif
      draw_indicators();
      backbuffer = (backbuffer + 1) % 2;
      if (ret < 0) {
        printf("Failed to sceDisplaySetFrameBuf: 0x%x\n", ret);
      } else {
        frame_count++;
      }
    }
  }

  // if (numframes++ % 6 == 0)
  //   return DR_NEED_IDR;

  return DR_OK;
}

extern unsigned char msx[];
void display_message(int gX, int gY, char *format, ...) {
  char text[0x1000];

  va_list opt;
  va_start(opt, format);
  vsnprintf(text, sizeof(text), format, opt);
  va_end(opt);

  int c, i, j, l;
  unsigned char *font;
  unsigned int *vram_ptr;
  unsigned int *vram;

  int fontSize = 8;
  float zoom = (float) fontSize / 8;
  for (c = 0; c < strlen(text); c++) {
    char ch = text[c];
    vram = framebuffer[backbuffer] + (gX + gY * 960) * 4;

    for (i = l = 0; i < fontSize; i++, l += fontSize) {
      font = &msx[ (int)ch * 8] + (int) (i / zoom);
      vram_ptr  = vram;
      for (j = 0; j < fontSize; j++) {
        if ((*font & (128 >> (int) (j/zoom)))) *vram_ptr = 0xffffffff;
        vram_ptr++;
      }
      vram += 960;
    }

    gX += fontSize;
  }
}

void draw_indicators() {
//  if (active_poor_net_indicator) {
    // TODO draw image instead text
    display_message(60, 40, "poor network");
//  }
}

void vitavideo_start() {
  active_video_thread = true;
}

void vitavideo_stop() {
  active_video_thread = false;
}

void vitavideo_show_poor_net_indicator() {
  active_poor_net_indicator = true;
}

void vitavideo_hide_poor_net_indicator() {
  active_poor_net_indicator = false;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_vita = {
  .setup = vita_setup,
  .cleanup = vita_cleanup,
  .submitDecodeUnit = vita_submit_decode_unit,
  .capabilities = CAPABILITY_SLICES_PER_FRAME(2) | CAPABILITY_DIRECT_SUBMIT,
};
