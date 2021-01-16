/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "lc_channel.h"
#include "lc_runtime.h"

static inline bool liteco_chan_wblocked(liteco_chan_t *const chan);
static inline bool liteco_chan_rblocked(liteco_chan_t *const chan);
static inline void liteco_chan_lock(liteco_chan_t *const chan);
static inline void liteco_chan_unlock(liteco_chan_t *const chan);
// 将当前协程加入到写等待队列
static inline int liteco_chan_wblock(liteco_chan_t *const chan, void *const ele, const bool select);
static int liteco_chan_wdblock(liteco_chan_t *const chan);
// 将ele写入到chan中，返回值若为true时，则当前协程继续执行；返回值若为false时，当前协程让出执行
static inline bool liteco_chan_w(liteco_chan_t *const chan, void *const ele);
// 将当前协程加入到读等待队列
static inline int liteco_chan_rblock(liteco_chan_t *const chan, void **ele_store, const bool select);
static int liteco_chan_rdblock(liteco_chan_t *const chan);
// 从chan中读出一个消息，返回值若为true时，则当前协程继续执行；返回值若为false时，当前协程让出执行
static inline bool liteco_chan_r(void **const ele_store, liteco_chan_t *const chan);

static liteco_case_t *liteco_select_choose(liteco_case_t *const cases, const uint32_t count, const bool blocked);

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
    chan->full = false;

    liteco_link_init(&chan->w);
    liteco_link_init(&chan->r);

    pthread_mutex_init(&chan->mtx, NULL);

    chan->co_ready = co_ready;
    chan->proc = proc;

    return liteco_chan_err_success;
}

int liteco_chan_push(liteco_chan_t *const chan, void *const ele, const bool blocked) {
    if (chan->closed) {
        return liteco_chan_err_closed;
    }

    int ret = liteco_chan_err_success;
    liteco_chan_lock(chan);
    if (!liteco_chan_w(chan, ele)) {
        if (blocked) {
            // 若不能写入，则当前协程应让出执行，并将当前协程记录在chan的等待写队列中
            liteco_chan_wblock(chan, ele, false);
            liteco_set_status(liteco_curr, liteco_status_running, liteco_status_waiting);
            liteco_chan_unlock(chan);
            liteco_yield();
            liteco_chan_lock(chan);
        }
        else {
            ret = liteco_chan_err_push_failed;
        }
    }
    liteco_chan_unlock(chan);

    return ret;
}

int liteco_chan_close(liteco_chan_t *const chan) {
    if (chan->closed) {
        return liteco_chan_err_success;
    }

    liteco_chan_lock(chan);
    chan->closed = true;

    while (!liteco_link_empty(&chan->w)) {
        liteco_waiter_t *w = liteco_link_next(&chan->w);
        liteco_link_remove(w);
        chan->co_ready(chan->proc, w->co);
        free(w);
    }
    while (!liteco_link_empty(&chan->r)) {
        liteco_waiter_t *r = liteco_link_next(&chan->r);
        liteco_link_remove(r);
        chan->co_ready(chan->proc, r->co);
        free(r);
    }

    liteco_chan_unlock(chan);
    return liteco_chan_err_success;
}

int liteco_chan_unenforceable_push(liteco_chan_t *const chan, void *const ele) {
    if (chan->closed) {
        return liteco_chan_err_closed;
    }

    liteco_chan_lock(chan);
    if (!liteco_chan_w(chan, ele)) {
        liteco_waiter_t *w = malloc(sizeof(liteco_waiter_t));
        if (w) {
            w->co = liteco_joinignore_co;
            w->ele.s_ele = ele;
            w->select = false;
            liteco_link_insert_after(&chan->w, w);
        }
    }
    liteco_chan_unlock(chan);

    return liteco_chan_err_success;
}

void *liteco_chan_unenforceable_pop(liteco_chan_t *const chan) {
    if (chan->closed) {
        return liteco_chan_pop_failed;
    }

    void *ret = NULL;
    liteco_chan_lock(chan);
    if (!liteco_chan_r(&ret, chan)) {
        ret = liteco_chan_pop_failed;
    }
    liteco_chan_unlock(chan);

    return ret;
}

static uint8_t pop_fail = 0;
void *const liteco_chan_pop_failed = &pop_fail;

void *liteco_chan_pop(liteco_chan_t *const chan, const bool blocked) {
    if (chan->closed) {
        return NULL;
    }

    void *ret = NULL;
    liteco_chan_lock(chan);
    if (!liteco_chan_r(&ret, chan)) {
        if (blocked) {
            // 若不能读取，则当前协程应让出执行，并将当前协程记录在chan的等待读队列中
            liteco_chan_rblock(chan, &ret, false);
            liteco_set_status(liteco_curr, liteco_status_running, liteco_status_waiting);
            liteco_chan_unlock(chan);
            liteco_yield();
            liteco_chan_lock(chan);
        }
        else {
            ret = liteco_chan_pop_failed;
        }
    }
    liteco_chan_unlock(chan);

    return ret;
}

static liteco_case_t *liteco_select_choose(liteco_case_t *const cases, const uint32_t count, const bool blocked) {
    uint32_t i;
    for (i = 0; i < count; i++) {
        liteco_chan_lock(cases[i].chan);
    }
    uint32_t avail = count;
    for (i = 0; i < count; i++) {
        switch (cases[i].type) {
        case liteco_casetype_push:
            if (!liteco_chan_wblocked(cases[i].chan)) {
                avail = i;
            }
            break;

        case liteco_casetype_pop:
            if (!liteco_chan_rblocked(cases[i].chan)) {
                avail = i;
            }
            break;
        }

        if (avail != count) {
            break;
        }
    }

    if (avail != count) {
        for (i = 0; i < count; i++) {
            liteco_chan_unlock(cases[i].chan);
        }
        return cases + avail;
    }

    if (!blocked) {
        for (i = 0; i < count; i++) {
            liteco_chan_unlock(cases[i].chan);
        }
        return NULL;
    }

loop:
    for (i = 0; i < count; i++) {
        switch (cases[i].type) {
        case liteco_casetype_push:
            liteco_chan_wblock(cases[i].chan, cases[i].ele, true);
            break;
        case liteco_casetype_pop:
            liteco_chan_rblock(cases[i].chan, &cases[i].ele, true);
            break;
        }
    }

    liteco_set_status(liteco_curr, liteco_status_running, liteco_status_waiting);
    for (i = 0; i < count; i++) {
        liteco_chan_unlock(cases[i].chan);
    }
    liteco_yield();
    for (i = 0; i < count; i++) {
        liteco_chan_lock(cases[i].chan);
    }

    for (i = 0; i < count; i++) {
        switch (cases[i].type) {
        case liteco_casetype_push:
            liteco_chan_wdblock(cases[i].chan);
            break;
        case liteco_casetype_pop:
            liteco_chan_rdblock(cases[i].chan);
            break;
        }
    }
    for (i = 0; i < count; i++) {
        switch (cases[i].type) {
        case liteco_casetype_push:
            if (!liteco_chan_wblocked(cases[i].chan)) {
                avail = i;
            }
            break;

        case liteco_casetype_pop:
            if (!liteco_chan_rblocked(cases[i].chan)) {
                avail = i;
            }
            break;
        }

        if (avail != count) {
            break;
        }
    }
    if (avail == count) {
        goto loop;
    }
    for (i = 0; i < count; i++) {
        liteco_chan_unlock(cases[i].chan);
    }

    return cases + avail;
}

liteco_case_t *liteco_select(liteco_case_t *const cases, const uint32_t count, const bool blocked) {
    for ( ;; ) {
        liteco_case_t *const selected_case = liteco_select_choose(cases, count, blocked);
        if (!selected_case) {
            return NULL;
        }

        int push_result = 0;
        switch (selected_case->type) {
        case liteco_casetype_push: 
            push_result = liteco_chan_push(selected_case->chan, selected_case->ele, false);
            if (push_result == liteco_chan_err_success) {
                return selected_case;
            }
            break;
        case liteco_casetype_pop:
            selected_case->ele = liteco_chan_pop(selected_case->chan, false);
            if (selected_case->ele != liteco_chan_pop_failed) {
                return selected_case;
            }
            break;
        }
    }
}

static inline int liteco_chan_wblock(liteco_chan_t *const chan, void *const ele, const bool select) {
    liteco_waiter_t *w = malloc(sizeof(liteco_waiter_t));
    if (!w) {
        return liteco_chan_err_internal_error;
    }
    liteco_link_init(w);
    w->co = liteco_curr;
    w->ele.s_ele = ele;
    w->select = select;
    liteco_link_insert_after(&chan->w, w);

    return liteco_chan_err_success;
}

static int liteco_chan_wdblock(liteco_chan_t *const chan) {
    liteco_waiter_t *curr;

    for (curr = liteco_link_next(&chan->w); curr != &chan->w; curr = liteco_link_next(curr)) {
        if (curr->co == liteco_curr) {
            liteco_waiter_t *const prev = liteco_link_prev(curr);

            liteco_link_remove(curr);
            free(curr);

            curr = prev;
        }
    }

    return liteco_chan_err_success;
}

static inline bool liteco_chan_w(liteco_chan_t *const chan, void *const ele) {
    if (chan->closed) {
        return true;
    }
    if (liteco_chan_wblocked(chan)) {
        return false;
    }

    if (!liteco_link_empty(&chan->r)) {
        liteco_waiter_t *r = liteco_link_next(&chan->r);
        bool select = r->select;
        if (!select) {
            *r->ele.r_store = ele;
        }
        liteco_link_remove(r);
        chan->co_ready(chan->proc, r->co);
        free(r);

        return !select;
    }
    else {
        *((void **) liteco_array_get(chan->queue, chan->tail)) = ele;
        liteco_chan_queue_push(chan);
        return true;
    }
}

static inline int liteco_chan_rblock(liteco_chan_t *const chan, void **ele_store, const bool select) {
    liteco_waiter_t *r = malloc(sizeof(liteco_waiter_t));
    if (!r) {
        return liteco_chan_err_internal_error;
    }
    liteco_link_init(r);
    r->co = liteco_curr;
    r->ele.r_store = ele_store;
    r->select = select;
    liteco_link_insert_after(&chan->r, r);

    return liteco_chan_err_success;
}

static int liteco_chan_rdblock(liteco_chan_t *const chan) {
    liteco_waiter_t *curr;

    for (curr = liteco_link_next(&chan->r); curr != &chan->r; curr = liteco_link_next(curr)) {
        if (curr->co == liteco_curr) {
            liteco_waiter_t *const prev = liteco_link_prev(curr);

            liteco_link_remove(curr);
            free(curr);

            curr = prev;
        }
    }

    return liteco_chan_err_success;
}

static inline bool liteco_chan_r(void **const ele_store, liteco_chan_t *const chan) {
    if (chan->closed) {
        return true;
    }

    if (liteco_chan_rblocked(chan)) {
        return false;
    }

    if (!liteco_link_empty(&chan->w)) {
        liteco_waiter_t *w = liteco_link_next(&chan->w);
        bool select = w->select;
        if (!select) {
            *ele_store = w->ele.s_ele;
        }
        liteco_link_remove(w);
        chan->co_ready(chan->proc, w->co);
        free(w);

        return !select;
    }
    else {
        *ele_store = *(void **) liteco_array_get(chan->queue, chan->head);
        liteco_chan_queue_pop(chan);

        if (!liteco_link_empty(&chan->w)) {
            liteco_waiter_t *w = liteco_link_next(&chan->w);
            bool select = w->select;
            if (!select) {
                *((void **) liteco_array_get(chan->queue, chan->tail)) = w->ele.s_ele;
                liteco_chan_queue_push(chan);
            }
            liteco_link_remove(w);
            chan->co_ready(chan->proc, w->co);
            free(w);
        }

        return true;
    }
}

static inline void liteco_chan_lock(liteco_chan_t *const chan) {
    pthread_mutex_lock(&chan->mtx);
}

static inline void liteco_chan_unlock(liteco_chan_t *const chan) {
    pthread_mutex_unlock(&chan->mtx);
}

static inline bool liteco_chan_wblocked(liteco_chan_t *const chan) {
    if (chan->closed) {
        return false;
    }
    if (!liteco_link_empty(&chan->r)) {
        return false;
    }
    if (chan->queue) {
        return liteco_chan_queue_full(chan);
    }

    return true;
}

static inline bool liteco_chan_rblocked(liteco_chan_t *const chan) {
    if (chan->closed) {
        return false;
    }
    if (!liteco_link_empty(&chan->w)) {
        return false;
    }
    if (chan->queue) {
        return liteco_chan_queue_empty(chan);
    }

    return true;
}

