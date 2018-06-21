#include "audio.h"

#include <stdio.h>
#include <switch.h>
#include <opus/opus_multistream.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

static SDL_AudioDeviceID audioDevice;
static OpusMSDecoder* decoder;
static short pcmBuffer[FRAME_SIZE * MAX_CHANNEL_COUNT];
static int channelCount;

static void switch_audio_cleanup() {
  // Close the Opus decoder
  if (decoder) {
    opus_multistream_decoder_destroy(decoder);
    decoder = NULL;
  }

  // Close the SDL audio device
  SDL_CloseAudioDevice(audioDevice);

  fprintf(stderr, "[AUDIO] Cleaned up renderer\n");
}

static int switch_audio_init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig, void* context, int arFlags) {
  fprintf(stderr,
          "[AUDIO] Setup renderer: sampleRate=%d channelCount=%d\n",
          opusConfig->sampleRate,
          opusConfig->channelCount);

  // Initialize the Opus decoder
  int error;
  channelCount = opusConfig->channelCount;
  decoder = opus_multistream_decoder_create(opusConfig->sampleRate, opusConfig->channelCount,
                                            opusConfig->streams, opusConfig->coupledStreams,
                                            opusConfig->mapping, &error);
  if (error != OPUS_OK) {
    fprintf(stderr, "[AUDIO] Could not create Opus multi-stream decoder: %d\n", error);
    goto err;
  }

  // Initialize the SDL audio system
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  SDL_AudioSpec specWant, specHave;
  SDL_zero(specWant);
  specWant.freq = opusConfig->sampleRate;
  specWant.format = AUDIO_S16LSB;
  specWant.channels = opusConfig->channelCount;
  specWant.samples = 4096;

  // Open the SDL audio device
  audioDevice = SDL_OpenAudioDevice(NULL, 0, &specWant, &specHave, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if (audioDevice == 0) {
    fprintf(stderr, "[AUDIO] Failed to open SDL audio device: %s\n", SDL_GetError());
    goto err;
  }

  if (specHave.format != specWant.format) {
      fprintf(stderr, "[AUDIO] Opened SDL audio device with different format: want=%d have=%d\n", specWant.format, specHave.format);
  }

  // Begin audio playing on the device
  SDL_PauseAudioDevice(audioDevice, 0);

  return 0;

err:
  switch_audio_cleanup();
  return -1;
}

static void switch_audio_decode_and_play_sample(char* data, int length) {
  int result = opus_multistream_decode(decoder, data, length, pcmBuffer, FRAME_SIZE, 0);
  if (result < 0) {
    printf("[AUDIO] Unable to decode Opus packet: %d\n", result);
    return;
  }

  SDL_QueueAudio(audioDevice, pcmBuffer, result * channelCount * sizeof(short));
}

AUDIO_RENDERER_CALLBACKS audio_callbacks_switch = {
  .init = switch_audio_init,
  .cleanup = switch_audio_cleanup,
  .decodeAndPlaySample = switch_audio_decode_and_play_sample,
  .capabilities = CAPABILITY_DIRECT_SUBMIT,
};
