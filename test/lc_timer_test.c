#include "liteco.h"
#include <stdio.h>

static int times = 0;
void cb(liteco_timer_t *const timer) {

    times++;

    if (times == 10) {
        printf("HERE\n");
        liteco_timer_stop(timer);
    }

    printf("HELLO\n");
}

int main() {
    liteco_eloop_t eloop;
    liteco_eloop_init(&eloop);

    liteco_timer_t timer;
    liteco_timer_init(&eloop, &timer);

    liteco_timer_start(&timer, cb, 1000 * 1000, 1000 * 1000);

    liteco_eloop_run(&eloop);

    return 0;
}
