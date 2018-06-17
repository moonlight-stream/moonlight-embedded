#include "gui.h"

#include "../video/video.h"
#include <libavcodec/avcodec.h>

static bool shouldExitStream = 0;

static void gui_draw_frame(AVFrame *frame) {
  // Update the scene texture with the most recent frame
  int ret = SDL_UpdateYUVTexture(
    gui.streamTexture,
    NULL,
    frame->data[0], frame->linesize[0], // Y
    frame->data[1], frame->linesize[1], // U
    frame->data[2], frame->linesize[2]  // V
  );

  if (ret < 0) {
    fprintf(stderr, "[GUI] Could not update YUV texture with frame: %s\n", SDL_GetError());
  }
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
          gui_draw_frame(frame);
        }
        break;

      case SDL_QUIT:
        shouldExitStream = 1;
        break;
      }
    }

    // Display the updated frame
    SDL_RenderClear(gui.renderer);
    SDL_RenderCopy(gui.renderer, gui.streamTexture, NULL, NULL);
    SDL_RenderPresent(gui.renderer);
  }
}
