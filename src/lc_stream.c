#include "liteco.h"
#include "liteco/darwin.h"
#include "platform/internal.h"
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

static void liteco_stream_io_cb(liteco_eloop_t *const eloop, liteco_io_t *const handler, const uint32_t flags);

int liteco_stream_init(liteco_eloop_t *const eloop, liteco_stream_t *const stream, const liteco_handler_type_t type) {
    liteco_handler_init(stream, eloop, type);
    stream->alloc_cb = NULL;
    stream->connection_cb = NULL;
    stream->io.key = -1;
    stream->recv_cb = NULL;

    return 0;
}

int liteco_stream_listen(liteco_stream_t *const stream, const int backlog, liteco_stream_connection_cb cb) {
    switch (stream->type) {
    case liteco_handler_type_pipe:
        return liteco_pipe_listen((liteco_pipe_t *) stream, backlog, cb);
    default:
        return -1;
    }
}

int liteco_stream_accept(liteco_stream_t *const server, liteco_stream_t *const client) {
    if (server->accept_fd == -1) {
        return -1;
    }

    switch (client->type) {
    case liteco_handler_type_pipe:
        client->io.key = server->accept_fd;
        break;

    default:
        return -2;
    }
    server->accept_fd = -1;

    liteco_eloop_stream_add(server->eloop, server);

    return 0;
}

int liteco_stream_recv(liteco_stream_t *const stream, liteco_stream_alloc_cb alloc_cb, liteco_stream_recv_cb recv_cb) {
    stream->alloc_cb = alloc_cb;
    stream->recv_cb = recv_cb;

    liteco_io_init(&stream->io, liteco_stream_io_cb, stream->io.key);

    liteco_eloop_stream_add(stream->eloop, stream);

    return 0;
}

int liteco_stream_send(liteco_stream_t *const stream, const void *const buf, const size_t b_size) {
    if (stream->io.key == -1) {
        return -1;
    }

    if (send(stream->io.key, buf, b_size, 0) != 0) {
        return -1;
    }

    return 0;
}

#include <stdio.h>

static void liteco_stream_io_cb(liteco_eloop_t *const eloop, liteco_io_t *const handler, const uint32_t flags) {
    (void) eloop;
    (void) flags;

    liteco_stream_t *const stream = container_of(handler, liteco_stream_t, io);
    if (!stream->alloc_cb || !stream->recv_cb) {
        return;
    }

    for ( ;; ) {
        struct iovec buf = { .iov_base = NULL, .iov_len = 0 };
        stream->alloc_cb(stream, &buf.iov_base, &buf.iov_len);
        if (buf.iov_base == NULL || buf.iov_len == 0) {
            stream->recv_cb(stream, -1, buf.iov_base, buf.iov_len);
            return;
        }

        bool ipc = stream->type == liteco_handler_type_pipe && ((liteco_pipe_t *) stream)->ipc;
        int ret = 0;
        if (ipc) {
            struct msghdr hdr = {
                .msg_name = NULL,
                .msg_namelen = 0,
                .msg_iov = &buf,
                .msg_iovlen = 1,
                .msg_control = NULL,
                .msg_controllen = 0,
                .msg_flags = 0
            };

            do {
                ret = recvmsg(stream->io.key, &hdr, 0);
            } while (ret == -1 && errno == EINTR);
        }
        else {
            do {
                ret = read(stream->io.key, buf.iov_base, buf.iov_len);
            } while (ret == -1 && errno == EINTR);
        }

        if (ret <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                stream->recv_cb(stream, 0, buf.iov_base, buf.iov_len);
            }
            else {
                stream->recv_cb(stream, errno, buf.iov_base, buf.iov_len);
            }
            return;
        }
        else {
            stream->recv_cb(stream, ret, buf.iov_base, buf.iov_len);
        }
    }
}

void liteco_server_io_cb(liteco_eloop_t *const eloop, liteco_io_t *const io, const uint32_t flags) {
    (void) flags;

    liteco_stream_t *const stream = container_of(io, liteco_stream_t, io);

    liteco_eloop_stream_add(eloop, stream);

    while (stream->io.key != -1) {
        int ret = liteco_platform_accept(stream->io.key);

        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            }

            if (errno == ECONNABORTED) {
                continue;
            }
        }

        if (ret < 0) {
            stream->connection_cb(stream, ret);
        }

        stream->accept_fd = ret;
        stream->connection_cb(stream, 0);

        if (stream->accept_fd != -1) {
            liteco_eloop_stream_remove(eloop, stream);
            return;
        }
    }
}
