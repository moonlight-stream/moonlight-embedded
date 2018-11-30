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
#include "sps.h"

#include <Limelight.h>

#include <stdbool.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/display.h>
#include <psp2/videodec.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdarg.h>

enum {
  VITA_VIDEO_INIT_OK                  = 0,
  VITA_VIDEO_ERROR_NO_MEM             = 0x80010001,
  VITA_VIDEO_ERROR_INIT_LIB           = 0x80010002,
  VITA_VIDEO_ERROR_QUERY_DEC_MEMSIZE  = 0x80010003,
  VITA_VIDEO_ERROR_ALLOC_MEM          = 0x80010004,
  VITA_VIDEO_ERROR_GET_MEMBASE        = 0x80010005,
  VITA_VIDEO_ERROR_CREATE_DEC         = 0x80010006,
};

#define DECODER_BUFFER_SIZE 92*1024

static char* ffmpeg_buffer = NULL;

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
};

int backbuffer;
void *framebuffer[2];
enum VideoStatus video_status = NOT_INIT;

SceAvcdecCtrl *decoder = NULL;
SceUID displayblock = -1;
SceUID decoderblock = -1;
SceVideodecQueryInitInfoHwAvcdec *init = NULL;
SceAvcdecQueryDecoderInfo *decoder_info = NULL;

static void vita_cleanup() {
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
    if (ffmpeg_buffer != NULL) {
      free(ffmpeg_buffer);
      ffmpeg_buffer = NULL;
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

    ret = sceDisplaySetFrameBuf(&framebuf, 1);
    printf("SetFrameBuf: 0x%x\n", ret);

    ffmpeg_buffer = malloc(DECODER_BUFFER_SIZE);
    if (ffmpeg_buffer == NULL) {
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

  return VITA_VIDEO_INIT_OK;

cleanup:
  vita_cleanup();
  return ret;
}

static unsigned numframes;
static bool active_video_thread = true;

static int vita_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  #if 0
  PLENTRY entry = decodeUnit->bufferList;
  while (entry != NULL) {
    fwrite(entry->data, entry->length, 1, fd);
    entry = entry->next;
  }
  #endif

  SceAvcdecAu au = {0};
  SceAvcdecArrayPicture array_picture = {0};
  struct SceAvcdecPicture picture = {0};
  struct SceAvcdecPicture *pictures = { &picture };
  array_picture.numOfElm = 1;
  array_picture.pPicture = &pictures;

  picture.size = sizeof(picture);
  picture.frame.pixelType = 0;
  picture.frame.framePitch = LINE_SIZE;
  picture.frame.frameWidth = SCREEN_WIDTH;
  picture.frame.frameHeight = SCREEN_HEIGHT;
  picture.frame.pPicture[0] = framebuffer[backbuffer];

  if (decodeUnit->fullLength < DECODER_BUFFER_SIZE) {
    gs_sps_fix(decodeUnit, 0);
    PLENTRY entry = decodeUnit->bufferList;
    int length = 0;
    while (entry != NULL) {
      memcpy(ffmpeg_buffer+length, entry->data, entry->length);
      length += entry->length;
      entry = entry->next;
    }

    au.es.pBuf = ffmpeg_buffer;
    au.es.size = decodeUnit->fullLength;
    au.dts.lower = 0xFFFFFFFF;
    au.dts.upper = 0xFFFFFFFF;
    au.pts.lower = 0xFFFFFFFF;
    au.pts.upper = 0xFFFFFFFF;

    int ret = 0;
    ret = sceAvcdecDecode(decoder, &au, &array_picture);
    if (ret < 0)
      printf("sceAvcdecDecode (len=0x%x): 0x%x numOfOutput %d\n", decodeUnit->fullLength, ret, array_picture.numOfOutput);
    if (ret < 0) {
      return DR_NEED_IDR;
    }

    if (array_picture.numOfOutput == 1) {
#if 0
      static uint64_t prev_frame;
      uint64_t cur_frame;
      sceRtcGetCurrentTick(&cur_frame);
      sceClibPrintf("got frame in %d us\n", (int)(cur_frame - prev_frame));
      prev_frame = cur_frame;
#endif

      if (active_video_thread) {
        SceDisplayFrameBuf framebuf = { 0 };
        framebuf.size = sizeof(framebuf);
        framebuf.base = framebuffer[backbuffer];
        framebuf.pitch = SCREEN_WIDTH;
        framebuf.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
        framebuf.width = SCREEN_WIDTH;
        framebuf.height = SCREEN_HEIGHT;

        int ret = sceDisplaySetFrameBuf(&framebuf, (config.disable_vsync) ? SCE_DISPLAY_SETBUF_IMMEDIATE : SCE_DISPLAY_SETBUF_NEXTFRAME);
        backbuffer = (backbuffer + 1) % 2;
        if (ret < 0)
          printf("Failed to sceDisplaySetFrameBuf: 0x%x\n", ret);
      }
    }
  } else {
    printf("Video decode buffer too small");
    exit(1);
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

void vitavideo_start() {
  active_video_thread = true;
}

void vitavideo_stop() {
  active_video_thread = false;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_vita = {
  .setup = vita_setup,
  .cleanup = vita_cleanup,
  .submitDecodeUnit = vita_submit_decode_unit,
  .capabilities = CAPABILITY_SLICES_PER_FRAME(2) | CAPABILITY_DIRECT_SUBMIT,
};
