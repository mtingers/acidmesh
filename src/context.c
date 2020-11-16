#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "mesh.h"
#include "sequence.h"
#include "datatree.h"
#include "container.h"
#include "context.h"

static size_t context_id = 0;

struct context *ctx_init(void)
{
    struct context *c = safe_malloc(sizeof(*c), __LINE__);
    c->seqs_len = 0;
    c->seqs = NULL;
    c->prev_ctx = NULL;
    c->next_ctx = NULL;
    c->context_id = context_id;
    context_id++;
    return c;
}
