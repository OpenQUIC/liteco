/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "coroutine.h"
#include <sys/time.h>
#include <sched.h>

__thread liteco_co_t *liteco_curr = NULL;

static void liteco_run(void *const);
static inline uint64_t liteco_now();

int liteco_create(liteco_co_t *const co,
                  int (*fn) (void *const), void *const arg,
                  int (*finished_cb) (liteco_co_t *const),
                  uint8_t *const st, const size_t st_size) {
    co->p_ctx = NULL;
    co->fn = fn;
    co->arg = arg;
    co->status = liteco_status_starting;
    co->finished_cb = finished_cb;
    co->st = st;
    co->st_size = st_size;

    liteco_context_init(&co->ctx, st, st_size, liteco_run, co);
    co->ret = 0;

    return 0;
}

static void liteco_run(void *const co_) {
    liteco_co_t *const co = co_;
    co->ret = co->fn(co->arg);
    liteco_set_status(co, liteco_status_running, liteco_status_terminate);
    liteco_yield();
}

liteco_status_t liteco_resume(liteco_co_t *const co) {
    static __thread liteco_context_t default_ctx;
    switch (co->status) {
    case liteco_status_terminate:
        return liteco_status_terminate;

    case liteco_status_starting:
        co->status = liteco_status_readying;
        break;

    default:
        break;
    }

    liteco_co_t *rem_co = liteco_curr;

    co->p_ctx = rem_co ? &rem_co->ctx : &default_ctx;

    liteco_curr = co;
    liteco_set_status(liteco_curr, liteco_status_readying, liteco_status_running);
    liteco_context_swap(*co->p_ctx, co->ctx);

    liteco_curr = rem_co;

    if (co->status == liteco_status_terminate) {
        if (co->finished_cb) {
            co->finished_cb(co);
        }
        return liteco_status_terminate;
    }

    return co->status;
}

void liteco_yield() {
    liteco_co_t *const co = liteco_curr;
    if (co->status == liteco_status_running) {
        co->status = liteco_status_readying;
    }

    liteco_context_swap(co->ctx, *co->p_ctx);
}

void liteco_set_status(liteco_co_t *const co, const liteco_status_t from, const liteco_status_t to) {
    int i;
    int x;
    uint64_t next;

    if (from == liteco_status_waiting && co->status == liteco_status_running) {
        return;
    }

    for (i = 0; !liteco_cas(&co->status, from, to); i++) {
        if (i == 0) {
            next = liteco_now() + 5;
        }
        if (liteco_now() < next) {
            for (x = 0; x < 10 && co->status != from; x++) {
                liteco_cas_yield(1);
            }
        }
        else {
            sched_yield();
            next = liteco_now() + 2;
        }
    }
}

static inline uint64_t liteco_now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}
