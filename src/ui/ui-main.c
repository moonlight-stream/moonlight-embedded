#include "ui-main.h"

int ui_main_init() {
  ui_state = sui_state_push(NULL, &MoonlightUiStateInitial);
  ui_shouldExitApp = false;

  MoonlightUiStateInitial.init();
  MoonlightUiStateSettings.init();
  MoonlightUiStateConnecting.init();
  MoonlightUiStateConnectionFailed.init();
  MoonlightUiStateGamesList.init();
  MoonlightUiStateStreaming.init();

  return 0;
}

void ui_main_loop() {
  while(appletMainLoop() && !ui_shouldExitApp)
  {
      SUIInput *input = sui_input_poll(CONTROLLER_P1_AUTO);
      MoonlightUiState *state = ui_state->state;
      state->update(input);
      state->render();
  }
}

void ui_main_cleanup() {
  MoonlightUiStateInitial.cleanup();
  MoonlightUiStateSettings.cleanup();
  MoonlightUiStateConnecting.cleanup();
  MoonlightUiStateConnectionFailed.cleanup();
  MoonlightUiStateGamesList.cleanup();
  MoonlightUiStateStreaming.cleanup();
}
