/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "liteco.h"
#include "liteco/linux.h"
#include <assert.h>

int liteco_io_init(liteco_io_t *const io, liteco_io_cb cb, int fd) {
    assert(cb != NULL);
    assert(fd >= -1);

    liteco_rbt_node_init(io);

    io->cb = cb;
    io->key = fd;

    io->listening_events = 0;

    return 0;
}

int liteco_io_start(struct liteco_eloop_s *const eloop, liteco_io_t *const io, const uint32_t events) {
    io->listening_events |= events;

    if (liteco_rbt_is_nil(liteco_rbt_find(eloop->mon, &io->key))) {
        liteco_rbt_insert(&eloop->mon, io);
    }

    return 0;
}

int liteco_io_stop(struct liteco_eloop_s *const eloop, liteco_io_t *const io, const uint32_t events) {
    if (liteco_rbt_is_nil(liteco_rbt_find(eloop->mon, &io->key))) {
        return 0;
    }

    io->listening_events &= ~events;

    if (!io->listening_events) {
        liteco_io_t *const del = io;
        liteco_rbt_remove(&eloop->mon, &del);
        *io = *del;
    }

    return 0;
}

