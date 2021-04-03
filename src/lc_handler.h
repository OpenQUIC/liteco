/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_HANDLER_H__
#define __LITECO_HANDLER_H__

#define LITECO_HANDLER_FIELDS \
    void (*cb) (liteco_handler_t *const handler); 
    

typedef struct liteco_handler_s liteco_handler_t;
struct liteco_handler_s {
    LITECO_HANDLER_FIELDS
};

#endif
