#include "application.h"

#include <time.h>
#include <string.h>

Application::Application() {
    // Initialize Switch services
    SocketInitConfig socketConfig;
    memcpy(&socketConfig, socketGetDefaultInitConfig(), sizeof(SocketInitConfig));
    socketConfig.sb_efficiency = 8;
    socketInitialize(&socketConfig);
    setInitialize();
    plInitialize();
    nxlinkStdio();

    server_ = std::make_unique<Server>(&config_);
    ui_ = new SUI();
    ui_should_exit_ = false;
    
    push_state(new UiStateInitial(this));
}

Application::~Application() {
    if (server_->opened()) {
        server_->close();
    }

    delete ui_;

    plExit();
    setExit();
    socketExit();
}

void Application::start() {
    // Begin the connection to the server
    server_->open();

    // Begin the UI main loop
    while (appletMainLoop() && !ui_should_exit_) {
        if (ui_states_.size() == 0) 
            break;

        SUIInput *input = ui_->inputPoll(CONTROLLER_P1_AUTO);
        UiState *state = ui_states_.back();
        UiStateResult state_result = state->update(input);

        if (state_result == UiStateResultNormal) {
            SDL_SetRenderDrawColor(ui()->renderer, 0xeb, 0xeb, 0xeb, 0xff);
            SDL_RenderClear(ui()->renderer);
            state->render();
            SDL_RenderPresent(ui()->renderer);
        }
        else if (state_result == UiStateResultExit) {
            pop_state();
        }
    }
}

void Application::stop() {
    ui_should_exit_ = true;
}

SUI *Application::ui() {
    return ui_;
}

void Application::push_state(UiState *state) {
    // Notify the new state that it is being entered
    UiState *parent = ui_states_.size() ? ui_states_.back() : nullptr;
    state->enter(parent);
    ui_states_.push_back(state);
}

void Application::pop_state() {
    // Notify the current state that it is being exited
    UiState *state = ui_states_.back();

    state->exit();
    ui_states_.pop_back();
    delete state;
}