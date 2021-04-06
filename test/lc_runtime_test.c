#include "liteco.h"
#include <stdio.h>

uint8_t st[4096];

liteco_chan_t chan;

int cb(void *const arg) {
    (void) arg;
    liteco_chan_pop(&chan, true);
    printf("Hello\n");
    return 0;
}

uint8_t st2[4096];

int cb2(void *const arg) {
    (void) arg;
    liteco_chan_push(&chan, NULL, false);
    printf("world\n");
    return 0;
}

int main() {
    liteco_co_t co;
    liteco_co_init(&co, cb, NULL, st, sizeof(st));

    liteco_co_t co2;
    liteco_co_init(&co2, cb2, NULL, st2, sizeof(st2));

    liteco_eloop_t eloop;
    liteco_eloop_init(&eloop);

    liteco_runtime_t rt;
    liteco_runtime_init(&eloop, &rt);

    liteco_chan_init(&chan, 0, &rt);

    liteco_runtime_join(&rt, &co2);
    liteco_runtime_join(&rt, &co);

    liteco_eloop_run(&eloop);

    return 0;
}
