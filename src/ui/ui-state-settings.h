#pragma once

#include "ui-state.h"

class UiStateSettings : public UiState {
public:
    UiStateSettings(Application *application);
    ~UiStateSettings();

    UiStateResult update(SUIInput *input) override;

private:
    SUIGridContainer *state_grid_;
    SUIGridContainer *menu_sidebar_grid_;
    SUIGridContainer *menu_content_grid_;
};
