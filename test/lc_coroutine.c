#include "liteco.h"
#include <stdio.h>

uint8_t st[4096];

int cb(void *const arg) {
    (void) arg;

    printf("HELLO\n");

    liteco_yield();

    printf("WORLD\n");

    return 0;
}

int main() {
    liteco_co_t co;
    liteco_init(&co, cb, NULL, st, sizeof(st));

    liteco_resume(&co);

    return 0;
}
