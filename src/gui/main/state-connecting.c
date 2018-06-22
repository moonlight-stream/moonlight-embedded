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

void main_update_connecting(Input *input) {
  if (props.frame == 0) {
//    sprintf(props.pin, "%d%d%d%d", (int)random() % 10, (int)random() % 10, (int)random() % 10, (int)random() % 10);
    sprintf(props.pin, "0000");
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

  // Draw the connecting image
  draw_texture(props.connectingTexture, (gui.width - props.connectingWidth) / 2, MARGIN_TOP + 75, props.connectingWidth, props.connectingHeight);

  // Draw the guide text
  int textNormalHeight = text_ascent(gui.fontNormal);
  int textEnterWidth, textEnterX, textEnterY;
  char *textEnter = "Enter the following PIN on the target PC:";
  measure_text(gui.fontNormal, textEnter, &textEnterWidth, NULL);
  textEnterX = (gui.width - textEnterWidth) / 2;
  textEnterY = MARGIN_TOP + 75 + props.connectingHeight + 90;
  draw_text(gui.fontNormal, textEnter, textEnterX, textEnterY, darkColor, false, -1);

  // Draw the PIN text
  int textMassiveHeight = text_ascent(gui.fontHeading);
  int textPinWidth, textPinX, textPinY;
  measure_text(gui.fontMassive, props.pin, &textPinWidth, NULL);
  textPinX = (gui.width - textPinWidth) / 2;
  textPinY = MARGIN_TOP + 75 + props.connectingHeight + 90 + textNormalHeight + 60;
  draw_text(gui.fontMassive, props.pin, textPinX, textPinY, darkColor, false, -1);

  // Draw the heading
  draw_top_header("Moonlight  ›  Connection");

  // Draw the OK and Back actions on the bottom toolbar
  draw_bottom_toolbar(2, "OK", ToolbarActionA, "Back", ToolbarActionB);

  SDL_RenderPresent(gui.renderer);
}

void main_cleanup_connecting() {
  if (props.connectingTexture) {
    SDL_DestroyTexture(props.connectingTexture);
    props.connectingTexture = NULL;
  }
}
