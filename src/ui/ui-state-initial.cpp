#include "ui-state-initial.h"
#include "ui-state-connection.h"
#include "ui-state-settings.h"
#include "../application.h"

// Binary file that contains the logo payload
#include "moonlight_switch_logo_png.h"

UiStateInitial::UiStateInitial(Application *application) 
    : UiState(application) 
{
    logo_texture_ = ui()->loadPNG(moonlight_switch_logo_png, moonlight_switch_logo_png_size);
    
    if (!logo_texture_) {
        fprintf(stderr, "[GUI, initial] Could not load logo: %s\n", SDL_GetError());
        return;
    }
    SDL_QueryTexture(logo_texture_, NULL, NULL, &logo_width_, &logo_height_);

    logo_image_ = new SUIImage("image-logo", logo_texture_);
    logo_image_->bounds().x = (ui()->width - logo_width_) / 2;
    logo_image_->bounds().y = 150;
    logo_image_->bounds().w = logo_width_;
    logo_image_->bounds().h = logo_height_;

    connect_button_ = new SUIButton("button-connect", "Connect");
    connect_button_->bounds().x = ui()->width/2 - connect_button_->bounds().w/2;
    connect_button_->bounds().y = 100 + logo_height_ + (ui()->height - SUI_MARGIN_BOTTOM - logo_height_ - 100)/2 - connect_button_->bounds().h/2;
    connect_button_->addListener(SUIEventClick, std::bind(&UiStateInitial::handleConnectClick, this, _1, _2));

    settings_button_ = new SUIButton("button-settings", "Settings");
    settings_button_->bounds().x = ui()->width/2 - settings_button_->bounds().w/2;
    settings_button_->bounds().y = connect_button_->bounds().y + connect_button_->bounds().h + 15;
    settings_button_->addListener(SUIEventClick, std::bind(&UiStateInitial::handleSettingsClick, this, _1, _2));

    content()->addChild(logo_image_);
    content()->addChild(connect_button_);
    content()->addChild(settings_button_);

    stage()->setFocusedElement(connect_button_);

    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem(std::string("OK"), SUIToolbarActionA));
    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem(std::string("Exit"), SUIToolbarActionB));
}

UiStateInitial::~UiStateInitial() {
    if (logo_texture_)
        SDL_DestroyTexture(logo_texture_);

    if (logo_image_)
        delete logo_image_;

    if (connect_button_) 
        delete connect_button_;

    if (settings_button_) 
        delete settings_button_;
}

UiStateResult UiStateInitial::update(SUIInput *input) {
    UiState::update(input);

    if (input->buttons.down & KEY_B) {
        return UiStateResultExit;
    }

    return UiStateResultNormal;
}

void UiStateInitial::handleConnectClick(SUIElement *, SUIEvent) {
    application_->push_state(new UiStateConnection(application_));
}

void UiStateInitial::handleSettingsClick(SUIElement *, SUIEvent) {
    application_->push_state(new UiStateSettings(application_));
}