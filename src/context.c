#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#if TEST_CONTEXT
#include <dirent.h>
#endif
#include "context.h"
#include "forest.h"
#include "wordbank.h"
#include "util.h"


struct ctx_nback_deque *ctx_nback_new(size_t n)
{
    struct ctx_nback_deque *cnd = safe_malloc(sizeof(*cnd), __LINE__);
    size_t i = 0;
    cnd->n = n;
    cnd->trees = safe_malloc(sizeof(*cnd->trees)*n, __LINE__);
    for(i = 0; i < n; i++) {
        cnd->trees[i] = NULL;
    }
    return cnd;
}

void ctx_nback_free(struct ctx_nback_deque *cnd)
{
    safe_free(cnd->trees, __LINE__);
    safe_free(cnd, __LINE__);
    cnd = NULL;
}

void ctx_nback_append(struct ctx_nback_deque *cnd, struct tree *t)
{
    size_t i = 0;
    // move pointers back 1, then append to end
    for(i = 0; i < cnd->n-1; i++) {
        cnd->trees[i] = cnd->trees[i+1];
    }
    cnd->trees[cnd->n-1] = t;
}

#if TEST_CONTEXT

void ctx_dump_test(struct ctx_nback_deque *cnd)
{
    size_t i = 0;
    printf("------------------------\n");
    for(i = 0; i < 3; i++) {
        if(cnd->trees[i]) {
            printf("tree:%lu:%lu\n", i, cnd->trees[i]->index);
        } else {
            printf("tree:%lu:-\n", i);
        }
    }
}
int main(void)
{
    struct ctx_nback_deque *cnd = ctx_nback_new(3);
    struct tree *t1 = tree_new(0, 0);
    struct tree *t2 = tree_new(1, 1);
    struct tree *t3 = tree_new(2, 2);
    struct tree *t4 = tree_new(3, 3);
    struct tree *t5 = tree_new(4, 4);
    struct tree *t6 = tree_new(5, 5);;
    ctx_nback_append(cnd, t1); ctx_dump_test(cnd);
    ctx_nback_append(cnd, t2); ctx_dump_test(cnd);
    ctx_nback_append(cnd, t3); ctx_dump_test(cnd);
    ctx_nback_append(cnd, t4); ctx_dump_test(cnd);
    ctx_nback_append(cnd, t5); ctx_dump_test(cnd);
    ctx_nback_append(cnd, t6); ctx_dump_test(cnd);

    return 0;
}
#endif
