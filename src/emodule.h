/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_EMODULE_H__
#define __LITECO_EMODULE_H__

#define LITECO_EMODULE_FIELD \
    int fd; \
    void (*cb) (liteco_emodule_t *const emodule); 
    

typedef struct liteco_emodule_s liteco_emodule_t;
struct liteco_emodule_s {
    LITECO_EMODULE_FIELD
};

#endif
