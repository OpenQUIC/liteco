/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_COROUTINE_H__
#define __LITECO_COROUTINE_H__

#include "lc_context.h"
#include "lc_link.h"
#include <pthread.h>

typedef enum liteco_status_e liteco_status_t;
enum liteco_status_e {
    liteco_status_unknow = 0,
    liteco_status_starting,
    liteco_status_readying,
    liteco_status_running,
    liteco_status_waiting,
    liteco_status_terminate
};

typedef struct liteco_co_s liteco_co_t;

typedef struct liteco_finished_s liteco_finished_t;
struct liteco_finished_s {
    LITECO_STACK_BASE

    void *arg;
    int (*finished_cb) (void *const);
};

typedef struct liteco_co_s liteco_co_t;
struct liteco_co_s {
    liteco_context_t *p_ctx;

    liteco_context_t ctx;
    uint8_t *st;
    size_t st_size;

    liteco_status_t status;
    pthread_mutex_t mtx;

    int (*fn) (void *const);
    void *arg;

    liteco_finished_t finished;

    int ret;
};

extern __thread liteco_co_t *liteco_curr;

int liteco_create(liteco_co_t *const co, int (*fn) (void *const), void *const arg, uint8_t *const st, const size_t st_size);

liteco_status_t liteco_resume(liteco_co_t *const co);

void liteco_yield();

void liteco_finished(liteco_co_t *const co, int (*finished_cb) (void *const), void *const arg);

static inline void liteco_curr_finished(int (*finished_cb) (void *const), void *const arg) {
    liteco_finished(liteco_curr, finished_cb, arg);
}

void liteco_set_status(liteco_co_t *const co, const liteco_status_t from, const liteco_status_t to);

static inline void liteco_co_lock(liteco_co_t *const co) {
    pthread_mutex_lock(&co->mtx);
}

static inline void liteco_co_unlock(liteco_co_t *const co) {
    pthread_mutex_unlock(&co->mtx);
}

#endif
