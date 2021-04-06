#include "liteco/lc_heap.h"
#include <stddef.h>
#include <stdlib.h>
#include <sys/malloc.h>
#include <stdio.h>

struct test {
    LITECO_HEAPNODE_BASE

    int key;
};

liteco_heap_cmp_result_t cb (liteco_heapnode_t *const p, liteco_heapnode_t *const c) {
    struct test *const pt = (struct test *) p;
    struct test *const ct = (struct test *) c;

    return pt->key > ct->key ? LITECO_HEAP_SWAP : LITECO_HEAP_KEEP;
}

int main() {
    liteco_heap_t heap;
    liteco_heap_init(&heap, cb);

    uint32_t i;
    for (i = 32; i > 0; i--) {
        struct test *t = malloc(sizeof(*t));
        liteco_heapnode_init(t);
        t->key = i;

        liteco_heap_insert(&heap, (liteco_heapnode_t *) t);
    }


    liteco_heap_remove(&heap, heap.root);
    liteco_heap_remove(&heap, heap.root);
    liteco_heap_remove(&heap, heap.root);

    printf("%d\n", ((struct test *) heap.root)->key);

    return 0;
}
