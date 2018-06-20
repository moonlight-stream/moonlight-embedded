#include "audio.h"

#include <stdio.h>
#include <switch.h>
#include <libavcodec/avcodec.h>

static AVCodec *codec;
static AVCodecContext *decoder;
static AVPacket *packet;
static AVFrame *frame;

#define SAMPLE_RATE 48000
#define CHANNEL_COUNT 2
#define FRAME_RATE (1000 / 30)
#define SAMPLE_COUNT (SAMPLE_RATE / FRAME_RATE)
#define BYTES_PER_SAMPLE 2

static uint32_t dataSize = (SAMPLE_COUNT * CHANNEL_COUNT * BYTES_PER_SAMPLE);
static uint32_t bufferSize = (SAMPLE_COUNT * CHANNEL_COUNT * BYTES_PER_SAMPLE + 0xfff) & ~0xfff;
static uint8_t *sampleBuffer;

static AudioOutBuffer outBuffer;

static void switch_audio_cleanup() {
  fprintf(stderr, "[VIDEO] Cleaned up renderer\n");
}

static int switch_audio_init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig, void* context, int arFlags) {
  fprintf(stderr, "[AUDIO] Setup renderer: audioConfiguration=%d arFlags=%d\n",
          audioConfiguration,
          arFlags
          );

  Result rc = audoutInitialize();
  if (R_FAILED(rc)) {
    fprintf(stderr, "[AUDIO] Could not initialize Switch audio service\n");
    goto err;
  }
  else {
    fprintf(stderr, "[AUDIO] Sample rate: 0x%x\n", audoutGetSampleRate());
    fprintf(stderr, "[AUDIO] Channel count: 0x%x\n", audoutGetChannelCount());
    fprintf(stderr, "[AUDIO] PCM format: 0x%x\n", audoutGetPcmFormat());
    fprintf(stderr, "[AUDIO] Device state: 0x%x\n", audoutGetDeviceState());
    fprintf(stderr, "[AUDIO] dataSize: 0x%x\n", dataSize);
    fprintf(stderr, "[AUDIO] bufferSize: 0x%x\n", bufferSize);
  }

  // Start audio playback.
  rc = audoutStartAudioOut();
  if (R_FAILED(rc)) {
    fprintf(stderr, "[AUDIO] audoutStartAudioOut() returned 0x%x\n", rc);
    goto err;
  }

  codec = avcodec_find_decoder(AV_CODEC_ID_OPUS);
  if (!codec) {
    fprintf(stderr, "[AUDIO] Could not find Opus codec\n");
    goto err;
  }

  decoder = avcodec_alloc_context3(codec);
  if (!decoder) {
    fprintf(stderr, "[AUDIO] Could not allocate Opus codec context\n");
    goto err;
  }

  decoder->sample_rate = 48000;
  decoder->request_sample_fmt = AV_SAMPLE_FMT_S32;

  if (avcodec_open2(decoder, codec, NULL) < 0) {
    fprintf(stderr, "[AUDIO] Could not open Opus codec for decoding\n");
    goto err;
  }

  packet = av_packet_alloc();
  if (!packet) {
    fprintf(stderr, "[AUDIO] Could not allocate audio packet\n");
    goto err;
  }

  frame = av_frame_alloc();
  if (!frame) {
    fprintf(stderr, "[AUDIO] Could not allocate audio frame\n");
    goto err;
  }

  sampleBuffer = malloc(bufferSize);
  if (!sampleBuffer) {
    fprintf(stderr, "[AUDIO] Could not allocate decoding buffer storage\n");
    goto err;
  }

  return 0;

err:
  switch_audio_cleanup();
  return -1;
}

static void switch_audio_decode_and_play_sample(char* data, int length) {
  int ret;

  // Collect the decode data into an AV packet and send for decoding
  packet->data = data;
  packet->size = length;
  ret = avcodec_send_packet(decoder, packet);
  if (ret < 0) {
    fprintf(stderr, "[AUDIO] Unable to send audio sample to decoder (error: %d)\n", ret);
    return;
  }

  // Receive the decoded frame from the decoder
  ret = avcodec_receive_frame(decoder, frame);
  if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
    return;
  }
  else if (ret < 0) {
    fprintf(stderr, "[AUDIO] Unable to receive decoded audio frame (error: %d)\n", ret);
    return;
  }

  // Submit the sample to the audio system
  fprintf(stderr, "[AUDIO] Received sample\n");

  for (int i = 0; i < frame->nb_samples; i++) {
    uint32_t offset = i * sizeof(uint32_t);
    memcpy(sampleBuffer + offset, frame->data[0] + offset, sizeof(uint32_t));
  }

  outBuffer.next = NULL;
  outBuffer.buffer = sampleBuffer;
  outBuffer.buffer_size = bufferSize;
  outBuffer.data_size = dataSize;
  outBuffer.data_offset = 0;

  AudioOutBuffer *releasedBuffer = NULL;
  Result rc = audoutPlayBuffer(&outBuffer, &releasedBuffer);
  if (R_FAILED(rc)) {
    fprintf(stderr, "[AUDIO] Could not play audio sample\n");
  }
}

AUDIO_RENDERER_CALLBACKS audio_callbacks_switch = {
  .init = switch_audio_init,
  .cleanup = switch_audio_cleanup,
  .decodeAndPlaySample = switch_audio_decode_and_play_sample,
  .capabilities = CAPABILITY_DIRECT_SUBMIT,
};
