#pragma once

#include "ui-state.h"

class UiStateStreaming : public UiState {
public:
    UiStateStreaming(Application *application);
    ~UiStateStreaming();

    void enter(UiState *parent) override;
    void exit() override;

    UiStateResult update(SUIInput *input) override;

private:
    void drawFrame(uint8_t **data, int *linesize);

    SUIImage *stream_image_;
    SDL_Texture *stream_texture_;
};