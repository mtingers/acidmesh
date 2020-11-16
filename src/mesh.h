#ifndef MESH_H
#define MESH_H 1

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
void link_last_contexts(struct mesh *f);

#endif
