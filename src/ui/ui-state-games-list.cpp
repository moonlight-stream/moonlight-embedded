#include "ui-state-games-list.h"
#include "ui-state-streaming.h"
#include "../application.h"

UiStateGamesList::UiStateGamesList(Application *application) 
    : UiState(application),
      state_(GamesListState::Initial)
{
    header_text_ = "Moonlight  ›  Games";
    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem("OK", SUIToolbarAction::A));
    toolbar_items_.push_back(content()->graphics()->makeToolbarActionItem("Exit", SUIToolbarAction::B));

    grid_ = new SUIGridContainer("game-grid");
    grid_->bounds().x = SUI_MARGIN_SIDE;
    grid_->bounds().y = SUI_MARGIN_TOP + SUI_MARGIN_SIDE;
    grid_->bounds().w = ui()->width - 2*SUI_MARGIN_SIDE;
    grid_->bounds().h = ui()->height - 2*(SUI_MARGIN_TOP + SUI_MARGIN_SIDE);
    grid_->minimumColumnSpacing() = 20;
    grid_->minimumRowSpacing() = 20;
    content()->addChild(grid_);
}

UiStateGamesList::~UiStateGamesList() {
    for (auto button : buttons_) {
        delete button;
    }
}

void UiStateGamesList::enter(UiState *parent) {
    enterInitial();
}

UiStateResult UiStateGamesList::update(SUIInput *input) {
    UiState::update(input);

    UiStateResult result;

    switch (state_) {
        case GamesListState::Initial:
            result = updateInitial(input);
            break;

        case GamesListState::Loading:
            result = updateLoading(input);
            break;

        case GamesListState::LoadFailed:
            result = updateLoadFailed(input);
            break;

        case GamesListState::LoadSucceeded:
            result = updateLoadSucceeded(input);
            break;
    }

    if (result.type != UiStateResultType::Normal) {
        return result;
    }

    if (input->buttons.down & KEY_B) {
        return UiStateResultType::PopState;
    }

    return UiStateResultType::Normal;
}

void UiStateGamesList::enterInitial() {

}

UiStateResult UiStateGamesList::updateInitial(SUIInput *input) {
    // Begin loading the streaming applications and switch to the loading state
    apps_promise_ = application()->server()->apps();
    enterLoading();

    return UiStateResultType::Normal;
}

void UiStateGamesList::enterLoading() {
    state_ = GamesListState::Loading;
}

UiStateResult UiStateGamesList::updateLoading(SUIInput *input) {
    // Check if the application list has completed loading
    if (apps_promise_->isResolved()) {
        enterLoadSucceeded();
    }
    else if (apps_promise_->isRejected()) {
        enterLoadFailed();
    }

    return UiStateResultType::Normal;
}

void UiStateGamesList::enterLoadFailed() {
    state_ = GamesListState::LoadFailed;
}

UiStateResult UiStateGamesList::updateLoadFailed(SUIInput *input) {
    return UiStateResultType::Normal;
}

void UiStateGamesList::enterLoadSucceeded() {
    apps_ = apps_promise_->resolvedValue().value();
   
    for (int i = 0; i < apps_.size(); i++) {
        auto app = apps_[i];

        int r = i / 5;
        int c = i % 5;
        char label[20];
        snprintf(label, sizeof(label), "game-%d", app.id);

        UiGameButton *button = new UiGameButton(label);
        button->app() = &apps_[i];
        button->text() = app.name;
        button->addListener(SUIEvent::Click, std::bind(&UiStateGamesList::handleGameButtonClicked, this, _1, _2));
        grid_->addChildCell(button, r, c);
        buttons_.push_back(button);
    }

    grid_->layout();

    state_ = GamesListState::LoadSucceeded;
}

UiStateResult UiStateGamesList::updateLoadSucceeded(SUIInput *input) {
    return UiStateResultType::Normal;
}

void UiStateGamesList::handleGameButtonClicked(SUIElement *element, SUIEvent) {
    UiGameButton *button = static_cast<UiGameButton *>(element);
    logPrint("Starting app: %s\n", button->app()->name.c_str());
    
    application()->pushState(new UiStateStreaming(application(), button->app()));
}