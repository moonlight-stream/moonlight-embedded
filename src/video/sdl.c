/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015 Iwan Timmer
 *
 * Moonlight is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonlight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight; if not, see <http://www.gnu.org/licenses/>.
 */

#include "ffmpeg.h"

#include "limelight-common/Limelight.h"

#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_opengles.h>
#include <stdbool.h>

#define DECODER_BUFFER_SIZE 92*1024
#define ODROID_THREAD_COUNT 4

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *bmp = NULL;
static int screen_width, screen_height;
static char* ffmpeg_buffer;

static void sdl_setup(int width, int height, int redrawRate, void* context, int drFlags) {
  static bool thread_slice = false;
  if(height < 1080 && redrawRate < 60)
    thread_slice = true;
  if (ffmpeg_init(width, height, thread_slice, ODROID_THREAD_COUNT) < 0) {
    fprintf(stderr, "Couldn't initialize video decoding\n");
    exit(1);
  }
  
  ffmpeg_buffer = malloc(DECODER_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);
  if (ffmpeg_buffer == NULL) {
    fprintf(stderr, "Not enough memory\n");
    exit(1);
  }

  screen_width = width;
  screen_height = height;
}

static void sdl_cleanup() {
  ffmpeg_destroy();
}

static int sdl_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  if (window == NULL) {
    window = SDL_CreateWindow("Moonlight", SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if(!window) {
      fprintf(stderr, "SDL: could not create window - exiting\n");
      exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

    SDL_GLContext *ctx = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, ctx);

    SDL_Log("Vendor     : %s\n", glGetString(GL_VENDOR));
    SDL_Log("Renderer   : %s\n", glGetString(GL_RENDERER));
    SDL_Log("Version    : %s\n", glGetString(GL_VERSION));
    SDL_Log("Extensions : %s\n", glGetString(GL_EXTENSIONS));
    SDL_Log("\n");

    SDL_SetRelativeMouseMode(SDL_TRUE);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
      printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
      exit(1);
    }
    SDL_ShowCursor(SDL_DISABLE);
    bmp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
    if (!bmp) {
      fprintf(stderr, "SDL: could not create texture - exiting\n");
      exit(1);
    }
  }

  if (decodeUnit->fullLength < DECODER_BUFFER_SIZE) {
    PLENTRY entry = decodeUnit->bufferList;
    int length = 0;
    while (entry != NULL) {
      memcpy(ffmpeg_buffer+length, entry->data, entry->length);
      length += entry->length;
      entry = entry->next;
    }

    int ret = ffmpeg_decode(ffmpeg_buffer, length);
    if (ret == 1) {
      AVFrame* frame = ffmpeg_get_frame();

      SDL_UpdateYUVTexture(bmp, NULL, frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1], frame->data[2], frame->linesize[2]);
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, bmp, NULL, NULL);
      SDL_RenderPresent(renderer);
    }
  } else {
    fprintf(stderr, "Video decode buffer too small");
    exit(1);
  }

  return DR_OK;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_sdl = {
  .setup = sdl_setup,
  .cleanup = sdl_cleanup,
  .submitDecodeUnit = sdl_submit_decode_unit,
  .capabilities = CAPABILITY_DIRECT_SUBMIT | CAPABILITY_REFERENCE_FRAME_INVALIDATION,
};
