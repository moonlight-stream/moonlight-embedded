#include "gui.h"
#include "text.h"
#include "button.h"

#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>

#include "moonlight_switch_logo_png.h"
#include "button_a_png.h"
#include "button_b_png.h"

#define MARGIN_SIDE 30
#define MARGIN_TOP 88
#define MARGIN_BOTTOM 73

#define MARGIN_TOOLBAR_SIDE 30
#define MARGIN_BETWEEN_TOOLBAR_ICON_TEXT  10
#define MARGIN_BETWEEN_TOOLBAR_BUTTONS    44

static enum State {
  StateInitial,
  StateConnecting,
  StateGamesList,
  StateStreaming
} state;

static bool shouldExitApp = 0;

static uint32_t darkColor = RGBA8(0x2d, 0x2d, 0x2d, 0xff);

static SDL_Texture *logoTexture;
static int logoWidth, logoHeight;

static SDL_Texture *buttonATexture;
static SDL_Texture *buttonBTexture;
static int buttonAWidth, buttonAHeight, buttonBWidth, buttonBHeight;

static Button connectButton;

static void render_initial() {
  SDL_SetRenderDrawColor(gui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(gui.renderer);

  // Draw the logo
  draw_texture(logoTexture, (gui.width - logoWidth) / 2, 100, logoWidth, logoHeight);

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
  button_render(&connectButton);

  SDL_RenderPresent(gui.renderer);
}

static void update_initial(uint64_t keys) {
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

static void render_connecting()  {
  SDL_SetRenderDrawColor(gui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(gui.renderer);

  // Draw the "toolbar" separators
  hlineColor(gui.renderer, MARGIN_SIDE, gui.width - MARGIN_SIDE, MARGIN_TOP, darkColor);
  hlineColor(gui.renderer, MARGIN_SIDE, gui.width - MARGIN_SIDE, gui.height - MARGIN_BOTTOM, darkColor);

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

static void update_connecting(uint64_t keys) {
//  char pin[5];
//  sprintf(pin, "%d%d%d%d", (int)random() % 10, (int)random() % 10, (int)random() % 10, (int)random() % 10);
//  printf("Please enter the following PIN on the target PC: %s\n", pin);
//  if (gs_pair(&server, &pin[0]) != GS_OK) {
//    fprintf(stderr, "Failed to pair to server: %s\n", gs_error);
//  } else {
//    printf("Succesfully paired\n");
//  }
}

int gui_main_init() {
  logoTexture = load_png(moonlight_switch_logo_png, moonlight_switch_logo_png_size);
  if (!logoTexture) {
    fprintf(stderr, "[GUI] Could not load logo: %s\n", SDL_GetError());
    return -1;
  }
  SDL_QueryTexture(logoTexture, NULL, NULL, &logoWidth, &logoHeight);

  buttonATexture = load_png(button_a_png, button_a_png_size);
  if (!buttonATexture) {
    fprintf(stderr, "[GUI] Could not load button A image: %s\n", SDL_GetError());
    return -1;
  }
  SDL_QueryTexture(buttonATexture, NULL, NULL, &buttonAWidth, &buttonAHeight);

  buttonBTexture = load_png(button_b_png, button_b_png_size);
  if (!buttonBTexture) {
    fprintf(stderr, "[GUI] Could not load button B image: %s\n", SDL_GetError());
    return -1;
  }
  SDL_QueryTexture(buttonBTexture, NULL, NULL, &buttonBWidth, &buttonBHeight);

  button_init(&connectButton);
  connectButton.text = "Connect";
  connectButton.x = gui.width/2 - connectButton.width/2;
  connectButton.y = 100 + logoHeight + (gui.height - MARGIN_BOTTOM - logoHeight - 100)/2 - connectButton.height/2;
  connectButton.focused = true;

  return 0;
}

void gui_main_loop() {
  state = StateInitial;

  while(appletMainLoop() && !shouldExitApp)
  {
      //Scan all the inputs. This should be done once for each frame
      hidScanInput();

      //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
      u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

      switch (state) {
      case StateInitial:
        update_initial(kDown);
        render_initial();
        break;

      case StateConnecting:
        update_connecting(kDown);
        render_connecting();
        break;
      }
  }
}

void gui_main_cleanup() {
  if (logoTexture) {
    SDL_DestroyTexture(logoTexture);
    logoTexture = NULL;
  }
}
