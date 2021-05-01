/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "liteco.h"
#include "platform/internal.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <stddef.h>

static void liteco_udp_io_cb(liteco_eloop_t *const eloop, liteco_io_t *const handler, const uint32_t flags);

int liteco_udp_init(liteco_eloop_t *const eloop, liteco_udp_t *const udp) {
    liteco_handler_init(udp, eloop, liteco_handler_type_udp);

    int fd = liteco_udp_socket(AF_INET);
    if (fd < 0) {
        return -1;
    }

    liteco_io_init(&udp->io, liteco_udp_io_cb, fd);
    udp->alloc_cb = NULL;
    udp->recv_cb = NULL;

    return 0;
}

int liteco_udp_bind(liteco_udp_t *const udp, const struct sockaddr *const addr) {
    size_t addr_len = 0;
    switch (addr->sa_family) {
    case AF_INET:
        addr_len = sizeof(struct sockaddr_in);
        break;
    case AF_INET6:
        addr_len = sizeof(struct sockaddr_in6);
        break;
    default:
        return -1;
    }

    return bind(udp->io.key, addr, addr_len);
}

int liteco_udp_sendto(liteco_udp_t *const udp, struct sockaddr *const addr, void *const buf, const size_t len) {
    size_t addr_len = 0;
    switch (addr->sa_family) {
    case AF_INET:
        addr_len = sizeof(struct sockaddr_in);
        break;
    case AF_INET6:
        addr_len = sizeof(struct sockaddr_in6);
        break;
    default:
        return -1;
    }

    struct msghdr msg = {
        .msg_name = addr,
        .msg_namelen = addr_len,
        .msg_iov = &(struct iovec){ .iov_base = buf, .iov_len = len },
        .msg_iovlen = 1,
        .msg_control = NULL,
        .msg_controllen = 0,
        .msg_flags = 0
    };

    return sendmsg(udp->io.key, &msg, 0);
}

int liteco_udp_recv(liteco_udp_t *const udp, liteco_udp_alloc_cb alloc_cb, liteco_udp_recv_cb recv_cb) {
    udp->alloc_cb = alloc_cb;
    udp->recv_cb = recv_cb;

    liteco_eloop_udp_add(udp->eloop, udp);

    return 0;
}

static void liteco_udp_io_cb(liteco_eloop_t *const eloop, liteco_io_t *const handler, const uint32_t flags) {
    (void) eloop;
    (void) flags;

    liteco_udp_t *const udp = container_of(handler, liteco_udp_t, io);
    if (!udp->alloc_cb || !udp->recv_cb)  {
        return;
    }

    for ( ;; ) {
        struct sockaddr_storage peer;
        struct iovec buf = { .iov_base = NULL, .iov_len = 0 };
        udp->alloc_cb(udp, &buf.iov_base, &buf.iov_len);
        if (buf.iov_base == NULL || buf.iov_len == 0) {
            udp->recv_cb(udp, -1, NULL, 0);
            return;
        }

        struct msghdr hdr = {
            .msg_name = &peer,
            .msg_namelen = sizeof(struct sockaddr_storage),
            .msg_iov = &buf,
            .msg_iovlen = 1,
            .msg_control = NULL,
            .msg_controllen = 0,
            .msg_flags = 0
        };

        int ret = recvmsg(udp->io.key, &hdr, 0);
        if (ret == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                udp->recv_cb(udp, 0, NULL, 0);
                return;
            }
            else {
                udp->recv_cb(udp, errno, buf.iov_base, buf.iov_len);
                return;
            }
        }
        else {
            udp->recv_cb(udp, ret, buf.iov_base, buf.iov_len);
        }
    }
}

int liteco_udp_chan_init(liteco_eloop_t *const eloop, liteco_runtime_t *const rt, liteco_udp_chan_t *const uchan) {
    liteco_chan_init(&uchan->chan, 0, rt);
    liteco_udp_init(eloop, &uchan->udp);

    return 0;
}

/*static void liteco_udp_cb(liteco_emodule_t *const emodule);*/

/*int liteco_udp_init(liteco_eloop_t *const eloop, liteco_udp_t *const udp, int sa_family) {*/
    /*udp->fd = socket(sa_family, SOCK_DGRAM, 0);*/
    /*if (udp->fd == -1) {*/
        /*return liteco_udp_err_internal_error;*/
    /*}*/
    /*int flags = fcntl(udp->fd, F_GETFL);*/
    /*fcntl(udp->fd, F_SETFL, flags | O_NONBLOCK);*/

    /*udp->rchan = NULL;*/
    /*udp->cb = liteco_udp_cb;*/
    /*udp->alloc_cb = NULL;*/

    /*return liteco_eloop_add(eloop, (liteco_emodule_t *) udp);*/
/*}*/

/*int liteco_udp_bind(liteco_udp_t *const udp, const struct sockaddr *const sockaddr, const socklen_t socklen) {*/
    /*if (bind(udp->fd, sockaddr, socklen) != 0) {*/
        /*return liteco_udp_err_internal_error;*/
    /*}*/
    /*memcpy(&udp->local_addr, sockaddr, socklen);*/

    /*return liteco_udp_err_success;*/
/*}*/

/*int liteco_udp_sendto(liteco_udp_t *const udp,*/
                      /*const struct sockaddr *const sockaddr, socklen_t socklen,*/
                      /*const void *const data, const uint32_t datalen) {*/
    /*if (sendto(udp->fd, data, datalen, 0, sockaddr, socklen) <= 0) {*/
        /*return liteco_udp_err_internal_error;*/
    /*}*/

    /*return liteco_udp_err_success;*/
/*}*/

/*int liteco_udp_set_recv(liteco_udp_t *const udp, int (*alloc_cb) (liteco_udp_pkt_t **const, liteco_udp_t *const), liteco_chan_t *const rchan) {*/
    /*udp->alloc_cb = alloc_cb;*/
    /*udp->rchan = rchan;*/

    /*return liteco_udp_err_success;*/
/*}*/

/*static void liteco_udp_cb(liteco_emodule_t *const emodule) {*/
    /*liteco_udp_t *const udp = (liteco_udp_t *) emodule;*/

    /*if (!udp->rchan || !udp->alloc_cb) {*/
        /*return;*/
    /*}*/

    /*for ( ;; ) {*/
        /*liteco_udp_pkt_t *pkt;*/
        /*if (udp->alloc_cb(&pkt, udp) != liteco_udp_err_success) {*/
            /*return;*/
        /*}*/
        /*pkt->local_addr = udp->local_addr;*/
        /*socklen_t socklen = sizeof(struct sockaddr);*/
        /*int ret = recvfrom(udp->fd, pkt->data, pkt->cap, 0, &pkt->remote_addr.addr, &socklen);*/
        /*if (ret <= 0) {*/
            /*pkt->recovery(pkt);*/
            /*return;*/
        /*}*/
        /*pkt->len = ret;*/

        /*if (liteco_chan_unenforceable_push(udp->rchan, pkt) != liteco_chan_err_success) {*/
            /*pkt->recovery(pkt);*/
            /*return;*/
        /*}*/
    /*}*/
/*}*/
