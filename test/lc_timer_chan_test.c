#include "liteco.h"
#include <stdio.h>

liteco_timer_chan_t tchan;

int cb(void *arg) {
    (void) arg;

    liteco_timer_chan_start(&tchan, 5 * 1000 * 1000, 0);

    printf("HERE\n");
    liteco_chan_pop(&tchan.chan, true);

    printf("HERE\n");

    return 0;
}

uint8_t st[4096];

int main() {
    liteco_eloop_t loop;
    liteco_eloop_init(&loop);

    liteco_co_t co;
    liteco_co_init(&co, cb, NULL, st, sizeof(st));

    liteco_runtime_t rt;
    liteco_runtime_init(&loop, &rt);

    liteco_runtime_join(&rt, &co);

    liteco_timer_chan_init(&loop, &rt, &tchan);

    liteco_eloop_run(&loop);
}
