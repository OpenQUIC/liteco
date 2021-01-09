/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "runtime.h"
#include <malloc.h>

int liteco_runtime_init(liteco_runtime_t *const rt) {
    pthread_mutex_init(&rt->mtx, NULL);
    liteco_link_init(&rt->rq);

    return liteco_runtime_err_success;
}

int liteco_runtime_join(liteco_runtime_t *const rt, liteco_co_t *const co) {
    liteco_ready_t *rd = malloc(sizeof(liteco_ready_t));
    if (!rd) {
        return liteco_runtime_err_internal_error;
    }
    liteco_link_init(rd);
    rd->co = co;

    pthread_mutex_lock(&rt->mtx);
    liteco_link_insert_before(&rt->rq, rd);
    pthread_mutex_unlock(&rt->mtx);

    return liteco_runtime_err_success;
}

liteco_co_t *liteco_runtime_pop(liteco_runtime_t *const rt) {
    pthread_mutex_lock(&rt->mtx);
    if (liteco_link_empty(&rt->rq)) {
        pthread_mutex_unlock(&rt->mtx);
        return NULL;
    }
    liteco_ready_t *const rd = liteco_link_next(&rt->rq);
    liteco_co_t *const co = rd->co;

    liteco_link_remove(rd);
    free(rd);
    pthread_mutex_unlock(&rt->mtx);

    return co;
}

void liteco_runtime_readycb(void *const runtime_, liteco_co_t *const co) {
    liteco_runtime_t *const rt = runtime_;
    liteco_set_status(co, liteco_status_waiting, liteco_status_readying);
    liteco_runtime_join(rt, co);
}
