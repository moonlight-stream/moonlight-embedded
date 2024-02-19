/*
 * This file is part of Moonlight Embedded.
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

#include "cpu.h"
#include "util.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_GETAUXVAL
#include <sys/auxv.h>

#ifndef HWCAP2_AES
#define HWCAP2_AES (1 << 0)
#endif
#endif

#if defined(__linux__) && defined(__riscv)
#if __has_include(<sys/hwprobe.h>)
#include <sys/hwprobe.h>
#else
#include <unistd.h>

#if __has_include(<asm/hwprobe.h>)
#include <asm/hwprobe.h>
#include <sys/syscall.h>
#else
#define __NR_riscv_hwprobe 258
struct riscv_hwprobe {
    int64_t key;
    uint64_t value;
};
#define RISCV_HWPROBE_KEY_IMA_EXT_0 4
#endif

// RISC-V Scalar AES [E]ncryption and [D]ecryption
#ifndef RISCV_HWPROBE_EXT_ZKND
#define RISCV_HWPROBE_EXT_ZKND (1 << 11)
#define RISCV_HWPROBE_EXT_ZKNE (1 << 12)
#endif

// RISC-V Vector AES
#ifndef RISCV_HWPROBE_EXT_ZVKNED
#define RISCV_HWPROBE_EXT_ZVKNED (1 << 21)
#endif

static int __riscv_hwprobe(struct riscv_hwprobe *pairs, size_t pair_count,
                           size_t cpu_count, unsigned long *cpus,
                           unsigned int flags)
{
    return syscall(__NR_riscv_hwprobe, pairs, pair_count, cpu_count, cpus, flags);
}

#endif
#endif

bool has_fast_aes() {
#if defined(HAVE_GETAUXVAL) && (defined(__arm__) || defined(__aarch64__))
  #if defined(__arm__) && defined(HWCAP2_AES)
    return !!(getauxval(AT_HWCAP2) & HWCAP2_AES);
  #elif defined(__aarch64__)
    return !!(getauxval(AT_HWCAP) & HWCAP_AES);
  #else
    return false;
  #endif
#elif defined(HAVE_BICS_AES)
  return __builtin_cpu_supports("aes");
#elif defined(__BUILTIN_CPU_SUPPORTS__) && defined(__powerpc__)
  return __builtin_cpu_supports("vcrypto");
#elif defined(__linux__) && defined(__riscv)
    struct riscv_hwprobe pairs[1] = {
        { RISCV_HWPROBE_KEY_IMA_EXT_0, 0 },
    };

    // If this syscall is not implemented, we'll get -ENOSYS
    // and the value field will remain zero.
    __riscv_hwprobe(pairs, sizeof(pairs) / sizeof(struct riscv_hwprobe), 0, NULL, 0);

    return (pairs[0].value & (RISCV_HWPROBE_EXT_ZKNE | RISCV_HWPROBE_EXT_ZKND)) ==
               (RISCV_HWPROBE_EXT_ZKNE | RISCV_HWPROBE_EXT_ZKND) ||
           (pairs[0].value & RISCV_HWPROBE_EXT_ZVKNED);
#elif __SIZEOF_SIZE_T__ == 4
  #warning Unknown 32-bit platform. Assuming AES is slow on this CPU.
  return false;
#else
  #warning Unknown 64-bit platform. Assuming AES is fast on this CPU.
  return true;
#endif
}

bool has_slow_aes() {
#ifdef __arm__
  char cpuinfo[4096] = {};
  if (read_file("/proc/cpuinfo", cpuinfo, sizeof(cpuinfo) - 1) > 0) {
    // If this is a ARMv6 CPU (like the Pi 1), we'll assume it's not
    // powerful enough to handle audio encryption. The Pi 1 could
    // barely handle Opus decoding alone.
    if (strstr(cpuinfo, "ARMv6")) {
      return true;
    }
  }
#endif

  return false;
}