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
    w->seqs = NULL;
    w->len = len;
    w->data = safe_malloc(len+1, __LINE__);
    memcpy(w->data, data, len);
    w->data[len] = '\0';
    return w;
}

struct datatree *datatree_init()
{
    struct datatree *wb = safe_malloc(sizeof(*wb), __LINE__);
    wb->count = 1;
    wb->datas = data_init("]", 1);
    return wb;
}

void data_add_sequence(struct data *w, struct sequence *s)
{
    size_t i = 0;
    if(s->depth+1 > w->seqs_len) {
        if(!w->seqs) {
            // no references exist at this depth, so allocate to this depth
            // then NULL out those that don't exist yet
            w->seqs = safe_malloc(sizeof(*w->seqs)*(s->depth+1), __LINE__);
            for(i = 0; i <= s->depth; i++) {
                w->seqs[i] = NULL;
            }
        } else {
            w->seqs = safe_realloc(w->seqs,
                sizeof(*w->seqs)*(s->depth+1), __LINE__);
            for(i = w->seqs_len; i <= s->depth; i++) {
                w->seqs[i] = NULL;
            }
        }
        w->seqs[s->depth] = s;
        w->seqs_len = s->depth+1;
    } else {
        // this depth exists, so we must check if it's NULL and add if so
        if(!w->seqs[s->depth]) {
            w->seqs[s->depth] = s;
        }
    }
}

struct data *data_insert(struct mesh *f, const char *data, size_t len)
{
    return _data_insert(f->wb, data, len);
}

struct data *_data_insert(struct datatree *wb, const char *data, size_t len)
{
    int rc;
    struct data *cur = wb->datas;
    assert(wb->datas != NULL);
    while(cur) {
        rc = bncmp(data, cur->data, len, cur->len);
        if(rc < 0) {
            if(!cur->left) {
                cur->left = data_init(data, len);
                wb->count++;
                return cur->left;
            }
            cur = cur->left;
        } else if(rc > 0) {
            if(!cur->right) {
                cur->right = data_init(data, len);
                wb->count++;
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
    struct data *cur = f->wb->datas;
    assert(f->wb->datas);
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

