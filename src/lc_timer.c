/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "lc_timer.h"
#include <unistd.h>

static void liteco_timer_cb(liteco_emodule_t *const emodule);

int liteco_timer_init(liteco_eloop_t *const eloop, liteco_timer_t *const timer, liteco_chan_t *const chan) {
    timer->fd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);
    timer->cb = liteco_timer_cb;
    timer->chan = chan;

    return liteco_eloop_add(eloop, (liteco_emodule_t *) timer);
}

int liteco_timer_expire(liteco_timer_t *const timer, const uint64_t timeout, const uint64_t interval) {
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
        liteco_chan_unenforceable_push(timer->chan, NULL);
    }
}

int liteco_timer_close(liteco_timer_t *const timer) {
    close(timer->fd);

    return liteco_timer_err_success;
}
