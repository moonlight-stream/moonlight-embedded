#pragma once

#include <switchui.h>

#include <Limelight.h>
#include "../config.h"
#include "../connection.h"

typedef struct _MoonlightUiState {
  int state;
  int (*init)();
  void (*update)(SUIInput *input);
  void (*render)();
  void (*cleanup)();
} MoonlightUiState;

SUIState *ui_state;
bool ui_shouldExitApp;

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


// Client and server configuration
extern CONFIGURATION config;
extern SERVER_DATA server;

