///*
// * This file is part of Moonlight Embedded.
// *
// * Copyright (C) 2015-2017 Iwan Timmer
// *
// * Moonlight is free software; you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation; either version 3 of the License, or
// * (at your option) any later version.
// *
// * Moonlight is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with Moonlight; if not, see <http://www.gnu.org/licenses/>.
// */

#include "server.h"
#include "application-info.h"

#include <stdio.h>
#include <stdarg.h>
#include <functional>

extern "C" {
    #include "libgamestream/errors.h"
    #include "video/video.h"
    #include "audio/audio.h"
}

#define MOONLIGHT_DATA_DIR "sdmc:/switch/moonlight-switch/"

// Moonlight connection callbacks
static void connection_stage_starting(int stage) {}
static void connection_stage_complete(int stage) {}
static void connection_stage_failed(int stage, long errorCode) {}
static void connection_started(void) {
 printf("[*] Connection started\n");
}

static void connection_terminated(long error) {
 perror("[*] Connection terminated");
}

static void connection_display_message(const char *msg) {
 printf("[*] %s\n", msg);
}

static void connection_display_transient_message(const char *msg) {
 printf("[*] %s\n", msg);
}

static void connection_log_message(const char* format, ...) {
 va_list arglist;
 va_start(arglist, format);
 vprintf(format, arglist);
 va_end(arglist);
}

CONNECTION_LISTENER_CALLBACKS connection_callbacks = {
    connection_stage_starting,
    connection_stage_complete,
    connection_stage_failed,
    connection_started,
    connection_terminated,
    connection_display_message,
    connection_display_transient_message,
    connection_log_message,
};



Server::Server(PCONFIGURATION config)
    : config_(config),
      opened_(false),
      server_{0},
      pin_{0},
      apps_{0}
{
    // Initialize the random PIN used for authentication
    // sprintf(props.pin, "%d%d%d%d", (int)random() % 10, (int)random() % 10, (int)random() % 10, (int)random() % 10);
    sprintf(pin_, "0000");

    threadCreate(
        &worker_thread_,
        [](void *context) { static_cast<Server *>(context)->thread_(); },
        this,
        0x10000,
        0x2C,
        -2
    );

    ueventCreate(&event_connect_, true);
    ueventCreate(&event_pair_, true);
    ueventCreate(&event_apps_, true);
    ueventCreate(&event_start_stream_, true);
    ueventCreate(&event_stop_stream_, true);
    ueventCreate(&event_close_, true);

    connect_promise_ = new promise<bool, ServerError>();
    pair_promise_ = new promise<bool, ServerError>();
    apps_promise_ = new promise<std::vector<ApplicationInfo>, ServerError>();
    start_stream_promise_ = new promise<bool, ServerError>();
    stop_stream_promise_ = new promise<bool, ServerError>();
}

Server::~Server() {
    threadClose(&worker_thread_);

    delete connect_promise_;
    delete pair_promise_;
    delete apps_promise_;
}

void Server::open() {
    threadStart(&worker_thread_);
    opened_ = true;
}

void Server::close() {
    ueventSignal(&event_close_);
    threadWaitForExit(&worker_thread_);
    opened_ = false;
}

bool Server::opened() {
    return opened_;
}

bool Server::paired() {
    return server_.paired;
}

const char *Server::pin() {
    return pin_;
}

promise<bool, ServerError> *Server::connect() {
    // Tell the worker to initiate the connection to the server
    ueventSignal(&event_connect_);

    logPrint("Did signal connect\n");
    return connect_promise_;
}

promise<bool, ServerError> *Server::pair() {
    // Tell the worker to initiate server pairing
    ueventSignal(&event_pair_);
    
    logPrint("Did signal pair\n");
    return pair_promise_;
}

promise<std::vector<ApplicationInfo>, ServerError> *Server::apps() {
    // Tell the worker to fetch the app list, and return a future
    ueventSignal(&event_apps_);
    return apps_promise_;
}

promise<bool, ServerError> *Server::startStream(int id) {
    // Tell the worker to start streaming the given ID
    stream_app_id_ = id;
    ueventSignal(&event_start_stream_);
    return start_stream_promise_;
}

promise<bool, ServerError> *Server::stopStream() {
    // Tell the worker to stop streaming the given ID
    stream_app_id_ = -1;
    ueventSignal(&event_stop_stream_);
    return stop_stream_promise_;
}

CONNECTION_LISTENER_CALLBACKS Server::getCallbacks() {
    return connection_callbacks;
}

void Server::thread_() {
    logPrint("Server thread started\n");

    Result rc;
    int index = -1;

    while (1) {
        rc = waitMulti(&index, -1,
                       waiterForUEvent(&event_connect_),
                       waiterForUEvent(&event_pair_),
                       waiterForUEvent(&event_apps_),
                       waiterForUEvent(&event_start_stream_),
                       waiterForUEvent(&event_stop_stream_),
                       waiterForUEvent(&event_close_));

        if (R_SUCCEEDED(rc)) {
            logPrint("Got index of thread event: %d\n", index);

            switch (index) {
            case 0: threadConnect_(); break;
            case 1: threadPair_(); break;
            case 2: threadApps_(); break;
            case 3: threadStartStream_(); break;
            case 4: threadStopStream_(); break;
            }

            if (index == 5) {
                // Close connection
                break;
            }
        }
    };
}

void Server::threadConnect_() {
    int ret = gs_init(&server_, config_->address, MOONLIGHT_DATA_DIR "key", config_->debug_level, config_->unsupported);

    if (ret == GS_OK) {
        connect_promise_->resolve(true);
    }
    else {
        connect_promise_->reject(ServerError(ret, gs_error));
    }
}

void Server::threadPair_() {
    int ret = gs_pair(&server_, &pin_[0]);

    if (ret == GS_OK) {
        pair_promise_->resolve(true);
    }
    else {
        pair_promise_->reject(ServerError(ret, gs_error));
    }
}

void Server::threadApps_() {
    // Get the apps list from the streaming server
    int ret = gs_applist(&server_, &apps_);

    if (ret == GS_OK) {
        std::vector<ApplicationInfo> apps = ApplicationInfo::wrapList(apps_);
        apps_promise_->resolve(apps);

        // Load each of the images for each app
        for (auto app : apps) {
            char *image_data;
            size_t image_size;
            int image_ret = gs_app_boxart(&server_, app.id, &image_data, &image_size);

            if (image_ret == GS_OK) {
                app.image->resolve(std::make_pair(image_data, image_size));
            }
            else {
                app.image->reject(ServerError(image_ret, gs_error));
            }
        }
    }
    else {
        apps_promise_->reject(ServerError(ret, gs_error));
    }
}

void Server::threadStartStream_() {
    int gamepads = 0;
    int gamepad_mask = 0;
    for (int i = 0; i < gamepads && i < 4; i++)
        gamepad_mask = (gamepad_mask << 1) + 1;

    int ret = gs_start_app(&server_, &config_->stream, stream_app_id_, config_->sops, config_->localaudio, gamepad_mask);

    if (ret < 0) {
        char err[255];

        if (ret == GS_NOT_SUPPORTED_4K)
            sprintf(err, "Server doesn't support 4K\n");
        else if (ret == GS_NOT_SUPPORTED_MODE)
            sprintf(err, "Server doesn't support %dx%d (%d fps) or try --unsupported option\n", config_->stream.width, config_->stream.height, config_->stream.fps);
        else if (ret == GS_ERROR)
            sprintf(err, "Game stream error: %s\n", gs_error);
        else
            sprintf(err, "Error code starting app: %d\n", ret);

        start_stream_promise_->reject(ServerError(ret, err));
        return;
    }

    int drFlags = 0;
    if (config_->fullscreen) {
        drFlags |= DISPLAY_FULLSCREEN;
    }

    if (config_->debug_level > 0) {
        printf("Stream %d x %d, %d fps, %d kbps\n", config_->stream.width, config_->stream.height, config_->stream.fps, config_->stream.bitrate);
    }

    LiStartConnection(&server_.serverInfo, &config_->stream, &connection_callbacks, &decoder_callbacks_switch, &audio_callbacks_switch, NULL, drFlags, config_->audio_device, 0);

    start_stream_promise_->resolve(true);
}

void Server::threadStopStream_() {
    LiStopConnection();
    stop_stream_promise_->resolve(true);
}
