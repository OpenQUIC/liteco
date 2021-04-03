/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "lc_epoll.h"
#include "lc_event.h"
#if defined(__linux__)
#include <sys/eventfd.h>
#endif
#include <unistd.h>

static void liteco_event_accept(liteco_emodule_t *const emodule);

int liteco_event_init(liteco_eloop_t *const eloop, liteco_event_t *const event, bool semaphore) {
#if defined(__linux__)
    int flags = EFD_CLOEXEC | EFD_NONBLOCK;
    if (semaphore) {
        flags |= EFD_SEMAPHORE;
    }
    event->fd = eventfd(0, flags);
#elif defined(__APPLE__)

#endif
    event->cb = liteco_event_accept;

    return liteco_eloop_add(eloop, (liteco_emodule_t *) event);
}

int liteco_event_setup(liteco_event_t *const event, void (*cb) (liteco_event_t *const, const uint64_t)) {
    event->accept_cb = cb;
    return liteco_event_err_success;
}

int liteco_event_dispatch(liteco_event_t *const event, uint64_t times) {
    eventfd_write(event->fd, times);
    return liteco_event_err_success;
}

static void liteco_event_accept(liteco_emodule_t *const emodule) {
    liteco_event_t *const event = (liteco_event_t *) emodule;

    eventfd_t ret;
    if (!eventfd_read(event->fd, &ret) && event->accept_cb) {
        event->accept_cb(event, ret);
    }
}

int liteco_event_close(liteco_event_t *const event) {
    close(event->fd);
    return liteco_event_err_success;
}
