/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_HEAP_H__
#define __LITECO_HEAP_H__

#include <stdint.h>
#include <stddef.h>

enum liteco_heap_cmp_result_e {
    LITECO_HEAP_SWAP = 0,
    LITECO_HEAP_KEEP
};
typedef enum liteco_heap_cmp_result_e liteco_heap_cmp_result_t;

typedef struct liteco_heapnode_s liteco_heapnode_t;
typedef struct liteco_heap_s liteco_heap_t;

typedef liteco_heap_cmp_result_t (*liteco_heap_cmp_cb) (liteco_heapnode_t *const p, liteco_heapnode_t *const c);

#define LITECO_HEAPNODE_BASE \
    liteco_heapnode_t *hp_p; \
    liteco_heapnode_t *hp_r; \
    liteco_heapnode_t *hp_l; \

struct liteco_heapnode_s { LITECO_HEAPNODE_BASE };

struct liteco_heap_s {
    liteco_heapnode_t *root;
    uint32_t cnt;

    liteco_heap_cmp_cb cmp;
};

#define liteco_heapnode_init(n) { \
    (n)->hp_p = NULL;         \
    (n)->hp_l = NULL;         \
    (n)->hp_r = NULL;         \
}

int liteco_heap_init(liteco_heap_t *const heap, liteco_heap_cmp_cb cb);

int liteco_heap_insert(liteco_heap_t *const heap, liteco_heapnode_t *const node);

int liteco_heap_remove(liteco_heap_t *const heap, liteco_heapnode_t *const node);

#endif
