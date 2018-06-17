#pragma once

#include <SDL2/SDL.h>
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

  // Stream specific
  SDL_Texture *streamTexture;
} GUI;

GUI gui;

int gui_init();
void gui_main_loop();
void gui_stream_loop();
void gui_cleanup();

// Client and server configuration
extern CONFIGURATION config;
extern SERVER_DATA server;
