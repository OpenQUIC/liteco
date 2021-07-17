#include "liteco.h"
#include <stdio.h>

void alloc_cb(liteco_stream_t *const pipe, void **const b_ptr, size_t *const b_size) {
    (void) pipe;

    *b_ptr = malloc(512);
    *b_size = 512;
}

void recv_cb(liteco_stream_t *const pipe, int ret, const void *const buf, const size_t b_size) {
    (void) pipe;
    (void) b_size;

    printf("%d\n", ret);

    if (ret <= 0) {
        // remote closed
        return;
    }

    printf("%*s", ret, (char *) buf);
    free((void *) buf);
}

void connect_cb(liteco_stream_t *const server, int status) {
    (void) status;

    liteco_pipe_t *client = malloc(sizeof(liteco_pipe_t));
    liteco_pipe_init(server->eloop, client, false);

    liteco_stream_accept(server, (liteco_stream_t *) client);

    liteco_stream_recv((liteco_stream_t *) client, alloc_cb, recv_cb);
}

int main() {
    liteco_eloop_t eloop;
    liteco_eloop_init(&eloop);

    liteco_pipe_t pipe;
    liteco_pipe_init(&eloop, &pipe, false);

    liteco_pipe_bind(&pipe, "./pipe_server");
    liteco_stream_listen((liteco_stream_t *) &pipe, 10, connect_cb);

    liteco_eloop_run(&eloop);

    return 0;
}
