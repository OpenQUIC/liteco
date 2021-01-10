/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "timer.h"
#include "runtime.h"
#include <malloc.h>
#include <sys/eventfd.h>

static void liteco_runtime_cb(liteco_emodule_t *const emodule);
static inline void liteco_runtime_lock(liteco_runtime_t *const rt);
static inline void liteco_runtime_unlock(liteco_runtime_t *const rt);

uint8_t joinignore_co;
liteco_co_t *const liteco_joinignore_co = (liteco_co_t *) &joinignore_co;

int liteco_runtime_init(liteco_runtime_t *const rt) {
    pthread_mutex_init(&rt->mtx, NULL);
    liteco_link_init(&rt->rq);
    rt->fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE);
    rt->cb = liteco_runtime_cb;
    return liteco_runtime_err_success;
}

int liteco_runtime_join(liteco_runtime_t *const rt, liteco_co_t *const co) {
    liteco_ready_t *rd = malloc(sizeof(liteco_ready_t));
    if (!rd) {
        return liteco_runtime_err_internal_error;
    }
    liteco_link_init(rd);
    rd->co = co;

    liteco_runtime_lock(rt);
    liteco_link_insert_before(&rt->rq, rd);
    eventfd_write(rt->fd, 1);
    liteco_runtime_unlock(rt);

    return liteco_runtime_err_success;
}

liteco_co_t *liteco_runtime_pop(liteco_runtime_t *const rt) {
    liteco_runtime_lock(rt);
    if (liteco_link_empty(&rt->rq)) {
        liteco_runtime_unlock(rt);
        return NULL;
    }
    liteco_ready_t *const rd = liteco_link_next(&rt->rq);
    liteco_co_t *const co = rd->co;

    liteco_link_remove(rd);
    free(rd);
    liteco_runtime_unlock(rt);

    return co;
}

void liteco_runtime_readycb(void *const runtime_, liteco_co_t *const co) {
    liteco_runtime_t *const rt = runtime_;

    if (co == liteco_joinignore_co) {
        return;
    }

    liteco_set_status(co, liteco_status_waiting, liteco_status_readying);
    liteco_runtime_join(rt, co);
}

static void liteco_runtime_cb(liteco_emodule_t *const emodule) {
    liteco_runtime_t *const rt = (liteco_runtime_t *) emodule;

    eventfd_t ret;
    while (eventfd_read(rt->fd, &ret) == 0) {
        liteco_co_t *const co = liteco_runtime_pop(rt);
        if (co) {
            liteco_resume(co);
        }
    }
}

static inline void liteco_runtime_lock(liteco_runtime_t *const rt) {
    pthread_mutex_lock(&rt->mtx);
}

static inline void liteco_runtime_unlock(liteco_runtime_t *const rt) {
    pthread_mutex_unlock(&rt->mtx);
}
