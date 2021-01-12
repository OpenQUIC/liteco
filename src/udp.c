/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "udp.h"
#include "runtime.h"
#include <fcntl.h>

static void liteco_udp_cb(liteco_emodule_t *const emodule);

int liteco_udp_create(liteco_udp_t *const udp,
                      const uint32_t rcount, const uint32_t scount,
                      void (*co_ready) (void *const, liteco_co_t *const), void *const proc) {

    udp->fd = -1;
    udp->cb = liteco_udp_cb;
    udp->recvalloc_cb = NULL;

    liteco_chan_create(&udp->rchan, rcount, co_ready, proc);
    liteco_chan_create(&udp->schan, scount, co_ready, proc);

    return liteco_udp_err_success;
}

int liteco_udp_bind(liteco_udp_t *const udp, const struct sockaddr *const sockaddr, const socklen_t socklen) {
    udp->fd = socket(sockaddr->sa_family, SOCK_DGRAM, 0);
    if (udp->fd == -1) {
        return liteco_udp_err_internal_error;
    }

    int flags = fcntl(udp->fd, F_GETFL);
    fcntl(udp->fd, F_SETFL, flags | O_NONBLOCK);

    if (bind(udp->fd, sockaddr, socklen) != 0) {
        return liteco_udp_err_internal_error;
    }
    return liteco_udp_err_success;
}

int liteco_udp_send(liteco_udp_t *const udp, liteco_udp_pkt_t *const pkt) {
    return liteco_chan_unenforceable_push(&udp->schan, pkt);
}

int liteco_udp_sendto(liteco_udp_t *const udp,
                      const struct sockaddr *const sockaddr, socklen_t socklen,
                      const void *const data, const uint32_t datalen) {
    if (sendto(udp->fd, data, datalen, 0, sockaddr, socklen) <= 0) {
        return liteco_udp_err_internal_error;
    }

    return liteco_udp_err_success;
}

int liteco_udp_set_recvalloc(liteco_udp_t *const udp, int (*alloc_cb) (liteco_udp_pkt_t **const, liteco_udp_t *const)) {
    udp->recvalloc_cb = alloc_cb;

    return liteco_udp_err_success;
}

static void liteco_udp_cb(liteco_emodule_t *const emodule) {
    liteco_udp_t *const udp = (liteco_udp_t *) emodule;

    for ( ;; ) {
        liteco_udp_pkt_t *pkt;
        if (udp->recvalloc_cb(&pkt, udp) != liteco_udp_err_success) {
            return;
        }
        socklen_t socklen = 0;
        int ret = recvfrom(udp->fd, pkt->data, pkt->cap, 0, &pkt->remote_addr.addr, &socklen);
        if (ret <= 0) {
            pkt->recovery(pkt);
            return;
        }
        pkt->len = ret;

        if (liteco_chan_unenforceable_push(&udp->rchan, pkt) != liteco_chan_err_success) {
            pkt->recovery(pkt);
            return;
        }
    }
}
