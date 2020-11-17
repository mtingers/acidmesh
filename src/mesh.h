#ifndef MESH_H
#define MESH_H 1

struct mesh {
    pthread_mutex_t lock;
    size_t container_len;
    struct container **containers;
    struct datatree *dt;
    struct sequence *first_item;
    struct sequence *last_item;
    size_t ctxs_len;
    struct context **ctxs;
    int ctx_use_prev;
    size_t total_sequence_data_refs;
    double dt_stats_top;
};

void mesh_lock(struct mesh *m);
void mesh_unlock(struct mesh *m);

struct mesh *mesh_init(void);
void link_last_contexts(struct mesh *f);
void link_last_contexts_r(struct mesh *m);

#endif
