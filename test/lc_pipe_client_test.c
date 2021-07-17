#include "liteco.h"
#include <stdio.h>

int main() {
    liteco_eloop_t eloop;
    liteco_eloop_init(&eloop);

    liteco_pipe_t pipe;
    liteco_pipe_init(&eloop, &pipe, false);

    int ret = liteco_pipe_connect(&pipe, "./pipe_server");

    liteco_stream_send((liteco_stream_t *) &pipe, "Hello", 6);

    printf("%d\n", ret);

    return 0;
}
