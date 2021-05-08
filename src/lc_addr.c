#include "liteco.h"
#include <sys/socket.h>

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
