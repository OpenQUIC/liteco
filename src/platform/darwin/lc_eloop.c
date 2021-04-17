/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "liteco.h"
#include "liteco/darwin.h"
#include "liteco/lc_heap.h"
#include "liteco/lc_link.h"
#include "liteco/lc_rbt.h"
#include "platform/darwin/internal.h"
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

static void liteco_eloop_timer_active(liteco_eloop_t *const eloop, const struct timeval now);
static struct timespec liteco_eloop_timer_wait(liteco_eloop_t *const eloop, const struct timeval now);
static liteco_heap_cmp_result_t liteco_timer_heap_cmp_cb(liteco_heapnode_t *const p, liteco_heapnode_t *const c);

int liteco_eloop_init(liteco_eloop_t *const eloop) {
    eloop->closed = false;
    eloop->async_wfd = -1;

    liteco_rbt_init(eloop->mon);
    liteco_rbt_init(eloop->reg);

    eloop->kqueue_fd = kqueue();

    liteco_link_init(&eloop->async);

    liteco_heap_init(&eloop->timer_heap, liteco_timer_heap_cmp_cb);

    return 0;
}

int liteco_eloop_async_init(liteco_eloop_t *const eloop) {
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

int liteco_eloop_timer_add(liteco_eloop_t *const eloop, liteco_timer_t *const timer) {
    if (timer->active) {
        return 0;
    }
    liteco_heapnode_init(&timer->hp_handle);
    liteco_heap_insert(&eloop->timer_heap, &timer->hp_handle);
    timer->active = true;
    return 0;
}

int liteco_eloop_timer_remove(liteco_eloop_t *const eloop, liteco_timer_t *const timer) {
    if (!timer->active) {
        return 0;
    }
    liteco_heap_remove(&eloop->timer_heap, &timer->hp_handle);
    timer->active = false;
    return 0;
}

static void liteco_eloop_timer_active(liteco_eloop_t *const eloop, const struct timeval now) {
    while (eloop->timer_heap.root != NULL) {
        liteco_timer_t *const timer = container_of(eloop->timer_heap.root, liteco_timer_t, hp_handle);

        if (liteco_timer_active(timer, now)) {
            liteco_eloop_timer_remove(eloop, timer);
            timer->cb(timer);

            if ((timer->interval.tv_sec != 0 || timer->interval.tv_usec != 0) && !timer->stop) {
                liteco_timer_set_timeout_current(timer);
                liteco_timer_add_timeout_interval(timer);

                liteco_eloop_timer_add(eloop, timer);
            }
        }
        else {
            break;
        }
    }
}

static struct timespec liteco_eloop_timer_wait(liteco_eloop_t *const eloop, const struct timeval now) {
    if (eloop->timer_heap.root == NULL) {
        return (struct timespec) { .tv_sec = 0, .tv_nsec = 0 };
    }

    liteco_timer_t *const timer = container_of(eloop->timer_heap.root, liteco_timer_t, hp_handle);
    struct timeval timeout = liteco_timer_kevent_timeout(timer);
    if (timeout.tv_usec < now.tv_usec) {
        timeout.tv_sec--;
        timeout.tv_usec += 1000 * 1000;
    }

    return (struct timespec) { .tv_sec = timeout.tv_sec - now.tv_sec, .tv_nsec = (timeout.tv_usec - now.tv_usec) * 1000 };
}

int liteco_eloop_udp_add(liteco_eloop_t *const eloop, liteco_udp_t *const udp) {
    liteco_io_start(eloop, &udp->io, EVFILT_READ);

    return 0;
}

int liteco_eloop_run(liteco_eloop_t *const eloop) {
    struct kevent aevt[32];

    while (!eloop->closed) {
        liteco_io_t *io = NULL;
        liteco_rbt_foreach(io, eloop->mon) {
            liteco_eloop_reg_io(eloop, io);
        }

        struct timeval now;
        gettimeofday(&now, NULL);

        liteco_eloop_timer_active(eloop, now);

        struct timespec waittime = liteco_eloop_timer_wait(eloop, now);
        int evts_cnt = kevent(eloop->kqueue_fd, NULL, 0, aevt, 32, &waittime);

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

static liteco_heap_cmp_result_t liteco_timer_heap_cmp_cb(liteco_heapnode_t *const p, liteco_heapnode_t *const c) {
    liteco_timer_t *const pt = container_of(p, liteco_timer_t, hp_handle);
    liteco_timer_t *const ct = container_of(c, liteco_timer_t, hp_handle);

    if (ct->timeout.tv_sec < pt->timeout.tv_sec) {
        return LITECO_HEAP_SWAP;
    }
    else if (ct->timeout.tv_sec == pt->timeout.tv_sec && ct->timeout.tv_usec < pt->timeout.tv_usec) {
        return LITECO_HEAP_SWAP;
    }

    return LITECO_HEAP_KEEP;
}
