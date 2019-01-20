#include "ui-state-connection.h"
#include "../application.h"

#include "moonlight_switch_connecting_png.h"
#include "moonlight_switch_connection_failed_png.h"

UiStateConnection::UiStateConnection(Application *application) 
    : UiState(application),
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
    connection_failed_image_->bounds().x = (ui()->width - connecting_width_) / 2;
    connection_failed_image_->bounds().y = SUI_MARGIN_TOP + 75;
    connection_failed_image_->bounds().w = connecting_width_;
    connection_failed_image_->bounds().h = connecting_height_;

    content()->addChild(connecting_image_);

    header_text_ = "Moonlight  ›  Connection";
    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem("OK", SUIToolbarActionA));
    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem("Exit", SUIToolbarActionB));
}

UiStateConnection::~UiStateConnection() {
    if (connecting_texture_)
        SDL_DestroyTexture(connecting_texture_);

    if (connecting_image_) 
        delete connecting_image_;
}

void UiStateConnection::enter(UiState *parent) {
    printf("[%f] Starting server connection\n", counter_->secondsSinceFirstTick());
    connection_promise_ = application()->server()->connect();
    printf("[%f] Got server connection promise\n", counter_->secondsSinceFirstTick());
}

void UiStateConnection::exit() {

}

UiStateResult UiStateConnection::update(SUIInput *input) {
    UiState::update(input);

    if (connection_promise_) {
        if (connection_promise_->isResolved()) {
            printf("[%f] Connection succeeded\n", counter_->secondsSinceFirstTick());

            if (pairing_promise_ == nullptr) {
                printf("[%f] Starting pairing\n", counter_->secondsSinceFirstTick());
                pairing_promise_ = application()->server()->pair();
            }
        }
        else if (connection_promise_->isRejected()) {
            printf("[%f] Connection failed: %s\n", counter_->secondsSinceFirstTick(), connection_promise_->rejectedValue()->error.c_str());
        }
    }

    if (pairing_promise_) {
        if (pairing_promise_->isResolved()) {
            printf("[%f] Pairing succeeded\n", counter_->secondsSinceFirstTick());
        }
        else if (pairing_promise_->isRejected()) {
            printf("[%f] Pairing failed: %s\n", counter_->secondsSinceFirstTick(), pairing_promise_->rejectedValue()->error.c_str());
        }
    }

    if (input->buttons.down & KEY_B) {
        return UiStateResultExit;
    }

    return UiStateResultNormal;
}