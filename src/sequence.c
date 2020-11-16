#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include "util.h"
#include "sequence.h"
#include "datatree.h"
#include "container.h"
#include "context.h"
#include "mesh.h"

struct sequence *sequence_init(struct data *w, size_t depth)
{
    struct sequence *s = safe_malloc(sizeof(*s), __LINE__);
    s->depth = depth;
    s->parents = NULL;
    s->data = w;
    s->prev_len = 0;
    s->next_len = 0;
    s->nexts = NULL;
    s->prevs = NULL;
    s->ctxs_len = 0;
    s->contexts = NULL;
    return s;
}

void sequence_dump(struct mesh *f)
{
    size_t i = 0;
    for(i = 0; i < f->container_len; i++) {
        dump_container(f->containers[i], i, 0);
    }
    dump_datas(f->dt->datas, 0);
    for(i = 0; i < f->container_len; i++) {
        printf("depth-data-count:%lu|%lu\n", i, f->containers[i]->count-1);
    }
    size_t j = 0;
    int max = 10;
    printf("Context samples:\n");
    for(i = 1; i < f->ctxs_len; i++) {
        if(max-- < 1)
            break;
        printf("    ");
        for(j = 0; j < f->ctxs[i]->seqs_len; j++) {
            printf("%s ", f->ctxs[i]->seqs[j]->data->data);
        }
        printf("\n");
    }
    printf("Total datas: %lu\n", f->dt->count-1);
    printf("Total contexts:%lu\n", f->ctxs_len-1);
}

void sequence_add_parent(struct sequence *s, struct sequence *parent)
{
    int rc;
    struct container *cur;
    if(!s->parents) {
        s->parents = container_init();
        s->parents->seq = parent;
        s->parents->count++;
        return;
    }
    cur = s->parents;
    while(cur) {
        rc = bncmp(cur->seq->data->data, parent->data->data,
            cur->seq->data->len, parent->data->len);
        if(rc > 0) {
            if(!cur->right) {
                cur->right = container_init();
                cur->right->seq = parent;
                s->parents->count++;
                return;
            }
            cur = cur->right;
        } else if(rc < 0) {
            if(!cur->left) {
                cur->left = container_init();
                cur->left->seq = parent;
                s->parents->count++;
                return;
            }
            cur = cur->left;
        } else {
            //exists
            return;
        }
    }
    fprintf(stderr, "ERROR: sequence_add_parent() failed due to programmer error.\n");
    exit(1);
}

// TODO: Maybe break up into smaller functions, but might actually end up being
// harder to understand.
void sequence_insert(struct mesh *f, const char *data, size_t data_len, size_t depth)
{
    int rc;
    size_t i = 0;
    struct data *w = data_find(f, data, data_len);
    struct sequence *s = NULL;
    struct sequence *return_seq = NULL;
    struct sequence *prev_seq = NULL;
    struct sequence *parent_seq = NULL;
    struct container *cur = NULL;
    int has_been_linked = 0;
    struct context *ctx = NULL;

    // 1. Allow recalculation of datatree
    // 2. Set/reset mesh first and last items
    // 3. Find or create data word
    // 4. Allocate and insert container at depth
    // 5. Create or lookup sequence and insert if applicable
    // 6. Link prev/next sequences
    // 7. Add parent sequence
    // 8. Set mesh first and last items
    // 9. Add global and sequence contexts linked

    // allow recalulation of datatree stats since a new item is added
    recalculate_datatree_stats_on();

    DEBUG_PRINT(("sequence_insert(): data=%s depth=%lu prev_seq:%s\n",
        data, depth, (prev_seq) ? "yes" : "no"));

    // reset the mesh item pointers on first item in sequence
    if(depth < 1) {
        f->last_item = NULL;
        f->first_item = NULL;
    } else {
        parent_seq = f->first_item;
        prev_seq = f->last_item;
    }

    // make sure the data exists in the datatree
    if(!w) {
        DEBUG_PRINT(("sequence_insert(): create data\n"));
        w = data_insert(f, data, data_len);
    }

    // When a new depth is reached, expand
    if(f->container_len < depth+1) {
        if(f->container_len+1 < depth) {
            fprintf(stderr,
                "ERROR: Programmer error: depth jumped greater than +1 from previous max depth (%lu->%lu).\n",
                f->container_len, depth);
            exit(1);
        }
        DEBUG_PRINT(("sequence_insert(): expand containers: %lu -> %lu\n",
            f->container_len, depth));
        if(f->container_len < 1) { //!f->containers) {
            f->containers = safe_malloc(sizeof(*f->containers)*(depth+1), __LINE__);
            for(i = 0; i <= depth; i++) {
                f->containers[i] = NULL; //container_init();
            }
        } else {
            f->containers = safe_realloc(f->containers,
                sizeof(*f->containers)*(depth+1), __LINE__);
            for(i = f->container_len; i <= depth; i++) {
                f->containers[i] = NULL; //container_init();
            }
        }
        f->container_len = depth+1;
        f->containers[depth] = container_init();
        f->containers[depth]->count++;
    }
    s = sequence_init(w, depth);
    DEBUG_PRINT(("sequence_insert(): sequence_init(): %s %lu\n", s->data->data, depth));
    if(!f->containers[depth]) {
        f->containers[depth] = container_init();
    }
    if(!f->containers[depth]->seq) {
        DEBUG_PRINT(("sequence_insert(): !f->containers[depth]->seq\n"));
        // The 1st item in this container, so simply add the seq
        f->containers[depth]->seq = s;
        return_seq = s;
        f->containers[depth]->count++;
    } else {
        DEBUG_PRINT(("sequence_insert(): f->containers[depth]->seq\n"));
        // Not the first seq in this container at this depth, so insert into
        // the container seq by data. Traverse the binary seq.
        cur = f->containers[depth];
        while(cur) {
            rc = bncmp(cur->seq->data->data, data,
                cur->seq->data->len, data_len);
            if(rc < 0) {
                if(!cur->left) {
                    DEBUG_PRINT(("while(cur): new-left\n"));
                    cur->left = container_init();
                    cur->left->seq = s;
                    return_seq = cur->left->seq;
                    f->containers[depth]->count++;
                    break;
                }
                cur = cur->left;
            } else if (rc > 0) {
                if(!cur->right) {
                    DEBUG_PRINT(("while(cur): new-right\n"));
                    cur->right = container_init();
                    cur->right->seq = s;
                    return_seq = cur->right->seq;
                    f->containers[depth]->count++;
                    break;
                }
                cur = cur->right;
            } else {
                // this data exists at this depth in this container seq
                DEBUG_PRINT(("while(cur): found\n"));
                return_seq = cur->seq;
                break;
            }
        }
    }
    // link prevs and nexts
    if(prev_seq) {
        DEBUG_PRINT(("sequence_insert(): prev_seq\n"));
        // Check if this link already exists in both directions
        for(i = 0; i < prev_seq->next_len; i++) {
            if(prev_seq->nexts[i] == return_seq) {
                //if(bncmp(prev_seq->nexts[i]->data->data, s->data->data,
                //    prev_seq->nexts[i]->data->len, s->data->len) == 0) {
                has_been_linked = 1;
                break;
            }
        }
        if(has_been_linked < 1) {
            DEBUG_PRINT(("sequence_insert(): !found-link\n"));
            if(prev_seq->next_len < 1) {
                prev_seq->nexts = safe_malloc(
                    sizeof(*prev_seq->nexts)*(prev_seq->next_len+1),
                    __LINE__
                );

            } else {
                prev_seq->nexts = safe_realloc(
                    prev_seq->nexts,
                    sizeof(*prev_seq->nexts)*(prev_seq->next_len+1),
                    __LINE__
                );
            }
            prev_seq->nexts[prev_seq->next_len] = return_seq;
            prev_seq->next_len++;
        }
        has_been_linked = 0;
        for(i = 0; i < return_seq->prev_len; i++) {
            if(return_seq->prevs[i] == prev_seq) {
                //if(bncmp(prev_seq->nexts[i]->data->data, s->data->data,
                //    prev_seq->nexts[i]->data->len, s->data->len) == 0) {
                has_been_linked = 1;
                break;
            }
        }
        if(has_been_linked < 1) {
            if(return_seq->prev_len < 1) {
                return_seq->prevs = safe_malloc(
                    sizeof(*return_seq->prevs)*(return_seq->prev_len+1),
                    __LINE__
                );

            } else {
                return_seq->prevs = safe_realloc(
                    return_seq->prevs,
                    sizeof(*return_seq->prevs)*(return_seq->prev_len+1),
                    __LINE__
                );
            }
            return_seq->prevs[return_seq->prev_len] = prev_seq;
            return_seq->prev_len++;
        }
    }
    if(parent_seq) {
        DEBUG_PRINT(("sequence_add_parent(): parent_seq\n"));
        sequence_add_parent(return_seq, parent_seq);
    } else {
        DEBUG_PRINT(("sequence_add_parent(): return_seq\n"));
        sequence_add_parent(return_seq, return_seq);
    }
    // Add this seq reference (data at this depth) to the data itself for
    // reverse lookups from a data.
    data_add_sequence(f, w, return_seq);

    // Set the first seq in the sequence if depth == 0
    if(depth < 1) {
        f->first_item = return_seq;
    }
    // Set the last item inserted in this sequence
    f->last_item = return_seq;

    // start of a new context that can be linked (new file being read and tokenized)
    if(!f->ctxs || depth < 1) {
        ctx = ctx_init();
        if(!f->ctxs) {
            f->ctxs = safe_malloc(sizeof(*f->ctxs), __LINE__);
        } else {
            f->ctxs = safe_realloc(f->ctxs, sizeof(*f->ctxs)*(f->ctxs_len+1), __LINE__);
        }
        f->ctxs[f->ctxs_len] = ctx;
        ctx->seqs = safe_malloc(sizeof(*ctx->seqs), __LINE__);
        ctx->seqs[ctx->seqs_len] = return_seq;
        f->ctxs_len++;
        ctx->seqs_len++;
    } else {
        ctx = f->ctxs[f->ctxs_len-1];
        ctx->seqs = safe_realloc(ctx->seqs, sizeof(*ctx->seqs)*(ctx->seqs_len+1), __LINE__);
        ctx->seqs[ctx->seqs_len] = return_seq;
        ctx->seqs_len++;
    }
    // Now add the reverse lookup to the return_seq sequence
    if(!return_seq->contexts) {
        return_seq->contexts = safe_malloc(sizeof(*return_seq->contexts), __LINE__);
    } else {
        return_seq->contexts = safe_realloc(return_seq->contexts,
            sizeof(*return_seq->contexts)*(return_seq->ctxs_len+1), __LINE__);
    }
    return_seq->contexts[return_seq->ctxs_len] = ctx;
    return_seq->ctxs_len++;
}


