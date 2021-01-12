/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_LOOP_H__
#define __LITECO_LOOP_H__

#include "link.h"
#include "emodule.h"

#include "epoll.h"

#define liteco_idle_err_success 0

#define liteco_eloop_err_success 0

typedef struct liteco_eloop_s liteco_eloop_t;
typedef struct liteco_idle_s liteco_idle_t;

struct liteco_idle_s {
    LITECO_LINKNODE_BASE

    void (*cb) (liteco_idle_t *const);
};

int liteco_idle_init(liteco_eloop_t *const eloop, liteco_idle_t *const idle);
int liteco_idle_start(liteco_idle_t *const idle, void (*cb) (liteco_idle_t *const));

struct liteco_eloop_s {

    liteco_epoll_t events;

    liteco_idle_t idle;
};

int liteco_eloop_init(liteco_eloop_t *const eloop);
int liteco_eloop_run(liteco_eloop_t *const eloop, const int timeout);

#endif
