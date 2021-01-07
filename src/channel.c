/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "channel.h"

static inline bool liteco_chan_wblocked(liteco_chan_t *const chan);
static inline bool liteco_chan_rblocked(liteco_chan_t *const chan);
static inline void liteco_chan_lock(liteco_chan_t *const chan);
static inline void liteco_chan_unlock(liteco_chan_t *const chan);
// 将当前协程加入到写等待队列
static inline int liteco_chan_wblock(liteco_chan_t *const chan, void *const ele);
// 将ele写入到chan中，返回值若为true时，则当前协程继续执行；返回值若为false时，当前协程让出执行
static inline bool liteco_chan_w(liteco_chan_t *const chan, void *const ele);
// 将当前协程加入到读等待队列
static inline int liteco_chan_rblock(liteco_chan_t *const chan, void **ele_store);
// 从chan中读出一个消息，返回值若为true时，则当前协程继续执行；返回值若为false时，当前协程让出执行
static inline bool liteco_chan_r(void **const ele_store, liteco_chan_t *const chan);

int liteco_chan_create(liteco_chan_t *const chan, uint32_t ele_count,
                       void (*co_ready) (void *const, liteco_co_t *const),
                       void *const proc) {
    if (ele_count == 0) {
        chan->queue = NULL;
    }
    else {
        chan->queue = liteco_array_create(ele_count, sizeof(void *));
        if (!chan->queue) {
            return liteco_chan_err_internal_error;
        }
    }

    chan->closed = false;

    chan->head = 0;
    chan->tail = 0;

    liteco_link_init(&chan->w);
    liteco_link_init(&chan->r);

    pthread_mutex_init(&chan->mtx, NULL);

    chan->co_ready = co_ready;
    chan->proc = proc;

    return liteco_chan_err_success;
}

int liteco_chan_push(liteco_chan_t *const chan, void *const ele) {
    if (chan->closed) {
        return liteco_chan_err_closed;
    }

    liteco_chan_lock(chan);
    while (!liteco_chan_w(chan, ele)) {
        // 若不能写入，则当前协程应让出执行，并将当前协程记录在chan的等待写队列中
        liteco_chan_wblock(chan, ele);
        liteco_set_status(liteco_curr, liteco_status_running, liteco_status_waiting);
        liteco_chan_unlock(chan);
        liteco_yield();
        liteco_chan_lock(chan);
    }
    liteco_chan_unlock(chan);

    return liteco_chan_err_success;
}

void *liteco_chan_pop(liteco_chan_t *const chan) {
    if (chan->closed) {
        return NULL;
    }

    void *ret = NULL;
    liteco_chan_lock(chan);
    while (!liteco_chan_r(&ret, chan)) {
        // 若不能读取，则当前协程应让出执行，并将当前协程记录在chan的等待读队列中
        liteco_chan_rblock(chan, &ret);
        liteco_set_status(liteco_curr, liteco_status_running, liteco_status_waiting);
        liteco_chan_unlock(chan);
        liteco_yield();
        liteco_chan_lock(chan);
    }
    liteco_chan_unlock(chan);

    return ret;
}

static inline int liteco_chan_wblock(liteco_chan_t *const chan, void *const ele) {
    liteco_waiter_t *waiter = malloc(sizeof(liteco_waiter_t));
    if (!waiter) {
        return liteco_chan_err_internal_error;
    }
    liteco_link_init(waiter);
    waiter->co = liteco_curr;
    waiter->ele.s_ele = ele;
    liteco_link_insert_after(&chan->w, waiter);

    return liteco_chan_err_success;
}

static inline bool liteco_chan_w(liteco_chan_t *const chan, void *const ele) {
    if (chan->closed) {
        return true;
    }
    if (liteco_chan_wblocked(chan)) {
        return false;
    }

    if (chan->queue) {
        *((void **) liteco_array_get(chan->queue, chan->tail)) = ele;
        chan->tail = (chan->tail + 1) % chan->queue->ele_count;
    }
    else {
        liteco_waiter_t *r = liteco_link_next(&chan->r);
        *r->ele.r_store = ele;
        chan->co_ready(chan->proc, r->co);
        liteco_link_remove(r);
        free(r);
    }
    return true;
}

static inline int liteco_chan_rblock(liteco_chan_t *const chan, void **ele_store) {
    liteco_waiter_t *waiter = malloc(sizeof(liteco_waiter_t));
    if (!waiter) {
        return liteco_chan_err_internal_error;
    }
    liteco_link_init(waiter);
    waiter->co = liteco_curr;
    waiter->ele.r_store = ele_store;
    liteco_link_insert_after(&chan->r, waiter);

    return liteco_chan_err_success;
}

static inline bool liteco_chan_r(void **const ele_store, liteco_chan_t *const chan) {
    if (chan->closed) {
        return true;
    }

    if (liteco_chan_rblocked(chan)) {
        return false;
    }

    if (chan->queue && chan->tail != chan->head) {
        *ele_store = *(void **) liteco_array_get(chan->queue, chan->head);
        chan->head = (chan->head + 1) % chan->queue->ele_count;
    }
    else {
        liteco_waiter_t *w = liteco_link_next(&chan->w);
        *ele_store = w->ele.s_ele;
        chan->co_ready(chan->proc, w->co);
        liteco_link_remove(w);
        free(w);
    }
    return true;
}

static inline void liteco_chan_lock(liteco_chan_t *const chan) {
    pthread_mutex_lock(&chan->mtx);
}

static inline void liteco_chan_unlock(liteco_chan_t *const chan) {
    pthread_mutex_unlock(&chan->mtx);
}

static inline bool liteco_chan_wblocked(liteco_chan_t *const chan) {
    if (chan->queue) {
        // 如果存在队列，判断队列是否已满
        return (chan->tail + 1) % chan->queue->ele_size == chan->head;
    }
    else {
        // 如果不存在队列，判断是否有等待读取的协程
        return liteco_link_empty(&chan->r);
    }
}

static inline bool liteco_chan_rblocked(liteco_chan_t *const chan) {
    if (chan->queue) {
        // 如果存在队列，判断队列是否为空
        return chan->tail == chan->head;
    }
    else {
        // 如果不存在队列，判断是否有等待写入的协程
        return liteco_link_empty(&chan->w);
    }
}

