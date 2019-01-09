#pragma once

#include "ui-state.h"

class UiStateConnecting : public UiState {
public:
    UiStateConnecting(Application *application);
    ~UiStateConnecting();

    UiStateResult update(SUIInput *input) override;
    void render() override;

private:
    SDL_Texture *connecting_texture_;
    int connecting_width_;
    int connecting_height_;

    SUIImage *connecting_image_;
};
