#include "video.h"

#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <switch.h>

static AVCodec *codec;
static AVCodecContext *decoder;
static AVPacket *packet;
static AVFrame *frame;

#define DECODE_BUFFER_SIZE 1024 * 128
static uint8_t *decodeBuffer;

static void switch_cleanup(void) {
  fprintf(stderr, "[VIDEO] Cleaned up renderer\n");
}

static int switch_setup(int videoFormat, int width, int height, int redrawRate, void* context, int drFlags) {
  fprintf(stderr, "[VIDEO] Setup renderer: videoFormat=%d width=%d height=%d redrawRate=%d\n",
          videoFormat,
          width,
          height,
          redrawRate
          );

  codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (!codec) {
    fprintf(stderr, "[VIDEO] Could not find H264 codec\n");
    goto err;
  }

  decoder = avcodec_alloc_context3(codec);
  if (!decoder) {
    fprintf(stderr, "[VIDEO] Could not allocate H264 codec context\n");
    goto err;
  }

  decoder->width = width;
  decoder->height = height;
  decoder->pix_fmt = AV_PIX_FMT_YUV420P;

  if (avcodec_open2(decoder, codec, NULL) < 0) {
    fprintf(stderr, "[VIDEO] Could not open H264 codec for decoding\n");
    goto err;
  }

  packet = av_packet_alloc();
  if (!packet) {
    fprintf(stderr, "[VIDEO] Could not allocate video packet\n");
    goto err;
  }

  frame = av_frame_alloc();
  if (!frame) {
    fprintf(stderr, "[VIDEO] Could not allocate video frame\n");
    goto err;
  }

  decodeBuffer = malloc(DECODE_BUFFER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE);
  if (!decodeBuffer) {
    fprintf(stderr, "[VIDEO] Could not allocate decoding buffer storage\n");
    goto err;
  }

  return 0;

err:
  switch_cleanup();
  return -1;
}

static void switch_start(void) {
  fprintf(stderr, "[VIDEO] Started renderer\n");
}

static void switch_stop(void) {
  fprintf(stderr, "[VIDEO] Stopped renderer\n");
}

static int switch_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  int ret;

  // Collect the data from this decode unit
  PLENTRY bufferEntry = decodeUnit->bufferList;
  int bufferLength = 0;
  while (bufferEntry) {
    memcpy(decodeBuffer + bufferLength, bufferEntry->data, bufferEntry->length);
    bufferLength += bufferEntry->length;
    bufferEntry = bufferEntry->next;
  }

  // Collect the decode data into an AV packet and send for decoding
  packet->data = decodeBuffer;
  packet->size = bufferLength;
  ret = avcodec_send_packet(decoder, packet);
  if (ret < 0) {
    fprintf(stderr, "[VIDEO] Unable to send frame %d to decoder (error: %d)\n", decodeUnit->frameNumber, ret);
    return DR_NEED_IDR;
  }

  // Receive the decoded frame from the decoder
  ret = avcodec_receive_frame(decoder, frame);
  if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
    return DR_OK;
  }
  else if (ret < 0) {
    fprintf(stderr, "[VIDEO] Unable to receive decoded frame %d (error: %d)\n", decodeUnit->frameNumber, ret);
    return DR_NEED_IDR;
  }

  // Obtain access to the framebuffer
  uint32_t width, height;
  uint8_t *framebuf = gfxGetFramebuffer(&width, &height);
  uint32_t *fbptr = (uint32_t *)framebuf;

  if (width != frame->width || height != frame->height) {
    fprintf(stderr, "[VIDEO] Frame dimension mismatch: screen=(%d,%d) frame=(%d,%d)\n", width, height, frame->width, frame->height);
  }

  // Write the frame to the buffer
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      int yy = frame->data[0][(j * frame->linesize[0]) + i];
      *fbptr++ = RGBA8(yy, yy, yy, 0xff);
    }
  }

  return DR_OK;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_switch = {
  .setup = switch_setup,
  .start = switch_start,
  .stop = switch_stop,
  .cleanup = switch_cleanup,
  .submitDecodeUnit = switch_submit_decode_unit,
  .capabilities = CAPABILITY_DIRECT_SUBMIT,
};
