#include "util.h"

#include <cstdarg>
#include <stdio.h>

struct LoggerData {
    Mutex log_mutex_;
    bool enabled_;
    uint64_t first_tick_;
} __logger_data;

void logInit(bool enabled) {
    __logger_data.log_mutex_ = 0;
    __logger_data.enabled_ = enabled;
    __logger_data.first_tick_ = svcGetSystemTick();
}

void logPrint(const char *format, ...) {
    if (!__logger_data.enabled_) {
        return;
    }
    
    mutexLock(&__logger_data.log_mutex_);
    va_list va;
    va_start(va, format);

    printf("[thread:%ld]", thread());
    printf("[time:%f]", secondsSinceTick(__logger_data.first_tick_));
    printf(" ");
    vprintf(format, va);

    va_end(va);
    mutexUnlock(&__logger_data.log_mutex_);
}

uint64_t thread() {
    uint64_t thread_id = 0;
    svcGetThreadId(&thread_id, CUR_THREAD_HANDLE);
    return thread_id;
}

float secondsSinceTick(uint64_t tick) {
    return (float)(svcGetSystemTick() - tick) / TICKS_PER_SECOND;
}