#pragma once

#include "common.h"
#include "element.h"
#include "scene.h"
#include "text.h"

#define MARGIN_SIDE 30
#define MARGIN_TOP 88
#define MARGIN_BOTTOM 73

#define MARGIN_TOOLBAR_SIDE 30
#define MARGIN_BETWEEN_TOOLBAR_ICON_TEXT  10
#define MARGIN_BETWEEN_TOOLBAR_BUTTONS    44

#define COLOR_DARK   0xff2d2d2d
#define COLOR_LIGHT  0xff6d6d6d

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  int width;
  int height;

  PlFontData fontData;
  TTF_Font *fontSmall;
  TTF_Font *fontNormal;
  TTF_Font *fontHeading;
  TTF_Font *fontMassive;

  SDL_Texture *buttonATexture;
  SDL_Texture *buttonBTexture;
  int buttonAWidth, buttonAHeight;
  int buttonBWidth, buttonBHeight;
} SwitchUI;

SwitchUI ui;

enum ToolbarAction {
  ToolbarActionA, ToolbarActionB
};

int ui_init();
void ui_cleanup();

/*
 * Draw the actions on the bottom toolbar with the chosen icons and text
 *
 * @param count   number of actions that will be drawn
 * @param ...     list of pairs of action text and ToolbarAction
 *
 * @example `draw_bottom_toolbar(2, "OK", ToolbarActionA, "Back", ToolbarActionB);
 */
void draw_bottom_toolbar(int count, ...);

/*
 * Draw the text on the top header
 *
 * @param text    text to display on the top of the header
 */
void draw_top_header(const char *text);

SDL_Texture *load_png(const void *data, size_t size);
SDL_Texture *load_png_rescale(const void *data, size_t size, int width, int height);
void draw_texture(SDL_Texture *texture, int x, int y, int w, int h);
void draw_clipped_texture(SDL_Texture *texture, int x, int y, int w, int h, Rect *clip);

void draw_clipped_box_bounds(Rect *bounds, Rect *clip, uint32_t color);
void draw_clipped_box(int x, int y, int width, int height, Rect *clip, uint32_t color);

void draw_clipped_rectangle_bounds(Rect *bounds, Rect *clip, uint32_t color);
void draw_clipped_rectangle(int x, int y, int width, int height, Rect *clip, uint32_t color);

Rect get_clip(Element *element);
Rect intersect_bounds_clip(int x, int y, int width, int height, Rect *clip);
