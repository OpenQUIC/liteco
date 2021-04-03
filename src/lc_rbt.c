/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "liteco/lc_rbt.h"

const liteco_rbt_t rbt_nil = {
    .rb_p        = liteco_rbt_nil,
    .rb_r        = liteco_rbt_nil,
    .rb_l        = liteco_rbt_nil,
    .morris_link = liteco_rbt_nil,
    .rb_color    = LITECO_RBT_BLACK
};

static inline void __lr(liteco_rbt_t **const root, liteco_rbt_t *const node);
static inline void __rr(liteco_rbt_t **const root, liteco_rbt_t *const node);
static void __fix(liteco_rbt_t **const root, liteco_rbt_t *node);
static liteco_rbt_t *__min(liteco_rbt_t *node);
static inline liteco_rbt_t *__sib(const liteco_rbt_t *const node);
static inline void __assign(liteco_rbt_t **const root, liteco_rbt_t *const tgt, liteco_rbt_t *const ref);
static void __rm1(liteco_rbt_t **const root, liteco_rbt_t *const node);
static void __rm2(liteco_rbt_t **const root, liteco_rbt_t *const node);
static void __rm3(liteco_rbt_t **const root, liteco_rbt_t *const node);
static void __rm4(liteco_rbt_t **const root, liteco_rbt_t *const node);
static void __rm5(liteco_rbt_t **const root, liteco_rbt_t *const node);
static void __rm6(liteco_rbt_t **const root, liteco_rbt_t *const node);
static inline void __replace(liteco_rbt_t **const root, liteco_rbt_t *const lf, liteco_rbt_t *const rt);

int liteco_rbt_insert_impl(liteco_rbt_t **const root, liteco_rbt_t *const node, liteco_rbt_cmp_cb cb, const size_t key_off) {
    liteco_rbt_t *rb_p = liteco_rbt_nil;
    liteco_rbt_t **in = root;

    while (liteco_rbt_is_not_nil(*in)) {
        rb_p = *in;
        switch (cb(((void *) node) + key_off, rb_p)) {
        case LITECO_RBT_EQ:
            return liteco_rbt_err_conflict;
        case LITECO_RBT_LS:
            in = &rb_p->rb_l;
            break;
        case LITECO_RBT_GT:
            in = &rb_p->rb_r;
            break;
        }
    }
    node->rb_p = rb_p;
    *in = node;

    __fix(root, node);

    return liteco_rbt_err_success;
}

liteco_rbt_t *liteco_rbt_find_impl(liteco_rbt_t *const root, const void *const key, liteco_rbt_cmp_cb cb) {
    liteco_rbt_t *ret = root;
    while (liteco_rbt_is_not_nil(ret)) {
        switch (cb(key, ret)) {
        case LITECO_RBT_EQ:
            return ret;
        case LITECO_RBT_LS:
            ret = ret->rb_l;
            break;
        case LITECO_RBT_GT:
            ret = ret->rb_r;
            break;
        }
    }

    return liteco_rbt_nil;
}

int liteco_rbt_remove_impl(liteco_rbt_t **const root, liteco_rbt_t **const node) {
    liteco_rbt_t *ref = *node;
    if (liteco_rbt_is_not_nil(ref->rb_l) && liteco_rbt_is_not_nil(ref->rb_r)) {
        liteco_rbt_t *nex = __min(ref->rb_r);
        liteco_rbt_t mid;

        __assign(root, &mid, nex);
        __assign(root, nex, ref);
        __assign(root, ref, &mid);
    }
    liteco_rbt_t *chd = liteco_rbt_is_not_nil(ref->rb_l) ? ref->rb_l : ref->rb_r;
    if (ref->rb_color == LITECO_RBT_BLACK) {
        ref->rb_color = chd->rb_color;
        __rm1(root, ref);
    }
    __replace(root, ref, chd);
    if (liteco_rbt_is_nil(ref->rb_p) && liteco_rbt_is_not_nil(chd)) {
        chd->rb_color = LITECO_RBT_BLACK;
    }
    *node = ref;

    return liteco_rbt_err_success;
}

static inline void __lr(liteco_rbt_t **const root, liteco_rbt_t *const node) {
    liteco_rbt_t *chd = node->rb_r;
    node->rb_r = chd->rb_l;
    if (liteco_rbt_is_not_nil(chd->rb_l)) {
        chd->rb_l->rb_p = node;
    }
    if (liteco_rbt_is_not_nil(chd)) {
        chd->rb_p = node->rb_p;
    }
    if (liteco_rbt_is_nil(node->rb_p)) {
        *root = chd;
    }
    else if (node == node->rb_p->rb_l) {
        node->rb_p->rb_l = chd;
    }
    else {
        node->rb_p->rb_r = chd;
    }
    if (liteco_rbt_is_not_nil(chd)) {
        chd->rb_l = node;
    }
    node->rb_p = chd;
}

static inline void __rr(liteco_rbt_t **const root, liteco_rbt_t *const node) {
    liteco_rbt_t *const chd = node->rb_l;
    node->rb_l = chd->rb_r;
    if (liteco_rbt_is_not_nil(chd->rb_r)) {
        chd->rb_r->rb_p = node;
    }
    if (liteco_rbt_is_not_nil(chd)) {
        chd->rb_p = node->rb_p;
    }
    if (liteco_rbt_is_nil(node->rb_p)) {
        *root = chd;
    }
    else if (node == node->rb_p->rb_l) {
        node->rb_p->rb_l = chd;
    }
    else {
        node->rb_p->rb_r = chd;
    }
    if (liteco_rbt_is_not_nil(chd)) {
        chd->rb_r = node;
    }
    node->rb_p = chd;
}

static void __fix(liteco_rbt_t **const root, liteco_rbt_t *node) {
    liteco_rbt_t *uncle = NULL;

    while (node->rb_p->rb_color == LITECO_RBT_RED) {
        if (node->rb_p == node->rb_p->rb_p->rb_l) {
            uncle = node->rb_p->rb_p->rb_r;
            if (uncle->rb_color == LITECO_RBT_RED) {
                uncle->rb_color = LITECO_RBT_BLACK;
                node->rb_p->rb_color = LITECO_RBT_BLACK;
                node->rb_p->rb_p->rb_color = LITECO_RBT_RED;
                node = node->rb_p->rb_p;
            }
            else {
                if (node == node->rb_p->rb_r) {
                    node = node->rb_p;
                    __lr(root, node);
                }
                node->rb_p->rb_color = LITECO_RBT_BLACK;
                node->rb_p->rb_p->rb_color = LITECO_RBT_RED;
                __rr(root, node->rb_p->rb_p);
            }
        }
        else {
            uncle = node->rb_p->rb_p->rb_l;
            if (uncle->rb_color == LITECO_RBT_RED) {
                uncle->rb_color = LITECO_RBT_BLACK;
                node->rb_p->rb_color = LITECO_RBT_BLACK;
                node->rb_p->rb_p->rb_color = LITECO_RBT_RED;
                node = node->rb_p->rb_p;
            }
            else {
                if (node == node->rb_p->rb_l) {
                    node = node->rb_p;
                    __rr(root, node);
                }
                node->rb_p->rb_color = LITECO_RBT_BLACK;
                node->rb_p->rb_p->rb_color = LITECO_RBT_RED;
                __lr(root, node->rb_p->rb_p);
            }
        }
    }
    (*root)->rb_color = LITECO_RBT_BLACK;
}

static liteco_rbt_t *__min(liteco_rbt_t *node) {
    while (liteco_rbt_is_not_nil(node)) {
        node = node->rb_l;
    }
    return node;
}

static inline void __assign(liteco_rbt_t **const root, liteco_rbt_t *const tgt, liteco_rbt_t *const ref) {
    *tgt = *ref;

    if (liteco_rbt_is_nil(ref->rb_p)) {
        *root = tgt;
    }
    else if (ref->rb_p->rb_l == ref) {
        ref->rb_p->rb_l = tgt;
    }
    else {
        ref->rb_p->rb_r = tgt;
    }

    if (liteco_rbt_is_not_nil(ref->rb_l)) {
        ref->rb_l->rb_p = tgt;
    }
    if (liteco_rbt_is_not_nil(ref->rb_r)) {
        ref->rb_r->rb_p = tgt;
    }
}

static inline liteco_rbt_t *__sib(const liteco_rbt_t *const node) {
    return node->rb_p->rb_l == node ? node->rb_p->rb_r : node->rb_p->rb_l;
}

static void __rm1(liteco_rbt_t **const root, liteco_rbt_t *const node) {
    if (liteco_rbt_is_not_nil(node->rb_p)) {
        __rm2(root, node);
    }
}

static void __rm2(liteco_rbt_t **const root, liteco_rbt_t *const node) {
    liteco_rbt_t *sib = __sib(node);

    if (sib->rb_color == LITECO_RBT_RED) {
        node->rb_p->rb_color = LITECO_RBT_RED;
        sib->rb_color = LITECO_RBT_BLACK;
        if (node == node->rb_p->rb_l) {
            __lr(root, node->rb_p);
        }
        else {
            __rr(root, node->rb_p);
        }
    }

    __rm3(root, node);
}

static void __rm3(liteco_rbt_t **const root, liteco_rbt_t *const node) {
    liteco_rbt_t *sib = __sib(node);

    if (node->rb_p->rb_color == LITECO_RBT_BLACK &&
        sib->rb_color == LITECO_RBT_BLACK &&
        sib->rb_l->rb_color == LITECO_RBT_BLACK &&
        sib->rb_r->rb_color == LITECO_RBT_BLACK) {
        sib->rb_color = LITECO_RBT_RED;
        __rm1(root, node->rb_p);
    }
    else {
        __rm4(root, node);
    }
}

static void __rm4(liteco_rbt_t **const root, liteco_rbt_t *const node) {
    liteco_rbt_t *sib = __sib(node);

    if (node->rb_p->rb_color == LITECO_RBT_RED &&
        sib->rb_color == LITECO_RBT_BLACK &&
        sib->rb_l->rb_color == LITECO_RBT_BLACK &&
        sib->rb_r->rb_color == LITECO_RBT_BLACK) {
        sib->rb_color = LITECO_RBT_RED;
        node->rb_p->rb_color = LITECO_RBT_BLACK;
    }
    else {
        __rm5(root, node);
    }
}

static void __rm5(liteco_rbt_t **const root, liteco_rbt_t *const node) {
    liteco_rbt_t *sib = __sib(node);

    if (node->rb_p->rb_l == node &&
        sib->rb_color == LITECO_RBT_BLACK &&
        sib->rb_l->rb_color == LITECO_RBT_RED &&
        sib->rb_r->rb_color == LITECO_RBT_BLACK) {
        sib->rb_color = LITECO_RBT_RED;
        sib->rb_l->rb_color = LITECO_RBT_BLACK;
        __rr(root, sib);
    }
    else if (node->rb_p->rb_r == node &&
             sib->rb_color == LITECO_RBT_BLACK &&
             sib->rb_r->rb_color == LITECO_RBT_RED &&
             sib->rb_l->rb_color == LITECO_RBT_BLACK) {
        sib->rb_color = LITECO_RBT_RED;
        sib->rb_r->rb_color = LITECO_RBT_BLACK;
        __lr(root, sib);
    }

    __rm6(root, node);
}

static void __rm6(liteco_rbt_t **const root, liteco_rbt_t *const node) {
    liteco_rbt_t *sib = __sib(node);

    sib->rb_color = node->rb_p->rb_color;
    node->rb_p->rb_color = LITECO_RBT_BLACK;
    if (node == node->rb_p->rb_l) {
        sib->rb_r->rb_color = LITECO_RBT_BLACK;
        __lr(root, node->rb_p);
    }
    else {
        sib->rb_l->rb_color = LITECO_RBT_BLACK;
        __rr(root, node->rb_p);
    }
}

static inline void __replace(liteco_rbt_t **const root, liteco_rbt_t *const lf, liteco_rbt_t *const rt) {
    if (lf == *root) {
        *root = rt;
    }
    else if (lf == lf->rb_p->rb_l) {
        lf->rb_p->rb_l = rt;
    }
    else {
        lf->rb_p->rb_r = rt;
    }
    if (liteco_rbt_is_not_nil(rt)) {
        rt->rb_p = lf->rb_p;
    }
}

liteco_rbt_cmp_result_t liteco_rbt_uint64_cmp_cb(const void *const key, const liteco_rbt_t *const node) {
    const uint64_t spec_key = *(const uint64_t *) key;
    liteco_uint64_rbt_t *const spec_node = (liteco_uint64_rbt_t *) node;

    if (spec_key == spec_node->key) {
        return LITECO_RBT_EQ;
    }
    else if (spec_key < spec_node->key) {
        return LITECO_RBT_LS;
    }
    else {
        return LITECO_RBT_GT;
    }
}

liteco_rbt_cmp_result_t liteco_rbt_int_cmp_cb(const void *const key, const liteco_rbt_t *const node) {
    const int spec_key = *(const uint64_t *) key;
    liteco_int_rbt_t *const spec_node = (liteco_int_rbt_t *) node;

    if (spec_key == spec_node->key) {
        return LITECO_RBT_EQ;
    }
    else if (spec_key < spec_node->key) {
        return LITECO_RBT_LS;
    }
    else {
        return LITECO_RBT_GT;
    }
}

liteco_rbt_t *liteco_rbt_iterator_next(liteco_rbt_iterator_t *const itr) {
#define __r(n) (liteco_rbt_is_not_nil((n)->rb_r) ? (n)->rb_r : (n)->morris_link)
#define __l(n) (n)->rb_l

    if (itr->interrupt[0]) {
        goto travel_0;
    }
    if (itr->interrupt[1]) {
        goto travel_1;
    }

    while (!liteco_rbt_iterator_end(itr)) {
        if (liteco_rbt_is_nil(__l(itr->ptr))) {
            itr->interrupt[0] = true;
            return itr->ptr;

travel_0:
            itr->interrupt[0] = false;
            itr->ptr = __r(itr->ptr);
        }
        else {
            itr->mr = __l(itr->ptr);
            while (liteco_rbt_is_not_nil(__r(itr->mr)) && __r(itr->mr) != itr->ptr) { itr->mr = __r(itr->mr); }
            if (liteco_rbt_is_nil(__r(itr->mr))) {
                itr->mr->morris_link = itr->ptr;
                itr->ptr = __l(itr->ptr);
            }
            else {
                itr->interrupt[1] = true;
                return itr->ptr;

travel_1:
                itr->interrupt[1] = false;
                itr->ptr = __r(itr->mr);
                itr->mr->morris_link = liteco_rbt_nil;
                itr->ptr = __r(itr->ptr);
            }
        }
    }

#undef __r
#undef __l

    return liteco_rbt_nil;
}
