/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#if __APPLE__

#ifndef __LITECO_KQUEUE_H__
#define __LITECO_KQUEUE_H__

#include "liteco.h"
#include <stdbool.h>
#include <sys/event.h>

#define LITECO_KQUEUE_MAX_EVENT_COUNT 32

#define liteco_kqueue_err_success 0

typedef struct liteco_kqueue_s liteco_kqueue_t;
struct liteco_kqueue_s {
    bool closed;
    int fd;
};

int liteco_kqueue_init(liteco_kqueue_t *const kque);
int liteco_kqueue_add(liteco_kqueue_t *const kque, liteco_handler_t *const handler, const uint32_t kqueevents);
int liteco_kqueue_run(liteco_kqueue_t *const kque, const int timeout);
int liteco_kqueue_close(liteco_kqueue_t *const kque);

#endif

#endif
