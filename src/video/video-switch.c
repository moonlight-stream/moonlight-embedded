#include "video.h"
#include "libgamestream/sps.h"

#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <switch.h>
#include <SDL2/SDL.h>

static AVCodec *codec;
static AVCodecContext *decoder;
static AVPacket *packet;
static AVFrame *frame;

#define DECODE_BUFFER_SIZE 1024 * 128
static uint8_t *decodeBuffer;

static void switch_video_cleanup(void) {
  fprintf(stderr, "[VIDEO] Cleaned up renderer\n");
}

static int switch_video_setup(int videoFormat, int width, int height, int redrawRate, void* context, int drFlags) {
  fprintf(stderr, "[VIDEO] Setup renderer: videoFormat=%d width=%d height=%d redrawRate=%d\n",
          videoFormat,
          width,
          height,
          redrawRate
          );

  gs_sps_init(width, height);

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
  switch_video_cleanup();
  return -1;
}

static void switch_video_start(void) {
  fprintf(stderr, "[VIDEO] Started renderer\n");
}

static void switch_video_stop(void) {
  fprintf(stderr, "[VIDEO] Stopped renderer\n");
}

static int switch_video_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  int ret;

  // Decode the SPS headers for the H.264 frame
  gs_sps_fix(decodeUnit, 0);

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

  // Submit the frame to the event system
  SDL_Event frameEvent;
  frameEvent.type = SDL_USEREVENT;
  frameEvent.user.code = VIDEO_FRAME_EVENT;
  frameEvent.user.data1 = frame->data;
  frameEvent.user.data2 = frame->linesize;
  SDL_PushEvent(&frameEvent);

  return DR_OK;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_switch = {
  .setup = switch_video_setup,
  .start = switch_video_start,
  .stop = switch_video_stop,
  .cleanup = switch_video_cleanup,
  .submitDecodeUnit = switch_video_submit_decode_unit,
  .capabilities = CAPABILITY_DIRECT_SUBMIT,
};
