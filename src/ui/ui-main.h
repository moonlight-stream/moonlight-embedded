#pragma once

#include <switchui.h>

#include <Limelight.h>
#include "../config.h"
#include "../connection.h"

enum MoonlightUiState {
  StateInitial,
  StateSettings,
  StateConnecting,
  StateConnectionFailed,
  StateGamesList,
  StateStreaming
};

State *ui_state;
bool ui_shouldExitApp;


int ui_main_init();
void ui_main_loop();
void ui_main_cleanup();

int main_init_initial();
void main_update_initial(Input *input);
void main_render_initial();
void main_cleanup_initial();

int main_init_settings();
void main_update_settings(Input *input);
void main_render_settings();
void main_cleanup_settings();

int main_init_connecting();
void main_update_connecting(Input *input);
void main_render_connecting();
void main_cleanup_connecting();

int main_init_connection_failed();
void main_update_connection_failed(Input *input);
void main_render_connection_failed();
void main_cleanup_connection_failed();

int main_init_games_list();
void main_update_games_list(Input *input);
void main_render_games_list();
void main_cleanup_games_list();

int main_init_streaming();
void main_update_streaming(Input *input);
void main_render_streaming();
void main_cleanup_streaming();
void main_set_streaming_game(PAPP_LIST game);

// Client and server configuration
extern CONFIGURATION config;
extern SERVER_DATA server;

