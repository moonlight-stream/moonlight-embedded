#pragma once

#include "ui-state.h"

using namespace std::placeholders;

class UiStateInitial : public UiState {
public:
    UiStateInitial(Application *application);
    ~UiStateInitial();

    UiStateResult update(SUIInput *input) override;

private:
    void handleConnectClick(SUIElement *, SUIEvent);
    void handleSettingsClick(SUIElement *, SUIEvent);

    SDL_Texture *logo_texture_;
    int logo_width_;
    int logo_height_;

    SUIImage *logo_image_;
    SUIButton *connect_button_;
    SUIButton *settings_button_;
};
