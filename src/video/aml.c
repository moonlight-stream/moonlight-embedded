/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015-2017 Iwan Timmer
 * Copyright (C) 2016 OtherCrashOverride, Daniel Mehrwald
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

#include <Limelight.h>

#include <sys/utsname.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <codec.h>
#include <errno.h>
#include <pthread.h>

#include <linux/videodev2.h>

#include "../util.h"
#include "video.h"

#define SYNC_OUTSIDE 0x02
#define UCODE_IP_ONLY_PARAM 0x08
#define MAX_WRITE_ATTEMPTS 5
#define EAGAIN_SLEEP_TIME 2 * 1000

static codec_para_t codecParam = { 0 };
static pthread_t displayThread;
static int videoFd = -1;
static volatile bool done = false;
void *pkt_buf = NULL;
size_t pkt_buf_size = 0;

void* aml_display_thread(void* unused) {
  while (!done) {
    struct v4l2_buffer vbuf = { 0 };
    vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(videoFd, VIDIOC_DQBUF, &vbuf) < 0) {
      if (errno == EAGAIN) {
        usleep(500);
        continue;
      }
      fprintf(stderr, "VIDIOC_DQBUF failed: %d\n", errno);
      break;
    }

    if (ioctl(videoFd, VIDIOC_QBUF, &vbuf) < 0) {
      fprintf(stderr, "VIDIOC_QBUF failed: %d\n", errno);
      break;
    }
  }
  printf("Display thread terminated\n");
  return NULL;
}

int aml_setup(int videoFormat, int width, int height, int redrawRate, void* context, int drFlags) {

  codecParam.handle             = -1;
  codecParam.cntl_handle        = -1;
  codecParam.audio_utils_handle = -1;
  codecParam.sub_handle         = -1;
  codecParam.has_video          = 1;
  codecParam.noblock            = 0;
  codecParam.stream_type        = STREAM_TYPE_ES_VIDEO;
  codecParam.am_sysinfo.param   = 0;

#ifdef STREAM_TYPE_FRAME
  codecParam.dec_mode           = STREAM_TYPE_FRAME;
#endif

#ifdef FRAME_BASE_PATH_AMLVIDEO_AMVIDEO
  codecParam.video_path         = FRAME_BASE_PATH_AMLVIDEO_AMVIDEO;
#endif

  if (videoFormat & VIDEO_FORMAT_MASK_H264) {
    if (width > 1920 || height > 1080) {
      codecParam.video_type = VFORMAT_H264_4K2K;
      codecParam.am_sysinfo.format = VIDEO_DEC_FORMAT_H264_4K2K;
    } else {
      codecParam.video_type = VFORMAT_H264;
      codecParam.am_sysinfo.format = VIDEO_DEC_FORMAT_H264;

      // Workaround for decoding special case of C1, 1080p, H264
      int major, minor;
      struct utsname name;
      uname(&name);
      int ret = sscanf(name.release, "%d.%d", &major, &minor);
      if (ret == 2 && !(major > 3 || (major == 3 && minor >= 14)) && width == 1920 && height == 1080)
          codecParam.am_sysinfo.param = (void*) UCODE_IP_ONLY_PARAM;
    }
  } else if (videoFormat & VIDEO_FORMAT_MASK_H265) {
    codecParam.video_type = VFORMAT_HEVC;
    codecParam.am_sysinfo.format = VIDEO_DEC_FORMAT_HEVC;
#ifdef CODEC_TAG_AV1
  } else if (videoFormat & VIDEO_FORMAT_MASK_AV1) {
    codecParam.video_type = VFORMAT_AV1;
    codecParam.am_sysinfo.format = VIDEO_DEC_FORMAT_AV1;
#endif
  } else {
    printf("Video format not supported\n");
    return -1;
  }

  codecParam.am_sysinfo.width = width;
  codecParam.am_sysinfo.height = height;
  codecParam.am_sysinfo.rate = 96000 / redrawRate;
  codecParam.am_sysinfo.param = (void*) ((size_t) codecParam.am_sysinfo.param | SYNC_OUTSIDE);

  int ret;
  if ((ret = codec_init(&codecParam)) != 0) {
    fprintf(stderr, "codec_init error: %x\n", ret);
    return -2;
  }

  if ((ret = codec_set_freerun_mode(&codecParam, 1)) != 0) {
    fprintf(stderr, "Can't set Freerun mode: %x\n", ret);
    return -2;
  }

  char vfm_map[2048] = {};
  char* eol;
  if (read_file("/sys/class/vfm/map", vfm_map, sizeof(vfm_map) - 1) > 0 && (eol = strchr(vfm_map, '\n'))) {
    *eol = 0;

    // If amlvideo is in the pipeline, we must spawn a display thread
    printf("VFM map: %s\n", vfm_map);
    if (strstr(vfm_map, "amlvideo")) {
      printf("Using display thread for amlvideo pipeline\n");

      videoFd = open("/dev/video10", O_RDONLY | O_NONBLOCK);
      if (videoFd < 0) {
        fprintf(stderr, "Failed to open video device: %d\n", errno);
        return -3;
      }

      pthread_create(&displayThread, NULL, aml_display_thread, NULL);
    }
  }

  ensure_buf_size(&pkt_buf, &pkt_buf_size, INITIAL_DECODER_BUFFER_SIZE);

  return 0;
}

void aml_cleanup() {
  if (videoFd >= 0) {
    done = true;
    pthread_join(displayThread, NULL);
    close(videoFd);
  }

  codec_close(&codecParam);
  free(pkt_buf);
}

int aml_submit_decode_unit(PDECODE_UNIT decodeUnit) {

  ensure_buf_size(&pkt_buf, &pkt_buf_size, decodeUnit->fullLength);

  int written = 0, length = 0, errCounter = 0, api;
  PLENTRY entry = decodeUnit->bufferList;
  do {
    memcpy(pkt_buf+length, entry->data, entry->length);
    length += entry->length;
    entry = entry->next;
  } while (entry != NULL);

  codec_checkin_pts(&codecParam, decodeUnit->presentationTimeMs);
  while (length > 0) {
    api = codec_write(&codecParam, pkt_buf+written, length);
    if (api < 0) {
      if (errno != EAGAIN) {
        fprintf(stderr, "codec_write() error: %x %d\n", errno, api);
        codec_reset(&codecParam);
        break;
      } else {
        if (++errCounter == MAX_WRITE_ATTEMPTS) {
          fprintf(stderr, "codec_write() timeout\n");
          break;
        }
        usleep(EAGAIN_SLEEP_TIME);
      }
    } else {
      written += api;
      length -= api;
    }
  }

  return length ? DR_NEED_IDR : DR_OK;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_aml = {
  .setup = aml_setup,
  .cleanup = aml_cleanup,
  .submitDecodeUnit = aml_submit_decode_unit,

  // We may delay in aml_submit_decode_unit() for a while, so we can't set CAPABILITY_DIRECT_SUBMIT
  .capabilities = CAPABILITY_REFERENCE_FRAME_INVALIDATION_HEVC,
};
