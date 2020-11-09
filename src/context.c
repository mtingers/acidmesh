#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "forest.h"
#include "wordbank.h"
#include "container.h"
#include "context.h"

struct context *ctx_init()
{
    struct context *c = safe_malloc(sizeof(*c), __LINE__);
    c->trees_len = 0;
    c->trees = NULL;
    c->prev_ctx = NULL;
    c->next_ctx = NULL;
    return c;
}
/*
struct contexts *ctxs_init()
{
    struct contexts *cs = safe_malloc(sizeof(*cs), __LINE__);
    cs->cs_len = 0;
    cs->cs = NULL;
    return cs;
}
*/

/*
    t0-0 -> t0-1 -> t0-2
    t1-0 -> t1-1 -> t1-2 -> t1-3
    t2-0 -> t2-1
    t3-0 -> t3-1 -> t3-2


ctx:
    c->t = t0-0


*/


