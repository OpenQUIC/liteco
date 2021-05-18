/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "liteco.h"
#include "platform/internal.h"
#include <malloc.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

static void liteco_async_io_cb(liteco_eloop_t *const eloop, liteco_io_t *const io, const uint32_t flags);
static bool liteco_async_spin(liteco_async_t *const handler);
static void liteco_eloop_reg_io(liteco_eloop_t *const eloop, liteco_io_t *const io);

static void liteco_eloop_timer_active(liteco_eloop_t *const eloop, const struct timespec now);
static void liteco_timer_io_cb(liteco_eloop_t *const eloop, liteco_io_t *const io, const uint32_t flags);

int liteco_eloop_init(liteco_eloop_t *const eloop) {
    eloop->closed = false;

    liteco_rbt_init(eloop->mon);
    liteco_rbt_init(eloop->reg);

    eloop->epoll_fd = epoll_create(1);
    eloop->timer_io.key = -1;

    eloop->async_cnt = 0;
    liteco_link_init(&eloop->async);

    return 0;
}

int liteco_eloop_async_init(liteco_eloop_t *const eloop) {
    int fd = 0;

    if (eloop->async_cnt != 0) {
        return 0;
    }

    if ((fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)) < 0) {
        return -1;
    }

    liteco_io_init(&eloop->async_io, liteco_async_io_cb, fd);
    liteco_io_start(eloop, &eloop->async_io, EPOLLIN | EPOLLET);

    return 0;
}

static void liteco_async_io_cb(liteco_eloop_t *const eloop, liteco_io_t *const io, const uint32_t flags) {
    (void) flags;

    eventfd_t val;
    if (eventfd_read(io->key, &val)) {
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
    do { ret = eventfd_write(eloop->async_io.key, 1); } while (ret == -1);

    return 0;
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

int liteco_eloop_timer_init(liteco_eloop_t *const eloop) {
    if (eloop->timer_io.key != -1) {
        return 0;
    }

    int fd = timerfd_create(CLOCK_MONOTONIC, EFD_CLOEXEC | EFD_NONBLOCK);
    liteco_io_init(&eloop->timer_io, liteco_timer_io_cb, fd);
    liteco_io_start(eloop, &eloop->timer_io, EPOLLIN | EPOLLET);
    return 0;
}

int liteco_eloop_timer_add(liteco_eloop_t *const eloop, liteco_timer_t *const timer) {
    if (timer->active) {
        return 0;
    }
    liteco_heapnode_init(&timer->hp_handle);
    liteco_heap_insert(&eloop->timer_heap, &timer->hp_handle);
    timer->active = true;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    liteco_eloop_timer_active(eloop, now);

    return 0;
}

int liteco_eloop_timer_remove(liteco_eloop_t *const eloop, liteco_timer_t *const timer) {
    if (!timer->active) {
        return 0;
    }
    liteco_heap_remove(&eloop->timer_heap, &timer->hp_handle);
    timer->active = false;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    liteco_eloop_timer_active(eloop, now);

    return 0;
}

static void liteco_eloop_timer_active(liteco_eloop_t *const eloop, const struct timespec now) {
    while (eloop->timer_heap.root != NULL) {
        liteco_timer_t *const timer = container_of(eloop->timer_heap.root, liteco_timer_t, hp_handle);

        if (liteco_timer_active(timer, now)) {
            liteco_eloop_timer_remove(eloop, timer);
            timer->cb(timer);

            if ((timer->interval.tv_sec != 0 || timer->interval.tv_nsec != 0) && !timer->stop) {
                liteco_timer_set_timeout_current(timer);
                liteco_timer_add_timeout_interval(timer);

                liteco_eloop_timer_add(eloop, timer);
            }
        }
        else {
            timerfd_settime(eloop->timer_io.key, 1, &(struct itimerspec) {
                                .it_interval = { .tv_sec = 0, .tv_nsec = 0 },
                                .it_value = liteco_timer_timerfd_timeout(timer)
                            }, NULL);
            break;
        }
    }
}

static void liteco_timer_io_cb(liteco_eloop_t *const eloop, liteco_io_t *const io, const uint32_t flags) {
    (void) flags;

    uint64_t exp = 0;
    read(io->key, &exp, sizeof(exp));

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    liteco_eloop_timer_active(eloop, now);
}

int liteco_eloop_run(liteco_eloop_t *const eloop) {
    struct epoll_event aevt[32];

    while (!eloop->closed) {
        liteco_io_t *io = NULL;
        liteco_rbt_foreach(io, eloop->mon) {
            liteco_eloop_reg_io(eloop, io);
        }

        int evts_cnt = epoll_wait(eloop->epoll_fd, aevt, 32, -1);

        if (evts_cnt <= 0) {
            continue;
        }

        int i;
        for (i = 0; i < evts_cnt; i++) {
            liteco_io_t *const io = aevt[i].data.ptr;
            io->cb(eloop, io, aevt[i].events);
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

    struct epoll_event evt;
    evt.events = io->listening_events;
    evt.data.ptr = io;

    epoll_ctl(eloop->epoll_fd, EPOLL_CTL_ADD, io->key, &evt);
}

int liteco_eloop_close(liteco_eloop_t *const eloop) {
    if (eloop->closed) {
        return 0;
    }
    eloop->closed = true;
    liteco_platform_close(eloop->epoll_fd);

    return 0;
}
