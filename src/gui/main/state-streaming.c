#include "gui-main.h"

#include "../../video/video.h"
#include <libavcodec/avcodec.h>

static struct {
  SDL_Texture *streamTexture;
  PAPP_LIST game;

  int frame;
} props = {0};

static void draw_frame(uint8_t **data, int *linesize) {
  // Update the scene texture with the most recent frame
  int ret = SDL_UpdateYUVTexture(
    props.streamTexture,
    NULL,
    data[0], linesize[0], // Y
    data[1], linesize[1], // U
    data[2], linesize[2]  // V
  );

  if (ret < 0) {
    fprintf(stderr, "[GUI, streaming] Could not update YUV texture with frame: %s\n", SDL_GetError());
  }
}


int main_init_streaming() {
  props.streamTexture = SDL_CreateTexture(gui.renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, gui.width, gui.height);
  if (!props.streamTexture) {
    fprintf(stderr, "[GUI, streaming] Could not create SDL texture for streaming: %s\n", SDL_GetError());
    return -1;
  }

  return 0;
}

void main_update_streaming(Input *input) {
  if (props.frame == 0) {
    stream_start(&server, &config, props.game->id, platform_check(config.platform));
  }

  // Read events from the queue
  SDL_Event event;
  uint8_t **lastVideoFrameData = NULL;
  int *lastVideoFrameLineSize = NULL;
  int framesSkipped = 0;

  while (SDL_PollEvent(&event) > 0) {
    switch (event.type) {
    case SDL_USEREVENT:
      if (event.user.code == VIDEO_FRAME_EVENT) {
        if (lastVideoFrameData) {
          framesSkipped++;
        }

        lastVideoFrameData = (uint8_t **)event.user.data1;
        lastVideoFrameLineSize = (int *)event.user.data2;
      }
      break;
    }
  }

  if (lastVideoFrameData) {
    draw_frame(lastVideoFrameData, lastVideoFrameLineSize);
    fprintf(stderr, "[VIDEO] Skipped %d frames before rendering frame %d\n", framesSkipped, props.frame);
  }

  if (input->keys & KEY_PLUS) {
    stream_stop(platform_check(config.platform));
    state = StateGamesList;
  }

  props.frame++;
}

void main_render_streaming() {
  // Display the updated frame
  SDL_RenderClear(gui.renderer);
  SDL_RenderCopy(gui.renderer, props.streamTexture, NULL, NULL);
  SDL_RenderPresent(gui.renderer);
}

void main_cleanup_streaming() {
  if (props.streamTexture) {
    SDL_DestroyTexture(props.streamTexture);
    props.streamTexture = NULL;
  }
}

void main_set_streaming_game(PAPP_LIST game) {
  props.game = game;
}
