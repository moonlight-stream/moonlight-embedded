#include "ui-state-streaming.h"
#include "../application.h"

extern "C" {
    #include "../video/video.h"
}

#include <libavcodec/avcodec.h>

UiStateStreaming::UiStateStreaming(Application *application, ApplicationInfo *app)
    : UiState(application),
      app_(app)
{
    stream_texture_ = SDL_CreateTexture(ui()->renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, ui()->width, ui()->height);
    stream_image_ = new SUIImage("stream-image", stream_texture_);
    stream_image_->bounds() = { 0, 0, ui()->width, ui()->height};
    content_->addChild(stream_image_);
}

UiStateStreaming::~UiStateStreaming() {
    delete stream_image_;

    if (stream_texture_) {
        SDL_DestroyTexture(stream_texture_);
    }
}

void UiStateStreaming::enter(UiState *parent) {
    application()->server()->startStream(app_->id);
}

void UiStateStreaming::exit() {
    application()->server()->stopStream();
}

UiStateResult UiStateStreaming::update(SUIInput *input) {
    UiState::update(input);

    // Read events from the queue
    SDL_Event event;
    uint8_t **last_frame_data = NULL;
    int *last_frame_line_size = NULL;
    int frames_skipped = 0;
    while (SDL_PollEvent(&event) > 0) {
        if (event.type == SDL_USEREVENT && event.user.code == VIDEO_FRAME_EVENT) {
            if (last_frame_data)
            {
                frames_skipped++;
            }

            last_frame_data = (uint8_t **)event.user.data1;
            last_frame_line_size = (int *)event.user.data2;
        }
    }

    if (last_frame_data) {
        drawFrame(last_frame_data, last_frame_line_size);
    }

    // Exit the stream on + click
    if (input->buttons.down & KEY_PLUS) {
        return UiStateResultType::PopState;
    }

    return UiStateResultType::Normal;
}

void UiStateStreaming::drawFrame(uint8_t **data, int *linesize) {
  // Update the scene texture with the most recent frame
  int ret = SDL_UpdateYUVTexture(
    stream_texture_,
    NULL,
    data[0], linesize[0], // Y
    data[1], linesize[1], // U
    data[2], linesize[2]  // V
  );

  if (ret < 0) {
    fprintf(stderr, "[GUI, streaming] Could not update YUV texture with frame: %s\n", SDL_GetError());
  }
}