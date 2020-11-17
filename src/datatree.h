#ifndef WORDBANK_H
#define WORDBANK_H 1

#include "mesh.h"


// seqs[depth][index]

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

struct datatree_stat {
    struct data *data_ptr;
    size_t count;
    double percent;
};

void recalculate_datatree_stats_on(void);
void recalculate_datatree_stats_off(void);
int recalculate_datatree_stats_get(void);
struct data *data_init(const char *data, size_t len);
struct datatree *datatree_init(void);
struct data *data_insert(struct mesh *m, const char *data, size_t len);
struct data *data_insert_r(struct mesh *m, const char *data, size_t len);
struct data *_data_insert(struct datatree *wb, const char *data, size_t len);
struct data *data_find(struct mesh *m, const char *data, size_t len);
struct data *data_find_r(struct mesh *m, const char *data, size_t len);
void data_add_sequence(struct mesh *m, struct data *w, struct sequence *s);
void data_add_sequence_r(struct mesh *m, struct data *w, struct sequence *s);
void dump_datas(struct data *w, size_t indent);
void dump_datas_r(struct mesh *m, struct data *w, size_t indent);
void datatree_stats(struct mesh *m, int print_top_bottom);
void datatree_stats_r(struct mesh *m, int print_top_bottom);
int datatree_sort_cmp(const void *p1, const void *p2);

#endif
