#pragma once

#include "../gui.h"
#include "../text.h"
#include "../button.h"

#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>

#define MARGIN_SIDE 30
#define MARGIN_TOP 88
#define MARGIN_BOTTOM 73

#define MARGIN_TOOLBAR_SIDE 30
#define MARGIN_BETWEEN_TOOLBAR_ICON_TEXT  10
#define MARGIN_BETWEEN_TOOLBAR_BUTTONS    44

extern enum MainState {
  StateInitial,
  StateConnecting,
  StateConnectionFailed,
  StateGamesList,
  StateStreaming
} state;

extern bool shouldExitApp;
extern uint32_t darkColor;
extern SDL_Texture *buttonATexture;
extern SDL_Texture *buttonBTexture;
extern int buttonAWidth, buttonAHeight, buttonBWidth, buttonBHeight;

int main_init_initial();
void main_update_initial(uint64_t keys);
void main_render_initial();
void main_cleanup_initial();

int main_init_connecting();
void main_update_connecting(uint64_t keys);
void main_render_connecting();
void main_cleanup_connecting();

int main_init_connection_failed();
void main_update_connection_failed(uint64_t keys);
void main_render_connection_failed();
void main_cleanup_connection_failed();

int main_init_games_list();
void main_update_games_list(uint64_t keys);
void main_render_games_list();
void main_cleanup_games_list();

int main_init_streaming();
void main_update_streaming(uint64_t keys);
void main_render_streaming();
void main_cleanup_streaming();
