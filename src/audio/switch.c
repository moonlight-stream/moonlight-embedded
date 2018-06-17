#include "audio.h"

#include <stdio.h>

static int switch_audio_init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig, void* context, int arFlags) {
  /// TODO: Implement
  return 0;
}

static void switch_audio_cleanup() {
  /// TODO: Implement
}

static void switch_audio_decode_and_play_sample(char* data, int length) {
  /// TODO: Implement
}

AUDIO_RENDERER_CALLBACKS audio_callbacks_switch = {
  .init = switch_audio_init,
  .cleanup = switch_audio_cleanup,
  .decodeAndPlaySample = switch_audio_decode_and_play_sample,
  .capabilities = CAPABILITY_DIRECT_SUBMIT,
};
