/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#ifndef __LITECO_UDP_H__
#define __LITECO_UDP_H__

#include "emodule.h"
#include "channel.h"
#include <netinet/in.h>

#define liteco_udp_err_internal_error -1001
#define liteco_udp_err_success 0

typedef union liteco_sockaddr_u liteco_sockaddr_t;
union liteco_sockaddr_u {
    struct sockaddr_in in;
    struct sockaddr_in6 in6;
    struct sockaddr addr;
};

typedef struct liteco_udp_pkt_s liteco_udp_pkt_t;
struct liteco_udp_pkt_s {
    void (*recovery) (liteco_udp_pkt_t *const);

    liteco_sockaddr_t remote_addr;

    uint32_t cap;
    uint32_t len;
    uint8_t data[0];
};

typedef struct liteco_udp_s liteco_udp_t;
struct liteco_udp_s {
    LITECO_EMODULE_FIELD

    liteco_chan_t schan;
    liteco_chan_t rchan;

    int (*recvalloc_cb) (liteco_udp_pkt_t **const, liteco_udp_t *const);
};

int liteco_udp_create(liteco_udp_t *const udp,
                      const uint32_t rcount, const uint32_t scount,
                      void (*co_ready) (void *const, liteco_co_t *const), void *const proc);
int liteco_udp_set_recvalloc(liteco_udp_t *const udp, int (*alloc_cb) (liteco_udp_pkt_t **const, liteco_udp_t *const));
int liteco_udp_bind(liteco_udp_t *const udp, const struct sockaddr *const sockaddr, const socklen_t socklen);
int liteco_udp_send(liteco_udp_t *const udp, liteco_udp_pkt_t *const pkt);
int liteco_udp_sendto(liteco_udp_t *const udp,
                      const struct sockaddr *const sockaddr, socklen_t socklen,
                      const void *const data, const uint32_t datalen);

#endif
