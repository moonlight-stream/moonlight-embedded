#pragma once

#include "ui-state.h"
#include "ui-game-button.h"
#include "../promise.h"
#include "../server.h"
#include "../application-info.h"

class UiStateGamesList : public UiState {
public:
    UiStateGamesList(Application *application);
    ~UiStateGamesList();

    void enter(UiState *parent) override;
    UiStateResult update(SUIInput *input) override;

private:
    void enterInitial();
    UiStateResult updateInitial(SUIInput *input);

    void enterLoading();
    UiStateResult updateLoading(SUIInput *input);

    void enterLoadFailed();
    UiStateResult updateLoadFailed(SUIInput *input);

    void enterLoadSucceeded();
    UiStateResult updateLoadSucceeded(SUIInput *input);

    void handleGameButtonClicked(SUIElement *element, SUIEvent);

    enum class GamesListState {
        Initial,
        Loading,
        LoadFailed,
        LoadSucceeded
    } state_;

    SUIGridContainer *grid_;
    std::vector<UiGameButton *> buttons_;

    promise<std::vector<ApplicationInfo>, ServerError> *apps_promise_;
    std::vector<ApplicationInfo> apps_;
};
