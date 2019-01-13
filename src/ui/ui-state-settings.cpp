#include "ui-state-settings.h"
#include "../application.h"

UiStateSettings::UiStateSettings(Application *application) 
: UiState(application) 
{
    header_text_ = "Moonlight  ›  Settings";
    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem("OK", SUIToolbarActionA));
    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem("Exit", SUIToolbarActionB));

    state_grid_ = new SUIGridContainer("state-grid");
    state_grid_->bounds().x = SUI_MARGIN_SIDE;
    state_grid_->bounds().y = SUI_MARGIN_TOP + SUI_MARGIN_SIDE;
    state_grid_->bounds().w = ui()->width - 2*SUI_MARGIN_SIDE;
    state_grid_->bounds().h = ui()->height - 2*(SUI_MARGIN_TOP + SUI_MARGIN_SIDE);
    content()->addChild(state_grid_);

    menu_sidebar_grid_ = new SUIGridContainer("menu-sidebar-grid");
    menu_sidebar_grid_->bounds().x = 0;
    menu_sidebar_grid_->bounds().y = 0;
    menu_sidebar_grid_->bounds().w = 410 - 78;
    menu_sidebar_grid_->bounds().h = state_grid_->bounds().h;
    state_grid_->addChildCell(menu_sidebar_grid_, 0, 0);
    
    for (int i = 0; i < 5; i++) {
        char label[20];
        snprintf(label, sizeof(label), "Setting %d", i);

        SUIElement *button = new SUIButton(label, label);
        menu_sidebar_grid_->addChildCell(button, i, 0);
    }

    menu_content_grid_ = new SUIGridContainer("menu-content-grid");
    menu_content_grid_->bounds().x = 410;
    menu_content_grid_->bounds().y = 0;
    menu_content_grid_->bounds().w = state_grid_->bounds().w - 410;
    menu_content_grid_->bounds().h = state_grid_->bounds().h;
    state_grid_->addChildCell(menu_content_grid_, 0, 1);

    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 5; c++) {
            char label[20];
            snprintf(label, sizeof(label), "setting-button-%d-%d", r, c);

            SUIElement *button = new SUIButton(label, "Test");
            button->bounds().w = 100;
            button->bounds().h = 100;
            menu_content_grid_->addChildCell(button, r, c);
        }
    }

    menu_content_grid_->layout();
    menu_sidebar_grid_->layout();
    state_grid_->layout();
}

UiStateSettings::~UiStateSettings() {
    
}

UiStateResult UiStateSettings::update(SUIInput *input) {
    UiState::update(input);

    if (input->buttons.down & KEY_B) {
        return UiStateResultExit;
    } 

    return UiStateResultNormal;
}