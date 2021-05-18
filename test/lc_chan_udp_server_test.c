#include "liteco.h"
#include <alloca.h>
#include <stdio.h>
#include <arpa/inet.h>

uint8_t st[8192];

void alloc_cb(liteco_udp_chan_t *const uchan, liteco_udp_chan_ele_t **const ele) {
    (void) uchan;

    *ele = malloc(sizeof(liteco_udp_chan_ele_t) + 256);
    (*ele)->b_size = 256;
}

int co_cb(void *const args) {
    liteco_runtime_t *const rt = args;

    liteco_udp_chan_t uchan;
    liteco_udp_chan_init(rt->eloop, rt, &uchan);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10011);
    inet_aton("127.0.0.1", &addr.sin_addr);
    liteco_udp_chan_bind(&uchan, (struct sockaddr *) &addr);

    liteco_udp_chan_recv(&uchan, alloc_cb);

    liteco_udp_chan_ele_t *const ele = liteco_udp_chan_pop(&uchan, true);

    printf("%s\n", ele->buf);

    liteco_udp_chan_close(&uchan);
    return 0;
}

int main() {
    liteco_eloop_t eloop;
    liteco_eloop_init(&eloop);

    liteco_runtime_t rt;
    liteco_runtime_init(&eloop, &rt);

    liteco_co_t co;
    liteco_co_init(&co, co_cb, &rt, st, sizeof(st));
    liteco_runtime_join(&rt, &co);

    liteco_eloop_run(&eloop);
}
