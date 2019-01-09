#include "ui-state-connecting.h"
#include "../application.h"

#include "moonlight_switch_connecting_png.h"

UiStateConnecting::UiStateConnecting(Application *application) 
: UiState(application) 
{
    connecting_texture_ = ui()->loadPNG(moonlight_switch_connecting_png, moonlight_switch_connecting_png_size);
    
    if (!connecting_texture_) {
        fprintf(stderr, "[GUI, connecting] Could not load connecting image: %s\n", SDL_GetError());
        return;
    }
    SDL_QueryTexture(connecting_texture_, NULL, NULL, &connecting_width_, &connecting_height_);

    connecting_image_ = new SUIImage(connecting_texture_);
    connecting_image_->bounds().x = (ui()->width - connecting_width_) / 2;
    connecting_image_->bounds().y = SUI_MARGIN_TOP + 75;
    connecting_image_->bounds().w = connecting_width_;
    connecting_image_->bounds().h = connecting_height_;

    content()->addChild(connecting_image_);

    header_text_ = "Moonlight  ›  Connection";
    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem("OK", SUIToolbarActionA));
    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem("Exit", SUIToolbarActionB));
}

UiStateConnecting::~UiStateConnecting() {
    if (connecting_texture_)
        SDL_DestroyTexture(connecting_texture_);

    if (connecting_image_) 
        delete connecting_image_;
}

UiStateResult UiStateConnecting::update(SUIInput *input) {
    UiState::update(input);

    if (input->buttons.down & KEY_B) {
        return UiStateResultExit;
    }

    return UiStateResultNormal;
}

void UiStateConnecting::render() {
    UiState::render();
}