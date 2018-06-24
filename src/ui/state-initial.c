#include "ui-main.h"

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
  props.connectButton.e.bounds.x = ui.width/2 - props.connectButton.e.bounds.w/2;
  props.connectButton.e.bounds.y = 100 + props.logoHeight + (ui.height - MARGIN_BOTTOM - props.logoHeight - 100)/2 - props.connectButton.e.bounds.h/2;
  props.connectButton.focused = true;

  button_init(&props.settingsButton);
  props.settingsButton.text = "Settings";
  props.settingsButton.e.bounds.x = ui.width/2 - props.settingsButton.e.bounds.w/2;
  props.settingsButton.e.bounds.y = props.connectButton.e.bounds.y + props.connectButton.e.bounds.h + 15;
  props.settingsButton.focused = false;

  button_set_init(&props.buttons, Vertical);
  button_set_add(&props.buttons, &props.connectButton);
  button_set_add(&props.buttons, &props.settingsButton);

  return 0;
}

void main_update_initial(Input *input) {
  Button *clicked = button_set_update(&props.buttons, input, NULL);

  if (clicked == &props.connectButton) {
    ui_state = state_push(ui_state, StateConnecting);
  }
  else if (clicked == &props.settingsButton) {
    ui_state = state_push(ui_state, StateSettings);
  }
  else if (input->buttons.down & KEY_B) {
    ui_shouldExitApp = true;
  }
}

void main_render_initial() {
  SDL_SetRenderDrawColor(ui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(ui.renderer);

  // Draw the logo
  draw_texture(props.logoTexture, (ui.width - props.logoWidth) / 2, 150, props.logoWidth, props.logoHeight);

  // Draw the OK action on the bottom toolbar
  draw_bottom_toolbar(1, "OK", ToolbarActionA);

  // Draw the main buttons
  button_set_render(&props.buttons);

  SDL_RenderPresent(ui.renderer);
}

void main_cleanup_initial() {
  if (props.logoTexture) {
    SDL_DestroyTexture(props.logoTexture);
    props.logoTexture = NULL;
  }
}
