/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015, 2016 Iwan Timmer
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

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <amcodec/codec.h>

static codec_para_t codecParam = { 0 };
static const size_t SYNC_OUTSIDE = (2);

static int osd_blank(char *path,int cmd) {
  int fd;
  char bcmd[16];

  fd = open(path, O_CREAT|O_RDWR | O_TRUNC, 0644);

  if(fd>=0) {
    sprintf(bcmd,"%d",cmd);
    int ret = write(fd,bcmd,strlen(bcmd));
    if (ret < 0) {
      printf("osd_blank error during write: %x\n", ret);
    }
    close(fd);
    return 0;
  }

  return -1;
}

void aml_setup(int videoFormat, int width, int height, int redrawRate, void* context, int drFlags) {
  osd_blank("/sys/class/graphics/fb0/blank",1);
  osd_blank("/sys/class/graphics/fb1/blank",0);

  codecParam.stream_type = STREAM_TYPE_ES_VIDEO;
  codecParam.has_video = 1;
  codecParam.noblock = 0;

  switch (videoFormat) {
    case VIDEO_FORMAT_H264:
      if (width > 1920 || height > 1080) {
        codecParam.video_type = VFORMAT_H264_4K2K;
        codecParam.am_sysinfo.format = VIDEO_DEC_FORMAT_H264_4K2K;
      } else {
        codecParam.video_type = VFORMAT_H264;
        codecParam.am_sysinfo.format = VIDEO_DEC_FORMAT_H264;
      }
      break;
    case VIDEO_FORMAT_H265:
      codecParam.video_type = VFORMAT_HEVC;
      codecParam.am_sysinfo.format = VIDEO_DEC_FORMAT_HEVC;
      break;
    default:
      printf("Video format not supported\n");
      exit(1);
  }

  codecParam.am_sysinfo.width = width;
  codecParam.am_sysinfo.height = height;
  codecParam.am_sysinfo.rate = 96000 / redrawRate;
  codecParam.am_sysinfo.param = (void *)(SYNC_OUTSIDE);

  int api = codec_init(&codecParam);
  if (api != 0) {
    fprintf(stderr, "codec_init error: %x\n", api);
    exit(1);
  }
}

void aml_cleanup() {
  int api = codec_close(&codecParam);
  osd_blank("/sys/class/graphics/fb0/blank",0);
  osd_blank("/sys/class/graphics/fb1/blank",0);
}

int aml_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  int result = DR_OK;
  PLENTRY entry = decodeUnit->bufferList;
  while (entry != NULL) {
    int api = codec_write(&codecParam, entry->data, entry->length);
    if (api != entry->length) {
      fprintf(stderr, "codec_write error: %x\n", api);
      codec_reset(&codecParam);
      result = DR_NEED_IDR;
      break;
    }

    entry = entry->next;
  }
  return result;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_aml = {
  .setup = aml_setup,
  .cleanup = aml_cleanup,
  .submitDecodeUnit = aml_submit_decode_unit,
  .capabilities = CAPABILITY_DIRECT_SUBMIT | CAPABILITY_SLICES_PER_FRAME(8),
};
