/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015-2017 Iwan Timmer
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

#include "imx_vpu.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <vpu_io.h>
#include <vpu_lib.h>

#define STREAM_BUF_SIZE 0x200000
#define PS_SAVE_SIZE 0x080000

#define MODE420 1
#define MODE422 2
#define MODE224 3

#define WORST_SLICE_SIZE 3188

static vpu_mem_desc mem_desc = {0};
static vpu_mem_desc ps_mem_desc = {0};
static vpu_mem_desc slice_mem_desc = {0};

static DecHandle handle = {0};
static DecParam decparam = {0};
static DecBufInfo bufinfo = {0};

static int regfbcount, stride;

static bool initialized = false;

static FrameBuffer *fb;

static int currentFrame;

bool vpu_init() {
  return vpu_Init(NULL) == RETCODE_SUCCESS;
}

void vpu_setup(struct vpu_buf* buffers[], int bufferCount, int width, int height) {
  mem_desc.size = STREAM_BUF_SIZE;
  if (IOGetPhyMem(&mem_desc)){
    fprintf(stderr, "Can't get physical memory address\n");
    exit(EXIT_FAILURE);
  }

  if (IOGetVirtMem(&mem_desc) == -1) {
    fprintf(stderr, "Can't get virtual memory address\n");
    exit(EXIT_FAILURE);
  }

  ps_mem_desc.size = PS_SAVE_SIZE;
  if (IOGetPhyMem(&ps_mem_desc)) {
    fprintf(stderr, "Can't get physical memory address\n");
    exit(EXIT_FAILURE);
  }

  DecOpenParam oparam = {0};
  oparam.bitstreamFormat = STD_AVC;
  oparam.bitstreamBuffer = mem_desc.phy_addr;
  oparam.bitstreamBufferSize = STREAM_BUF_SIZE;
  oparam.pBitStream = (Uint8 *) mem_desc.virt_uaddr;
  oparam.reorderEnable = 1;
  oparam.mp4DeblkEnable = 0;
  oparam.chromaInterleave = 0;
  oparam.avcExtension = oparam.mp4Class = 0;
  oparam.mjpg_thumbNailDecEnable = 0;
  oparam.mapType = LINEAR_FRAME_MAP;
  oparam.tiled2LinearEnable = 0;
  oparam.bitstreamMode = 1;

  oparam.psSaveBuffer = ps_mem_desc.phy_addr;
  oparam.psSaveBufferSize = PS_SAVE_SIZE;

  if (vpu_DecOpen(&handle, &oparam) != RETCODE_SUCCESS) {
    fprintf(stderr, "Can't open video decoder\n");
    exit(EXIT_FAILURE);
  }

  decparam.dispReorderBuf = 0;
  decparam.skipframeMode = 0;
  decparam.skipframeNum = 0;
  decparam.iframeSearchEnable = 0;

  int phy_slicebuf_size = WORST_SLICE_SIZE * 1024;

  slice_mem_desc.size = phy_slicebuf_size;
  if (IOGetPhyMem(&slice_mem_desc)){
    fprintf(stderr, "Can't get slice physical address\n");
    exit(EXIT_FAILURE);
  }

  regfbcount = bufferCount;

  fb = calloc(regfbcount, sizeof(FrameBuffer));
  if (fb == NULL) {
    fprintf(stderr, "Can't allocate framebuffers\n");
    exit(EXIT_FAILURE);
  }

  stride = ((width + 15) & ~15);
  int picHeight = ((height + 15) & ~15);
  int img_size = stride * picHeight;
  vpu_mem_desc *mvcol_md = NULL;

  int mjpg_fmt = MODE420;
  int divX = (mjpg_fmt == MODE420 || mjpg_fmt == MODE422) ? 2 : 1;
  int divY = (mjpg_fmt == MODE420 || mjpg_fmt == MODE224) ? 2 : 1;

  mvcol_md = calloc(regfbcount, sizeof(vpu_mem_desc));

  for (int i = 0; i < regfbcount; i++) {
    fb[i].myIndex = i;
    fb[i].bufY = buffers[i]->offset;
    fb[i].bufCb = fb[i].bufY + img_size;
    fb[i].bufCr = fb[i].bufCb + (img_size / divX / divY);

    /* allocate MvCol buffer here */
    memset(&mvcol_md[i], 0, sizeof(vpu_mem_desc));
    mvcol_md[i].size = img_size / divX / divY;
    if (IOGetPhyMem(&mvcol_md[i])) {
      fprintf(stderr, "Can't get physical address of colomn buffer\n");
      exit(EXIT_FAILURE);
    }
    fb[i].bufMvCol = mvcol_md[i].phy_addr;
  }

  bufinfo.avcSliceBufInfo.bufferBase = slice_mem_desc.phy_addr;
  bufinfo.avcSliceBufInfo.bufferSize = phy_slicebuf_size;

  bufinfo.maxDecFrmInfo.maxMbX = stride / 16;
  bufinfo.maxDecFrmInfo.maxMbY = picHeight / 16;
  bufinfo.maxDecFrmInfo.maxMbNum = stride * picHeight / 256;

  int delay = -1;
  vpu_DecGiveCommand(handle, DEC_SET_FRAME_DELAY, &delay);
}

int vpu_get_frame() {
  return currentFrame;
}

bool vpu_decode(PDECODE_UNIT decodeUnit) {
  Uint32 space;
  PhysicalAddress pa_read_ptr, pa_write_ptr;
  if (vpu_DecGetBitstreamBuffer(handle, &pa_read_ptr, &pa_write_ptr, &space) != RETCODE_SUCCESS) {
    fprintf(stderr, "Can't get video decoder buffer\n");
    exit(EXIT_FAILURE);
  }
  Uint32 target_addr = mem_desc.virt_uaddr + (pa_write_ptr - mem_desc.phy_addr);

  if (space < decodeUnit->fullLength) {
    fprintf(stderr, "Not enough space in buffer %d/%d\n", decodeUnit->fullLength, space);
  }

  PLENTRY entry = decodeUnit->bufferList;
  int written = 0;
  while (entry != NULL) {
    if ( (target_addr + entry->length) > mem_desc.virt_uaddr + STREAM_BUF_SIZE) {
      int room = mem_desc.virt_uaddr + STREAM_BUF_SIZE - target_addr;
      memcpy((void *)target_addr, entry->data, room);
      memcpy((void *)mem_desc.virt_uaddr, entry->data + room, entry->length - room);
      target_addr = mem_desc.virt_uaddr + entry->length - room;
    } else {
      memcpy((void *)target_addr, entry->data, entry->length);
      target_addr += entry->length;
    }

    entry = entry->next;
  }
  vpu_DecUpdateBitstreamBuffer(handle, decodeUnit->fullLength);

  if (!initialized) {
    initialized = true;
    DecInitialInfo info = {0};
    vpu_DecSetEscSeqInit(handle, 1);
    vpu_DecGetInitialInfo(handle, &info);
    vpu_DecSetEscSeqInit(handle, 0);
    if (vpu_DecRegisterFrameBuffer(handle, fb, regfbcount, stride, &bufinfo) != RETCODE_SUCCESS) {
      fprintf(stderr, "Can't register decoder to framebuffer\n");
      exit(EXIT_FAILURE);
    }
  }

  if (vpu_DecStartOneFrame(handle, &decparam) != RETCODE_SUCCESS) {
    fprintf(stderr, "Can't start decoding\n");
    exit(EXIT_FAILURE);
  }

  int loop_id = 0;
  while (vpu_IsBusy()) {
    if (loop_id > 50) {
      vpu_SWReset(handle, 0);
      fprintf(stderr, "VPU is too long busy\n");
      exit(EXIT_FAILURE);
    }
    vpu_WaitForInt(100);
    loop_id++;
  }

  DecOutputInfo outinfo = {0};
  if (vpu_DecGetOutputInfo(handle, &outinfo) != RETCODE_SUCCESS) {
    fprintf(stderr, "Can't get output info\n");
    exit(EXIT_FAILURE);
  }

  if (outinfo.decodingSuccess & 0x10) {
    return false;
  } else if (outinfo.notSufficientPsBuffer) {
    fprintf(stderr, "Not enough space in stream buffer\n");
    exit(EXIT_FAILURE);
  } else if (outinfo.notSufficientSliceBuffer) {
    fprintf(stderr, "Not enough space in slice buffer\n");
    exit(EXIT_FAILURE);
  }

  if (outinfo.indexFrameDisplay < 0) {
    fprintf(stderr, "Failed to decode frame\n");
    exit(EXIT_FAILURE);
  }

  currentFrame = outinfo.indexFrameDisplay;
  return true;
}

void vpu_clear(int disp_clr_index) {
  vpu_DecClrDispFlag(handle, disp_clr_index);
}

void vpu_cleanup() {
  IOFreePhyMem(&ps_mem_desc);
  IOFreePhyMem(&slice_mem_desc);

  IOFreeVirtMem(&mem_desc);
  IOFreePhyMem(&mem_desc);
  vpu_UnInit();
}
