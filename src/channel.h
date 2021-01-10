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
#define liteco_chan_err_push_failed -1003
#define liteco_chan_err_success 0

typedef struct liteco_waiter_s liteco_waiter_t;
struct liteco_waiter_s {
    LITECO_LINKNODE_BASE

    liteco_co_t *co;
    bool select;
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
    bool full;

    liteco_waiter_t w;
    liteco_waiter_t r;

    bool closed;

    pthread_mutex_t mtx;

    void (*co_ready) (void *const proc, liteco_co_t *const co);
    void *proc;
};

#define liteco_chan_queue_full(c) \
    ((c)->head == (c)->tail && (c)->full)

#define liteco_chan_queue_pop(c) \
    (c)->head = ((c)->head + 1) % (c)->queue->ele_count; \
    (c)->full = false

#define liteco_chan_queue_empty(c) \
    ((c)->head == (c)->tail && !(c)->full)

#define liteco_chan_queue_push(c) \
    (c)->tail = ((c)->tail + 1) % (c)->queue->ele_count; \
    (c)->full = (c)->head == (c)->tail

int liteco_chan_create(liteco_chan_t *const chan, uint32_t ele_count,
                       void (*co_ready) (void *const , liteco_co_t *const), void *const proc);

extern void *const liteco_chan_pop_failed;
int liteco_chan_push(liteco_chan_t *const chan, void *const ele, const bool blocked);
void *liteco_chan_pop(liteco_chan_t *const chan, const bool blocked);

int liteco_chan_unenforceable_push(liteco_chan_t *const chan, void *const ele);
void *liteco_chan_unenforceable_pop(liteco_chan_t *const chan);

typedef enum liteco_casetype_e liteco_casetype_t;
enum liteco_casetype_e {
    liteco_casetype_push = 0,
    liteco_casetype_pop
};

typedef struct liteco_case_s liteco_case_t;
struct liteco_case_s {
    liteco_chan_t *chan;
    liteco_casetype_t type;
    void *ele;
};

liteco_case_t *liteco_select(liteco_case_t *const cases, const uint32_t count, const bool blocked);

#endif
