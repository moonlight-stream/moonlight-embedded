#include "ui-state.h"
#include "../application.h"

UiState::UiState(Application *application)
    : application_(application) 
{
    counter_ = new UiFpsCounter(ui());
}

UiState::~UiState() {
    delete counter_;
}

void UiState::enter(UiState *parent) {}
void UiState::exit() {}

UiStateResult UiState::update(SUIInput *input) {
    counter_->update();

    return UiStateResultNormal;
}

void UiState::render() {
    
}

SUI *UiState::ui() {
    return application_->ui();
}