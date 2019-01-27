#pragma once

#include "ui-state.h"
#include "../application-info.h"

class UiStateStreaming : public UiState {
public:
    UiStateStreaming(Application *application, ApplicationInfo *app);
    ~UiStateStreaming();

    void enter(UiState *parent) override;
    void exit() override;

    UiStateResult update(SUIInput *input) override;

private:
    void drawFrame(uint8_t **data, int *linesize);

    ApplicationInfo *app_;
    SUIImage *stream_image_;
    SDL_Texture *stream_texture_;
};