#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

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
  TTF_Font *fontNormal;
  TTF_Font *fontHeading;
  TTF_Font *fontMassive;
} GUI;

typedef struct {
  uint64_t keys;
} Input;

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
void draw_texture(SDL_Texture *texture, int x, int y, int w, int h);

uint64_t milliseconds();

uint32_t interpolate(uint32_t a, uint32_t b, double t);

// Client and server configuration
extern CONFIGURATION config;
extern SERVER_DATA server;
