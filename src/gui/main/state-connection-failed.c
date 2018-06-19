#include "gui-main.h"

#include "moonlight_switch_connection_failed_png.h"

static struct {
  SDL_Texture *connectionFailedTexture;
  int connectionFailedWidth;
  int connectionFailedHeight;

  int frame;
  char pin[5];
} props = {0};

int main_init_connection_failed() {
  props.connectionFailedTexture = load_png(moonlight_switch_connection_failed_png, moonlight_switch_connection_failed_png_size);
  if (!props.connectionFailedTexture) {
    fprintf(stderr, "[GUI] Could not load connection failed image: %s\n", SDL_GetError());
    return -1;
  }
  SDL_QueryTexture(props.connectionFailedTexture, NULL, NULL, &props.connectionFailedWidth, &props.connectionFailedHeight);

  return 0;
}

void main_update_connection_failed(uint64_t keys) {

}

void main_render_connection_failed() {
  SDL_SetRenderDrawColor(gui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(gui.renderer);

  // Draw the "toolbar" separators
  hlineColor(gui.renderer, MARGIN_SIDE, gui.width - MARGIN_SIDE, MARGIN_TOP, darkColor);
  hlineColor(gui.renderer, MARGIN_SIDE, gui.width - MARGIN_SIDE, gui.height - MARGIN_BOTTOM, darkColor);

  // Draw the connection failed image
  draw_texture(props.connectionFailedTexture,
               (gui.width - props.connectionFailedWidth) / 2,
               MARGIN_TOP + 75,
               props.connectionFailedWidth,
               props.connectionFailedHeight);

  // Draw the guide text
  int textEnterWidth, textEnterHeight, textEnterX, textEnterY;
  measure_text(gui.fontNormal, "Unable to connect and pair with the target PC", &textEnterWidth, &textEnterHeight);
  textEnterX = (gui.width - textEnterWidth) / 2;
  textEnterY = MARGIN_TOP + 75 + props.connectionFailedHeight + 90;
  draw_text(gui.fontNormal, "Unable to connect and pair with the target PC", textEnterX, textEnterY, darkColor, false);

  // Draw the heading
  int textMoonlightWidth, textMoonlightHeight, textMoonlightX, textMoonlightY;
  measure_text(gui.fontHeading, "M", &textMoonlightWidth, &textMoonlightHeight);
  textMoonlightX = MARGIN_SIDE + MARGIN_TOOLBAR_SIDE;
  textMoonlightY = (MARGIN_TOP - textMoonlightHeight)/2 + 10;
  draw_text(gui.fontHeading, "Moonlight", textMoonlightX, textMoonlightY, darkColor, false);

  // Draw the OK button on the bottom toolbar
  int textOkWidth, textOkHeight, textOkX, textOkY, buttonOkX, buttonOkY;
  measure_text(gui.fontNormal, "OK", &textOkWidth, &textOkHeight);
  textOkX = gui.width - MARGIN_SIDE - MARGIN_TOOLBAR_SIDE - textOkWidth;
  textOkY = gui.height - MARGIN_BOTTOM + (MARGIN_BOTTOM - textOkHeight)/2;
  buttonOkX = textOkX - MARGIN_BETWEEN_TOOLBAR_ICON_TEXT - buttonAWidth;
  buttonOkY = gui.height - MARGIN_BOTTOM + (MARGIN_BOTTOM - buttonAHeight)/2;
  draw_text(gui.fontNormal, "OK", textOkX, textOkY, darkColor, false);
  draw_texture(buttonATexture, buttonOkX, buttonOkY, buttonAWidth, buttonAHeight);

  // Draw the Back button on the bottom toolbar
  int textBackWidth, textBackHeight, textBackX, textBackY, buttonBackX, buttonBackY;
  measure_text(gui.fontNormal, "Back", &textBackWidth, &textBackHeight);
  textBackX = buttonOkX - MARGIN_BETWEEN_TOOLBAR_BUTTONS - textBackWidth;
  textBackY = gui.height - MARGIN_BOTTOM + (MARGIN_BOTTOM - textBackHeight)/2;
  buttonBackX = textBackX - MARGIN_BETWEEN_TOOLBAR_ICON_TEXT - buttonBWidth;
  buttonBackY = gui.height - MARGIN_BOTTOM + (MARGIN_BOTTOM - buttonBHeight)/2;
  draw_text(gui.fontNormal, "Back", textBackX, textBackY, darkColor, false);
  draw_texture(buttonBTexture, buttonBackX, buttonBackY, buttonBWidth, buttonBHeight);

  SDL_RenderPresent(gui.renderer);
}

void main_cleanup_connection_failed() {
  if (props.connectionFailedTexture) {
    SDL_DestroyTexture(props.connectionFailedTexture);
    props.connectionFailedTexture = NULL;
  }
}
