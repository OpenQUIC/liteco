/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_RUNTIME_H__
#define __LITECO_RUNTIME_H__

#include "coroutine.h"
#include "link.h"
#include <pthread.h>

#define liteco_runtime_err_internal_error -1001
#define liteco_runtime_err_success 0

typedef struct liteco_ready_s liteco_ready_t;
struct liteco_ready_s {
    LITECO_LINKNODE_BASE

    liteco_co_t *co;
};

typedef struct liteco_runtime_s liteco_runtime_t;
struct liteco_runtime_s {
    pthread_mutex_t mtx;

    liteco_ready_t rq;
};

int liteco_runtime_init(liteco_runtime_t *const rt);
int liteco_runtime_join(liteco_runtime_t *const rt, liteco_co_t *const co);
liteco_co_t *liteco_runtime_pop(liteco_runtime_t *const rt);

void liteco_runtime_readycb(void *const runtime_, liteco_co_t *const co);

#endif
