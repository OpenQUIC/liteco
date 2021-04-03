/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_EVENT_H__
#define __LITECO_EVENT_H__

#include "lc_emodule.h"
#include "lc_eloop.h"
#include <stdbool.h>

#define liteco_event_err_success 0

typedef struct liteco_event_s liteco_event_t;
struct liteco_event_s {
    LITECO_EMODULE_FIELD

    void (*accept_cb) (liteco_event_t *const, const uint64_t);
};

int liteco_event_init(liteco_eloop_t *const eloop, liteco_event_t *const event, bool semaphore);
int liteco_event_setup(liteco_event_t *const event, void (*cb) (liteco_event_t *const, const uint64_t));
int liteco_event_dispatch(liteco_event_t *const event, uint64_t times);
int liteco_event_close(liteco_event_t *const event);

#endif
