/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_TIMER_H__
#define __LITECO_TIMER_H__

#include "coroutine.h"
#include "emodule.h"
#include "channel.h"
#include <stdint.h>
#include <sys/timerfd.h>

#define liteco_timer_err_success 0

typedef struct liteco_timer_s liteco_timer_t;
struct liteco_timer_s {
    LITECO_EMODULE_FIELD

    liteco_chan_t chan;
};

int liteco_timer_create(liteco_timer_t *const timer, void (*co_ready) (void *const, liteco_co_t *const), void *const proc);
int liteco_timer_set(liteco_timer_t *const timer, const uint64_t timeout, const uint64_t interval);

#endif
