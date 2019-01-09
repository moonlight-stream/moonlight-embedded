#pragma once

#include "ui-state.h"

class UiStateGamesList : public UiState {
public:
    UiStateGamesList(Application *application);
    ~UiStateGamesList();

    UiStateResult update(SUIInput *input) override;
    void render() override;

private:
    SUIGridContainer *grid_;
    std::vector<SUIElement *> buttons_;
};
