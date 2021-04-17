/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_PLATFORM_INTERNAL_H__
#define __LITECO_PLATFORM_INTERNAL_H__

#include "liteco.h"
#if defined(__APPLE__)
#include "platform/darwin/internal.h"
#elif defined(__linux__)
#endif
#include <stdbool.h>
#include <sys/cdefs.h>

#define liteco_access_memory(var) (*(volatile typeof(var) *) &(var))

int liteco_platform_pipe(int fds[2], const int flags);
int liteco_platform_cloexec(int fd, bool set);
int liteco_platform_nonblock(int fd, bool set);
int liteco_platform_close(int fd);

extern bool liteco_cas(uint32_t *const ptr, const uint32_t old, const uint32_t next);

__liteco_header_inline void liteco_cpurelax() {
#if defined(__x86_64__)
    __asm__ __volatile__ ("rep; nop");
#endif
}

int liteco_eloop_async_init(liteco_eloop_t *const eloop);
int liteco_eloop_async_send(liteco_eloop_t *const eloop);

int liteco_eloop_timer_add(liteco_eloop_t *const eloop, liteco_timer_t *const timer);
int liteco_eloop_timer_remove(liteco_eloop_t *const eloop, liteco_timer_t *const timer);

#define container_of(ptr, type, member) ((type *) ((char *) (ptr) - offsetof(type, member)))

extern void liteco_context_init(liteco_context_t *const context, uint8_t *const st, size_t st_size, void (*fn) (void *const), void *const args);
extern void liteco_context_swap(liteco_context_t const from, liteco_context_t const to);
extern void liteco_cas_yield(uint32_t n);

void liteco_set_status(liteco_co_t *const co, const liteco_status_t from, const liteco_status_t to);

__liteco_header_inline void liteco_co_lock(liteco_co_t *const co) {
    pthread_mutex_lock(&co->mtx);
}

__liteco_header_inline void liteco_co_unlock(liteco_co_t *const co) {
    pthread_mutex_unlock(&co->mtx);
}

extern liteco_co_t *const liteco_ignore_awake_co;

#endif
