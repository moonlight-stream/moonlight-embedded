#include "ui-state.h"
#include "../application.h"

UiState::UiState(Application *application)
    : application_(application)
{
    stage_ = new SUIElement(ui());
    content_ = new SUIElement(ui());
    overlay_ = new SUIElement(ui());

    // Add the counter to the overlay
    counter_ = new SUIFpsCounter();
    overlay_->addChild(counter_);
    stage_->addChild(content_);
    stage_->addChild(overlay_);
}

UiState::~UiState() {
    delete counter_;
    delete overlay_;
    delete content_;
    delete stage_;
}

void UiState::enter(UiState *parent) {}
void UiState::exit() {}

UiStateResult UiState::update(SUIInput *input) {
    stage_->update(input);

    return UiStateResultNormal;
}

void UiState::render() {
    stage_->render();

    if (header_text_.size()) {
        content_->graphics()->drawTopHeader(header_text_);
    }

    if (toolbar_items_.size()) {
        content_->graphics()->drawBottomToolbar(toolbar_items_);
    }
}

SUI *UiState::ui() {
    return application_->ui();
}