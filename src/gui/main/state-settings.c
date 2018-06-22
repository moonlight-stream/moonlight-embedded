#include "gui-main.h"

#define MENU_SIDEBAR_WIDTH           410
#define MENU_SIDEBAR_BUTTON_WIDTH    (MENU_SIDEBAR_WIDTH - 2*MARGIN_SIDE)
#define MENU_SIDEBAR_BUTTON_SPACING  5
#define MENU_SIDEBAR_COLOR           RGBA8(0xf0, 0xf0, 0xf0, 0xff)

static struct {
  int frame;

  Scene sidebarScene;
  ButtonSet sidebarButtons;

  Button sidebarGeneralButton;
  Button sidebarStreamButton;
  Button sidebarInputButton;
} props = {0};

int main_init_settings() {
  button_init(&props.sidebarGeneralButton);
  props.sidebarGeneralButton.e.bounds.x = MARGIN_SIDE;
  props.sidebarGeneralButton.e.bounds.y = MARGIN_TOP + MARGIN_SIDE + 1 + 0*(BUTTON_DEFAULT_HEIGHT + MENU_SIDEBAR_BUTTON_SPACING);
  props.sidebarGeneralButton.e.bounds.width = MENU_SIDEBAR_BUTTON_WIDTH;
  props.sidebarGeneralButton.e.bounds.height = BUTTON_DEFAULT_HEIGHT;
  props.sidebarGeneralButton.text = "General";
  props.sidebarGeneralButton.contentRenderer = &button_renderer_content_menu;

  button_init(&props.sidebarStreamButton);
  props.sidebarStreamButton.e.bounds.x = MARGIN_SIDE;
  props.sidebarStreamButton.e.bounds.y = MARGIN_TOP + MARGIN_SIDE + 1 + 1*(BUTTON_DEFAULT_HEIGHT + MENU_SIDEBAR_BUTTON_SPACING);
  props.sidebarStreamButton.e.bounds.width = MENU_SIDEBAR_BUTTON_WIDTH;
  props.sidebarStreamButton.e.bounds.height = BUTTON_DEFAULT_HEIGHT;
  props.sidebarStreamButton.text = "Stream";
  props.sidebarStreamButton.contentRenderer = &button_renderer_content_menu;

  button_init(&props.sidebarInputButton);
  props.sidebarInputButton.e.bounds.x = MARGIN_SIDE;
  props.sidebarInputButton.e.bounds.y = MARGIN_TOP + MARGIN_SIDE + 1 + 2*(BUTTON_DEFAULT_HEIGHT + MENU_SIDEBAR_BUTTON_SPACING);
  props.sidebarInputButton.e.bounds.width = MENU_SIDEBAR_BUTTON_WIDTH;
  props.sidebarInputButton.e.bounds.height = BUTTON_DEFAULT_HEIGHT;
  props.sidebarInputButton.text = "Input";
  props.sidebarInputButton.contentRenderer = &button_renderer_content_menu;

  button_set_init(&props.sidebarButtons, Vertical);
  button_set_add(&props.sidebarButtons, &props.sidebarGeneralButton);
  button_set_add(&props.sidebarButtons, &props.sidebarStreamButton);
  button_set_add(&props.sidebarButtons, &props.sidebarInputButton);

  scene_init(&props.sidebarScene);
  scene_add_element(&props.sidebarScene, &props.sidebarGeneralButton);
  scene_add_element(&props.sidebarScene, &props.sidebarStreamButton);
  scene_add_element(&props.sidebarScene, &props.sidebarInputButton);
  props.sidebarScene.clip.x = 0;
  props.sidebarScene.clip.y = MARGIN_TOP + 1;
  props.sidebarScene.clip.width = MENU_SIDEBAR_WIDTH;
  props.sidebarScene.clip.height = gui.height - MARGIN_TOP - MARGIN_BOTTOM - 2;
  props.sidebarScene.padded.x = MARGIN_SIDE;
  props.sidebarScene.padded.y = MARGIN_TOP + MARGIN_SIDE + 1;
  props.sidebarScene.padded.width = MENU_SIDEBAR_WIDTH - 2*MARGIN_SIDE;
  props.sidebarScene.padded.height = gui.width - MARGIN_TOP - MARGIN_BOTTOM - 2*MARGIN_SIDE - 2;

  return 0;
}

void main_update_settings(Input *input) {
  scene_update(&props.sidebarScene, input);

  // Update the button set to handle movement, clicking
  Button *clicked = button_set_update(&props.sidebarButtons, input, NULL);
}

void main_render_settings() {
  SDL_SetRenderDrawColor(gui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(gui.renderer);

  // Draw the sidebar
  boxColor(gui.renderer,
           props.sidebarScene.clip.x,
           props.sidebarScene.clip.y,
           props.sidebarScene.clip.x + props.sidebarScene.clip.width,
           props.sidebarScene.clip.y + props.sidebarScene.clip.height,
           MENU_SIDEBAR_COLOR);
  scene_render(&props.sidebarScene);

  // Draw the heading
  draw_top_header("Moonlight  ›  Settings");

  // Draw the OK and Back actions on the bottom toolbar
  draw_bottom_toolbar(2, "OK", ToolbarActionA, "Back", ToolbarActionB);

  SDL_RenderPresent(gui.renderer);
}

void main_cleanup_settings() {

}
