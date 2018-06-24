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
  props.connectionFailedTexture = load_png(moonlight_switch_connection_failed_png, moonlight_switch_connection_failed_png_size);
  if (!props.connectionFailedTexture) {
    fprintf(stderr, "[GUI] Could not load connection failed image: %s\n", SDL_GetError());
    return -1;
  }
  SDL_QueryTexture(props.connectionFailedTexture, NULL, NULL, &props.connectionFailedWidth, &props.connectionFailedHeight);

  return 0;
}

void main_update_connection_failed(Input *input) {

}

void main_render_connection_failed() {
  SDL_SetRenderDrawColor(ui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(ui.renderer);

  // Draw the connection failed image
  draw_texture(props.connectionFailedTexture,
               (ui.width - props.connectionFailedWidth) / 2,
               MARGIN_TOP + 75,
               props.connectionFailedWidth,
               props.connectionFailedHeight);

  // Draw the message text
  int textNormalHeight = text_ascent(ui.fontNormal);
  int textEnterWidth, textEnterX, textEnterY;
  measure_text(ui.fontNormal, "Unable to connect to the target PC:", &textEnterWidth, NULL);
  textEnterX = (ui.width - textEnterWidth) / 2;
  textEnterY = MARGIN_TOP + 75 + props.connectionFailedHeight + 90;
  draw_text(ui.fontNormal, "Unable to connect to the target PC:", textEnterX, textEnterY, COLOR_DARK, false, -1);

  // Draw the error text
  int textErrorWidth, textErrorX, textErrorY;
  measure_text(ui.fontNormal, gs_error, &textErrorWidth, NULL);
  textErrorX = (ui.width - textErrorWidth) / 2;
  textErrorY = textEnterY + textNormalHeight + 10;
  draw_text(ui.fontNormal, gs_error, textErrorX, textErrorY, COLOR_LIGHT, false, -1);

  // Draw the heading
  draw_top_header("Moonlight  ›  Connection");

  // Draw the OK and Back actions on the bottom toolbar
  draw_bottom_toolbar(2, "OK", ToolbarActionA, "Back", ToolbarActionB);

  SDL_RenderPresent(ui.renderer);
}

void main_cleanup_connection_failed() {
  if (props.connectionFailedTexture) {
    SDL_DestroyTexture(props.connectionFailedTexture);
    props.connectionFailedTexture = NULL;
  }
}
