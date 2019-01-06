#include "ui-state-initial.h"
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

    scene_ = new SUIScene(ui());    

    connect_button_ = new SUIButton(scene_, "Connect");
    connect_button_->bounds()->x = ui()->width/2 - connect_button_->bounds()->w/2;
    connect_button_->bounds()->y = 100 + logo_height_ + (ui()->height - SUI_MARGIN_BOTTOM - logo_height_ - 100)/2 - connect_button_->bounds()->h/2;
    connect_button_->setFocused(true);

    settings_button_ = new SUIButton(scene_, "Settings");
    settings_button_->bounds()->x = ui()->width/2 - settings_button_->bounds()->w/2;
    settings_button_->bounds()->y = connect_button_->bounds()->y + connect_button_->bounds()->h + 15;
    settings_button_->setFocused(false);
    
    scene_->addElement(connect_button_);
    scene_->addElement(settings_button_);
}

UiStateInitial::~UiStateInitial() {
    if (logo_texture_)
        SDL_DestroyTexture(logo_texture_);

    if (connect_button_) 
        delete connect_button_;

    if (settings_button_) 
        delete settings_button_;

    if (scene_) 
        delete scene_;
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

    return UiStateResultNormal;
}

void UiStateInitial::render() {
    UiState::render();
    
    // Draw the logo
    ui()->drawTexture(logo_texture_, (ui()->width - logo_width_) / 2, 150, logo_width_, logo_height_);

    // Draw the OK action on the bottom toolbar
    ui()->drawBottomToolbar({
        std::make_tuple(std::string("OK"), SUIToolbarActionA)
    });

    // Draw the main buttons
    scene_->render();

    // Show the FPS meter
    counter_->render();
}