#include "ui-state.h"
#include "../application.h"

UiState::UiState(Application *application)
    : application_(application)
{
    stage_ = new SUIStage(ui(), "stage");
    content_ = new SUIContainer("content");
    overlay_ = new SUIContainer("overlay");

    // Add the containers to the stage
    stage_->addChild(content_);
    stage_->addChild(overlay_);
    
    // Position and resize the containers
    content_->setBounds(stage_->bounds());
    overlay_->setBounds(stage_->bounds());

    // Add the counter to the overlay
    counter_ = new SUIFpsCounter("fps-counter");
    overlay_->addChild(counter_);
}

UiState::~UiState() {
    delete counter_;
    delete overlay_;
    delete content_;
    delete stage_;
}

SUI *UiState::ui() {
    return application_->ui();
}

void UiState::enter(UiState *parent) {

}

void UiState::exit() {

}

UiStateResult UiState::update(SUIInput *input) {
    stage_->update(input);

    return UiStateResultNormal;
}

void UiState::render() {
    stage_->render();

    if (header_text_.size()) {
        overlay_->graphics()->drawTopHeader(header_text_);
    }

    if (toolbar_items_.size()) {
        overlay_->graphics()->drawBottomToolbar(toolbar_items_);
    }
}