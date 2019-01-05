#include "ui-main.h"

int ui_main_init() {
  ui_state = sui_state_push(NULL, &MoonlightUiStateInitial);
  ui_shouldExitApp = false;
  memset(&ui_counter, 0, sizeof(FpsCounter));

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

      // Calculate FPS
      ui_update_fps(&ui_counter);
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

void ui_update_fps(FpsCounter *counter) {
    uint64_t now = svcGetSystemTick();
    if (counter->_last_time) {
        uint64_t delta = now - counter->_last_time;

        counter->_tick_sum -= counter->_tick_list[counter->_tick_index];
        counter->_tick_sum += delta;
        counter->_tick_list[counter->_tick_index] = delta;

        if (++counter->_tick_index == MAX_TICK_SAMPLES)
            counter->_tick_index = 0;

        counter->ticks_per_frame = counter->_tick_sum / MAX_TICK_SAMPLES;
        counter->frames_per_second = TICKS_PER_SECOND / counter->ticks_per_frame;
    }

    counter->_last_time = now;
    counter->frame++;
}

void ui_draw_fps(FpsCounter *counter) {
    uint32_t fpsColor = RGBA8(180, 0, 0, 255);
    char fpsText[20];
    int fpsTextWidth, fpsTextHeight;
    snprintf(fpsText, 20, "FPS: %ld", counter->frames_per_second);
    sui_measure_text(ui.fontSmall, fpsText, &fpsTextWidth, &fpsTextHeight);

    boxColor(ui.renderer, 8, 8, 18 + fpsTextWidth + 2, 10 + fpsTextHeight + 2, RGBA8(255, 255, 255, 200));
    sui_draw_text(ui.fontSmall, fpsText, 18, 10, fpsColor, false, -1);

    short progressDigit = counter->frame % fpsTextHeight;
    boxColor(ui.renderer, 10, 10, 12, 10 + progressDigit, fpsColor);
}
