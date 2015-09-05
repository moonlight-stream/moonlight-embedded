/*
 * This file is part of Moonlight Embedded.
 *
 * Based on Moonlight Pc implementation
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

#include "ffmpeg.h"

#include <stdlib.h>
#include <libswscale/swscale.h>
#include <pthread.h>
#include <stdio.h>

// General decoder and renderer state
static AVPacket pkt;
static AVCodec* decoder;
static AVCodecContext* decoder_ctx;
static AVFrame* dec_frame;

#define BYTES_PER_PIXEL 4

// This function must be called before
// any other decoding functions
int ffmpeg_init(int width, int height, int perf_lvl, int thread_count) {
  // Initialize the avcodec library and register codecs
  av_log_set_level(AV_LOG_QUIET);
  avcodec_register_all();

  av_init_packet(&pkt);

  decoder = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (decoder == NULL) {
    printf("Couldn't find H264 decoder");
    return -1;
  }

  decoder_ctx = avcodec_alloc_context3(decoder);
  if (decoder_ctx == NULL) {
    printf("Couldn't allocate context");
    return -1;
  }

  if (perf_lvl & DISABLE_LOOP_FILTER)
    // Skip the loop filter for performance reasons
    decoder_ctx->skip_loop_filter = AVDISCARD_ALL;

  if (perf_lvl & LOW_LATENCY_DECODE)
    // Use low delay single threaded encoding
    decoder_ctx->flags |= CODEC_FLAG_LOW_DELAY;

  if (perf_lvl & SLICE_THREADING)
    decoder_ctx->thread_type = FF_THREAD_SLICE;
  else
    decoder_ctx->thread_type = FF_THREAD_FRAME;

  decoder_ctx->thread_count = thread_count;

  decoder_ctx->width = width;
  decoder_ctx->height = height;
  decoder_ctx->pix_fmt = PIX_FMT_YUV420P;

  int err = avcodec_open2(decoder_ctx, decoder, NULL);
  if (err < 0) {
    printf("Couldn't open codec");
    return err;
  }

  dec_frame = av_frame_alloc();
  if (dec_frame == NULL) {
    printf("Couldn't allocate frame");
    return -1;
  }

  return 0;
}

// This function must be called after
// decoding is finished
void ffmpeg_destroy(void) {
  if (decoder_ctx) {
    avcodec_close(decoder_ctx);
    av_free(decoder_ctx);
    decoder_ctx = NULL;
  }
  if (dec_frame) {
    av_frame_free(&dec_frame);
    dec_frame = NULL;
  }
}

AVFrame* ffmpeg_get_frame() {
  return dec_frame;
}

// packets must be decoded in order
// indata must be inlen + FF_INPUT_BUFFER_PADDING_SIZE in length
int ffmpeg_decode(unsigned char* indata, int inlen) {
  int err;
  int got_pic = 0;

  pkt.data = indata;
  pkt.size = inlen;

  while (pkt.size > 0) {
    got_pic = 0;
    err = avcodec_decode_video2(decoder_ctx, dec_frame, &got_pic, &pkt);
    if (err < 0) {
      fprintf(stderr, "Decode failed\n");
      got_pic = 0;
      break;
    }

    pkt.size -= err;
    pkt.data += err;
  }
  
  if (got_pic) {
    return 1;
  }

  return err < 0 ? err : 0;
}
