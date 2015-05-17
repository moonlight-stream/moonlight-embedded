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

#define DECODER_BUFFER_SIZE 92*1024

static SDL_Surface *screen;
static SDL_Overlay *bmp = NULL;
static int screen_width, screen_height;
static char* ffmpeg_buffer;

static void sdl_setup(int width, int height, int redrawRate, void* context, int drFlags) {
  if(SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
    exit(1);
  }

  screen = SDL_SetVideoMode(width, height, 0, 0);
  if(!screen) {
    fprintf(stderr, "SDL: could not set video mode - exiting\n");
    exit(1);
  }

  bmp = SDL_CreateYUVOverlay(width, height, SDL_YV12_OVERLAY, screen);
  
  int avc_flags = FAST_BILINEAR_FILTERING;
  if (ffmpeg_init(width, height, 2, avc_flags) < 0) {
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

static void sdl_release() {
  ffmpeg_destroy();
}

static int sdl_submit_decode_unit(PDECODE_UNIT decodeUnit) {
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
      SDL_LockYUVOverlay(bmp);

      AVPicture pict;
      pict.data[0] = bmp->pixels[0];
      pict.data[1] = bmp->pixels[2];
      pict.data[2] = bmp->pixels[1];

      pict.linesize[0] = bmp->pitches[0];
      pict.linesize[1] = bmp->pitches[2];
      pict.linesize[2] = bmp->pitches[1];

      ffmpeg_draw_frame(pict);
    
      SDL_UnlockYUVOverlay(bmp);
      
      SDL_Rect rect;
      rect.x = 0;
      rect.y = 0;
      rect.w = screen_width;
      rect.h = screen_height;
      SDL_DisplayYUVOverlay(bmp, &rect);
    }
  } else {
    fprintf(stderr, "Video decode buffer too small");
    exit(1);
  }

  return DR_OK;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_sdl = {
  .setup = sdl_setup,
  .start = NULL,
  .stop = NULL,
  .release = sdl_release,
  .submitDecodeUnit = sdl_submit_decode_unit,
};
