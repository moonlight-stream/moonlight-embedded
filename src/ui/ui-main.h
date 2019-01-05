#pragma once

#include <switch.h>
#include <switchui.h>

#include <Limelight.h>
#include "../config.h"
#include "../connection.h"

#define MAX_TICK_SAMPLES 100
#define TICKS_PER_SECOND 19200000

typedef struct _FpsCounter {
    uint64_t frame;
    uint64_t frames_per_second;
    uint64_t ticks_per_frame;

    uint64_t _last_time;
    uint64_t _tick_index;
    uint64_t _tick_sum;
    uint64_t _tick_list[MAX_TICK_SAMPLES];
} FpsCounter;

typedef struct _MoonlightUiState {
  int state;
  int (*init)();
  void (*update)(SUIInput *input);
  void (*render)();
  void (*cleanup)();
} MoonlightUiState;

SUIState *ui_state;
bool ui_shouldExitApp;
FpsCounter ui_counter;

extern MoonlightUiState MoonlightUiStateInitial;
extern MoonlightUiState MoonlightUiStateSettings;
extern MoonlightUiState MoonlightUiStateConnecting;
extern MoonlightUiState MoonlightUiStateConnectionFailed;
extern MoonlightUiState MoonlightUiStateGamesList;
extern MoonlightUiState MoonlightUiStateStreaming;
void main_set_streaming_game(PAPP_LIST game);

int ui_main_init();
void ui_main_loop();
void ui_main_cleanup();
MoonlightUiState *ui_get_state();

void ui_update_fps(FpsCounter *counter);
void ui_draw_fps(FpsCounter *counter);

// Client and server configuration
extern CONFIGURATION config;
extern SERVER_DATA server;

