/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_CHANNEL_H__
#define __LITECO_CHANNEL_H__

#include "array.h"
#include "link.h"
#include "coroutine.h"
#include <pthread.h>

#define liteco_chan_err_closed -1001
#define liteco_chan_err_internal_error -1002
#define liteco_chan_err_success 0

typedef struct liteco_waiter_s liteco_waiter_t;
struct liteco_waiter_s {
    LITECO_LINKNODE_BASE

    liteco_co_t *co;
    union {
        void **r_store;
        void *s_ele;
    } ele;
};

typedef struct liteco_chan_s liteco_chan_t;
struct liteco_chan_s {
    liteco_array_t *queue;
    uint32_t head;
    uint32_t tail;

    liteco_waiter_t w;
    liteco_waiter_t r;

    bool closed;

    pthread_mutex_t mtx;

    void (*co_ready) (void *const proc, liteco_co_t *const co);
    void *proc;
};

int liteco_chan_create(liteco_chan_t *const chan, uint32_t ele_count,
                       void (*co_ready) (void *const , liteco_co_t *const), void *const proc);

int liteco_chan_push(liteco_chan_t *const chan, void *const ele);
void *liteco_chan_pop(liteco_chan_t *const chan);

#endif
