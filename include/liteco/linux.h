/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_LINUX_H__
#define __LITECO_LINUX_H__

#include <stdint.h>
#include "liteco/lc_rbt.h"
#include "liteco/lc_link.h"
#include "liteco/lc_heap.h"

struct liteco_eloop_s;
struct liteco_io_s;

typedef void (*liteco_io_cb) (struct liteco_eloop_s *const eloop, struct liteco_io_s *const handler, const uint32_t flags);

typedef struct liteco_io_s liteco_io_t;
struct liteco_io_s {
    LITECO_RBT_KEY_INT_FIELDS

    liteco_io_cb cb;
    uint32_t listening_events;
};

int liteco_io_init(liteco_io_t *const io, liteco_io_cb cb, int fd);

int liteco_io_start(struct liteco_eloop_s *const eloop, liteco_io_t *const io, const uint32_t events);

int liteco_io_stop(struct liteco_eloop_s *const eloop, liteco_io_t *const io, const uint32_t events);

#define LITECO_ELOOP_FIELDS  \
    bool closed;             \
    liteco_int_rbt_t *mon;   \
    liteco_int_rbt_t *reg;   \
    int epoll_fd;            \
    liteco_io_t async_io;    \
    int async_cnt;           \
    liteco_linknode_t async; \

#define LITECO_TIMER_PLATFORM_FIELDS \
    liteco_heapnode_t hp_handle;     \
    struct timeval timeout;          \
    struct timeval interval;         \

#endif
