/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "timer.h"
#include <unistd.h>

static void liteco_timer_cb(liteco_emodule_t *const emodule);

uint8_t timer_co;
liteco_co_t *const liteco_timer_co = (liteco_co_t *) &timer_co;

int liteco_timer_create(liteco_timer_t *const timer, void (*co_ready) (void *const, liteco_co_t *const), void *const proc) {
    timer->fd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);
    timer->cb = liteco_timer_cb;

    liteco_chan_create(&timer->chan, 0, co_ready, proc);

    return liteco_timer_err_success;
}

int liteco_timer_set(liteco_timer_t *const timer, const uint64_t timeout, const uint64_t interval) {
    struct itimerspec spec = {
        .it_value = {
            .tv_sec = timeout / (1000 * 1000),
            .tv_nsec = timeout % (1000 * 1000) * 1000
        },
        .it_interval = {
            .tv_sec = interval / (1000 * 1000),
            .tv_nsec = interval % (1000 * 1000) * 1000
        }
    };

    timerfd_settime(timer->fd, 0, &spec, NULL);

    return liteco_timer_err_success;
}

static void liteco_timer_cb(liteco_emodule_t *const emodule) {
    liteco_timer_t *const timer = (liteco_timer_t *) emodule;

    uint64_t exp;
    while (read(timer->fd, &exp, sizeof(uint64_t)) == sizeof(uint64_t) && exp == 1) {
        liteco_timerchan_expired(&timer->chan);
    }
}
