#include "ui-main.h"

#include "moonlight_switch_connection_failed_png.h"

static struct {
  SDL_Texture *connectionFailedTexture;
  int connectionFailedWidth;
  int connectionFailedHeight;

  int frame;
  char pin[5];
} props = {0};

int main_init_connection_failed() {
  props.connectionFailedTexture = sui_load_png(moonlight_switch_connection_failed_png, moonlight_switch_connection_failed_png_size);
  if (!props.connectionFailedTexture) {
    fprintf(stderr, "[GUI] Could not load connection failed image: %s\n", SDL_GetError());
    return -1;
  }
  SDL_QueryTexture(props.connectionFailedTexture, NULL, NULL, &props.connectionFailedWidth, &props.connectionFailedHeight);

  return 0;
}

void main_update_connection_failed(SUIInput *input) {
  if (input->buttons.down & KEY_B) {
    ui_state = sui_state_pop(ui_state);
  }
}

void main_render_connection_failed() {
  SDL_SetRenderDrawColor(ui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(ui.renderer);

  // Draw the connection failed image
  sui_draw_texture(props.connectionFailedTexture,
               (ui.width - props.connectionFailedWidth) / 2,
               SUI_MARGIN_TOP + 75,
               props.connectionFailedWidth,
               props.connectionFailedHeight);

  // Draw the message text
  int textNormalHeight = sui_text_ascent(ui.fontNormal);
  int textEnterWidth, textEnterX, textEnterY;
  sui_measure_text(ui.fontNormal, "Unable to connect to the target PC:", &textEnterWidth, NULL);
  textEnterX = (ui.width - textEnterWidth) / 2;
  textEnterY = SUI_MARGIN_TOP + 75 + props.connectionFailedHeight + 90;
  sui_draw_text(ui.fontNormal, "Unable to connect to the target PC:", textEnterX, textEnterY, SUI_COLOR_DARK, false, -1);

  // Draw the error text
  int textErrorWidth, textErrorX, textErrorY;
  sui_measure_text(ui.fontNormal, gs_error, &textErrorWidth, NULL);
  textErrorX = (ui.width - textErrorWidth) / 2;
  textErrorY = textEnterY + textNormalHeight + 10;
  sui_draw_text(ui.fontNormal, gs_error, textErrorX, textErrorY, SUI_COLOR_LIGHT, false, -1);

  // Draw the heading
  sui_draw_top_header("Moonlight  ›  Connection");

  // Draw the OK and Back actions on the bottom toolbar
  sui_draw_bottom_toolbar(2, "OK", SUIToolbarActionA, "Back", SUIToolbarActionB);

  SDL_RenderPresent(ui.renderer);
}

void main_cleanup_connection_failed() {
  if (props.connectionFailedTexture) {
    SDL_DestroyTexture(props.connectionFailedTexture);
    props.connectionFailedTexture = NULL;
  }
}

MoonlightUiState MoonlightUiStateConnectionFailed = {
  .state = 3,
  .init = &main_init_connection_failed,
  .update = &main_update_connection_failed,
  .render = &main_render_connection_failed,
  .cleanup = &main_cleanup_connection_failed
};
