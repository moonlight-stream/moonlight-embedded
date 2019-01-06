#pragma once

#include "ui-state.h"

class UiStateInitial : public UiState {
public:
    UiStateInitial(Application *application);
    ~UiStateInitial();

    // void enter(UiState *parent);
    // void exit();

    UiStateResult update(SUIInput *input) override;
    void render() override;

private:
    SDL_Texture *logo_texture_;
    int logo_width_;
    int logo_height_;

    SUIScene *scene_;
    SUIButton *connect_button_;
    SUIButton *settings_button_;
    // SUIButtonSet buttons_;
};
