#include "gui-main.h"

#include "moonlight_switch_logo_png.h"

static struct {
  SDL_Texture *logoTexture;
  int logoWidth;
  int logoHeight;

  Button connectButton;
  Button settingsButton;
  ButtonSet buttons;
} props = {0};

int main_init_initial() {
  props.logoTexture = load_png(moonlight_switch_logo_png, moonlight_switch_logo_png_size);
  if (!props.logoTexture) {
    fprintf(stderr, "[GUI, initial] Could not load logo: %s\n", SDL_GetError());
    return -1;
  }
  SDL_QueryTexture(props.logoTexture, NULL, NULL, &props.logoWidth, &props.logoHeight);

  button_init(&props.connectButton);
  props.connectButton.text = "Connect";
  props.connectButton.x = gui.width/2 - props.connectButton.width/2;
  props.connectButton.y = 100 + props.logoHeight + (gui.height - MARGIN_BOTTOM - props.logoHeight - 100)/2 - props.connectButton.height/2;
  props.connectButton.focused = true;

  button_init(&props.settingsButton);
  props.settingsButton.text = "Settings";
  props.settingsButton.x = gui.width/2 - props.settingsButton.width/2;
  props.settingsButton.y = props.connectButton.y + props.connectButton.height + 15;
  props.settingsButton.focused = false;

  button_set_init(&props.buttons, Vertical);
  button_set_add(&props.buttons, &props.connectButton);
  button_set_add(&props.buttons, &props.settingsButton);

  return 0;
}

void main_update_initial(Input *input) {
  Button *clicked = button_set_update(&props.buttons, input);

  if (clicked == &props.connectButton) {
    state = StateConnecting;
  }
}

void main_render_initial() {
  SDL_SetRenderDrawColor(gui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(gui.renderer);

  // Draw the logo
  draw_texture(props.logoTexture, (gui.width - props.logoWidth) / 2, 150, props.logoWidth, props.logoHeight);

  // Draw the OK action on the bottom toolbar
  draw_bottom_toolbar(1, "OK", ToolbarActionA);

  // Draw the main buttons
  button_set_render(&props.buttons);

  SDL_RenderPresent(gui.renderer);
}

void main_cleanup_initial() {
  if (props.logoTexture) {
    SDL_DestroyTexture(props.logoTexture);
    props.logoTexture = NULL;
  }
}
