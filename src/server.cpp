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

#include <stdio.h>
#include <functional>

extern "C" {
    #include "libgamestream/errors.h"
}

#define MOONLIGHT_DATA_DIR "sdmc:/switch/moonlight-switch/"

Server::Server(PCONFIGURATION config)
    : config_(config),
      opened_(false),
      server_{0},
      pin_{0}
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
    apps_promise_ = new promise<PAPP_LIST, ServerError>();
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

promise<PAPP_LIST, ServerError> *Server::apps() {
    // Tell the worker to fetch the app list, and return a future
    ueventSignal(&event_apps_);
    return apps_promise_;
}

void Server::startStream() {

}

void Server::stopStream() {

}

CONNECTION_LISTENER_CALLBACKS Server::getCallbacks() {
    return {0};
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
    logPrint("Before gs_pair\n");
    
    int ret = gs_pair(&server_, &pin_[0]);
    logPrint("Got gs_pair result: %d\n", ret);

    if (ret == GS_OK) {
        logPrint("Resolving gs_pair\n");
        pair_promise_->resolve(true);
    }
    else {
        logPrint("[thread:%ld] Rejecting gs_pair\n");
        pair_promise_->reject(ServerError(ret, gs_error));
    }
}

void Server::threadApps_() {
    // Get the apps list from the streaming server
//    int rc = gs_applist(&server_, &apps_);

//    if (rc != GS_OK) {
//      apps_ = nullptr;
//    }

    // svcSleepThread(3000000000ull);

    // Update the promise state
    // apps_promise_.resolve("Sample apps string");
}

void Server::threadStartStream_() {

}

void Server::threadStopStream_() {
    
}


////pthread_t main_thread_id = 0;
////bool connection_debug;

////int pair_check(PSERVER_DATA server) {
////  if (!server->paired) {
////    fprintf(stderr, "You must pair with the PC first\n");
////    return 0;
////  }

////  return 1;
////}

////size_t get_app_list(PSERVER_DATA server, PAPP_LIST *list) {
////  if (gs_applist(server, list) != GS_OK) {
////    fprintf(stderr, "Can't get app list\n");
////    *list = NULL;
////    return 0;
////  }

////  PAPP_LIST curr = *list;
////  size_t count = 0;
////  while (curr) {
////    count++;
////    curr = curr->next;
////  }

////  return count;
////}

////int get_app_id(PSERVER_DATA server, const char *name) {
////  PAPP_LIST list = NULL;
////  if (gs_applist(server, &list) != GS_OK) {
////    fprintf(stderr, "Can't get app list\n");
////    return -1;
////  }

////  while (list != NULL) {
////    if (strcmp(list->name, name) == 0)
////      return list->id;

////    list = list->next;
////  }
////  return -1;
////}

////int stream_start(PSERVER_DATA server, PCONFIGURATION config, int appId, enum platform system) {
////  int gamepads = 0;
////  int gamepad_mask = 0;
////  for (int i = 0; i < gamepads && i < 4; i++)
////    gamepad_mask = (gamepad_mask << 1) + 1;

////  int ret = gs_start_app(server, &config->stream, appId, config->sops, config->localaudio, gamepad_mask);
////  if (ret < 0) {
////    if (ret == GS_NOT_SUPPORTED_4K)
////      fprintf(stderr, "Server doesn't support 4K\n");
////    else if (ret == GS_NOT_SUPPORTED_MODE)
////      fprintf(stderr, "Server doesn't support %dx%d (%d fps) or try --unsupported option\n", config->stream.width, config->stream.height, config->stream.fps);
////    else if (ret == GS_ERROR)
////      fprintf(stderr, "Gamestream error: %s\n", gs_error);
////    else
////      fprintf(stderr, "Errorcode starting app: %d\n", ret);

////    return -1;
////  }

////  int drFlags = 0;
////  if (config->fullscreen)
////    drFlags |= DISPLAY_FULLSCREEN;

////  if (config->debug_level > 0) {
////    printf("Stream %d x %d, %d fps, %d kbps\n", config->stream.width, config->stream.height, config->stream.fps, config->stream.bitrate);
////    connection_debug = true;
////  }

////  platform_start(system);
////  LiStartConnection(&server->serverInfo, &config->stream, &connection_callbacks, platform_get_video(system), platform_get_audio(system, config->audio_device), NULL, drFlags, config->audio_device, 0);

////  return 0;
////}

////int stream_stop(enum platform system) {
////  LiStopConnection();
////  platform_stop(system);

////  return 0;
////}

////// Moonlight connection callbacks
////static void connection_stage_starting(int stage) {}
////static void connection_stage_complete(int stage) {}
////static void connection_stage_failed(int stage, long errorCode) {}
////static void connection_started(void) {
////  printf("[*] Connection started\n");
////}
////static void connection_terminated(long error) {
////  perror("[*] Connection terminated");
////}
////static void connection_display_message(const char *msg) {
////  printf("[*] %s\n", msg);
////}
////static void connection_display_transient_message(const char *msg) {
////  printf("[*] %s\n", msg);
////}
////static void connection_log_message(const char* format, ...) {
////  va_list arglist;
////  va_start(arglist, format);
////  vprintf(format, arglist);
////  va_end(arglist);
////}

////CONNECTION_LISTENER_CALLBACKS connection_callbacks = {
////  .stageStarting = connection_stage_starting,
////  .stageComplete = connection_stage_complete,
////  .stageFailed = connection_stage_failed,
////  .connectionStarted = connection_started,
////  .connectionTerminated = connection_terminated,
////  .displayMessage = connection_display_message,
////  .displayTransientMessage = connection_display_transient_message,
////  .logMessage = connection_log_message,
////};
