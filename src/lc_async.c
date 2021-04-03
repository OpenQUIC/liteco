/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "liteco.h"
#include "liteco/lc_link.h"
#include "platform/internal.h"
#include <stdio.h>

int liteco_async_init(liteco_eloop_t *const eloop, liteco_async_t *const handler, liteco_async_cb cb) {
    liteco_eloop_init_async(eloop);

    liteco_handler_init(handler, eloop, liteco_handler_type_async);
    handler->cb = cb;
    liteco_link_init(&handler->link_handle);
    handler->status = LITECO_ASYNC_NO_PADDING;

    liteco_link_insert_before(&eloop->async, &handler->link_handle);

    return 0;
}

int liteco_async_send(liteco_async_t *const handler) {
    if (liteco_access_memory(handler->status) != LITECO_ASYNC_NO_PADDING) {
        return 0;
    }

    if (!liteco_cas(&handler->status, LITECO_ASYNC_NO_PADDING, LITECO_ASYNC_WORKING)) {
        return 0;
    }

    liteco_eloop_async_send(handler->eloop);

    if (!liteco_cas(&handler->status, LITECO_ASYNC_WORKING, LITECO_ASYNC_DONE)) {
        return -1;
    }

    return 0;
}
