#pragma once

#include "server.h"
#include "promise.h"
#include "ui/ui-state.h"
#include "ui/ui-state-initial.h"
#include "ui/switch/sui.h"

#include <switch.h>

#include <memory>
#include <vector>

#define MOONLIGHT_DATA_DIR "sdmc:/switch/moonlight-switch/"

class Application
{
public:
    Application();
    ~Application();

    void start();
    void stop();

    Server *server();
    SUI *ui();
    void push_state(UiState *state);
    void pop_state();

private:
    CONFIGURATION config_;
    std::unique_ptr<Server> server_;

    // UI
    SUI *ui_;
    std::vector<UiState *> ui_states_;
    bool ui_should_exit_;
};
