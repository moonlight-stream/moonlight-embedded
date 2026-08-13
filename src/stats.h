/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2026 Moonlight Embedded contributors
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

#pragma once

#include <stdbool.h>
#include <stdint.h>

/** Initialize stats collection and register a 1-second periodic reporter.
 *  Pass enabled=true to turn on statistics, false to make everything a no-op. */
void stats_init(bool enabled);

/** Called when a decode unit arrives — measures receive bitrate */
void stats_submit_decode_unit(int length);

/** Called when a frame has been decoded */
void stats_frame_decoded(void);

/** Called with decode duration in microseconds */
void stats_decode_finished(int64_t elapsed_us);

/** Called when a frame is displayed on screen */
void stats_frame_displayed(void);

/** Called on connection status update (pass the raw status code) */
void stats_connection_status(int status);

/** Force-print stats to stdout immediately */
void stats_print(void);
