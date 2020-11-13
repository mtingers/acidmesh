#ifndef WORDBANK_H
#define WORDBANK_H 1

#include "mesh.h"


struct data {
    char *data;
    struct data *left;
    struct data *right;
    size_t seqs_len;
    size_t total_seqs_refs;
    size_t len;
    size_t *sub_seqs_len;
    struct sequence ***seqs; // a list of seqs with depth as first key
    size_t stats_count;
    double stats_percent;
};

struct datatree {
    size_t count;
    struct data *datas;
};

struct data *data_init(const char *data, size_t len);
struct datatree *datatree_init(void);
struct data *data_insert(struct mesh *f, const char *data, size_t len);
struct data *_data_insert(struct datatree *wb, const char *data, size_t len);
struct data *data_find(struct mesh *f, const char *data, size_t len);
void data_add_sequence(struct mesh *m, struct data *w, struct sequence *s);
void dump_datas(struct data *w, size_t indent);

#endif
