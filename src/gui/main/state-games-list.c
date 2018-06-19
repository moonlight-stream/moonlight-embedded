#include "gui-main.h"

#define GAME_BUTTON_WIDTH       228
#define GAME_BUTTON_HEIGHT      228
#define GAME_BUTTON_SPACING     20
#define GAME_BUTTON_FLOW_SIZE   5

static struct {
  int frame;

  PAPP_LIST games;  
  size_t gamesCount;

  SDL_Texture **gamesArtTextures;
  int *gamesArtWidths;
  int *gamesArtHeights;

  Button **buttons;
  ButtonSet buttonSet;
} props = {0};

//static size_t test_game_list(PAPP_LIST *games) {
//  PAPP_LIST head = NULL;
//  PAPP_LIST prev = NULL;
//  size_t count = 0;

//  for (int i = 0; i < 8; i++) {
//    PAPP_LIST current = malloc(sizeof(*current));
//    current->id = 0;
//    current->name = malloc(strlen("Game Application XXXX") + 1);
//    sprintf(current->name, "Game Application %d", i+1);

//    if (prev) {
//      prev->next = current;
//    }
//    else {
//      head = current;
//    }

//    prev = current;
//    count++;
//  }

//  *games = head;
//  return count;
//}

static void game_button_renderer(Button *button) {
  int gameIndex = (int)button->user;

  // Measure the size of the game title
  int textWidth, textHeight = text_ascent(gui.fontSmall);
  text_measure(gui.fontSmall, button->text, &textWidth, NULL);

  // Draw the boxart of the game
  SDL_Texture *artTexture = props.gamesArtTextures[gameIndex];
  if (artTexture) {
    int artWidth = props.gamesArtWidths[gameIndex];
    int artHeight = props.gamesArtHeights[gameIndex];

    draw_texture(artTexture, button->x + 3, button->y + 3, button->width - 6, button->height - 16 - textHeight);
  }

  // Draw a small white border
  for (int i = 0; i < 3; i++) {
    rectangleColor(gui.renderer, button->x + i, button->y + i, button->x + button->width - i, button->y + button->height - i, BUTTON_FOCUSED_BACKGROUND);
  }

  // Draw the caption box
  boxColor(gui.renderer,
           button->x,
           button->y + button->height - 16 - textHeight,
           button->x + button->width,
           button->y + button->height,
           BUTTON_FOCUSED_BACKGROUND);

  // Draw the caption
  uint32_t textColor = button->focused ? BUTTON_FOCUSED_TEXT_COLOR : BUTTON_TEXT_COLOR;
  text_draw(gui.fontSmall, button->text, button->x + 8, button->y + button->height - 8 - textHeight, textColor, false, button->width - 16);
}

int main_init_games_list() {
  button_set_init(&props.buttonSet, FlowHorizontal);
  props.buttonSet.wrap = false;
  props.buttonSet.flowSize = GAME_BUTTON_FLOW_SIZE;

  return 0;
}

void main_update_games_list(Input *input) {
  if (props.frame == 0) {
    props.gamesCount = get_app_list(&server, &props.games);
    PAPP_LIST game = props.games;

    // Initialize memory for these dynamic items
    props.gamesArtTextures = malloc(props.gamesCount * sizeof(char *));
    props.gamesArtWidths = calloc(props.gamesCount, sizeof(int));
    props.gamesArtHeights = calloc(props.gamesCount, sizeof(int));
    props.buttons = malloc(props.gamesCount * sizeof(Button *));

    for (int i = 0; i < props.gamesCount; i++) {
      // Collect the box art for this game
      char *artData = NULL;
      int artSize = 0;
      int ret = gs_app_boxart(&server, game->id, &artData, &artSize);

      if (artData && artSize) {
        props.gamesArtTextures[i] = load_png(artData, artSize);
        SDL_QueryTexture(props.gamesArtTextures[i], NULL, NULL, &props.gamesArtWidths[i], &props.gamesArtWidths[i]);

        free(artData);
      }
      else {
        props.gamesArtTextures[i] = NULL;
      }

      // Allocate and initialize the button for this game
      props.buttons[i] = malloc(sizeof(Button));
      button_init(props.buttons[i]);
      props.buttons[i]->user = i;
      props.buttons[i]->renderer = &game_button_renderer;
      props.buttons[i]->text = game->name;
      props.buttons[i]->width = GAME_BUTTON_WIDTH;
      props.buttons[i]->height = GAME_BUTTON_HEIGHT;
      props.buttons[i]->x = MARGIN_SIDE + ((i % GAME_BUTTON_FLOW_SIZE) * (GAME_BUTTON_WIDTH + GAME_BUTTON_SPACING));
      props.buttons[i]->y = MARGIN_TOP + MARGIN_SIDE+ ((i / GAME_BUTTON_FLOW_SIZE) * (GAME_BUTTON_HEIGHT + GAME_BUTTON_SPACING));

      // Add the button to the button set
      button_set_add(&props.buttonSet, props.buttons[i]);

      // Visit the next game
      game = game->next;
    }
  }

  button_set_update(&props.buttonSet, input);

  props.frame++;
}

void main_render_games_list() {
  SDL_SetRenderDrawColor(gui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(gui.renderer);

  button_set_render(&props.buttonSet);

  draw_top_header("Moonlight  ›  Games");
  draw_bottom_toolbar(2, "OK", ToolbarActionA, "Back", ToolbarActionB);

  SDL_RenderPresent(gui.renderer);
}

void main_cleanup_games_list() {
  if (props.games) {
    free(props.games);
    props.games = NULL;
  }

  if (props.buttons) {
    for (int i = 0; i < props.gamesCount; i++) {
      free(props.buttons[i]);
      props.buttons[i] = NULL;
    }

    props.buttons = NULL;
  }

  button_set_cleanup(&props.buttonSet);
}
