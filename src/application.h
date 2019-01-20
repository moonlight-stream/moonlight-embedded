#pragma once

#include "util.h"
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
    void pushState(UiState *state);
    void popState();

    void log(const char *format, ...);

private:
    CONFIGURATION config_;
    Server *server_;

    // UI
    SUI *ui_;
    std::vector<UiState *> ui_states_;
    bool ui_should_exit_;

    Mutex log_mutex_;
};
