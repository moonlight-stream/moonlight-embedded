#pragma once

#include <switch.h>
#include <unistd.h>
#include <stdbool.h>

#define TICKS_PER_SECOND 19200000

void logInit(bool enabled);
void logPrint(const char *format, ...);

uint64_t thread();
float secondsSinceTick(uint64_t tick);