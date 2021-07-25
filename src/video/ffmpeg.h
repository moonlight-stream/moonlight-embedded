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

#include <stdbool.h>

#include <libavcodec/avcodec.h>

// Enable multi-threaded decoding
#define SLICE_THREADING 0x4
// Uses hardware acceleration
#define VDPAU_ACCELERATION 0x40
#define VAAPI_ACCELERATION 0x80

enum decoders {SOFTWARE, VDPAU, VAAPI};
extern enum decoders ffmpeg_decoder;

int ffmpeg_init(int videoFormat, int width, int height, int perf_lvl, int buffer_count, int thread_count);
void ffmpeg_destroy(void);

int ffmpeg_draw_frame(AVFrame *pict);
AVFrame* ffmpeg_get_frame(bool native_frame);
int ffmpeg_decode(unsigned char* indata, int inlen);
