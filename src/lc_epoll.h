/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_EPOLL_H__
#define __LITECO_EPOLL_H__

#include "lc_link.h"
#include "lc_emodule.h"
#include <sys/epoll.h>

#define LITECO_EPOLL_MAX_EVENT_COUNT 32

#define liteco_epoll_err_success 0
#define liteco_epoll_err_internal_error -1001

typedef struct liteco_epoll_s liteco_epoll_t;
struct liteco_epoll_s {
    int fd;
};

int liteco_epoll_init(liteco_epoll_t *const epoll);
int liteco_epoll_add(liteco_epoll_t *const epoll, liteco_emodule_t *const emodule, const uint32_t epevents);
int liteco_epoll_run(liteco_epoll_t *const epoll, const int timeout);

#endif
