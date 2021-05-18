#include "liteco/lc_rbt.h"
#include <stdlib.h>
#include <sys/malloc.h>
#include <stdio.h>
#include <assert.h>

struct uint64_key { LITECO_RBT_KEY_UINT64_FIELDS };

int main() {
    liteco_uint64_rbt_t *root;
    liteco_rbt_init(root);


    int i;
    for (i = 5; i < 10; i++) {
        struct uint64_key *ele = malloc(sizeof(struct uint64_key));
        liteco_rbt_node_init(ele);
        ele->key = i;

        liteco_rbt_insert(&root, ele);
    }

    for (i = 0; i < 10; i++) {
        uint64_t key = i;
        struct uint64_key *ele = (struct uint64_key *) liteco_rbt_uint64_find(root, &key);
        (void) ele;

        if (i >= 5) {
            assert(liteco_rbt_is_not_nil(ele));
        }
        else {
            assert(liteco_rbt_is_nil(ele));
        }
    }


    uint64_t exp = 5;
    struct uint64_key *itr;
    liteco_rbt_foreach(itr, root) {
        assert(itr->key == exp);
        exp++;
    }

    for (i = 0; i < 9; i++) {
        uint64_t key = i;
        struct uint64_key *ele = (struct uint64_key *) liteco_rbt_uint64_find(root, &key);

        if (liteco_rbt_is_not_nil(ele)) {
            liteco_rbt_remove(&root, &ele);
        }
    }

    liteco_rbt_foreach(itr, root) {
        assert(itr->key == 9);
    }

    printf("success\n");

    return 0;
}
