#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <pthread.h> 
#include "util.h"
#include "mesh.h"
#include "sequence.h"
#include "datatree.h"
#include "container.h"
#include "context.h"
#include "stm.h"

struct stm *stm_init(size_t nback)
{
    struct stm *s = safe_malloc(sizeof(*s), __LINE__);
    s->nback = nback;
    s->items_pos = 0;
    s->items_inserted = 0;
    s->direction = 1;
    s->items = safe_malloc(sizeof(*s->items)*nback, __LINE__);
    s->items_len = safe_malloc(sizeof(*s->items_len)*nback, __LINE__);
    return s;
}

void stm_insert(struct stm *s, const char *item, size_t item_len)
{
    s->items_inserted++;
    s->items[s->items_pos] = item;
    s->items_len[s->items_pos] = item_len;
    s->items_pos += s->direction;
    if(s->items_pos+1 >= s->nback) {
        s->direction = -1;
    } else if(s->items_pos < 1) {
        s->direction = 1;
    } 
}

void stm_insert_r(struct mesh *m, struct stm*s, const char *item, size_t item_len)
{
    mesh_lock(m);
    stm_insert(s, item, item_len);
    mesh_unlock(m);
}

void stm_free(struct stm *s)
{
    assert(s);
    safe_free(s->items, __LINE__);
    safe_free(s->items_len, __LINE__);
    safe_free(s);
}

