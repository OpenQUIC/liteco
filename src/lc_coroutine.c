/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "liteco.h"
#include "liteco/lc_link.h"
#include "platform/internal.h"
#include <sys/time.h>
#if defined(__linux__)
#include <malloc.h>
#elif defined(__APPLE__)
#include <stdlib.h>
#include <sys/malloc.h>
#endif
#include <sched.h>

__thread liteco_co_t *this_co = NULL;

static void liteco_run(void *const);
static inline uint64_t liteco_now();

int liteco_init(liteco_co_t *const co, int (*cb) (void *const), void *const arg, void *const st, const size_t st_size) {
    co->p_ctx = NULL;
    co->cb = cb;
    co->arg = arg;
    co->status = LITECO_STATUS_STARTING;
    co->st = st;
    co->st_size = st_size;

    liteco_context_init(&co->ctx, st, st_size, liteco_run, co);
    co->ret = 0;

    liteco_stack_init(&co->fin_st);

    return 0;
}

void liteco_finished(liteco_co_t *const co, int (*finished_cb) (void *const), void *const arg) {
    liteco_fin_t *finished = malloc(sizeof(liteco_fin_t));
    liteco_stack_init(finished);
    finished->finished_cb = finished_cb;
    finished->arg = arg;

    liteco_stack_push(&co->fin_st, finished);
}

static void liteco_run(void *const co_) {
    liteco_co_t *const co = co_;
    co->ret = co->cb(co->arg);
    liteco_set_status(co, LITECO_STATUS_RUNNING, LITECO_STATUS_TERMINATE);
    liteco_yield();
}

liteco_status_t liteco_resume(liteco_co_t *const co) {
    static __thread liteco_context_t default_ctx;
    switch (co->status) {
    case LITECO_STATUS_TERMINATE:
        return LITECO_STATUS_TERMINATE;

    case LITECO_STATUS_STARTING:
        co->status = LITECO_STATUS_READY;
        break;

    default:
        break;
    }

    liteco_co_t *rem_co = this_co;

    co->p_ctx = rem_co ? &rem_co->ctx : &default_ctx;

    this_co = co;
    liteco_set_status(this_co, LITECO_STATUS_READY, LITECO_STATUS_RUNNING);
    liteco_context_swap(*co->p_ctx, co->ctx);

    this_co = rem_co;

    if (co->status == LITECO_STATUS_TERMINATE) {
        while (!liteco_stack_empty(&co->fin_st)) {
            liteco_fin_t *const fin_cb = liteco_stack_top(&co->fin_st);
            liteco_stack_pop(&co->fin_st);

            fin_cb->finished_cb(fin_cb->arg);

            free(fin_cb);
        }

        return LITECO_STATUS_TERMINATE;
    }

    return co->status;
}

void liteco_yield() {
    liteco_co_t *const co = this_co;
    if (co->status == LITECO_STATUS_RUNNING) {
        co->status = LITECO_STATUS_READY;
    }

    liteco_context_swap(co->ctx, *co->p_ctx);
}

void liteco_set_status(liteco_co_t *const co, const liteco_status_t from, const liteco_status_t to) {
    int i;
    int x;
    uint64_t next;

    if (from == LITECO_STATUS_WAITING  && co->status == LITECO_STATUS_RUNNING) {
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
