#ifndef FOREST_H
#define FOREST_H 1

struct sequence {
    size_t depth;
    struct container *parents;
    struct data *data;
    size_t prev_len;
    size_t next_len;
    struct sequence **nexts;
    struct sequence **prevs;
    size_t ctxs_len;
    struct context **contexts;
};

struct mesh {
    size_t container_len;
    struct container **containers;
    struct datatree *dt;
    struct sequence *first_item;
    struct sequence *last_item;
    size_t ctxs_len;
    struct context **ctxs;
    int ctx_use_prev;
    size_t total_sequence_data_refs;
};

struct mesh *mesh_init(void);
void dump_sequence(struct mesh *f);
void sequence_insert(struct mesh *f, const char *data, size_t data_len, size_t depth);
struct sequence *sequence_init(struct data *w, size_t depth);
void sequence_add_parent(struct sequence *t, struct sequence *parent);
void link_last_contexts(struct mesh *f);
void datatree_stats(struct mesh *m);
#endif
