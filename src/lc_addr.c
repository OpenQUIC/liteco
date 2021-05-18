#include "liteco.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

size_t liteco_addr_type_size(const liteco_addr_t *const addr) {
    switch (((struct sockaddr *) addr)->sa_family) {
    case AF_INET:
        return sizeof(struct sockaddr_in);

    case AF_INET6:
        return sizeof(struct sockaddr_in6);

    default:
        return 0;
    }
}

liteco_addr_t liteco_ipv4(const char *const ip, const uint16_t port) {
    struct sockaddr_in addr;
    inet_aton(ip, &addr.sin_addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    return (liteco_addr_t){ .af_inet = addr };
}

