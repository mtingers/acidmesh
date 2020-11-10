#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "mesh.h"
#include "datatree.h"
#include "container.h"
#include "context.h"

struct context *ctx_init(void)
{
    struct context *c = safe_malloc(sizeof(*c), __LINE__);
    c->seqs_len = 0;
    c->seqs = NULL;
    c->prev_ctx = NULL;
    c->next_ctx = NULL;
    return c;
}
