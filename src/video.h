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

#include "limelight-common/Limelight.h"

extern DECODER_RENDERER_CALLBACKS decoder_callbacks_fake;
#ifdef HAVE_SDL
extern DECODER_RENDERER_CALLBACKS decoder_callbacks_sdl;
#endif
#ifdef HAVE_OMX
extern DECODER_RENDERER_CALLBACKS decoder_callbacks_omx;
#endif
#ifdef HAVE_IMX
extern DECODER_RENDERER_CALLBACKS decoder_callbacks_imx;
bool video_imx_init();
#endif
