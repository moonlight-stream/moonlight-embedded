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

#include "stats.h"
#include "loop.h"

#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <poll.h>
#include <sys/timerfd.h>
#include <unistd.h>

/* ---- thread safety ---------------------------------------------------- */
static pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ---- master switch ---------------------------------------------------- */
static bool stats_enabled = false;

/* ---- bitrate counters ------------------------------------------------- */
static int64_t total_bytes = 0;
static int64_t prev_total_bytes = 0;

/* ---- frame counters --------------------------------------------------- */
static int decoded_frames = 0;
static int displayed_frames = 0;
static int prev_decoded_frames = 0;
static int prev_displayed_frames = 0;

/* ---- decode timing  (per-interval accumulation) ----------------------- */
static int64_t decode_time_sum_us = 0;
static int     decode_time_count = 0;

/* ---- connection quality ----------------------------------------------- */
static int poor_connection_count = 0;

/* ---- last-report timestamp -------------------------------------------- */
static struct timespec last_report_ts;

/* -----------------------------------------------------------------------
 *  Core print logic – shared between timerfd handler and external callers
 * ----------------------------------------------------------------------- */
static void stats_do_print(double elapsed) {
    /* ---- atomically snapshot + reset per-interval counters ------------ */
    pthread_mutex_lock(&stats_mutex);

    int bytes_this  = (int)(total_bytes - prev_total_bytes);
    prev_total_bytes = total_bytes;

    int decoded_this  = decoded_frames - prev_decoded_frames;
    prev_decoded_frames = decoded_frames;

    int displayed_this = displayed_frames - prev_displayed_frames;
    prev_displayed_frames = displayed_frames;

    double avg_decode_us = (decode_time_count > 0)
                               ? (double)decode_time_sum_us / decode_time_count
                               : 0.0;
    decode_time_sum_us = 0;
    decode_time_count  = 0;

    int poor_count = poor_connection_count;
    poor_connection_count = 0;

    pthread_mutex_unlock(&stats_mutex);

    /* ---- format output ------------------------------------------------ */
    static char line[256];
    int pos = 0;

    double bitrate_mbps = (bytes_this * 8.0) / (elapsed * 1e6);
    double decode_fps   = decoded_this  / elapsed;
    double display_fps  = displayed_this / elapsed;
    double decode_ms    = avg_decode_us / 1000.0;

    pos += snprintf(line + pos, sizeof(line) - pos,
        "\r[STATS] 码率: %7.1f Mbps | 解码: %5.1f fps | 显示: %5.1f fps | 解码耗时: %5.2f ms",
        bitrate_mbps, decode_fps, display_fps, decode_ms);

    if (decoded_this > 0) {
        pos += snprintf(line + pos, sizeof(line) - pos,
            " | 帧大小: %5.0f KB",
            (double)bytes_this / decoded_this / 1024.0);
    }

    if (poor_count > 0) {
        pos += snprintf(line + pos, sizeof(line) - pos,
            " | ⚠ 弱网(%dx)", poor_count);
    }

    pos += snprintf(line + pos, sizeof(line) - pos, "        ");
    line[pos] = '\r';
    line[pos + 1] = '\0';

    fputs(line, stdout);
    fflush(stdout);
}

void stats_print(void) {
    if (!stats_enabled) return;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    double elapsed = (now.tv_sec - last_report_ts.tv_sec)
                   + (now.tv_nsec - last_report_ts.tv_nsec) / 1e9;
    if (elapsed < 1e-3)
        return;

    stats_do_print(elapsed);
    last_report_ts = now;
}

/* -----------------------------------------------------------------------
 *  Timer callback – runs once per second from the embedded event loop
 * ----------------------------------------------------------------------- */
static int stats_timer_handler(int fd) {
    uint64_t expirations;
    ssize_t r = read(fd, &expirations, sizeof(expirations));
    if (r != sizeof(expirations))
        return LOOP_OK;

    stats_print();
    return LOOP_OK;
}

/* -----------------------------------------------------------------------
 *  Public API
 * ----------------------------------------------------------------------- */
void stats_init(bool enabled) {
    stats_enabled = enabled;
    if (!stats_enabled)
        return;

    /* Create a 1-second periodic timer and register it in the event loop */
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (tfd < 0) {
        perror("stats: timerfd_create");
        return;
    }

    struct itimerspec ts = {
        .it_interval = { .tv_sec = 1, .tv_nsec = 0 },
        .it_value    = { .tv_sec = 1, .tv_nsec = 0 },
    };
    timerfd_settime(tfd, 0, &ts, NULL);

    loop_add_fd(tfd, stats_timer_handler, POLLIN);

    clock_gettime(CLOCK_MONOTONIC, &last_report_ts);
}

void stats_submit_decode_unit(int length) {
    if (!stats_enabled) return;
    pthread_mutex_lock(&stats_mutex);
    total_bytes += length;
    pthread_mutex_unlock(&stats_mutex);
}

void stats_frame_decoded(void) {
    if (!stats_enabled) return;
    pthread_mutex_lock(&stats_mutex);
    decoded_frames++;
    pthread_mutex_unlock(&stats_mutex);
}

void stats_decode_finished(int64_t elapsed_us) {
    if (!stats_enabled) return;
    pthread_mutex_lock(&stats_mutex);
    decode_time_sum_us += elapsed_us;
    decode_time_count++;
    pthread_mutex_unlock(&stats_mutex);
}

void stats_frame_displayed(void) {
    if (!stats_enabled) return;
    pthread_mutex_lock(&stats_mutex);
    displayed_frames++;
    pthread_mutex_unlock(&stats_mutex);
}

void stats_connection_status(int status) {
    if (!stats_enabled) return;
    /* Limelight defines: 1 = CONN_STATUS_OKAY, 2 = CONN_STATUS_POOR */
    if (status == 2) {
        pthread_mutex_lock(&stats_mutex);
        poor_connection_count++;
        pthread_mutex_unlock(&stats_mutex);
    }
}
