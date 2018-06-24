#include "ui-main.h"

static struct {
  int frame;

  Sidebar sidebar;
} props = {0};

int main_init_settings() {
  sidebar_init(&props.sidebar, 5,
               "General",
               "Stream",
               "Input",
               SIDEBAR_DIVIDER,
               "About");

  return 0;
}

void main_update_settings(Input *input) {
  int menu = sidebar_update(&props.sidebar, input);

  if (input->buttons.down & KEY_B) {
    ui_state = state_pop(ui_state);
  }
}

void main_render_settings() {
  SDL_SetRenderDrawColor(ui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(ui.renderer);

  // Draw the sidebar
  sidebar_render(&props.sidebar);

  // Draw the heading
  draw_top_header("Moonlight  ›  Settings");

  // Draw the OK and Back actions on the bottom toolbar
  draw_bottom_toolbar(2, "OK", ToolbarActionA, "Back", ToolbarActionB);

  SDL_RenderPresent(ui.renderer);
}

void main_cleanup_settings() {

}
