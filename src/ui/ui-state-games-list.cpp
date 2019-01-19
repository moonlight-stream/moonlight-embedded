#include "ui-state-games-list.h"
#include "../application.h"

UiStateGamesList::UiStateGamesList(Application *application) 
: UiState(application) 
{
    header_text_ = "Moonlight  ›  Games";
    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem("OK", SUIToolbarActionA));
    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem("Exit", SUIToolbarActionB));

    grid_ = new SUIGridContainer("game-grid");
    grid_->bounds().x = SUI_MARGIN_SIDE;
    grid_->bounds().y = SUI_MARGIN_TOP + SUI_MARGIN_SIDE;
    grid_->bounds().w = ui()->width - 2*SUI_MARGIN_SIDE;
    grid_->bounds().h = ui()->height - 2*(SUI_MARGIN_TOP + SUI_MARGIN_SIDE);
    grid_->minimumColumnSpacing() = 20;
    grid_->minimumRowSpacing() = 20;
    content()->addChild(grid_);
    
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 5; c++) {
            char label[20];
            snprintf(label, sizeof(label), "Game (%d-%d)", r + 1, c + 1);

            UiGameButton *button = new UiGameButton(label);
            button->text() = label;
            grid_->addChildCell(button, r, c);
            buttons_.push_back(button);
        }
    }

    grid_->layout();
}

UiStateGamesList::~UiStateGamesList() {
    for (auto button : buttons_) {
        delete button;
    }
}

UiStateResult UiStateGamesList::update(SUIInput *input) {
    UiState::update(input);

    if (input->buttons.down & KEY_B) {
        return UiStateResultExit;
    } 

    return UiStateResultNormal;
}