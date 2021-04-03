/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_ARRAY_H__
#define __LITECO_ARRAY_H__

#include <stdint.h>
#include <stddef.h>
#include <sys/cdefs.h>
#if __linux__
#include <malloc.h>
#elif __APPLE__
#include <stdlib.h>
#include <sys/malloc.h>
#endif

typedef struct liteco_array_s liteco_array_t;
struct liteco_array_s {
    uint32_t ele_count;
    uint32_t ele_size;
    uint8_t payload[0];
};

#define liteco_array_get(arr, i) \
    ((arr)->payload + ((arr)->ele_size * (i)))

__header_always_inline liteco_array_t *liteco_array_create(uint32_t ele_count, uint32_t ele_size) {
    liteco_array_t *arr = malloc(sizeof(liteco_array_t) + ele_count * ele_size);
    if (!arr) {
        return NULL;
    }

    arr->ele_count = ele_count;
    arr->ele_size = ele_size;

    return arr;
}

#endif
