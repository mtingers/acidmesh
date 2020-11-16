#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "sequence.h"
#include "datatree.h"
#include "mesh.h"

static int g_recalculate_datatree_stats = 1;

void recalculate_datatree_stats_on(void)
{
    g_recalculate_datatree_stats = 1;
}

void recalculate_datatree_stats_off(void)
{
    g_recalculate_datatree_stats = 1;
}

int recalculate_datatree_stats_get(void)
{
    return g_recalculate_datatree_stats;
}

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

void data_add_sequence(struct mesh *m, struct data *w, struct sequence *s)
{
    size_t i = 0;
    m->total_sequence_data_refs++;
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

void _datatree_stats(struct data *cur, struct datatree_stat **stats, size_t *stats_n)
{
    size_t n = *stats_n;
    size_t i = 0;
    assert(cur);
    *stats_n = n+1;
    stats[n] = safe_malloc(sizeof(**stats), __LINE__);
    stats[n]->data_ptr = cur;
    stats[n]->count = 0;
    for(i = 0; i < cur->seqs_len; i++) {
        stats[n]->count += cur->sub_seqs_len[i];
    }
    if(cur->left) {
        _datatree_stats(cur->left, stats, stats_n);
    }
    if(cur->right) {
        _datatree_stats(cur->right, stats, stats_n);
    }
}

int datatree_sort_cmp(const void *p1, const void *p2)
{
    struct datatree_stat *d1 = *(struct datatree_stat **)p1;
    struct datatree_stat *d2 = *(struct datatree_stat **)p2;
    if(d1->count < d2->count) {
        return -1;
    } else if(d1->count > d2->count) {
        return 1;
    }
    return 0;
}

void datatree_stats(struct mesh *m, int print_top_bottom)
{
    struct data *cur = m->dt->datas;
    struct datatree_stat **stats = safe_malloc(sizeof(*stats)*(m->dt->count), __LINE__);
    size_t stats_n = 0;
    size_t i = 0;
    if(g_recalculate_datatree_stats) {
        _datatree_stats(cur, stats, &stats_n);
        qsort(stats, stats_n, sizeof(*stats), datatree_sort_cmp);
    }
    g_recalculate_datatree_stats = 0;
    for(i = 0; i < stats_n; i++) {
        stats[i]->percent = (double)stats[i]->count/(double)m->total_sequence_data_refs * 100.0;
        stats[i]->data_ptr->stats_percent = stats[i]->percent;
        stats[i]->data_ptr->stats_count = stats[i]->count;
    }
    if(print_top_bottom > 0) {
        printf("---- top ----\n");
        for(i = stats_n-1; i > stats_n-125; i--) {
            printf("> %s -> %lu   [%.6f%%]\n", stats[i]->data_ptr->data, stats[i]->count, stats[i]->percent);
        }
        printf("---- bottom ----\n");
        for(i = 0; i < 25; i++) {
            printf("> %s -> %lu   [%.6f%%]\n", stats[i]->data_ptr->data, stats[i]->count, stats[i]->percent);
        }
    }
}

