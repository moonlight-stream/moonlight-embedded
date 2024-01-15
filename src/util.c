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

#include "util.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef HAVE_GETAUXVAL
#include <sys/auxv.h>

#ifndef HWCAP2_AES
#define HWCAP2_AES (1 << 0)
#endif
#endif

int write_bool(char *path, bool val) {
  int fd = open(path, O_RDWR);

  if(fd >= 0) {
    int ret = write(fd, val ? "1" : "0", 1);
    if (ret < 0)
      fprintf(stderr, "Failed to write %d to %s: %d\n", val ? 1 : 0, path, ret);

    close(fd);
    return 0;
  } else
    return -1;
}

int read_file(char *path, char* output, int output_len) {
  int fd = open(path, O_RDONLY);

  if(fd >= 0) {
    output_len = read(fd, output, output_len);
    close(fd);
    return output_len;
  } else
    return -1;
}

bool ensure_buf_size(void **buf, size_t *buf_size, size_t required_size) {
  if (*buf_size >= required_size)
    return false;

  *buf_size = required_size;
  *buf = realloc(*buf, *buf_size);
  if (!*buf) {
    fprintf(stderr, "Failed to allocate %zu bytes\n", *buf_size);
    abort();
  }

  return true;
}

bool has_fast_aes() {
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if defined(HAVE_GETAUXVAL) && (defined(__arm__) || defined(__aarch64__))
  #if defined(__arm__) && defined(HWCAP2_AES)
    return !!(getauxval(AT_HWCAP2) & HWCAP2_AES);
  #elif defined(__aarch64__)
    return !!(getauxval(AT_HWCAP) & HWCAP_AES);
  #else
    return false;
  #endif
#elif __has_builtin(__builtin_cpu_supports) && (defined(__i386__) || defined(__x86_64__))
  return __builtin_cpu_supports("aes");
#elif defined(__BUILTIN_CPU_SUPPORTS__) && defined(__powerpc__)
  return __builtin_cpu_supports("vcrypto");
#elif defined(__riscv)
  // TODO: Implement detection of RISC-V vector crypto extension when possible.
  // At the time of writing, no RISC-V hardware has it, so hardcode it off.
  return false;
#elif __SIZEOF_SIZE_T__ == 4
  #warning Unknown 32-bit platform. Assuming AES is slow on this CPU.
  return false;
#else
  #warning Unknown 64-bit platform. Assuming AES is fast on this CPU.
  return true;
#endif
}