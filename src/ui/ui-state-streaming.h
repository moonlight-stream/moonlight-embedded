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
    void clearFrame(uint8_t y = 0, uint8_t u = 0x80, uint8_t v = 0x80);

    ApplicationInfo *app_;
    SDL_Texture *stream_texture_;
    SUIImage *stream_image_;

    SUILoadingSpinner *spinner_;
};