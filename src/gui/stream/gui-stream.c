#include "../gui.h"

#include "../../video/video.h"
#include <libavcodec/avcodec.h>

static bool shouldExitStream = 0;
static SDL_Texture *streamTexture;

static void draw_frame(AVFrame *frame) {
  // Update the scene texture with the most recent frame
  int ret = SDL_UpdateYUVTexture(
    streamTexture,
    NULL,
    frame->data[0], frame->linesize[0], // Y
    frame->data[1], frame->linesize[1], // U
    frame->data[2], frame->linesize[2]  // V
  );

  if (ret < 0) {
    fprintf(stderr, "[GUI] Could not update YUV texture with frame: %s\n", SDL_GetError());
  }
}

int gui_stream_init() {
  streamTexture = SDL_CreateTexture(gui.renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, gui.width, gui.height);
  if (!streamTexture) {
    fprintf(stderr, "[GUI] Could not create SDL texture for streaming: %s\n", SDL_GetError());
    return -1;
  }

  return 0;
}

void gui_stream_loop() {
  while (appletMainLoop() && !shouldExitStream) {
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

      case SDL_QUIT:
        shouldExitStream = 1;
        break;
      }
    }

    // Display the updated frame
    SDL_RenderClear(gui.renderer);
    SDL_RenderCopy(gui.renderer, streamTexture, NULL, NULL);
    SDL_RenderPresent(gui.renderer);
  }
}

void gui_stream_cleanup() {
  if (streamTexture) {
    SDL_DestroyTexture(streamTexture);
    streamTexture = NULL;
  }
}
