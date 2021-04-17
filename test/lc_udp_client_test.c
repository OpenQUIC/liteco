#include "liteco.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    liteco_eloop_t eloop;
    liteco_eloop_init(&eloop);

    liteco_udp_t udp;
    liteco_udp_init(&eloop, &udp);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10010);
    inet_aton("127.0.0.1", &addr.sin_addr);
    liteco_udp_bind(&udp, (struct sockaddr *) &addr);

    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(10011);
    inet_aton("127.0.0.1", &peer.sin_addr);
    liteco_udp_sendto(&udp, (struct sockaddr *) &peer, "hello world", 11);

    return 0;
}
