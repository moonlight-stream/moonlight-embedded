#include "ui-state-initial.h"
#include "ui-state-connecting.h"
#include "ui-state-games-list.h"
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

    logo_image_ = new SUIImage(logo_texture_);
    logo_image_->bounds()->x = (ui()->width - logo_width_) / 2;
    logo_image_->bounds()->y = 150;
    logo_image_->bounds()->w = logo_width_;
    logo_image_->bounds()->h = logo_height_;

    connect_button_ = new SUIButton("Connect");
    connect_button_->bounds()->x = ui()->width/2 - connect_button_->bounds()->w/2;
    connect_button_->bounds()->y = 100 + logo_height_ + (ui()->height - SUI_MARGIN_BOTTOM - logo_height_ - 100)/2 - connect_button_->bounds()->h/2;
    connect_button_->setFocused(true);

    settings_button_ = new SUIButton("Settings");
    settings_button_->bounds()->x = ui()->width/2 - settings_button_->bounds()->w/2;
    settings_button_->bounds()->y = connect_button_->bounds()->y + connect_button_->bounds()->h + 15;
    settings_button_->setFocused(false);
    
    content()->addChild(logo_image_);
    content()->addChild(connect_button_);
    content()->addChild(settings_button_);

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

    // scene_->update(input);

    // SUIButton *clicked = sui_button_set_update(&buttons_, input, NULL);

    // if (clicked == &connect_button_) {
    //     // application_->push_state(nullptr);
    // }
    // else if (clicked == &settings_button_) {
    //     // application_->push_state(nullptr);
    // }
    // else
    if (input->buttons.down & KEY_B) {
        return UiStateResultExit;
    }

    if (input->buttons.down & KEY_A) {
        application_->push_state(new UiStateConnecting(application_));
    }

    if (input->buttons.down & KEY_X) {
        application_->push_state(new UiStateGamesList(application_));
    }

    return UiStateResultNormal;
}

void UiStateInitial::render() {
    UiState::render();
}