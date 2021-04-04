/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_H__
#define __LITECO_H__

#if defined(__linux__)
#include "liteco/linux.h"
#elif defined(__APPLE__)
#include "liteco/darwin.h"
#endif
#include "liteco/lc_link.h"
#include <pthread.h>

typedef struct liteco_handler_s liteco_handler_t;    
typedef struct liteco_async_s liteco_async_t;
typedef struct liteco_timer_S liteco_timer_t;
typedef struct liteco_eloop_s liteco_eloop_t;

typedef void (*liteco_async_cb) (liteco_async_t *const async);
typedef void (*liteco_timer_cb) (liteco_timer_t *const timer);

enum liteco_handler_type_e {
    liteco_handler_type_async = 0
};
typedef enum liteco_handler_type_e liteco_handler_type_t;

#define LITECO_HANDLER_FIELDS   \
    liteco_eloop_t *eloop;      \
    liteco_handler_type_t type; \

#define liteco_handler_init(_handler, _eloop, _type) \
    *((liteco_handler_t *) _handler) = (liteco_handler_t) { .eloop = _eloop, .type = _type }

struct liteco_handler_s { LITECO_HANDLER_FIELDS };


enum liteco_async_padding_status_e {
    LITECO_ASYNC_NO_PADDING = 0x00,
    LITECO_ASYNC_WORKING    = 0x01,
    LITECO_ASYNC_DONE       = 0x02,
};
typedef enum liteco_async_padding_status_e liteco_async_padding_status_t;

#define LITECO_ASYNC_FIELDS               \
    liteco_linknode_t link_handle;        \
    liteco_async_cb cb;                   \
    liteco_async_padding_status_t status; \

struct liteco_async_s { LITECO_HANDLER_FIELDS LITECO_ASYNC_FIELDS };

int liteco_async_init(liteco_eloop_t *const eloop, liteco_async_t *const handler, liteco_async_cb cb);
int liteco_async_send(liteco_async_t *const handler);

#define LITECO_TIMER_FIELDS \

struct liteco_timer_s { LITECO_HANDLER_FIELDS LITECO_TIMER_FIELDS };

struct liteco_eloop_s { LITECO_ELOOP_FIELDS };

int liteco_eloop_init(liteco_eloop_t *const eloop);
int liteco_eloop_run(liteco_eloop_t *const eloop);

typedef struct liteco_co_s liteco_co_t;
typedef struct liteco_fin_s liteco_fin_t;

enum liteco_status_e {
    LITECO_STATUS_UNKNOW = 0,
    LITECO_STATUS_STARTING,
    LITECO_STATUS_READY,
    LITECO_STATUS_RUNNING,
    LITECO_STATUS_WAITING,
    LITECO_STATUS_TERMINATE
};
typedef enum liteco_status_e liteco_status_t;

struct liteco_fin_s {
    LITECO_STACK_BASE

    void *arg;
    int (*finished_cb) (void *const);
};

/*
 * x86_64:
 *  [0010]: R8
 *  [0020]: R9
 *  [0030]: R10
 *  [0040]: R11
 *  [0050]: R12
 *  [0060]: R13
 *  [0070]: R14
 *  [0100]: R15
 *  [0110]: RDI
 *  [0120]: RSI
 *  [0130]: RBP
 *  [0140]: RBX
 *  [0150]: RDX
 *  [0160]: RAX
 *  [0170]: RCX
 *  [0200]: RSP
 *  [0210]: func_addr
 *
 */
typedef uint8_t liteco_context_t[256];

struct liteco_co_s {
    liteco_context_t *p_ctx;

    liteco_context_t ctx;

    void *st;
    size_t st_size;
    liteco_status_t status;

    pthread_mutex_t mtx;

    int (*cb) (void *const);
    void *arg;

    liteco_fin_t fin_st;

    int ret;
};

extern __thread liteco_co_t *this_co;

int liteco_init(liteco_co_t *const co, int (*cb) (void *const), void *const arg, void *const st, const size_t st_size);

liteco_status_t liteco_resume(liteco_co_t *const co);

void liteco_yield();

void liteco_finished(liteco_co_t *const co, int (*fin_cb) (void *const), void *const arg);

#endif
