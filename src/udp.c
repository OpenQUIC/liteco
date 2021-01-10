/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "udp.h"
#include "runtime.h"

int liteco_udp_create(liteco_udp_t *const udp,
                      const uint32_t rcount, const uint32_t scount,
                      void (*co_ready) (void *const, liteco_co_t *const), void *const proc) {

    liteco_chan_create(&udp->rchan, rcount, co_ready, proc);
    liteco_chan_create(&udp->schan, scount, co_ready, proc);

    return liteco_udp_err_success;
}

int liteco_udp_bind(liteco_udp_t *const udp, const struct sockaddr *const sockaddr, const socklen_t socklen) {
    if (bind(udp->fd, sockaddr, socklen) != 0) {
        return liteco_udp_err_internal_error;
    }
    return liteco_udp_err_success;
}

int liteco_udp_sendto(liteco_udp_t *const udp,
                      const struct sockaddr *const sockaddr, socklen_t socklen,
                      const void *const data, const uint32_t datalen) {
    if (sendto(udp->fd, data, datalen, 0, sockaddr, socklen) <= 0) {
        return liteco_udp_err_internal_error;
    }

    return liteco_udp_err_success;
}

