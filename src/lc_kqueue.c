/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "lc_emodule.h"
#include "liteco.h"
#if __APPLE__

#include "lc_kqueue.h"
#include <sys/event.h>
#include <stddef.h>
#include <sys/time.h>
#include <unistd.h>

int liteco_kqueue_init(liteco_kqueue_t *const kque) {
    kque->closed = false;
    kque->fd = kqueue();

    return liteco_kqueue_err_success;
}

int liteco_kqueue_add(liteco_kqueue_t *const kque, liteco_handler_t *const handler, const uint32_t kqueevents) {
    struct kevent e;
    EV_SET(&e, emodule->fd, kqueevents, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, emodule);

    return kevent(kque->fd, &e, 1, NULL, 0, NULL);
}

int liteco_kqueue_run(liteco_kqueue_t *const kque, const int timeout) {
    struct kevent events[LITECO_KQUEUE_MAX_EVENT_COUNT];

    if (kque->closed) {
        return 0;
    }

    int events_count = kevent(kque->fd, NULL, 0, events, LITECO_KQUEUE_MAX_EVENT_COUNT, &(const struct timespec) { .tv_sec = timeout / 1000, .tv_nsec = (timeout % 1000) * 1000 * 1000 });
    if (kque->closed) {
        return 0;
    }

    int i;
    for (i = 0; i < events_count; i++) {
        liteco_emodule_t *m = events[i].udata;
        if (m->cb) {
            m->cb(m);
        }
    }

    return events_count;
}

int liteco_kqueue_close(liteco_kqueue_t *const kque) {
    return close(kque->fd);
}

#endif
