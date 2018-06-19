#include "gui-main.h"

#include "../../video/video.h"
#include <libavcodec/avcodec.h>

static struct {
  SDL_Texture *streamTexture;
  PAPP_LIST game;

  int frame;
} props = {0};

static void draw_frame(AVFrame *frame) {
  // Update the scene texture with the most recent frame
  int ret = SDL_UpdateYUVTexture(
    props.streamTexture,
    NULL,
    frame->data[0], frame->linesize[0], // Y
    frame->data[1], frame->linesize[1], // U
    frame->data[2], frame->linesize[2]  // V
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
  while (SDL_PollEvent(&event) > 0) {
    switch (event.type) {
    case SDL_USEREVENT:
      if (event.user.code == VIDEO_FRAME_EVENT) {
        AVFrame *frame = (AVFrame *)event.user.data1;
        draw_frame(frame);
      }
      break;
    }
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
