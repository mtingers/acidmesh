#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "datatree.h"
#include "mesh.h"

struct data *data_init(const char *data, size_t len)
{
    struct data *w = safe_malloc(sizeof(*w), __LINE__);
    w->left = NULL;
    w->right = NULL;
    w->seqs_len = 0;
    w->sub_seqs_len = 0;
    w->seqs = NULL;
    w->len = len;
    w->data = safe_malloc(len+1, __LINE__);
    memcpy(w->data, data, len);
    w->data[len] = '\0';
    w->stats_count = 0;
    w->stats_percent = 0.0;
    return w;
}

struct datatree *datatree_init()
{
    struct datatree *dt = safe_malloc(sizeof(*dt), __LINE__);
    dt->count = 1;
    // seed the tree with a middle value
    dt->datas = data_init("]", 1);
    return dt;
}

// This is wrong because it overwrites by depth
// should have been struct sequence *** not **
//void data_add_sequence_old(struct data *w, struct sequence *s)
//{
//    size_t i = 0;
//    if(s->depth+1 > w->seqs_len) {
//        if(!w->seqs) {
//            // no references exist at this depth, so allocate to this depth
//            // then NULL out those that don't exist yet
//            w->seqs = safe_malloc(sizeof(*w->seqs)*(s->depth+1), __LINE__);
//            for(i = 0; i <= s->depth; i++) {
//                w->seqs[i] = NULL;
//            }
//        } else {
//            w->seqs = safe_realloc(w->seqs,
//                sizeof(*w->seqs)*(s->depth+1), __LINE__);
//            for(i = w->seqs_len; i <= s->depth; i++) {
//                w->seqs[i] = NULL;
//            }
//        }
//        w->seqs[s->depth] = s;
//        w->seqs_len = s->depth+1;
//    } else {
//        // this depth exists, so we must check if it's NULL and add if so
//        if(!w->seqs[s->depth]) {
//            w->seqs[s->depth] = s;
//        }
//    }
//}

void data_add_sequence(struct mesh *m, struct data *w, struct sequence *s)
{
    size_t i = 0;
    m->total_sequence_data_refs++;
    // ahhhhhhh!
    if(s->depth+1 > w->seqs_len) {
        if(!w->seqs) {
            // no references exist at this depth, so allocate to this depth
            // then NULL out those that don't exist yet
            w->seqs = safe_malloc(sizeof(*w->seqs)*(s->depth+1), __LINE__);
            w->sub_seqs_len = safe_malloc(sizeof(*w->sub_seqs_len)*(s->depth+1), __LINE__);
            for(i = 0; i <= s->depth; i++) {
                w->seqs[i] = NULL;
                w->sub_seqs_len[i] = 0;
            }

        } else {
            w->seqs = safe_realloc(w->seqs,
                sizeof(*w->seqs)*(s->depth+1), __LINE__);
            w->sub_seqs_len = safe_realloc(w->sub_seqs_len,
                    sizeof(*w->sub_seqs_len)*(s->depth+1), __LINE__);
            for(i = w->seqs_len; i <= s->depth; i++) {
                w->seqs[i] = NULL;
                w->sub_seqs_len[i] = 0;
            }
        }
        //w->seqs[s->depth] = s;
        // Does not exist so we can simply expand and add another item
        w->seqs[s->depth] = safe_malloc(sizeof(*w->seqs[s->depth]), __LINE__);
        w->seqs[s->depth][0] = s;
        w->sub_seqs_len[s->depth] = 1;
        w->seqs_len = s->depth+1;
    } else {
        // this depth exists, so we must check if it's NULL and add if so
        if(!w->seqs[s->depth]) {
            w->seqs[s->depth] = safe_malloc(sizeof(*w->seqs[s->depth]), __LINE__);
            w->seqs[s->depth][0] = s;
        } else {
            w->seqs[s->depth] = safe_realloc(w->seqs[s->depth], sizeof(*w->seqs[s->depth])*(w->sub_seqs_len[s->depth]+1), __LINE__);
            w->seqs[s->depth][w->sub_seqs_len[s->depth]] = s;
        }
        w->sub_seqs_len[s->depth]++;
    }
}

struct data *data_insert(struct mesh *f, const char *data, size_t len)
{
    return _data_insert(f->dt, data, len);
}

struct data *_data_insert(struct datatree *dt, const char *data, size_t len)
{
    int rc;
    struct data *cur = dt->datas;
    assert(dt->datas != NULL);
    while(cur) {
        rc = bncmp(data, cur->data, len, cur->len);
        if(rc < 0) {
            if(!cur->left) {
                cur->left = data_init(data, len);
                dt->count++;
                return cur->left;
            }
            cur = cur->left;
        } else if(rc > 0) {
            if(!cur->right) {
                cur->right = data_init(data, len);
                dt->count++;
                return cur->right;
            }
            cur = cur->right;
        } else {
            // exists, skip inserting new
            return cur;
        }
    }
    fprintf(stderr, "ERROR: data_insert() failed due to programmer error.\n");
    exit(1);
}

struct data *data_find(struct mesh *f, const char *data, size_t len)
{
    int rc;
    struct data *cur = f->dt->datas;
    assert(f->dt->datas);
    while(cur) {
        rc = bncmp(data, cur->data, len, cur->len);
        if(rc < 0) {
            cur = cur->left;
        } else if(rc > 0) {
            cur = cur->right;
        } else {
            return cur;
        }
    }
    return NULL;
}

void dump_datas(struct data *w, size_t indent)
{
    size_t i = 0;
    for(i = 0; i < indent*2; i++) {
        printf(" ");
    }
    printf("%s\n", w->data);
    if(w->left) {
        dump_datas(w->left, indent+1);
    }
    if(w->right) {
        dump_datas(w->right, indent+1);
    }
}

