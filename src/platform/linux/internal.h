/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_PLATFORM_LINUX_INTERNAL_H__
#define __LITECO_PLATFORM_LINUX_INTERNAL_H__

#include "liteco.h"
#include <stdint.h>
#include <sys/time.h>

#define __liteco_header_inline static inline

__liteco_header_inline void liteco_timer_set_timeout(liteco_timer_t *const timer, const uint64_t timeout) {
    timer->timeout.tv_sec = timeout / (1000 * 1000);
    timer->timeout.tv_nsec = (timeout % (1000 * 1000)) * 1000;
}

__liteco_header_inline void liteco_timer_set_interval(liteco_timer_t *const timer, const uint64_t interval) {
    timer->interval.tv_sec = interval / (1000 * 1000);
    timer->interval.tv_nsec = (interval % (1000 * 1000)) * 1000;
}

__liteco_header_inline void liteco_timer_set_timeout_current(liteco_timer_t *const timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->timeout);
}

__liteco_header_inline void liteco_timer_add_timeout(liteco_timer_t *const timer, const uint64_t timeout) {
    timer->timeout.tv_sec += timeout / (1000 * 1000);
    timer->timeout.tv_nsec += (timeout % (1000 * 1000)) * 1000;
    timer->timeout.tv_sec += timer->timeout.tv_nsec / (1000 * 1000 * 1000);
    timer->timeout.tv_nsec %= 1000 * 1000 * 1000;
}

__liteco_header_inline void liteco_timer_add_timeout_interval(liteco_timer_t *const timer) {
    timer->timeout.tv_sec += timer->interval.tv_sec;
    timer->timeout.tv_nsec += timer->interval.tv_nsec;
    timer->timeout.tv_sec += timer->timeout.tv_nsec / (1000 * 1000 * 1000);
    timer->timeout.tv_nsec %= 1000 * 1000 * 1000;
}

__liteco_header_inline bool liteco_timer_active(liteco_timer_t *const timer, struct timespec curr) {
    if (timer->timeout.tv_sec < curr.tv_sec) {
        return true;
    }
    else if (timer->timeout.tv_sec == curr.tv_sec && timer->timeout.tv_nsec < curr.tv_nsec) {
        return true;
    }

    return false;
}

__liteco_header_inline struct timespec liteco_timer_timerfd_timeout(liteco_timer_t *const timer) {
    return timer->timeout;
}

#endif
