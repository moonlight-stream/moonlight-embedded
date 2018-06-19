#include "gui-main.h"

#include "button_a_png.h"
#include "button_b_png.h"

enum MainState state = StateInitial;
bool shouldExitApp = 0;

uint32_t darkColor = RGBA8(0x2d, 0x2d, 0x2d, 0xff);
SDL_Texture *buttonATexture = NULL;
SDL_Texture *buttonBTexture = NULL;
int buttonAWidth, buttonAHeight, buttonBWidth, buttonBHeight;

int gui_main_init() {
  shouldExitApp = false;
  darkColor = RGBA8(0x2d, 0x2d, 0x2d, 0xff);

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

  main_init_initial();
  main_init_connecting();
  main_init_connection_failed();
  main_init_games_list();
  main_init_streaming();

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
        main_update_initial(kDown);
        main_render_initial();
        break;

      case StateConnecting:
        main_update_connecting(kDown);
        main_render_connecting();
        break;

      case StateConnectionFailed:
        main_update_connection_failed(kDown);
        main_render_connection_failed();
        break;

      case StateGamesList:
        main_update_games_list(kDown);
        main_render_games_list();
        break;

      case StateStreaming:
        main_update_streaming(kDown);
        main_render_streaming();
        break;
      }
  }
}

void gui_main_cleanup() {
  if (buttonATexture) {
    SDL_DestroyTexture(buttonATexture);
    buttonATexture = NULL;
  }

  if (buttonBTexture) {
    SDL_DestroyTexture(buttonBTexture);
    buttonBTexture = NULL;
  }

  main_cleanup_initial();
  main_cleanup_connecting();
  main_cleanup_connection_failed();
  main_cleanup_games_list();
  main_cleanup_streaming();
}
