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

    void handleRetryButtonClicked(SUIElement *, SUIEvent);

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

    SUIContainer *connecting_scene_;
    SUIImage *connecting_image_;
    SUIText *connecting_guide_text_;
    SUIText *connecting_pin_text_;

    SUIContainer *connection_failed_scene_;
    SUIText *connection_failed_guide_text_;
    SUIText *connection_failed_error_text_;
    SUIButton *connection_failed_retry_button_;
    SUIImage *connection_failed_image_;

    promise<bool, ServerError> *connection_promise_;
    promise<bool, ServerError> *pairing_promise_;
};
