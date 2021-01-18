/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_TIMER_H__
#define __LITECO_TIMER_H__

#include "lc_coroutine.h"
#include "lc_emodule.h"
#include "lc_channel.h"
#include "lc_eloop.h"
#include <stdint.h>
#include <sys/timerfd.h>

#define liteco_timer_err_success 0

typedef struct liteco_timer_s liteco_timer_t;
struct liteco_timer_s {
    LITECO_EMODULE_FIELD

    liteco_chan_t *chan;
};

int liteco_timer_init(liteco_eloop_t *const eloop, liteco_timer_t *const timer, liteco_chan_t *const chan);
int liteco_timer_expire(liteco_timer_t *const timer, const uint64_t timeout, const uint64_t interval);
int liteco_timer_close(liteco_timer_t *const timer);

#endif
