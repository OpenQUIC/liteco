/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_PLATFORM_DARWIN_INTERNAL_H__
#define __LITECO_PLATFORM_DARWIN_INTERNAL_H__

#include "liteco.h"
#include <stdbool.h>
#include <sys/cdefs.h>

#define liteco_access_memory(var) (*(volatile typeof(var) *) &(var))

int liteco_platform_pipe(int fds[2], const int flags);
int liteco_platform_cloexec(int fd, bool set);
int liteco_platform_nonblock(int fd, bool set);
int liteco_platform_close(int fd);

extern bool liteco_cas(uint32_t *const ptr, const uint32_t old, const uint32_t next);

#ifdef __header_always_inline
__header_always_inline void liteco_cpurelax() {
#else 
__always_inline void liteco_cpurelax() {
#endif
#if defined(__x86_64__)
    __asm__ __volatile__ ("rep; nop");
#endif
}

int liteco_eloop_init_async(liteco_eloop_t *const eloop);

int liteco_eloop_async_send(liteco_eloop_t *const eloop);

#define container_of(ptr, type, member) ((type *) ((char *) (ptr) - offsetof(type, member)))

#endif
