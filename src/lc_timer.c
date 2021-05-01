/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "liteco.h"
#include "liteco/lc_heap.h"
#include "platform/internal.h"
#include <sys/resource.h>
#include <unistd.h>

static void liteco_timer_chan_cb(liteco_timer_t *const timer);

int liteco_timer_init(liteco_eloop_t *const eloop, liteco_timer_t *const timer) {
    liteco_eloop_timer_init(eloop);

    liteco_handler_init(timer, eloop, liteco_handler_type_timer);
    liteco_heapnode_init(&timer->hp_handle);

    timer->cb = NULL;
    timer->active = false;
    timer->stop = false;

    liteco_timer_set_timeout(timer, 0);
    liteco_timer_set_interval(timer, 0);

    return 0;
}

int liteco_timer_start(liteco_timer_t *const timer, liteco_timer_cb cb, const uint64_t timeout, const uint64_t interval) {
    timer->stop = false;
    timer->cb = cb;

    liteco_timer_set_timeout_current(timer);
    liteco_timer_set_interval(timer, interval);
    if (timeout == 0) {
        liteco_timer_add_timeout_interval(timer);
    }
    else {
        liteco_timer_add_timeout(timer, timeout);
    }

    liteco_eloop_timer_add(timer->eloop, timer);

    return 0;
}

int liteco_timer_stop(liteco_timer_t *const timer) {
    timer->stop = true;
    liteco_eloop_timer_remove(timer->eloop, timer);
    return 0;
}

int liteco_timer_chan_init(liteco_eloop_t *const eloop, liteco_runtime_t *const rt, liteco_timer_chan_t *const tchan) {
    liteco_chan_init(&tchan->chan, 0, rt);
    liteco_timer_init(eloop, &tchan->timer);

    return 0;
}

int liteco_timer_chan_start(liteco_timer_chan_t *const tchan, const uint64_t timeout, const uint64_t interval) {
    return liteco_timer_start(&tchan->timer, liteco_timer_chan_cb, timeout, interval);
}

static void liteco_timer_chan_cb(liteco_timer_t *const timer) {
    liteco_timer_chan_t *const tchan = container_of(timer, liteco_timer_chan_t, timer);

    liteco_chan_unenforceable_push(&tchan->chan, NULL);
}

int liteco_timer_chan_stop(liteco_timer_chan_t *const tchan) {
    return liteco_timer_stop(&tchan->timer);
}
