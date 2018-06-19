#include "gui-main.h"

int main_init_games_list() {
  return 0;
}

void main_update_games_list(Input *input) {

}

void main_render_games_list() {
  SDL_SetRenderDrawColor(gui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(gui.renderer);
  SDL_RenderPresent(gui.renderer);
}

void main_cleanup_games_list() {

}
