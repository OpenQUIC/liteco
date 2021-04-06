/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_LINK_H__
#define __LITECO_LINK_H__

#define LITECO_LINKNODE_BASE \
    liteco_linknode_t *prev; \
    liteco_linknode_t *next; 

typedef struct liteco_linknode_s liteco_linknode_t;
struct liteco_linknode_s { LITECO_LINKNODE_BASE };

#define liteco_link_init(p) \
    (p)->next = (liteco_linknode_t *) (p); \
    (p)->prev = (liteco_linknode_t *) (p)

#define liteco_link_empty(p) \
    ((p)->next == (liteco_linknode_t *) (p))

#define liteco_link_next(p) \
    ((typeof(p)) (p)->next)

#define liteco_link_prev(p) \
    ((typeof(p)) (p)->prev)

#define liteco_link_insert_after(l, n)           \
    (n)->next = (l)->next;                       \
    (n)->prev = (liteco_linknode_t *) (l);       \
    (l)->next->prev = (liteco_linknode_t *) (n); \
    (l)->next = (liteco_linknode_t *) (n);

#define liteco_link_insert_before(l, n)          \
    (n)->prev = (l)->prev;                       \
    (n)->next = (liteco_linknode_t *) (l);       \
    (l)->prev->next = (liteco_linknode_t *) (n); \
    (l)->prev = (liteco_linknode_t *) (n)

#define liteco_link_remove(p) \
    (p)->next->prev = (p)->prev; \
    (p)->prev->next = (p)->next

#define liteco_link_foreach(e, l) \
    for ((e) = liteco_link_next((l)); (e) != (l); (e) = liteco_link_next((e)))

#define LITECO_STACK_BASE LITECO_LINKNODE_BASE

#define liteco_stack_init(s) liteco_link_init(s)

#define liteco_stack_push(s, p) liteco_link_insert_after(s, p)

#define liteco_stack_pop(s) liteco_link_remove(liteco_link_next(s))

#define liteco_stack_top(s) liteco_link_next(s)

#define liteco_stack_empty(s) liteco_link_empty(s)

#endif
