#include "ui-state-connection.h"
#include "ui-state-games-list.h"
#include "../application.h"

#include "moonlight_switch_connecting_png.h"
#include "moonlight_switch_connection_failed_png.h"

UiStateConnection::UiStateConnection(Application *application) 
    : UiState(application),
      state_(ConnectionState::Initial),
      connection_promise_(nullptr),
      pairing_promise_(nullptr)
{
    connecting_texture_ = ui()->loadPNG(moonlight_switch_connecting_png, moonlight_switch_connecting_png_size);
    connection_failed_texture_ = ui()->loadPNG(moonlight_switch_connection_failed_png, moonlight_switch_connection_failed_png_size);
    
    if (!connecting_texture_ || !connection_failed_texture_) {
        fprintf(stderr, "[GUI, connecting] Could not load image textures: %s\n", SDL_GetError());
        return;
    }

    SDL_QueryTexture(connecting_texture_, NULL, NULL, &connecting_width_, &connecting_height_);
    SDL_QueryTexture(connection_failed_texture_, NULL, NULL, &connection_failed_width_, &connection_failed_height_);

    connecting_image_ = new SUIImage("image-connecting", connecting_texture_);
    connecting_image_->bounds().x = (ui()->width - connecting_width_) / 2;
    connecting_image_->bounds().y = SUI_MARGIN_TOP + 75;
    connecting_image_->bounds().w = connecting_width_;
    connecting_image_->bounds().h = connecting_height_;

    connection_failed_image_ = new SUIImage("image-connection-failed", connection_failed_texture_);
    connection_failed_image_->bounds().x = (ui()->width - connection_failed_width_) / 2;
    connection_failed_image_->bounds().y = SUI_MARGIN_TOP + 75;
    connection_failed_image_->bounds().w = connection_failed_width_;
    connection_failed_image_->bounds().h = connection_failed_height_;
    connection_failed_image_->setVisible(false);

    content()->addChild(connecting_image_);
    content()->addChild(connection_failed_image_);

    header_text_ = "Moonlight  ›  Connection";
    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem("OK", SUIToolbarAction::A));
    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem("Exit", SUIToolbarAction::B));
}

UiStateConnection::~UiStateConnection() {
    if (connecting_texture_)
        SDL_DestroyTexture(connecting_texture_);

    if (connecting_image_) 
        delete connecting_image_;
}

void UiStateConnection::enter(UiState *parent) {
    enterInitial();
}

UiStateResult UiStateConnection::update(SUIInput *input) {
    UiState::update(input);

    UiStateResult result;

    switch (state_) {
        case ConnectionState::Initial:
            result = updateInitial(input);
            break;

        case ConnectionState::Connecting:
            result = updateConnecting(input);
            break;

        case ConnectionState::Pairing:
            result = updatePairing(input);
            break;

        case ConnectionState::Failed:
            result = updateFailed(input);
            break;
    }

    if (result.type != UiStateResultType::Normal) {
        return result;
    }

    if (input->buttons.down & KEY_B) {
        return UiStateResultType::PopState;
    }

    return UiStateResultType::Normal;
}

void UiStateConnection::enterInitial() {

}

UiStateResult UiStateConnection::updateInitial(SUIInput *input) {
    enterConnecting();
    
    return UiStateResultType::Normal;
}

void UiStateConnection::enterConnecting() {
    logPrint("Starting server connection\n");
    connection_promise_ = application()->server()->connect();
    state_ = ConnectionState::Connecting;
}

UiStateResult UiStateConnection::updateConnecting(SUIInput *input) {
    if (connection_promise_->isResolved()) {
        logPrint("Connection succeeded\n");
        enterPairing();
    }
    else if (connection_promise_->isRejected()) {
        logPrint("Connection failed: %s\n", connection_promise_->rejectedValue()->error.c_str());
        enterFailed();
    }
    
    return UiStateResultType::Normal;
}

void UiStateConnection::enterPairing() {
    logPrint("Starting pairing\n");
    pairing_promise_ = application()->server()->pair();
    state_ = ConnectionState::Pairing;
}

UiStateResult UiStateConnection::updatePairing(SUIInput *input) {
    if (pairing_promise_->isResolved()) {
        logPrint("Pairing succeeded\n");
        return UiStateResult(UiStateResultType::ReplaceState, new UiStateGamesList(application_));
    }
    else if (pairing_promise_->isRejected()) {
        logPrint("Pairing failed: %s\n", counter_->secondsSinceFirstTick(), pairing_promise_->rejectedValue()->error.c_str());
        enterFailed();
    }
    
    return UiStateResultType::Normal;
}

void UiStateConnection::enterFailed() {
    connecting_image_->setVisible(false);
    connection_failed_image_->setVisible(true);
    state_ = ConnectionState::Failed;
}

UiStateResult UiStateConnection::updateFailed(SUIInput *input) {
    return UiStateResultType::Normal;
}