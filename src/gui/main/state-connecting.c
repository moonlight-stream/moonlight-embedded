#include "gui-main.h"

#include "../gui.h"
#include "../text.h"
#include "../button.h"

#include "moonlight_switch_connecting_png.h"

static struct {
  SDL_Texture *connectingTexture;
  int connectingWidth;
  int connectingHeight;

  int frame;
  char pin[5];
} props = {0};

int main_init_connecting() {
  props.connectingTexture = load_png(moonlight_switch_connecting_png, moonlight_switch_connecting_png_size);
  if (!props.connectingTexture) {
    fprintf(stderr, "[GUI] Could not load connecting image: %s\n", SDL_GetError());
    return -1;
  }
  SDL_QueryTexture(props.connectingTexture, NULL, NULL, &props.connectingWidth, &props.connectingHeight);

  return 0;
}

void main_update_connecting(uint64_t keys) {
  if (props.frame == 0) {
    sprintf(props.pin, "%d%d%d%d", (int)random() % 10, (int)random() % 10, (int)random() % 10, (int)random() % 10);
  }
  else {
    if (gs_pair(&server, &props.pin[0]) == GS_OK) {
      state = StateGamesList;
    } else {
      state = StateConnectionFailed;
    }
  }

  props.frame++;
}


void main_render_connecting()  {
  SDL_SetRenderDrawColor(gui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(gui.renderer);

  // Draw the "toolbar" separators
  hlineColor(gui.renderer, MARGIN_SIDE, gui.width - MARGIN_SIDE, MARGIN_TOP, darkColor);
  hlineColor(gui.renderer, MARGIN_SIDE, gui.width - MARGIN_SIDE, gui.height - MARGIN_BOTTOM, darkColor);

  // Draw the connecting image
  draw_texture(props.connectingTexture, (gui.width - props.connectingWidth) / 2, MARGIN_TOP + 75, props.connectingWidth, props.connectingHeight);

  // Draw the guide text
  int textEnterWidth, textEnterHeight, textEnterX, textEnterY;
  measure_text(gui.fontNormal, "Enter the following PIN on the target PC:", &textEnterWidth, &textEnterHeight);
  textEnterX = (gui.width - textEnterWidth) / 2;
  textEnterY = MARGIN_TOP + 75 + props.connectingHeight + 90;
  draw_text(gui.fontNormal, "Enter the following PIN on the target PC:", textEnterX, textEnterY, darkColor, false);

  // Draw the PIN text
  int textPinWidth, textPinHeight, textPinX, textPinY;
  measure_text(gui.fontMassive, props.pin, &textPinWidth, &textPinHeight);
  textPinX = (gui.width - textPinWidth) / 2;
  textPinY = MARGIN_TOP + 75 + props.connectingHeight + 90 + textEnterHeight + 60;
  draw_text(gui.fontMassive, props.pin, textPinX, textPinY, darkColor, false);

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

void main_cleanup_connecting() {
  if (props.connectingTexture) {
    SDL_DestroyTexture(props.connectingTexture);
    props.connectingTexture = NULL;
  }
}
