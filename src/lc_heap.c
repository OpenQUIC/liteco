/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "liteco/lc_heap.h"
#include "liteco/lc_link.h"

static void liteco_heap_swap(liteco_heap_t *const heap, liteco_heapnode_t *const p, liteco_heapnode_t *const c);

int liteco_heap_init(liteco_heap_t *const heap, liteco_heap_cmp_cb cb) {
    heap->cmp = cb;
    heap->cnt = 0;
    heap->root = NULL;

    return 0;
}

#include <stdio.h>
int liteco_heap_insert(liteco_heap_t *const heap, liteco_heapnode_t *const node) {
    if (heap->cnt == 0) {
        heap->root = node;
        heap->cnt = 1;
        return 0;
    }

    heap->cnt++;

    uint32_t h = 0;
    uint32_t off = 0;
    for (h = 1; (1 << h) <= heap->cnt; h++);
    off = (1 << (h - 1)) - 1 - ((1 << h) - heap->cnt - 1);

    liteco_heapnode_t **in = &heap->root;
    liteco_heapnode_t *p = *in;
    uint32_t i;
    for (i = 1; i < h; i++) {
        p = *in;
        if ((off & (1 << (h - 1 - i))) != 0) {
            in = &p->hp_r;
        }
        else {
            in = &p->hp_l;
        }
    }
    *in = node;
    node->hp_p = p;

    while (node->hp_p != NULL && heap->cmp(node->hp_p, node) == LITECO_HEAP_SWAP) {
        liteco_heap_swap(heap, node->hp_p, node);
    }

    return 0;
}

static void liteco_heap_swap(liteco_heap_t *const heap, liteco_heapnode_t *const p, liteco_heapnode_t *const c) {
    liteco_heapnode_t t;

    t = *p;
    *p = *c;
    *c = t;

    p->hp_p = c;
    if (c->hp_l == c) {
        c->hp_l = p;
        if (c->hp_r) {
            c->hp_r->hp_p = c;
        }
    }
    else {
        c->hp_r = p;
        if (c->hp_l) {
            c->hp_l->hp_p = c;
        }
    }

    if (p->hp_l) {
        p->hp_l->hp_p = p;
    }
    if (p->hp_r) {
        p->hp_r->hp_p = p;
    }

    if (!c->hp_p) {
        heap->root = c;
    }
    else if (c->hp_p->hp_l == p) {
        c->hp_p->hp_l = c;
    }
    else {
        c->hp_p->hp_r = c;
    }
}

int liteco_heap_remove(liteco_heap_t *const heap, liteco_heapnode_t *const node) {
    if (heap->cnt == 0) {
        return 0;
    }

    uint32_t h = 0;
    uint32_t off = 0;
    for (h = 1; (1 << h) <= heap->cnt; h++);
    off = (1 << (h - 1)) - 1 - ((1 << h) - heap->cnt - 1);

    liteco_heapnode_t **max = &heap->root;
    liteco_heapnode_t *p = *max;
    uint32_t i;
    for (i = 1; i < h; i++) {
        p = *max;
        if ((off & (1 << (h - 1 - i))) != 0) {
            max = &p->hp_r;
        }
        else {
            max = &p->hp_l;
        }
    }

    heap->cnt--;

    liteco_heapnode_t *child = *max;
    *max = NULL;

    if (child == node) {
        if (child == heap->root) {
            heap->root = NULL;
        }
        return 0;
    }

    child->hp_l = node->hp_l;
    child->hp_r = node->hp_r;
    child->hp_p = node->hp_p;

    if (child->hp_l) { child->hp_l->hp_p = child; }
    if (child->hp_r) { child->hp_r->hp_p = child; }

    if (!node->hp_p) { heap->root = child; }
    else if (node->hp_p->hp_l == node) { node->hp_p->hp_l = child; }
    else { node->hp_p->hp_r = child; }

    for ( ;; ) {
        liteco_heapnode_t *small = child;
        if (child->hp_l && heap->cmp(small, child->hp_l) == LITECO_HEAP_SWAP) { small = child->hp_l; }
        if (child->hp_r && heap->cmp(small, child->hp_r) == LITECO_HEAP_SWAP) { small = child->hp_r; }
        if (small == child) { break; }

        liteco_heap_swap(heap, child, small);
    }

    while (child->hp_p && heap->cmp(child->hp_p, child) == LITECO_HEAP_SWAP) { liteco_heap_swap(heap, child->hp_p, child); }

    return 0;
}
