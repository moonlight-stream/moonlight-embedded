#include "ui-main.h"

#include "../video/video.h"
#include <libavcodec/avcodec.h>

static struct {
  SDL_Texture *streamTexture;
  PAPP_LIST game;

  FpsCounter counter;
  FpsCounter streamCounter;
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
  props.streamTexture = SDL_CreateTexture(ui.renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, ui.width, ui.height);
  if (!props.streamTexture) {
    fprintf(stderr, "[GUI, streaming] Could not create SDL texture for streaming: %s\n", SDL_GetError());
    return -1;
  }

  return 0;
}

void main_update_streaming(SUIInput *input) {
  if (props.counter.frame == 0) {
    stream_start(&server, &config, props.game->id, platform_check(config.platform));
  }

  // Send events to the server
  if (input->touch.touched) {
    /// TODO: Implement touch/mouse events, and change the touch input reading
  }
  else {
    short buttons = 0;
    uint8_t leftTrigger = 0, rightTrigger = 0;

    if ((input->buttons.down | input->buttons.held) & KEY_A) {
      buttons |= A_FLAG;
    }

    if ((input->buttons.down | input->buttons.held) & KEY_B) {
      buttons |= B_FLAG;
    }

    if ((input->buttons.down | input->buttons.held) & KEY_X) {
      buttons |= X_FLAG;
    }

    if ((input->buttons.down | input->buttons.held) & KEY_Y) {
      buttons |= Y_FLAG;
    }

    if ((input->buttons.down | input->buttons.held) & KEY_DUP) {
      buttons |= UP_FLAG;
    }

    if ((input->buttons.down | input->buttons.held) & KEY_DDOWN) {
      buttons |= DOWN_FLAG;
    }

    if ((input->buttons.down | input->buttons.held) & KEY_DLEFT) {
      buttons |= LEFT_FLAG;
    }

    if ((input->buttons.down | input->buttons.held) & KEY_DRIGHT) {
      buttons |= RIGHT_FLAG;
    }

    if ((input->buttons.down | input->buttons.held) & KEY_L) {
      buttons |= LB_FLAG;
    }

    if ((input->buttons.down | input->buttons.held) & KEY_R) {
      buttons |= RB_FLAG;
    }

    if ((input->buttons.down | input->buttons.held) & KEY_LSTICK) {
      buttons |= LS_CLK_FLAG;
    }

    if ((input->buttons.down | input->buttons.held) & KEY_RSTICK) {
      buttons |= RS_CLK_FLAG;
    }

    if ((input->buttons.down | input->buttons.held) & KEY_ZL) {
      leftTrigger = 0xff;
    }

    if ((input->buttons.down | input->buttons.held) & KEY_ZR) {
      rightTrigger = 0xff;
    }

    if (sui_input_test(input)) {
      int ret;

      ret= LiSendControllerEvent(buttons,
                                 leftTrigger,
                                 rightTrigger,
                                 input->joysticks.left.dx,
                                 input->joysticks.left.dy,
                                 input->joysticks.right.dx,
                                 input->joysticks.right.dy);
      if (ret < 0) {
        fprintf(stderr, "Could not send controller event: %d\n", ret);
      }

      short mouseDeltaX = (short)roundf(input->joysticks.right.dx / 32768.f * 20);
      short mouseDeltaY = -1 * (short)roundf(input->joysticks.right.dy / 32768.f * 20);
      if (fabsf(mouseDeltaX) >= 2 || fabsf(mouseDeltaY) >= 2) {
        ret = LiSendMouseMoveEvent(mouseDeltaX, mouseDeltaY);
        if (ret < 0) {
          fprintf(stderr, "Could not send mouse movement: %d\n", ret);
        }
      }
    }
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
    ui_update_fps(&props.streamCounter);
  }

  if (input->buttons.down & KEY_PLUS) {
    stream_stop(platform_check(config.platform));
    ui_state = sui_state_pop(ui_state);
  }

  ui_update_fps(&props.counter);
}

void main_render_streaming() {
  SDL_RenderClear(ui.renderer);

  // Display the updated frame
  SDL_RenderCopy(ui.renderer, props.streamTexture, NULL, NULL);

  ui_draw_fps(&props.streamCounter);

  SDL_RenderPresent(ui.renderer);
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

MoonlightUiState MoonlightUiStateStreaming = {
  .state = 5,
  .init = &main_init_streaming,
  .update = &main_update_streaming,
  .render = &main_render_streaming,
  .cleanup = &main_cleanup_streaming
};
