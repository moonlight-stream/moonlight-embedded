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

#include "../loop.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <libv4l2.h>

#include <linux/ioctl.h>
#include <linux/mxc_v4l2.h>
#include <linux/mxcfb.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#define MIN_FRAME_BUFFER_COUNT 18;

#define THRESHOLD 2

static int displaying;

static int fd;
static int queued_count;
static int disp_clr_index = 0;

static int pipefd[2];
static int clearpipefd[2];

bool video_imx_init() {
  return vpu_init();
}

static int frame_handle(int pipefd) {
  int frame, prevframe = -1;
  while (read(pipefd, &frame, sizeof(int)) > 0) {
    if (prevframe >= 0)
      write(clearpipefd[1], &prevframe, sizeof(int));

    prevframe = frame;
  }

  struct timeval tv;
  gettimeofday(&tv, 0);

  struct v4l2_buffer dbuf = {};
  dbuf.timestamp.tv_sec = tv.tv_sec;
  dbuf.timestamp.tv_usec = tv.tv_usec;
  dbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  dbuf.memory = V4L2_MEMORY_MMAP;

  dbuf.index = frame;
  if (ioctl(fd, VIDIOC_QUERYBUF, &dbuf) < 0) {
    fprintf(stderr, "Can't get output buffer\n");
    exit(EXIT_FAILURE);
  }

  dbuf.index = frame;
  dbuf.field = V4L2_FIELD_NONE;
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

    write(clearpipefd[1], &dbuf.index, sizeof(int));
  }

  return LOOP_OK;
}

int decoder_renderer_setup(int videoFormat, int width, int height, int redrawRate, void* context, int drFlags) {
  if (videoFormat != VIDEO_FORMAT_H264) {
    fprintf(stderr, "Video format not supported\n");
    return -1;
  }

  struct mxcfb_gbl_alpha alpha;

  int fd_fb = open("/dev/fb0", O_RDWR, 0);

  if (fd_fb < 0){
    fprintf(stderr, "Can't access framebuffer\n");
    return -2;
  }

  alpha.alpha = 0;
  alpha.enable = 1;
  if (ioctl(fd_fb, MXCFB_SET_GBL_ALPHA, &alpha) < 0){
    fprintf(stderr, "Can't set framebuffer output\n");
    return -2;
  }

  close(fd_fb);
  
  int regfbcount = MIN_FRAME_BUFFER_COUNT + 2;
  int picWidth = ((width + 15) & ~15);
  int picHeight = ((height + 15) & ~15);
  
  char v4l_device[16];
  sprintf(v4l_device, "/dev/video%d", 17);
  fd = open(v4l_device, O_RDWR, 0);
  if (fd < 0){
    fprintf(stderr, "Can't access video output\n");
    return -2;
  }

  struct v4l2_rect icrop = {0};
  icrop.left = 0;
  icrop.top = 0;
  icrop.width = width;
  icrop.height = height;

  struct v4l2_format fmt = {0};
  fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  fmt.fmt.pix.width = picWidth;
  fmt.fmt.pix.height = picHeight;
  fmt.fmt.pix.bytesperline = picWidth;
  fmt.fmt.pix.priv = (unsigned long)&icrop;
  fmt.fmt.pix.field = V4L2_FIELD_ANY;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
  if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
    fprintf(stderr, "Can't set source video format\n");
    return -2;
  }

  if (ioctl(fd, VIDIOC_G_FMT, &fmt) < 0) {
    fprintf(stderr, "Can't set output video format\n");
    exit(EXIT_FAILURE);
  }

  struct v4l2_requestbuffers reqbuf = {0};
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  reqbuf.memory = V4L2_MEMORY_MMAP;
  reqbuf.count = regfbcount;

  struct vpu_buf* buffers[regfbcount];

  if (ioctl(fd, VIDIOC_REQBUFS, &reqbuf) < 0) {
    fprintf(stderr, "Can't get video buffers\n");
    return -2;
  }

  if (reqbuf.count < regfbcount) {
    fprintf(stderr, "Not enough video buffers\n");
    return -2;
  }
    
  for (int i = 0; i < regfbcount; i++) {
    struct v4l2_buffer buffer = {0};
    struct vpu_buf *buf;

    buf = calloc(1, sizeof(struct vpu_buf));
    if (buf == NULL) {
      fprintf(stderr, "Not enough memory\n");
      return -2;
    }

    buffers[i] = buf;

    buffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;

    if (ioctl(fd, VIDIOC_QUERYBUF, &buffer) < 0) {
      fprintf(stderr, "Can't get video buffer\n");
      return -2;
    }
    buf->start = mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buffer.m.offset);

   /*
    * Workaround for new V4L interface change, this change
    * will be removed after V4L driver is updated for this.
    * Need to call QUERYBUF ioctl again after mmap.
    */
    if (ioctl(fd, VIDIOC_QUERYBUF, &buffer) < 0) {
      fprintf(stderr, "Can't set source video format\n");
      return -2;
    }

    buf->offset = buffer.m.offset;
    buf->length = buffer.length;

    if (buf->start == MAP_FAILED) {
      fprintf(stderr, "Failed to map video buffer\n");
      return -2;
    }
  }
  
  vpu_setup(buffers, regfbcount, width, height);

  if (pipe(pipefd) == -1 || pipe(clearpipefd) == -1) {
    fprintf(stderr, "Can't create communication channel between threads\n");
    return -2;
  }

  loop_add_fd(pipefd[0], &frame_handle, POLLIN);

  fcntl(clearpipefd[0], F_SETFL, O_NONBLOCK);
  fcntl(pipefd[0], F_SETFL, O_NONBLOCK);

  return 0;
}

static int decoder_renderer_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  int frame;
  while (read(clearpipefd[0], &frame, sizeof(int)) > 0)
    vpu_clear(frame);

  if (vpu_decode(decodeUnit)) {
    frame = vpu_get_frame();
    write(pipefd[1], &frame, sizeof(int));
  }
}

static void decoder_renderer_cleanup() {
  vpu_cleanup();
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_imx = {
  .setup = decoder_renderer_setup,
  .cleanup = decoder_renderer_cleanup,
  .submitDecodeUnit = decoder_renderer_submit_decode_unit,
  .capabilities = CAPABILITY_DIRECT_SUBMIT | CAPABILITY_SLICES_PER_FRAME(2),
};
