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

#include "limelight-common/Limelight.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <libv4l2.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/ioctl.h>
#include <linux/mxc_v4l2.h>
#include <linux/mxcfb.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#include <vpu_io.h>
#include <vpu_lib.h>

#define STREAM_BUF_SIZE 0x200000
#define PS_SAVE_SIZE 0x080000

#define MODE420 1
#define MODE422 2
#define MODE224 3

#define THRESHOLD 2
#define MIN_FRAME_BUFFER_COUNT 18;
#define WORST_SLICE_SIZE 3188

struct v4l_buf {
  void *start;
  off_t offset;
  size_t length;
};

static vpu_mem_desc mem_desc = {0};
static vpu_mem_desc ps_mem_desc = {0};
static vpu_mem_desc slice_mem_desc = {0};

static DecHandle handle = {0};
static DecParam decparam = {0};
static DecBufInfo bufinfo = {0};
static int fd;

static int regfbcount, stride;

static bool initialized = false, decoding = false, displaying = false;

static int queued_count;
static int disp_clr_index = 0;

static FrameBuffer *fb;
static struct v4l2_buffer dbuf;

bool video_imx_init() {
  return vpu_Init(NULL) == RETCODE_SUCCESS;
}

static void decoder_renderer_setup(int videoFormat, int width, int height, int redrawRate, void* context, int drFlags) {
  struct mxcfb_gbl_alpha alpha;

  dbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  dbuf.memory = V4L2_MEMORY_MMAP;
  
  int fd_fb = open("/dev/fb0", O_RDWR, 0);

  if (fd_fb < 0){
    fprintf(stderr, "Can't access framebuffer\n");
    exit(EXIT_FAILURE);
  }

  alpha.alpha = 0;
  alpha.enable = 1;
  if (ioctl(fd_fb, MXCFB_SET_GBL_ALPHA, &alpha) < 0){
    fprintf(stderr, "Can't set framebuffer output\n");
    exit(EXIT_FAILURE);
  }

  close(fd_fb);
  
  mem_desc.size = STREAM_BUF_SIZE;
  if (IOGetPhyMem(&mem_desc)){
    fprintf(stderr, "Can't get physical memory address\n");
    exit(EXIT_FAILURE);
  }

  if (IOGetVirtMem(&mem_desc) <= 0) {
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

  regfbcount = MIN_FRAME_BUFFER_COUNT + 2;
  int picWidth = ((width + 15) & ~15);
  int picHeight = ((height + 15) & ~15);
  stride = picWidth;

  int phy_slicebuf_size = WORST_SLICE_SIZE * 1024;

  slice_mem_desc.size = phy_slicebuf_size;
  if (IOGetPhyMem(&slice_mem_desc)){
    fprintf(stderr, "Can't get slice physical address\n");
    exit(EXIT_FAILURE);
  }

  fb = calloc(regfbcount, sizeof(FrameBuffer));
  if (fb == NULL) {
    fprintf(stderr, "Can't allocate framebuffers\n");
    exit(EXIT_FAILURE);
  }

  char v4l_device[16], node[8];
  sprintf(node, "%d", 17);
  strcpy(v4l_device, "/dev/video");
  strcat(v4l_device, node);
  fd = open(v4l_device, O_RDWR, 0);
  if (fd < 0){
    fprintf(stderr, "Can't access video output\n");
    exit(EXIT_FAILURE);
  }

  struct v4l2_format fmt = {0};
  fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  fmt.fmt.pix.width = picWidth;
  fmt.fmt.pix.height = picHeight;
  fmt.fmt.pix.bytesperline = picWidth;
  fmt.fmt.pix.field = V4L2_FIELD_ANY;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
  if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
    fprintf(stderr, "Can't set source video format\n");
    exit(EXIT_FAILURE);
  }

  if (ioctl(fd, VIDIOC_G_FMT, &fmt) < 0) {
    fprintf(stderr, "Can't set output video format\n");
    exit(EXIT_FAILURE);
  }

  struct v4l2_requestbuffers reqbuf = {0};
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  reqbuf.memory = V4L2_MEMORY_MMAP;
  reqbuf.count = regfbcount;

  struct v4l_buf* buffers[regfbcount];

  if (ioctl(fd, VIDIOC_REQBUFS, &reqbuf) < 0) {
    fprintf(stderr, "Can't get video buffers\n");
    exit(EXIT_FAILURE);
  }

  if (reqbuf.count < regfbcount) {
    fprintf(stderr, "Not enough video buffers\n");
    exit(EXIT_FAILURE);
  }
    
  for (int i = 0; i < regfbcount; i++) {
    struct v4l2_buffer buffer = {0};
    struct v4l_buf *buf;

    buf = calloc(1, sizeof(struct v4l_buf));
    if (buf == NULL) {
      fprintf(stderr, "Not enough memory\n");
      exit(EXIT_FAILURE);
    }

    buffers[i] = buf;

    buffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;

    if (ioctl(fd, VIDIOC_QUERYBUF, &buffer) < 0) {
      fprintf(stderr, "Can't get video buffer\n");
      exit(EXIT_FAILURE);
    }
    buf->start = mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buffer.m.offset);

   /*
    * Workaround for new V4L interface change, this change
    * will be removed after V4L driver is updated for this.
    * Need to call QUERYBUF ioctl again after mmap.
    */
    if (ioctl(fd, VIDIOC_QUERYBUF, &buffer) < 0) {
      fprintf(stderr, "Can't set source video format\n");
      exit(EXIT_FAILURE);
    }

    buf->offset = buffer.m.offset;
    buf->length = buffer.length;

    if (buf->start == MAP_FAILED) {
      fprintf(stderr, "Failed to map video buffer\n");
      exit(EXIT_FAILURE);
    }
  }

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

static int decoder_renderer_submit_decode_unit(PDECODE_UNIT decodeUnit) {
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

  if (!decoding) {
    if (vpu_DecStartOneFrame(handle, &decparam) != RETCODE_SUCCESS) {
      fprintf(stderr, "Can't start decoding\n");
      exit(EXIT_FAILURE);
    }
    decoding = true;
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

  if (decoding) {
    decoding = 0;

    DecOutputInfo outinfo = {0};
    if (vpu_DecGetOutputInfo(handle, &outinfo) != RETCODE_SUCCESS) {
      fprintf(stderr, "Can't get output info\n");
      exit(EXIT_FAILURE);
    }

    if (outinfo.decodingSuccess & 0x10) {
      return DR_OK;
    } else if (outinfo.notSufficientPsBuffer) {
      fprintf(stderr, "Not enough space in stream buffer\n");
      exit(EXIT_FAILURE);
    } else if (outinfo.notSufficientSliceBuffer) {
      fprintf(stderr, "Not enough space in slice buffer\n");
      exit(EXIT_FAILURE);
    }

    if (outinfo.indexFrameDisplay >= 0) {
      struct timeval tv;
      gettimeofday(&tv, 0);
      dbuf.timestamp.tv_sec = tv.tv_sec;
      dbuf.timestamp.tv_usec = tv.tv_usec;
      dbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
      dbuf.memory = V4L2_MEMORY_MMAP;

      dbuf.index = outinfo.indexFrameDisplay;
      if (ioctl(fd, VIDIOC_QUERYBUF, &dbuf) < 0) {
        fprintf(stderr, "Can't get output buffer\n");
        exit(EXIT_FAILURE);
      }

      dbuf.index = outinfo.indexFrameDisplay;
      dbuf.field =  V4L2_FIELD_NONE;
      if (ioctl(fd, VIDIOC_QBUF, &dbuf) < 0) {
        fprintf(stderr, "Can't get output buffer\n");
        exit(EXIT_FAILURE);
      }

      if (!displaying) {
        int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
          fprintf(stderr, "Failed to output video\n");
          exit(EXIT_FAILURE);
        }
        displaying = true;
      }

      queued_count++;

      if (queued_count > THRESHOLD) {
        dbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        dbuf.memory = V4L2_MEMORY_MMAP;
        if (ioctl(fd, VIDIOC_DQBUF, &dbuf) < 0) {
          fprintf(stderr, "Failed to dequeue buffer\n");
          exit(EXIT_FAILURE);
        } else
          queued_count--;
      }

      if (disp_clr_index >= 0)
        vpu_DecClrDispFlag(handle, disp_clr_index);
      
      disp_clr_index = outinfo.indexFrameDisplay;
    } else if (outinfo.indexFrameDisplay == -1) {
      fprintf(stderr, "Failed to decode frame\n");
      exit(EXIT_FAILURE);
    }
  }

  return DR_OK;
}

static void decoder_renderer_cleanup() {
  IOFreePhyMem(&ps_mem_desc);
  IOFreePhyMem(&slice_mem_desc);
  
  IOFreeVirtMem(&mem_desc);
  IOFreePhyMem(&mem_desc);
  vpu_UnInit();
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_imx = {
  .setup = decoder_renderer_setup,
  .cleanup = decoder_renderer_cleanup,
  .submitDecodeUnit = decoder_renderer_submit_decode_unit,
  .capabilities = CAPABILITY_DIRECT_SUBMIT | CAPABILITY_SLICES_PER_FRAME(2),
};
