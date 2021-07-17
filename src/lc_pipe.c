#include "liteco.h"
#include "liteco/darwin.h"
#include "platform/internal.h"
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

int liteco_pipe_init(liteco_eloop_t *const eloop, liteco_pipe_t *const pipe, const bool ipc) {
    liteco_stream_init(eloop, (liteco_stream_t *) pipe, liteco_handler_type_pipe);
    pipe->ipc = ipc;

    return 0;
}

int liteco_pipe_bind(liteco_pipe_t *const pipe, const char *const name) {
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));

    int sockfd = liteco_pipe_socket();
    if (sockfd < 0) {
        return -1;
    }

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, name);

    if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
        liteco_platform_close(sockfd);

        return -1;
    }

    pipe->io.key = sockfd;

    return 0;
}

int liteco_pipe_listen(liteco_pipe_t *const pipe, const int backlog, liteco_stream_connection_cb cb) {
    if (pipe->io.key == -1) {
        return -1;
    }

    if (pipe->ipc) {
        return -1;
    }

    if (listen(pipe->io.key, backlog) != 0) {
        return -1;
    }

    pipe->connection_cb = cb;
    liteco_io_init(&pipe->io, liteco_server_io_cb, pipe->io.key);
    liteco_eloop_stream_add(pipe->eloop, (liteco_stream_t *) pipe);

    return 0;
}

int liteco_pipe_connect(liteco_pipe_t *const pipe, const char *const name) {
    struct sockaddr_un addr;
    int sockfd = liteco_pipe_socket();
    if (sockfd < 0) {
        return -1;
    }

    pipe->io.key = sockfd;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    strcpy(addr.sun_path, name);
    addr.sun_family = AF_UNIX;

    int r;
    do {
        r = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    } while (r == -1 && errno == EINTR);

    return r;
}
