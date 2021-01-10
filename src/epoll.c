/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "epoll.h"

int liteco_epoll_init(liteco_epoll_t *const epoll) {
    epoll->fd = epoll_create(LITECO_EPOLL_MAX_EVENT_COUNT);

    return liteco_epoll_err_success;
}

int liteco_epoll_add(liteco_epoll_t *const epoll, liteco_emodule_t *const emodule, const uint32_t epevents) {
    struct epoll_event e;
    e.data.ptr = emodule;
    e.events = epevents;
    return epoll_ctl(epoll->fd, EPOLL_CTL_ADD, emodule->fd, &e);
}

int liteco_epoll_run(liteco_epoll_t *const epoll, const int timeout) {
    struct epoll_event events[LITECO_EPOLL_MAX_EVENT_COUNT];

    int events_count = epoll_wait(epoll->fd, events, LITECO_EPOLL_MAX_EVENT_COUNT, timeout);
    if (events_count == -1) {
        return liteco_epoll_err_internal_error;
    }

    int i;
    for (i = 0; i < events_count; i++) {
        liteco_emodule_t *m = events[i].data.ptr;
        m->cb(m);
    }

    return liteco_epoll_err_success;
}