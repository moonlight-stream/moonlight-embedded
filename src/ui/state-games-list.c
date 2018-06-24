#include "ui-main.h"

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

  SUIButton **buttons;
  SUIButtonSet buttonSet;

  SUIScene gamesListScene;
  SUIRect sceneClip;
  SUIRect sceneClipPadding;
} props = {0};

//static size_t test_game_list(PAPP_LIST *games) {
//  PAPP_LIST head = NULL;
//  PAPP_LIST prev = NULL;
//  size_t count = 0;
//  size_t total = 17;

//  for (int i = 0; i < total; i++) {
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

static void game_button_renderer(SUIButton *button) {
  int gameIndex = (int)button->user;

  SUIRect clip = sui_get_clip(button);

  // Measure the size of the game title
  int textWidth, textHeight = sui_text_ascent(ui.fontSmall);
  sui_measure_text(ui.fontSmall, button->text, &textWidth, NULL);

  // Draw the boxart of the game
  SDL_Texture *artTexture = props.gamesArtTextures[gameIndex];
  if (artTexture) {
    int artWidth = props.gamesArtWidths[gameIndex];
    int artHeight = props.gamesArtHeights[gameIndex];

    sui_draw_clipped_texture(artTexture,
                             button->e.bounds.x + 3,
                             button->e.bounds.y + 3,
                             button->e.bounds.w - 6,
                             button->e.bounds.h - 16 - textHeight,
                             &clip);
  }

  // Draw a small white border
  for (int i = 0; i < 3; i++) {
    sui_draw_clipped_rectangle(button->e.bounds.x + i,
                               button->e.bounds.y + i,
                               button->e.bounds.w - 2*i,
                               button->e.bounds.h - 2*i,
                               &clip,
                               SUI_BUTTON_FOCUSED_BACKGROUND);
  }

  // Draw the caption box
  sui_draw_clipped_box(button->e.bounds.x,
                       button->e.bounds.y + button->e.bounds.h - 16 - textHeight,
                       button->e.bounds.w,
                       16 + textHeight,
                       &clip,
                       SUI_BUTTON_FOCUSED_BACKGROUND);

  // Draw the caption
  uint32_t textColor = button->focused ? SUI_BUTTON_FOCUSED_TEXT_COLOR : SUI_BUTTON_TEXT_COLOR;
  sui_draw_clipped_text(ui.fontSmall,
                        button->text,
                        button->e.bounds.x + 8,
                        button->e.bounds.y + button->e.bounds.h - 8 - textHeight,
                        &clip,
                        textColor,
                        false,
                        button->e.bounds.w - 16);
}

int main_init_games_list() {
  sui_button_set_init(&props.buttonSet, FlowHorizontal);
  props.buttonSet.wrap = false;
  props.buttonSet.flowSize = GAME_BUTTON_FLOW_SIZE;

  sui_scene_init(&props.gamesListScene);
  props.gamesListScene.clip.x = 0;
  props.gamesListScene.clip.y = SUI_MARGIN_TOP + 1;
  props.gamesListScene.clip.w = ui.width;
  props.gamesListScene.clip.h = ui.height - SUI_MARGIN_TOP - SUI_MARGIN_BOTTOM - 2;
  props.gamesListScene.padded.x = SUI_MARGIN_SIDE;
  props.gamesListScene.padded.y = SUI_MARGIN_TOP + SUI_MARGIN_SIDE + 1;
  props.gamesListScene.padded.w = ui.width - 2*SUI_MARGIN_SIDE;
  props.gamesListScene.padded.h = ui.height - SUI_MARGIN_TOP - SUI_MARGIN_BOTTOM - 2*SUI_MARGIN_SIDE - 2;

  return 0;
}

void main_update_games_list(SUIInput *input) {
  if (props.frame == 0) {
    props.gamesCount = get_app_list(&server, &props.games);

    PAPP_LIST game = props.games;

    // Initialize memory for these dynamic items
    props.gamesArtTextures = malloc(props.gamesCount * sizeof(char *));
    props.gamesArtWidths = calloc(props.gamesCount, sizeof(int));
    props.gamesArtHeights = calloc(props.gamesCount, sizeof(int));
    props.buttons = malloc(props.gamesCount * sizeof(SUIButton *));

    for (int i = 0; i < props.gamesCount; i++) {
      // Collect the box art for this game
      char *artData = NULL;
      int artSize = 0;
      gs_app_boxart(&server, game->id, &artData, &artSize);

      if (artData && artSize) {
        props.gamesArtTextures[i] = sui_load_png_rescale(artData, artSize, GAME_BUTTON_WIDTH - 6, GAME_BUTTON_HEIGHT - 16 - sui_text_ascent(ui.fontSmall));
        SDL_QueryTexture(props.gamesArtTextures[i], NULL, NULL, &props.gamesArtWidths[i], &props.gamesArtHeights[i]);

        free(artData);
      }
      else {
        props.gamesArtTextures[i] = NULL;
      }

      // Allocate and initialize the button for this game
      SUIButton *button = malloc(sizeof(SUIButton));
      sui_button_init(button);
      button->text = game->name;
      button->contentRenderer = &game_button_renderer;
      button->user = i;
      button->e.bounds.w = GAME_BUTTON_WIDTH;
      button->e.bounds.h = GAME_BUTTON_HEIGHT;
      button->e.bounds.x = SUI_MARGIN_SIDE + ((i % GAME_BUTTON_FLOW_SIZE) * (GAME_BUTTON_WIDTH + GAME_BUTTON_SPACING));
      button->e.bounds.y = SUI_MARGIN_TOP + SUI_MARGIN_SIDE + ((i / GAME_BUTTON_FLOW_SIZE) * (GAME_BUTTON_HEIGHT + GAME_BUTTON_SPACING));
      props.buttons[i] = button;

      // Add the button to the button set and to the list scene
      sui_button_set_add(&props.buttonSet, props.buttons[i]);
      sui_scene_add_element(&props.gamesListScene, button);

      // Visit the next game
      game = game->next;
    }

    sui_scene_print(&props.gamesListScene);
  }

  SUIButton *clicked = NULL, *focused = NULL;
  clicked = sui_button_set_update(&props.buttonSet, input, &focused);

  if (clicked) {
    int gameIndex = (int)clicked->user;
    PAPP_LIST game = props.games;

    for (int i = 0; i < gameIndex; i++) {
      game = game->next;
    }

    main_set_streaming_game(game);
    ui_state = sui_state_push(ui_state, &MoonlightUiStateStreaming);
  }

  if (focused) {
    sui_scene_scroll_to_element(&props.gamesListScene, focused);
  }

  sui_scene_update(&props.gamesListScene, input);

  if (input->buttons.down & KEY_B) {
    ui_state = sui_state_pop(ui_state);
  }

  props.frame++;
}

void main_render_games_list() {
  SDL_SetRenderDrawColor(ui.renderer, 0xeb, 0xeb, 0xeb, 0xff);
  SDL_RenderClear(ui.renderer);

  sui_draw_top_header("Moonlight  ›  Games");
  sui_draw_bottom_toolbar(2, "OK", SUIToolbarActionA, "Back", SUIToolbarActionB);

  sui_scene_render(&props.gamesListScene);

  SDL_RenderPresent(ui.renderer);
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

  sui_button_set_cleanup(&props.buttonSet);
}

MoonlightUiState MoonlightUiStateGamesList = {
  .state = 4,
  .init = &main_init_games_list,
  .update = &main_update_games_list,
  .render = &main_render_games_list,
  .cleanup = &main_cleanup_games_list
};
