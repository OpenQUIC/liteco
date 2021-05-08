/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "liteco.h"
#include "liteco/darwin.h"
#include "platform/internal.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <stddef.h>

static void liteco_udp_io_cb(liteco_eloop_t *const eloop, liteco_io_t *const handler, const uint32_t flags);

static void liteco_udp_chan_alloc_udp_cb(liteco_udp_t *const udp, void **const b_ptr, size_t *const b_size);
static void liteco_udp_chan_recv_udp_cb(liteco_udp_t *const udp, int ret, const struct sockaddr *const peer, const void *const buf, const size_t b_size);

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
        udp->bd_addr.af_inet = *(struct sockaddr_in *) addr;
        break;
    case AF_INET6:
        addr_len = sizeof(struct sockaddr_in6);
        udp->bd_addr.af_inet6 = *(struct sockaddr_in6 *) addr;
        break;
    default:
        return -1;
    }

    return bind(udp->io.key, addr, addr_len);
}

int liteco_udp_sendto(liteco_udp_t *const udp, struct sockaddr *const addr, const void *const buf, const size_t len) {
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
        .msg_iov = &(struct iovec){ .iov_base = (void *) buf, .iov_len = len },
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

int liteco_udp_close(liteco_udp_t *const udp) {
    return liteco_io_stop(udp->eloop, &udp->io, udp->io.listening_events);
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
            udp->recv_cb(udp, -1, NULL, buf.iov_base, buf.iov_len);
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
                udp->recv_cb(udp, 0, NULL, buf.iov_base, buf.iov_len);
                return;
            }
            else {
                udp->recv_cb(udp, errno, (struct sockaddr *) hdr.msg_name, buf.iov_base, buf.iov_len);
                return;
            }
        }
        else {
            udp->recv_cb(udp, ret, (struct sockaddr *) hdr.msg_name, buf.iov_base, buf.iov_len);
        }
    }
}

static void liteco_udp_chan_alloc_udp_cb(liteco_udp_t *const udp, void **const b_ptr, size_t *const b_size) {
    liteco_udp_chan_t *const uchan = container_of(udp, liteco_udp_chan_t, udp);
    liteco_udp_chan_ele_t *ele = NULL;

    uchan->alloc_cb(uchan, &ele);
    if (!ele) {
        *b_ptr = NULL;
        *b_size = 0;
    }
    else {
        *b_ptr = ele->buf;
        *b_size = ele->b_size;
    }
}

static void liteco_udp_chan_recv_udp_cb(liteco_udp_t *const udp, int ret, const struct sockaddr *const peer, const void *const buf, const size_t b_size) {
    (void) b_size;

    liteco_udp_chan_t *const uchan = container_of(udp, liteco_udp_chan_t, udp);
    liteco_udp_chan_ele_t *const ele = container_of(buf, liteco_udp_chan_ele_t, buf);
    if (ret <= 0) {
        free(ele);
        return;
    }

    ele->ret = ret;
    ele->loc_addr = udp->bd_addr;
    switch (peer->sa_family) {
    case AF_INET:
        ele->rmt_addr.af_inet = *(struct sockaddr_in *) peer;
        break;
    case AF_INET6:
        ele->rmt_addr.af_inet6 = *(struct sockaddr_in6 *) peer;
        break;
    }

    liteco_chan_unenforceable_push(uchan->chan, ele);
}

int liteco_udp_chan_init(liteco_eloop_t *const eloop, liteco_udp_chan_t *const uchan) {
    uchan->chan = NULL;
    liteco_udp_init(eloop, &uchan->udp);

    return 0;
}

int liteco_udp_chan_bind(liteco_udp_chan_t *const uchan, const struct sockaddr *const addr, liteco_chan_t *const chan) {
    uchan->chan = chan;
    return liteco_udp_bind(&uchan->udp, addr);
}

int liteco_udp_chan_sendto(liteco_udp_chan_t  *const uchan, struct sockaddr *const addr, const void *const buf, const size_t len) {
    return liteco_udp_sendto(&uchan->udp, addr, buf, len);
}

int liteco_udp_chan_recv(liteco_udp_chan_t *const uchan, liteco_udp_chan_alloc_cb alloc_cb) {
    uchan->alloc_cb = alloc_cb;

    return liteco_udp_recv(&uchan->udp, liteco_udp_chan_alloc_udp_cb, liteco_udp_chan_recv_udp_cb);
}

int liteco_udp_chan_close(liteco_udp_chan_t *const uchan) {
    liteco_udp_close(&uchan->udp);

    return 0;
}
