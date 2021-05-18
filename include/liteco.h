/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_H__
#define __LITECO_H__

#include <netinet/in.h>
#include <sys/cdefs.h>
#if defined(__linux__)
#include "liteco/linux.h"
#elif defined(__APPLE__)
#include "liteco/darwin.h"
#endif
#include "liteco/lc_link.h"
#include "liteco/lc_array.h"
#include <pthread.h>
#include <sys/socket.h>

typedef struct liteco_handler_s liteco_handler_t;    
typedef struct liteco_async_s liteco_async_t;
typedef struct liteco_runtime_s liteco_runtime_t;
typedef struct liteco_timer_s liteco_timer_t;
typedef struct liteco_udp_s liteco_udp_t;
typedef struct liteco_eloop_s liteco_eloop_t;

typedef void (*liteco_async_cb) (liteco_async_t *const async);
typedef void (*liteco_timer_cb) (liteco_timer_t *const timer);
typedef void (*liteco_udp_alloc_cb) (liteco_udp_t *const udp, void **const b_ptr, size_t *const b_size);
typedef void (*liteco_udp_recv_cb) (liteco_udp_t *const udp, int ret, const struct sockaddr *const peer, const void *const buf, const size_t b_size);

enum liteco_handler_type_e {
    liteco_handler_type_async = 0,
    liteco_handler_type_timer,
    liteco_handler_type_udp,
    liteco_handler_type_runtime
};
typedef enum liteco_handler_type_e liteco_handler_type_t;

#define LITECO_HANDLER_FIELDS   \
    liteco_eloop_t *eloop;      \
    liteco_handler_type_t type; \

#define liteco_handler_init(_handler, _eloop, _type) \
    *((liteco_handler_t *) _handler) = (liteco_handler_t) { .eloop = _eloop, .type = _type }

struct liteco_handler_s { LITECO_HANDLER_FIELDS };

typedef union liteco_addr_u liteco_addr_t;

size_t liteco_addr_type_size(const liteco_addr_t *const addr);

union liteco_addr_u {
    struct sockaddr_in af_inet;
    struct sockaddr_in6 af_inet6;
};

liteco_addr_t liteco_ipv4(const char *const ip, const uint16_t port);

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

#define LITECO_TIMER_FIELDS      \
    LITECO_TIMER_PLATFORM_FIELDS \
    liteco_timer_cb cb;          \
    bool active;                 \
    bool stop;                   \

struct liteco_timer_s { LITECO_HANDLER_FIELDS LITECO_TIMER_FIELDS };

int liteco_timer_init(liteco_eloop_t *const eloop, liteco_timer_t *const timer);
int liteco_timer_start(liteco_timer_t *const timer, liteco_timer_cb cb, const uint64_t timeout, const uint64_t interval);
int liteco_timer_stop(liteco_timer_t *const timer);

#define LITECO_UDP_FIELDS         \
    liteco_io_t io;               \
    liteco_udp_alloc_cb alloc_cb; \
    liteco_udp_recv_cb recv_cb;   \
    liteco_addr_t bd_addr;        \

struct liteco_udp_s { LITECO_HANDLER_FIELDS LITECO_UDP_FIELDS };

int liteco_udp_init(liteco_eloop_t *const eloop, liteco_udp_t *const udp);
int liteco_udp_bind(liteco_udp_t *const udp, const struct sockaddr *const addr);
int liteco_udp_sendto(liteco_udp_t *const udp, struct sockaddr *const addr, const void *const buf, const size_t len);
int liteco_udp_recv(liteco_udp_t *const udp, liteco_udp_alloc_cb alloc_cb, liteco_udp_recv_cb recv_cb);
int liteco_udp_close(liteco_udp_t *const udp);

struct liteco_eloop_s { LITECO_ELOOP_FIELDS };

int liteco_eloop_init(liteco_eloop_t *const eloop);
int liteco_eloop_run(liteco_eloop_t *const eloop);
int liteco_eloop_close(liteco_eloop_t *const eloop);

typedef struct liteco_co_s liteco_co_t;
typedef struct liteco_fin_s liteco_fin_t;
typedef struct liteco_waiter_s liteco_waiter_t;
typedef struct liteco_case_s liteco_case_t;
typedef struct liteco_chan_s liteco_chan_t;
typedef struct liteco_timer_chan_s liteco_timer_chan_t;
typedef struct liteco_udp_chan_s liteco_udp_chan_t;
typedef struct liteco_udp_chan_ele_s liteco_udp_chan_ele_t;

typedef void (*liteco_udp_chan_alloc_cb) (liteco_udp_chan_t *const uchan, liteco_udp_chan_ele_t **const ele); 
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

int liteco_co_init(liteco_co_t *const co, int (*cb) (void *const), void *const arg, void *const st, const size_t st_size);
liteco_status_t liteco_resume(liteco_co_t *const co);
void liteco_yield();
void liteco_co_finished(liteco_co_t *const co, int (*fin_cb) (void *const), void *const arg);

typedef struct liteco_ready_s liteco_ready_t;
struct liteco_ready_s {
    LITECO_LINKNODE_BASE

    liteco_co_t *co;
};

#define LITECO_RUNTIME_FIELDS \
    liteco_async_t rt_async;  \
    liteco_ready_t rq;        \
    pthread_mutex_t mtx;      \

struct liteco_runtime_s { LITECO_HANDLER_FIELDS LITECO_RUNTIME_FIELDS };

int liteco_runtime_init(liteco_eloop_t *const eloop, liteco_runtime_t *const runtime);
int liteco_runtime_join(liteco_runtime_t *const rt, liteco_co_t *const co);

struct liteco_waiter_s {
    LITECO_LINKNODE_BASE

    liteco_co_t *co;
    bool select;
    union {
        void **r_store;
        void *s_ele;
    } ele;
};

struct liteco_chan_s {
    liteco_array_t *queue;
    uint32_t head;
    uint32_t tail;
    bool full;

    liteco_waiter_t w;
    liteco_waiter_t r;

    bool closed;

    pthread_mutex_t mtx;

    liteco_runtime_t *rt;
};
extern void *const liteco_chan_pop_failed;

int liteco_chan_init(liteco_chan_t *const chan, const uint32_t ele_cnt, liteco_runtime_t *const rt);
int liteco_chan_close(liteco_chan_t *const chan);
int liteco_chan_destory(liteco_chan_t *const chan);
int liteco_chan_push(liteco_chan_t *const chan, void *const ele, const bool blocked);
void *liteco_chan_pop(liteco_chan_t *const chan, const bool blocked);
int liteco_chan_unenforceable_push(liteco_chan_t *const chan, void *const ele);
void *liteco_chan_unenforceable_pop(liteco_chan_t *const chan);

enum liteco_casetype_e {
    liteco_casetype_push = 0,
    liteco_casetype_pop
};
typedef enum liteco_casetype_e liteco_casetype_t;

struct liteco_case_s {
    liteco_chan_t *chan;
    liteco_casetype_t type;
    void *ele;
};

liteco_case_t *liteco_select(liteco_case_t *const cases, const uint32_t count, const bool blocked);

struct liteco_timer_chan_s {
    liteco_timer_t timer;
    liteco_chan_t chan;
};

int liteco_timer_chan_init(liteco_eloop_t *const eloop, liteco_runtime_t *const rt, liteco_timer_chan_t *const tchan);
int liteco_timer_chan_start(liteco_timer_chan_t *const tchan, const uint64_t timeout, const uint64_t interval);
int liteco_timer_chan_stop(liteco_timer_chan_t *const tchan);
int liteco_timer_chan_close(liteco_timer_chan_t *const tchan);
void *liteco_timer_chan_pop(liteco_timer_chan_t *const tchan, const bool blocked);
liteco_chan_t *liteco_timer_chan(liteco_timer_chan_t *const tchan);

struct liteco_udp_chan_s {
    liteco_udp_t udp;
    liteco_chan_t *chan;

    liteco_udp_chan_alloc_cb alloc_cb;
};

struct liteco_udp_chan_ele_s {
    liteco_addr_t loc_addr;
    liteco_addr_t rmt_addr;

    int ret;
    size_t b_size;
    uint8_t buf[0];
};

int liteco_udp_chan_init(liteco_eloop_t *const eloop, liteco_udp_chan_t *const uchan);
int liteco_udp_chan_bind(liteco_udp_chan_t *const uchan, const struct sockaddr *const addr, liteco_chan_t *const chan);
int liteco_udp_chan_sendto(liteco_udp_chan_t *const uchan, struct sockaddr *const addr, const void *const buf, const size_t len);
int liteco_udp_chan_recv(liteco_udp_chan_t *const uchan, liteco_udp_chan_alloc_cb alloc_cb);
int liteco_udp_chan_close(liteco_udp_chan_t *const uchan);

#endif
