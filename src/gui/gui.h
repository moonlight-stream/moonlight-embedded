#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../input/switch.h"
#include "scene.h"

#include <switch.h>
#include <stdio.h>

#include <Limelight.h>
#include <client.h>
#include "../config.h"
#include "../connection.h"

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
} GUI;

GUI gui;

int gui_init();
void gui_cleanup();

int gui_main_init();
void gui_main_loop();
void gui_main_cleanup();

int gui_stream_init();
void gui_stream_loop();
void gui_stream_cleanup();

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

uint64_t milliseconds();

uint32_t interpolate(uint32_t a, uint32_t b, double t);
