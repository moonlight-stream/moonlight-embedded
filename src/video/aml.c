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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <codec.h>
#include <errno.h>

#define SYNC_OUTSIDE 0x02
#define UCODE_IP_ONLY_PARAM 0x08

static codec_para_t codecParam = { 0 };

int aml_setup(int videoFormat, int width, int height, int redrawRate, void* context, int drFlags) {
  codecParam.stream_type = STREAM_TYPE_ES_VIDEO;
  codecParam.has_video = 1;
  codecParam.noblock = 0;
  codecParam.am_sysinfo.param = 0;

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
      if (!(major > 3 || (major == 3 && minor >= 14)) && width == 1920 && height == 1080)
          codecParam.am_sysinfo.param = (void*) UCODE_IP_ONLY_PARAM;
    }
  } else if (videoFormat & VIDEO_FORMAT_MASK_H265) {
    codecParam.video_type = VFORMAT_HEVC;
    codecParam.am_sysinfo.format = VIDEO_DEC_FORMAT_HEVC;
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

  return 0;
}

void aml_cleanup() {
  codec_close(&codecParam);
}

int aml_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  PLENTRY entry = decodeUnit->bufferList;
  while (entry != NULL) {
    char* data = entry->data;
    int length = entry->length;

    while (length > 0) {
      int written = codec_write(&codecParam, data, length);
      if (written > 0) {
        data += written;
        length -= written;
      } else if (errno == EAGAIN) {
        usleep(500);
      } else {
        fprintf(stderr, "codec_write() failed: %d\n", errno);
        codec_reset(&codecParam);
        return DR_NEED_IDR;
      }
    }

    entry = entry->next;
  }
  return DR_OK;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_aml = {
  .setup = aml_setup,
  .cleanup = aml_cleanup,
  .submitDecodeUnit = aml_submit_decode_unit,

  // We may delay in aml_submit_decode_unit() for a while, so we can't set CAPABILITY_DIRECT_SUBMIT
  .capabilities = CAPABILITY_REFERENCE_FRAME_INVALIDATION_HEVC,
};
