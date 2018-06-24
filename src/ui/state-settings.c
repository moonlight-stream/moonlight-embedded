#include "ui-main.h"

static struct {
  int frame;

  SUISidebar sidebar;
} props = {0};

int main_init_settings() {
  sui_sidebar_init(&props.sidebar, 5,
               "General",
               "Stream",
               "Input",
               SUI_SIDEBAR_DIVIDER,
               "About");

  return 0;
}

void main_update_settings(SUIInput *input) {
  int menu = sui_sidebar_update(&props.sidebar, input);

  if (input->buttons.down & KEY_B) {
    ui_state = sui_state_pop(ui_state);
  }
}

void main_render_settings() {
  SDL_SetRenderDrawColor(ui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(ui.renderer);

  // Draw the sidebar
  sui_sidebar_render(&props.sidebar);

  // Draw the heading
  sui_draw_top_header("Moonlight  ›  Settings");

  // Draw the OK and Back actions on the bottom toolbar
  sui_draw_bottom_toolbar(2, "OK", SUIToolbarActionA, "Back", SUIToolbarActionB);

  SDL_RenderPresent(ui.renderer);
}

void main_cleanup_settings() {

}

MoonlightUiState MoonlightUiStateSettings = {
  .state = 1,
  .init = &main_init_settings,
  .update = &main_update_settings,
  .render = &main_render_settings,
  .cleanup = &main_cleanup_settings
};
