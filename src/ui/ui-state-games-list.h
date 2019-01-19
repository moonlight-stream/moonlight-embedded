#pragma once

#include "ui-state.h"
#include "ui-game-button.h"

class UiStateGamesList : public UiState {
public:
    UiStateGamesList(Application *application);
    ~UiStateGamesList();

    UiStateResult update(SUIInput *input) override;

private:
    SUIGridContainer *grid_;
    std::vector<UiGameButton *> buttons_;
};
