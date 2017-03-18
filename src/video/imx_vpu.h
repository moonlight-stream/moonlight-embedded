/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2017 Iwan Timmer
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

#include <Limelight.h>

#include <stdbool.h>

#include <sys/types.h>

struct vpu_buf {
  void *start;
  off_t offset;
  size_t length;
};

bool vpu_init();
void vpu_setup(struct vpu_buf* buffers[], int bufferCount, int stride, int height);

bool vpu_decode(PDECODE_UNIT decodeUnit);
int vpu_get_frame();
void vpu_clear(int disp_clr_index);

void vpu_cleanup();
