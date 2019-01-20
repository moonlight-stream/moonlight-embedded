#pragma once

#include "ui-state.h"
#include "../promise.h"
#include "../server.h"

class UiStateConnection : public UiState {
public:
    UiStateConnection(Application *application);
    ~UiStateConnection();

    void enter(UiState *parent) override;
    UiStateResult update(SUIInput *input) override;

private:
    void enterInitial();
    UiStateResult updateInitial(SUIInput *input);

    void enterConnecting();
    UiStateResult updateConnecting(SUIInput *input);

    void enterPairing();
    UiStateResult updatePairing(SUIInput *input);

    void enterFailed();
    UiStateResult updateFailed(SUIInput *input);

    enum class ConnectionState {
        Initial,
        Connecting,
        Pairing,
        Failed
    } state_;

    SDL_Texture *connecting_texture_;
    int connecting_width_;
    int connecting_height_;

    SDL_Texture *connection_failed_texture_;
    int connection_failed_width_;
    int connection_failed_height_;

    SUIImage *connecting_image_;
    SUIImage *connection_failed_image_;

    promise<bool, ServerError> *connection_promise_;
    promise<bool, ServerError> *pairing_promise_;
};
