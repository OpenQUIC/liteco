/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "lc_eloop.h"
#include <stdbool.h>
#include <stddef.h>

int liteco_idle_init(liteco_eloop_t *const eloop, liteco_idle_t *const idle) {
    liteco_link_init(idle);
    idle->cb = NULL;

    liteco_link_insert_after(&eloop->idle, idle);

    return liteco_idle_err_success;
}

int liteco_idle_start(liteco_idle_t *const idle, void (*cb) (liteco_idle_t *const)) {
    idle->cb = cb;
    return liteco_idle_err_success;
}

int liteco_eloop_init(liteco_eloop_t *const eloop) {
    eloop->closed = false;
    liteco_link_init(&eloop->idle);
    liteco_epoll_init(&eloop->events);
    return liteco_eloop_err_success;
}

int liteco_eloop_run(liteco_eloop_t *const eloop, const int timeout) {
    if (eloop->closed) {
        return liteco_eloop_err_closed;
    }

    if (!liteco_link_empty(&eloop->idle)) {
        int ret = liteco_epoll_run(&eloop->events, 0);
        if (ret <= 0) {
            return ret;
        }
        if (eloop->closed) {
            return liteco_eloop_err_closed;
        }

        liteco_idle_t *idle = NULL;
        liteco_link_foreach(idle, &eloop->idle) {
            if (idle->cb) {
                idle->cb(idle);
            }
        }
    }
    return liteco_epoll_run(&eloop->events, timeout);
}

int liteco_eloop_close(liteco_eloop_t *const eloop) {
    if (eloop->closed) {
        return liteco_eloop_err_closed;
    }
    eloop->closed = true;
    liteco_epoll_close(&eloop->events);

    return liteco_eloop_err_success;
}

int liteco_eloop_add(liteco_eloop_t *const eloop, liteco_emodule_t *const emodule) {
    return liteco_epoll_add(&eloop->events, emodule, EPOLLIN | EPOLLET);
}
