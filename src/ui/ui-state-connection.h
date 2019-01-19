#pragma once

#include "ui-state.h"
#include "../promise.h"
#include "../server.h"

class UiStateConnection : public UiState {
public:
    UiStateConnection(Application *application);
    ~UiStateConnection();

    void enter(UiState *parent) override;
    void exit() override;
    UiStateResult update(SUIInput *input) override;

private:
    SDL_Texture *connecting_texture_;
    int connecting_width_;
    int connecting_height_;

    SDL_Texture *connection_failed_texture_;
    int connection_failed_width_;
    int connection_failed_height_;

    SUIImage *connecting_image_;
    SUIImage *connection_failed_image_;

    promise<bool, ServerError>::future *connection_promise_;
    promise<bool, ServerError>::future *pairing_promise_;
};
