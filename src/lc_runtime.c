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
#if defined(__APPLE__)
#include <stdlib.h>
#include <sys/malloc.h>
#elif defined(__linux__)
#include <malloc.h>
#endif

static void liteco_runtime_cb(liteco_async_t *const async);
static inline void liteco_runtime_lock(liteco_runtime_t *const rt);
static inline void liteco_runtime_unlock(liteco_runtime_t *const rt);
static liteco_co_t *liteco_runtime_pop(liteco_runtime_t *const rt);
static bool liteco_runtime_exist(liteco_runtime_t *const rt, liteco_co_t *const co);

int liteco_runtime_init(liteco_eloop_t *const eloop, liteco_runtime_t *const rt) {
    liteco_handler_init(rt, eloop, liteco_handler_type_runtime);

    pthread_mutex_init(&rt->mtx, NULL);
    liteco_link_init(&rt->rq);

    liteco_async_init(eloop, &rt->rt_async, liteco_runtime_cb);

    return 0;
}

static bool liteco_runtime_exist(liteco_runtime_t *const rt, liteco_co_t *const co) {
    liteco_ready_t *check = NULL;

    liteco_link_foreach(check, &rt->rq) {
        if (check->co == co) {
            return true;
        }
    }
    return false;
}

int liteco_runtime_join(liteco_runtime_t *const rt, liteco_co_t *const co) {
    liteco_runtime_lock(rt);

    if (liteco_runtime_exist(rt, co)) {
        liteco_runtime_unlock(rt);
        return 0;
    }

    liteco_ready_t *rd = malloc(sizeof(liteco_ready_t));
    if (!rd) {
        liteco_runtime_unlock(rt);
        return -1;
    }
    liteco_link_init(rd);
    rd->co = co;

    liteco_link_insert_before(&rt->rq, rd);
    liteco_async_send(&rt->rt_async);

    liteco_runtime_unlock(rt);

    return 0;
}

static liteco_co_t *liteco_runtime_pop(liteco_runtime_t *const rt) {
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

static void liteco_runtime_cb(liteco_async_t *const async) {
    liteco_runtime_t *const rt = container_of(async, liteco_runtime_t, rt_async);

    liteco_ready_t rq;
    liteco_link_init(&rq);
    for ( ;; ) {
        liteco_co_t *const co = liteco_runtime_pop(rt);
        if (!co) {
            break;
        }
        liteco_resume(co);

        if (co->status == LITECO_STATUS_READY) {
            liteco_ready_t *nd = malloc(sizeof(liteco_ready_t));
            liteco_link_init(nd);
            nd->co = co;
            liteco_link_insert_before(&rq, nd);
        }
    }

    liteco_runtime_lock(rt);
    while (!liteco_link_empty(&rq)) {
        liteco_ready_t *const nd = liteco_link_next(&rq);
        liteco_link_remove(nd);

        if (liteco_runtime_exist(rt, nd->co)) {
            free(nd);
        }
        else {
            liteco_link_insert_before(&rt->rq, nd);
        }
    }
    liteco_async_send(&rt->rt_async);
    liteco_runtime_unlock(rt);
}

static inline void liteco_runtime_lock(liteco_runtime_t *const rt) {
    pthread_mutex_lock(&rt->mtx);
}

static inline void liteco_runtime_unlock(liteco_runtime_t *const rt) {
    pthread_mutex_unlock(&rt->mtx);
}
