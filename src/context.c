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
