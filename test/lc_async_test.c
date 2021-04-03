#include "liteco.h"
#include <stdio.h>

void fuck(liteco_async_t *const handle) {
    printf("FUCKYOU\n");

    liteco_async_send(handle);
}

int main() {
    liteco_eloop_t eloop;

    liteco_eloop_init(&eloop);

    liteco_async_t async;
    liteco_async_init(&eloop, &async, fuck);

    liteco_async_send(&async);

    liteco_eloop_run(&eloop);

    return 0;
}
