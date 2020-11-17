#ifndef SEQUENCE_H
#define SEQUENCE_H 1

#include "mesh.h"

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

void sequence_dump(struct mesh *f);
void sequence_dump_r(struct mesh *f);
void sequence_insert(struct mesh *f, const char *data, size_t data_len, size_t depth);
void sequence_insert_r(struct mesh *m, const char *data, size_t data_len, size_t depth);
struct sequence *sequence_init(struct data *w, size_t depth);
void sequence_add_parent(struct sequence *t, struct sequence *parent);
void sequence_add_parent_r(struct mesh *m, struct sequence *s, struct sequence *parent);
#endif
