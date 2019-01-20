#include "application.h"

#include <time.h>
#include <string.h>

#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/err.h>

Application::Application() :
    config_{0}
{
    // Initialize Switch services
    SocketInitConfig socketConfig;
    memcpy(&socketConfig, socketGetDefaultInitConfig(), sizeof(SocketInitConfig));
    socketConfig.sb_efficiency = 8;
    socketInitialize(&socketConfig);
    setInitialize();
    plInitialize();
    nxlinkStdio();

    // Initialize OpenSSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    // Seed the OpenSSL PRNG
    size_t seedlen = 2048;
    void *seedbuf = malloc(seedlen);
    csrngGetRandomBytes(seedbuf, seedlen);
    RAND_seed(seedbuf, seedlen);

    // Initialize application members
    server_ = new Server(&config_);
    ui_ = new SUI();
    ui_should_exit_ = false;

    // Parse the global Moonlight settings
    config_parse(MOONLIGHT_DATA_DIR "moonlight.ini", &config_);
    config_.debug_level = 2;

    push_state(new UiStateInitial(this));
}

Application::~Application() {
    if (server_->opened()) {
        server_->close();
    }

    delete server_;
    delete ui_;

    plExit();
    setExit();
    socketExit();
}

void Application::start() {
    // Begin the server worker thread
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

Server *Application::server() {
    return server_;
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