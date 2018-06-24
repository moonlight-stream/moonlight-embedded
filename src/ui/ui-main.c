#include "ui-main.h"

enum MainState state;
bool shouldExitApp;

int ui_main_init() {
  state = StateInitial;
  shouldExitApp = false;

  main_init_initial();
  main_init_settings();
  main_init_connecting();
  main_init_connection_failed();
  main_init_games_list();
  main_init_streaming();

  return 0;
}

void ui_main_loop() {
  while(appletMainLoop() && !shouldExitApp)
  {
      Input *input = switch_input_poll(CONTROLLER_P1_AUTO);

      switch (state) {
      case StateInitial:
        main_update_initial(input);
        main_render_initial();
        break;

      case StateSettings:
        main_update_settings(input);
        main_render_settings();
        break;

      case StateConnecting:
        main_update_connecting(input);
        main_render_connecting();
        break;

      case StateConnectionFailed:
        main_update_connection_failed(input);
        main_render_connection_failed();
        break;

      case StateGamesList:
        main_update_games_list(input);
        main_render_games_list();
        break;

      case StateStreaming:
        main_update_streaming(input);
        main_render_streaming();
        break;
      }
  }
}

void ui_main_cleanup() {
  main_cleanup_initial();
  main_cleanup_connecting();
  main_cleanup_connection_failed();
  main_cleanup_games_list();
  main_cleanup_streaming();
}
