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

    /**
     * Connecting/Pairing scene
     */

    connecting_image_ = new SUIImage("image-connecting", connecting_texture_);
    connecting_image_->bounds().x = (ui()->width - connecting_width_) / 2;
    connecting_image_->bounds().y = SUI_MARGIN_TOP + 75;
    connecting_image_->bounds().w = connecting_width_;
    connecting_image_->bounds().h = connecting_height_;

    connecting_guide_text_ = new SUIText("connecting-guide-text");
    connecting_guide_text_->text() = "Enter the following PIN on the target computer:";
    connecting_guide_text_->font() = ui()->font_normal;
    connecting_guide_text_->centered() = true;
    connecting_guide_text_->bounds() = {
        0,
        connecting_image_->bounds().y + connecting_image_->bounds().h + 90, 
        content()->bounds().w,
        content()->graphics()->measureTextAscent(connecting_guide_text_->font()) + 10
    };

    connecting_pin_text_ = new SUIText("connecting-pin-text");
    connecting_pin_text_->text() = application_->server()->pin();
    connecting_pin_text_->font() = ui()->font_massive;
    connecting_pin_text_->centered() = true;
    connecting_pin_text_->bounds() = {
        0,
        connecting_guide_text_->bounds().y + connecting_guide_text_->bounds().h + 60, 
        content()->bounds().w,
        content()->graphics()->measureTextAscent(connecting_pin_text_->font()) + 10
    };

    connecting_scene_ = new SUIContainer("connecting-scene");
    connecting_scene_->bounds() = content()->bounds();
    connecting_scene_->setVisible(true);
    connecting_scene_->addChild(connecting_image_);
    connecting_scene_->addChild(connecting_guide_text_);
    connecting_scene_->addChild(connecting_pin_text_);

    /**
     * Connection failed scene
     */

    connection_failed_image_ = new SUIImage("image-connection-failed", connection_failed_texture_);
    connection_failed_image_->bounds().x = (ui()->width - connection_failed_width_) / 2;
    connection_failed_image_->bounds().y = SUI_MARGIN_TOP + 75;
    connection_failed_image_->bounds().w = connection_failed_width_;
    connection_failed_image_->bounds().h = connection_failed_height_;

    connection_failed_guide_text_ = new SUIText("connection-failed-guide-text");
    connection_failed_guide_text_->text() = "Unable to connect to the target computer:";
    connection_failed_guide_text_->font() = ui()->font_normal;
    connection_failed_guide_text_->centered() = true;
    connection_failed_guide_text_->bounds() = {
        0,
        connecting_image_->bounds().y + connecting_image_->bounds().h + 90, 
        content()->bounds().w,
        content()->graphics()->measureTextAscent(connection_failed_guide_text_->font()) + 10
    };

    connection_failed_error_text_ = new SUIText("connection-failed-error-text");
    connection_failed_error_text_->text() = "";
    connection_failed_error_text_->color() = RGBA8(128, 128, 128, 255);
    connection_failed_error_text_->font() = ui()->font_normal;
    connection_failed_error_text_->centered() = true;
    connection_failed_error_text_->bounds() = {
        0,
        connection_failed_guide_text_->bounds().y + connection_failed_guide_text_->bounds().h + 10, 
        content()->bounds().w,
        content()->graphics()->measureTextAscent(connection_failed_error_text_->font()) + 10
    };

    connection_failed_retry_button_ = new SUIButton("connection-failed-retry-button");
    connection_failed_retry_button_->text() = "Retry";
    connection_failed_retry_button_->bounds().x = content()->bounds().w/2 - connection_failed_retry_button_->bounds().w/2;
    connection_failed_retry_button_->bounds().y = connection_failed_error_text_->bounds().y + connection_failed_error_text_->bounds().h + 20;
    connection_failed_retry_button_->addListener(SUIEvent::Click, std::bind(&UiStateConnection::handleRetryButtonClicked, this, _1, _2));

    connection_failed_scene_ = new SUIContainer("connection-failed-scene");
    connection_failed_scene_->bounds() = content()->bounds();
    connection_failed_scene_->setVisible(false);
    connection_failed_scene_->addChild(connection_failed_image_);
    connection_failed_scene_->addChild(connection_failed_guide_text_);
    connection_failed_scene_->addChild(connection_failed_error_text_);
    connection_failed_scene_->addChild(connection_failed_retry_button_);

    content()->addChild(connecting_scene_);
    content()->addChild(connection_failed_scene_);

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
    connecting_scene_->setVisible(false);
    connection_failed_scene_->setVisible(true);
    stage()->setFocusedElement(connection_failed_retry_button_);

    if (connection_promise_->isRejected()) {
        connection_failed_error_text_->text() = connection_promise_->rejectedValue()->error;
    }
    else if (pairing_promise_->isRejected()) {
        connection_failed_error_text_->text() = pairing_promise_->rejectedValue()->error;
    }

    state_ = ConnectionState::Failed;
}

UiStateResult UiStateConnection::updateFailed(SUIInput *input) {
    return UiStateResultType::Normal;
}

void UiStateConnection::handleRetryButtonClicked(SUIElement *, SUIEvent) {
    
}