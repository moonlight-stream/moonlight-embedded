#include "gui-main.h"

#include "button_a_png.h"
#include "button_b_png.h"

enum MainState state;
bool shouldExitApp;

uint32_t darkColor = RGBA8(0x2d, 0x2d, 0x2d, 0xff);
uint32_t lightColor = RGBA8(0x6d, 0x6d, 0x6d, 0xff);
SDL_Texture *buttonATexture = NULL;
SDL_Texture *buttonBTexture = NULL;
int buttonAWidth, buttonAHeight, buttonBWidth, buttonBHeight;

int gui_main_init() {
  state = StateInitial;
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
  while(appletMainLoop() && !shouldExitApp)
  {
      Input *input = switch_input_poll(CONTROLLER_P1_AUTO);

      switch (state) {
      case StateInitial:
        main_update_initial(input);
        main_render_initial();
        break;

      case StateConnecting:
        main_update_connecting(input);
        main_render_connecting();
        break;

      case StateConnectionFailed:
        main_update_connection_failed(input);
        main_render_connection_failed();
        break;

      case StateGamesList:
        main_update_games_list(input);
        main_render_games_list();
        break;

      case StateStreaming:
        main_update_streaming(input);
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

void draw_bottom_toolbar(int count, ...) {
  va_list args;
  va_start(args, count);

  int baseHeight = text_ascent(gui.fontNormal);
  int offsetX = gui.width - MARGIN_SIDE - MARGIN_TOOLBAR_SIDE;
  int offsetY = gui.height - MARGIN_BOTTOM + (MARGIN_BOTTOM - baseHeight)/2;

  for (int i = 0; i < count; i++) {
    // Obtain the button arguments
    char *text = va_arg(args, char *);
    enum ToolbarAction action = va_arg(args, enum ToolbarAction);

    SDL_Texture *iconTexture;
    int iconWidth, iconHeight;

    switch (action) {
      case ToolbarActionA:
        iconTexture = buttonATexture;
        iconWidth = buttonAWidth;
        iconHeight = buttonAHeight;
        break;

      case ToolbarActionB:
        iconTexture = buttonBTexture;
        iconWidth = buttonBWidth;
        iconHeight = buttonBHeight;
        break;

       default:
        continue;
    }

    // Measure the size of this particular label
    int textWidth;
    text_measure(gui.fontNormal, text, &textWidth, NULL);

    // Draw the text and icon
    text_draw(gui.fontNormal, text, offsetX - textWidth, offsetY, darkColor, false, -1);
    draw_texture(iconTexture,
                 offsetX - textWidth - MARGIN_BETWEEN_TOOLBAR_ICON_TEXT - iconWidth,
                 gui.height - MARGIN_BOTTOM + (MARGIN_BOTTOM - iconHeight)/2,
                 iconWidth,
                 iconHeight);

    offsetX = offsetX - textWidth - MARGIN_BETWEEN_TOOLBAR_ICON_TEXT - iconWidth - MARGIN_BETWEEN_TOOLBAR_BUTTONS;
  }


  // Draw the bottom separator
  hlineColor(gui.renderer, MARGIN_SIDE, gui.width - MARGIN_SIDE, gui.height - MARGIN_BOTTOM, darkColor);

  va_end(args);
}

void draw_top_header(const char *text) {
  // Draw the top separator
  hlineColor(gui.renderer, MARGIN_SIDE, gui.width - MARGIN_SIDE, MARGIN_TOP, darkColor);

  // Draw the text
  int textWidth, textHeight = text_ascent(gui.fontHeading);
  text_measure(gui.fontHeading, text, &textWidth, NULL);
  text_draw(gui.fontHeading,
            text,
            MARGIN_SIDE + MARGIN_TOOLBAR_SIDE,
            (MARGIN_TOP - textHeight)/2 + 10,
            darkColor,
            false,
            -1);
}
