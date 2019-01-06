/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015-2017 Iwan Timmer
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

#include <switch.h>

#include "config.h"
#include "promise.h"

//#include <Limelight.h>
//#include <client.h>
//#include <errors.h>

typedef char* PAPP_LIST;
typedef void* CONNECTION_LISTENER_CALLBACKS;
typedef void* SERVER_DATA;

class Server {
public:
    Server(PCONFIGURATION config);
    ~Server();

    void open();
    void close();

    bool opened();
    bool paired();
    future<PAPP_LIST> apps();

    void startStream();
    void stopStream();

    CONNECTION_LISTENER_CALLBACKS getCallbacks();

private:
    void thread_();
    void threadApps_();
    void threadStartStream_();
    void threadStopStream_();
    void threadClose_();

    SERVER_DATA server_;
    PCONFIGURATION config_;

    bool opened_;
    Thread worker_thread_;
    UEvent event_apps_;
    UEvent event_start_stream_;
    UEvent event_stop_stream_;
    UEvent event_close_;

    PAPP_LIST apps_;
    promise<PAPP_LIST> apps_promise_;

    CONNECTION_LISTENER_CALLBACKS callbacks_;
};

//int pair_check(PSERVER_DATA server);
//size_t get_app_list(PSERVER_DATA server, PAPP_LIST *list);
//int get_app_id(PSERVER_DATA server, const char *name);

//int stream_start(PSERVER_DATA server, PCONFIGURATION config, int appId, enum platform system);
//int stream_stop(enum platform system);

//extern CONNECTION_LISTENER_CALLBACKS connection_callbacks;
//extern bool connection_debug;
