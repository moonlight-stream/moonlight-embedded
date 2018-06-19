#include "gui-main.h"

#include "../gui.h"
#include "../text.h"
#include "../button.h"

#include "moonlight_switch_logo_png.h"

static struct {
  SDL_Texture *logoTexture;
  int logoWidth;
  int logoHeight;

  Button connectButton;
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

  return 0;
}

void main_update_initial(uint64_t keys) {
  if (keys & KEY_A) {
    state = StateConnecting;
  }

//  if (kDown & KEY_X) {
//    if (pair_check(&server)) {
//      get_app_list(&server);
//    }
//  }
//  else if (kDown & KEY_B) {
//    if (pair_check(&server)) {
//      enum platform system = platform_check(config.platform);
//      if (config.debug_level > 0)
//        printf("Beginning streaming on platform %s\n", platform_name(system));

//      if (system == 0) {
//        fprintf(stderr, "Platform '%s' not found\n", config.platform);
//        break;
//      }
//      config.stream.supportsHevc = config.codec != CODEC_H264 && (config.codec == CODEC_HEVC || platform_supports_hevc(system));

//      stream_start(&server, &config, system);
//      gui_stream_loop();
//      stream_stop(system);
//    }
//  }
//  else if (kDown & KEY_Y) {
//    if (gs_unpair(&server) != GS_OK) {
//      fprintf(stderr, "Failed to unpair to server: %s\n", gs_error);
//    } else {
//      printf("Succesfully unpaired\n");
//    }
//  }
//  else if (kDown & KEY_PLUS) {
//    break;
//  }
}

void main_render_initial() {
  SDL_SetRenderDrawColor(gui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(gui.renderer);

  // Draw the logo
  draw_texture(props.logoTexture, (gui.width - props.logoWidth) / 2, 100, props.logoWidth, props.logoHeight);

  // Draw the bottom "toolbar" separator
  hlineColor(gui.renderer, MARGIN_SIDE, gui.width - MARGIN_SIDE, gui.height - MARGIN_BOTTOM, darkColor);

  // Draw the OK button on the bottom toolbar
  int textOkWidth, textOkHeight, textOkX, textOkY, buttonOkX, buttonOkY;
  measure_text(gui.fontNormal, "OK", &textOkWidth, &textOkHeight);
  textOkX = gui.width - MARGIN_SIDE - MARGIN_TOOLBAR_SIDE - textOkWidth;
  textOkY = gui.height - MARGIN_BOTTOM + (MARGIN_BOTTOM - textOkHeight)/2;
  buttonOkX = textOkX - MARGIN_BETWEEN_TOOLBAR_ICON_TEXT - buttonAWidth;
  buttonOkY = gui.height - MARGIN_BOTTOM + (MARGIN_BOTTOM - buttonAHeight)/2;
  draw_text(gui.fontNormal, "OK", textOkX, textOkY, darkColor, false);
  draw_texture(buttonATexture, buttonOkX, buttonOkY, buttonAWidth, buttonAHeight);

  // Draw the main connect button
  button_render(&props.connectButton);

  SDL_RenderPresent(gui.renderer);
}

void main_cleanup_initial() {
  if (props.logoTexture) {
    SDL_DestroyTexture(props.logoTexture);
    props.logoTexture = NULL;
  }
}
