#include "liteco.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>

uint8_t buf[256];

void alloc(liteco_udp_t *const udp, void **const b_ptr, size_t *const b_size) {
    (void) udp;

    *b_ptr = buf;
    *b_size = 256;
}

void recv_cb(liteco_udp_t *const udp, int ret, const struct sockaddr *const addr, const void *const buf, const size_t b_size) {
    (void) addr;
    (void) udp;
    (void) b_size;
    if (ret == 0) {
        return;
    }

    printf("%s\n", (char *) buf);
}

int main() {
    liteco_eloop_t eloop;
    liteco_eloop_init(&eloop);

    liteco_udp_t udp;
    liteco_udp_init(&eloop, &udp);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10011);
    inet_aton("127.0.0.1", &addr.sin_addr);
    liteco_udp_bind(&udp, (struct sockaddr *) &addr);

    liteco_udp_recv(&udp, alloc, recv_cb);

    liteco_eloop_run(&eloop);

    return 0;
}
