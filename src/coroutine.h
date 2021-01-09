/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_COROUTINE_H__
#define __LITECO_COROUTINE_H__

#include "context.h"
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
struct liteco_co_s {
    liteco_context_t *p_ctx;

    liteco_context_t ctx;
    uint8_t *st;
    size_t st_size;

    liteco_status_t status;
    pthread_mutex_t mtx;

    int (*fn) (void *const);
    void *arg;

    int (*finished_cb) (liteco_co_t *const);

    int ret;
};

extern __thread liteco_co_t *liteco_curr;

int liteco_create(liteco_co_t *const co,
                  int (*fn) (void *const), void *const arg,
                  int (*finished_cb) (liteco_co_t *const),
                  uint8_t *const st, const size_t st_size);
liteco_status_t liteco_resume(liteco_co_t *const co);
void liteco_yield();

void liteco_set_status(liteco_co_t *const co, const liteco_status_t from, const liteco_status_t to);

static inline void liteco_co_lock(liteco_co_t *const co) {
    pthread_mutex_lock(&co->mtx);
}

static inline void liteco_co_unlock(liteco_co_t *const co) {
    pthread_mutex_unlock(&co->mtx);
}

#endif
