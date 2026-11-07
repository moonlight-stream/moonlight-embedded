#include "native-mic.h"

#include <Limelight.h>
#include <SDL.h>
#include <opus.h>
#include <stdbool.h>
#include <stdio.h>

#define FRAME_SAMPLES 240
#define FRAME_BYTES (FRAME_SAMPLES * sizeof(opus_int16))
#define ENCODED_BYTES 20

static SDL_AudioDeviceID device;
static SDL_Thread* worker;
static SDL_atomic_t running;
static NATIVE_MIC_SENDER* sender;
static OpusEncoder* encoder;
static bool audio_initialized;

static int SDLCALL capture_thread(void* unused) {
  opus_int16 pcm[FRAME_SAMPLES];
  unsigned char encoded[ENCODED_BYTES];

  while (SDL_AtomicGet(&running)) {
    if (SDL_GetAudioDeviceStatus(device) != SDL_AUDIO_PLAYING) {
      fprintf(stderr, "Microphone device disconnected\n");
      break;
    }

    Uint32 available = SDL_GetQueuedAudioSize(device);
    if (available < FRAME_BYTES) {
      SDL_Delay(1);
      continue;
    }
    // Drop queued audio after a scheduling stall to keep latency bounded.
    if (available > FRAME_BYTES * 20) {
      SDL_ClearQueuedAudio(device);
      continue;
    }

    if (SDL_DequeueAudio(device, pcm, FRAME_BYTES) != FRAME_BYTES ||
        opus_encode(encoder, pcm, FRAME_SAMPLES, encoded, sizeof(encoded)) != ENCODED_BYTES ||
        LiSendNativeMicFrame(sender, encoded, sizeof(encoded)) != 0) {
      fprintf(stderr, "Microphone forwarding stopped after a capture or transport error\n");
      break;
    }
  }

  SDL_PauseAudioDevice(device, 1);
  SDL_AtomicSet(&running, 0);
  return 0;
}

void native_mic_stop(void) {
  SDL_AtomicSet(&running, 0);
  if (worker != NULL) {
    SDL_WaitThread(worker, NULL);
    worker = NULL;
  }
  if (device != 0) {
    SDL_CloseAudioDevice(device);
    device = 0;
  }
  if (encoder != NULL) {
    opus_encoder_destroy(encoder);
    encoder = NULL;
  }
  LiDestroyNativeMicSender(sender);
  sender = NULL;
  if (audio_initialized) {
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    audio_initialized = false;
  }
}

void native_mic_start(void) {
  sender = LiCreateNativeMicSender();
  if (sender == NULL) {
    fprintf(stderr, "Microphone unavailable: encrypted native GameStream endpoint on private IPv4 required\n");
    return;
  }

  int error;
  encoder = opus_encoder_create(48000, 1, OPUS_APPLICATION_RESTRICTED_LOWDELAY, &error);
  if (error != OPUS_OK || encoder == NULL ||
      opus_encoder_ctl(encoder, OPUS_SET_BITRATE(32000)) != OPUS_OK ||
      opus_encoder_ctl(encoder, OPUS_SET_VBR(0)) != OPUS_OK ||
      opus_encoder_ctl(encoder, OPUS_SET_DTX(0)) != OPUS_OK) {
    fprintf(stderr, "Microphone encoder could not start\n");
    native_mic_stop();
    return;
  }

  if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
    fprintf(stderr, "Microphone audio initialization failed: %s\n", SDL_GetError());
    native_mic_stop();
    return;
  }
  audio_initialized = true;

  SDL_AudioSpec requested = {0};
  requested.freq = 48000;
  requested.format = AUDIO_S16SYS;
  requested.channels = 1;
  requested.samples = 256;
  device = SDL_OpenAudioDevice(NULL, 1, &requested, NULL, 0);
  if (device == 0) {
    fprintf(stderr, "Default microphone could not open: %s\n", SDL_GetError());
    native_mic_stop();
    return;
  }

  SDL_AtomicSet(&running, 1);
  SDL_PauseAudioDevice(device, 0);
  worker = SDL_CreateThread(capture_thread, "native-mic", NULL);
  if (worker == NULL) {
    fprintf(stderr, "Microphone worker could not start: %s\n", SDL_GetError());
    native_mic_stop();
    return;
  }
  fprintf(stderr, "Microphone forwarding ON. End the stream to stop forwarding.\n");
}
