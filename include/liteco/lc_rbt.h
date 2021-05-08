/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_RBT_H__
#define __LITECO_RBT_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define liteco_rbt_err_success 0
#define liteco_rbt_err_conflict -1

enum liteco_rbt_color_e {
    LITECO_RBT_BLACK = 0,
    LITECO_RBT_RED
};
typedef enum liteco_rbt_color_e liteco_rbt_color_t;

enum liteco_rbt_cmp_result_e {
    LITECO_RBT_EQ = 0,
    LITECO_RBT_LS,
    LITECO_RBT_GT
};
typedef enum liteco_rbt_cmp_result_e liteco_rbt_cmp_result_t;

#define LITECO_RBT_FIELDS      \
    liteco_rbt_t *rb_p;        \
    liteco_rbt_t *rb_r;        \
    liteco_rbt_t *rb_l;        \
    liteco_rbt_t *morris_link; \
    liteco_rbt_color_t rb_color;

#define liteco_rbt_left(node) (*(typeof(node) *) &node->rb_l)
#define liteco_rbt_right(node) (*(typeof(node) *) &node->rb_r)
#define liteco_rbt_parent(node) (*(typeof(node) *) &node->rb_p)
#define liteco_rbt_color(node) node->rb_color

typedef struct liteco_rbt_s liteco_rbt_t;
struct liteco_rbt_s { LITECO_RBT_FIELDS };

extern const liteco_rbt_t liteco_rbt_nil_instance;
#define liteco_rbt_nil ((liteco_rbt_t *) &liteco_rbt_nil_instance)

#define liteco_rbt_is_nil(node) (((liteco_rbt_t *) node) == liteco_rbt_nil)
#define liteco_rbt_is_not_nil(node) !liteco_rbt_is_nil(node)

#define liteco_rbt_init(node) ((node) = (typeof(node)) liteco_rbt_nil)

#define liteco_rbt_node_init(node) {    \
    node->rb_l = liteco_rbt_nil;        \
    node->rb_r = liteco_rbt_nil;        \
    node->rb_p = liteco_rbt_nil;        \
    node->morris_link = liteco_rbt_nil; \
    node->rb_color = LITECO_RBT_RED;    \
}

typedef liteco_rbt_cmp_result_t (*liteco_rbt_cmp_cb) (const void *const key, const liteco_rbt_t *const node);

int liteco_rbt_insert_impl(liteco_rbt_t **const root, liteco_rbt_t *const node, liteco_rbt_cmp_cb cb, const size_t key_off);
liteco_rbt_t *liteco_rbt_find_impl(liteco_rbt_t *const root, const void *const key, liteco_rbt_cmp_cb cb);
int liteco_rbt_remove_impl(liteco_rbt_t **const root, liteco_rbt_t **const node);
liteco_rbt_t *liteco_rbt_min_impl(liteco_rbt_t *const root);

#define liteco_rbt_remove(root, node) liteco_rbt_remove_impl((liteco_rbt_t **) root, (liteco_rbt_t **) node)
#define liteco_rbt_min(root) ((typeof(root)) liteco_rbt_min_impl((liteco_rbt_t *) (root)))

typedef struct liteco_rbt_iterator_s liteco_rbt_iterator_t;
struct liteco_rbt_iterator_s {
    bool interrupt[2];

    liteco_rbt_t *ptr;
    liteco_rbt_t *mr;
};

#define liteco_rbt_iterator_init(root) (liteco_rbt_iterator_t) { .interrupt = { false, false }, .ptr = (liteco_rbt_t *) root, .mr = liteco_rbt_nil }

#define liteco_rbt_iterator_end(itr) liteco_rbt_is_nil((itr)->ptr)

liteco_rbt_t *liteco_rbt_iterator_next(liteco_rbt_iterator_t *const itr);

#define _CAT(a, b) _CAT_I(a, b)
#define _CAT_I(a, b) _CAT_II(~, a ## b)
#define _CAT_II(p, res) res
#define UNIQUE_NAME(base) _CAT(base, __LINE__)

#define liteco_rbt_foreach(node, root)                                        \
    liteco_rbt_iterator_t UNIQUE_NAME(itr) = liteco_rbt_iterator_init(root);  \
    for ((node) = (typeof(node)) liteco_rbt_iterator_next(&UNIQUE_NAME(itr)); \
         !liteco_rbt_iterator_end(&UNIQUE_NAME(itr));                         \
         (node) = (typeof(node)) liteco_rbt_iterator_next(&UNIQUE_NAME(itr)))

#define LITECO_RBT_KEY_UINT64_FIELDS \
    LITECO_RBT_FIELDS                \
    uint64_t key; 

typedef struct liteco_uint64_rbt_s liteco_uint64_rbt_t;
struct liteco_uint64_rbt_s { LITECO_RBT_KEY_UINT64_FIELDS };

liteco_rbt_cmp_result_t liteco_rbt_uint64_cmp_cb(const void *const key, const liteco_rbt_t *const node);

#define liteco_rbt_uint64_insert(root, node) \
    liteco_rbt_insert_impl((liteco_rbt_t **) root, (liteco_rbt_t *) node, liteco_rbt_uint64_cmp_cb, offsetof(liteco_uint64_rbt_t, key))
#define liteco_rbt_uint64_find(root, finded_key) \
    liteco_rbt_find_impl((liteco_rbt_t *) root, finded_key, liteco_rbt_uint64_cmp_cb)

#define LITECO_RBT_KEY_INT_FIELDS \
    LITECO_RBT_FIELDS             \
    int key;

typedef struct liteco_int_rbt_s liteco_int_rbt_t;
struct liteco_int_rbt_s { LITECO_RBT_KEY_INT_FIELDS };

liteco_rbt_cmp_result_t liteco_rbt_int_cmp_cb(const void *const key, const liteco_rbt_t *const node);

#define liteco_rbt_int_insert(root, node) \
    liteco_rbt_insert_impl((liteco_rbt_t **) root, (liteco_rbt_t *) node, liteco_rbt_int_cmp_cb, offsetof(liteco_int_rbt_t, key))
#define liteco_rbt_int_find(root, finded_key) \
    liteco_rbt_find_impl((liteco_rbt_t *) root, finded_key, liteco_rbt_int_cmp_cb)

#define liteco_rbt_insert(root, node) (_Generic((*root)->key, \
    uint64_t: liteco_rbt_uint64_insert(root, node),           \
    int:      liteco_rbt_int_insert(root, node)               \
))

#define liteco_rbt_find(root, key) ((typeof(root)) _Generic(*key, \
    uint64_t: liteco_rbt_uint64_find(root, key),                  \
    int: liteco_rbt_int_find(root, key)                           \
))

#endif
