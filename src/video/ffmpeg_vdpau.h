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

#include <X11/Xlib.h>
#include <libavcodec/avcodec.h>

int vdpau_init_lib(Display* display);
int vdpau_init(AVCodecContext* decoder_ctx, int width, int height);
void vdpau_destroy();
AVFrame* vdpau_get_frame(AVFrame* dec_frame);
int vdpau_init_presentation(Drawable win, int width, int height, int display_width, int display_height);
void vdpau_queue(AVFrame* dec_frame);
