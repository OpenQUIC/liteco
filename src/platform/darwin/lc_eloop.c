/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "liteco.h"
#include "liteco/darwin.h"
#include "liteco/lc_link.h"
#include "liteco/lc_rbt.h"
#include "platform/internal.h"
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/event.h>
#include <sys/time.h>

#include <stdio.h>

static bool liteco_async_spin(liteco_async_t *const handler);
static void liteco_async_io_cb(liteco_eloop_t *const eloop, liteco_io_t *const io, const uint32_t flags);
static void liteco_eloop_reg_io(liteco_eloop_t *const eloop, liteco_io_t *const io);

int liteco_eloop_init(liteco_eloop_t *const eloop) {
    eloop->closed = false;
    eloop->async_wfd = -1;

    liteco_rbt_init(eloop->mon);
    liteco_rbt_init(eloop->reg);

    eloop->kqueue_fd = kqueue();

    liteco_link_init(&eloop->async);

    return 0;
}

int liteco_eloop_init_async(liteco_eloop_t *const eloop) {
    int err = 0;
    int pipefd[2];
    
    if (eloop->async_wfd != -1) {
        return 0;
    }

    if ((err = liteco_platform_pipe(pipefd, 0))) {
        return err;
    }

    liteco_platform_nonblock(pipefd[0], true);
    liteco_platform_nonblock(pipefd[1], true);

    liteco_io_init(&eloop->async_r, liteco_async_io_cb, pipefd[0]);
    liteco_io_start(eloop, &eloop->async_r, EVFILT_READ);

    eloop->async_wfd = pipefd[1];

    return 0;
}

static void liteco_async_io_cb(liteco_eloop_t *const eloop, liteco_io_t *const io, const uint32_t flags) {
    (void) flags;

    uint8_t buf;
    ssize_t len = 0;

    assert(io == &eloop->async_r);

    for ( ;; ) {
        len = read(io->key, &buf, sizeof(buf));

        if (len == sizeof(buf)) {
            continue;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break;
        }

        if (errno == EINTR) {
            continue;
        }

        return;
    }

    liteco_linknode_t *itr = NULL;
    liteco_link_foreach(itr, &eloop->async) {
        liteco_async_t *const handler = container_of(itr, liteco_async_t, link_handle);

        if (!liteco_async_spin(handler)) {
            continue;
        }

        if (!handler->cb) {
            continue;
        }

        handler->cb(handler);
    }
}

int liteco_eloop_async_send(liteco_eloop_t *const eloop) {
    int ret;
    const void *buf = "";
    ssize_t len = 1;

    do { ret = write(eloop->async_wfd, buf, len); } while (ret == -1 && errno == EINTR);

    if (ret == len) { return 0; }

    return -1;
}

static bool liteco_async_spin(liteco_async_t *const handler) {
    for ( ;; ) {
        int i;
        for (i = 0; i < 997; i++) {

            if (liteco_access_memory(handler->status) != LITECO_ASYNC_WORKING) {
                return liteco_cas(&handler->status, LITECO_ASYNC_DONE, LITECO_ASYNC_NO_PADDING);
            }

            liteco_cpurelax();
        }

        sched_yield();
    }
}

int liteco_eloop_run(liteco_eloop_t *const eloop) {
    struct kevent aevt[32];

    while (!eloop->closed) {
        if (eloop->async_wfd != -1) {
            liteco_eloop_reg_io(eloop, &eloop->async_r);
        }

        int evts_cnt = kevent(eloop->kqueue_fd, NULL, 0, aevt, 32, &(struct timespec) { .tv_sec = 1, .tv_nsec = 0 });

        if (evts_cnt <= 0) {
            continue;
        }

        int i;
        for (i = 0; i < evts_cnt; i++) {
            liteco_io_t *const io = (liteco_io_t *) aevt[i].udata;
            io->cb(eloop, io, aevt[i].filter);
        }
    }

    return 0;
}

static void liteco_eloop_reg_io(liteco_eloop_t *const eloop, liteco_io_t *const io) {
    if (liteco_rbt_is_not_nil(liteco_rbt_find(eloop->reg, &io->key))) {
        return;
    }

    liteco_int_rbt_t *fd_mark = malloc(sizeof(liteco_int_rbt_t));
    if (!fd_mark) {
        return;
    }
    liteco_rbt_node_init(fd_mark);
    fd_mark->key = io->key;
    liteco_rbt_insert(&eloop->reg, fd_mark);

    struct kevent evt;
    EV_SET(&evt, io->key, io->listening_events, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, io);

    kevent(eloop->kqueue_fd, &evt, 1, NULL, 0, NULL);
}
