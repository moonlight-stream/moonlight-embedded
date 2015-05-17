/*
 * This file is part of Moonlight Embedded.
 * 
 * Based on Moonlight Pc implementation
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

#include <libavcodec/avcodec.h>

// Disables the deblocking filter at the cost of image quality
#define DISABLE_LOOP_FILTER 0x1
// Uses the low latency decode flag (disables multithreading)
#define LOW_LATENCY_DECODE 0x2
// Threads process each slice, rather than each frame
#define SLICE_THREADING 0x4
// Uses nonstandard speedup tricks
#define FAST_DECODE 0x8
// Uses bilinear filtering instead of bicubic
#define BILINEAR_FILTERING 0x10
// Uses a faster bilinear filtering with lower image quality
#define FAST_BILINEAR_FILTERING 0x20

int ffmpeg_init(int width, int height, int perf_lvl, int thread_count);
void ffmpeg_destroy(void);

int ffmpeg_draw_frame(AVPicture pict);
int ffmpeg_decode(unsigned char* indata, int inlen);
