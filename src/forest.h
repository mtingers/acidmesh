#ifndef FOREST_H
#define FOREST_H 1

struct tree {
    size_t depth;
    struct container *parents;
    struct word *word;
    size_t prev_len;
    size_t next_len;
    struct tree **nexts;
    struct tree **prevs;
};

struct forest {
    size_t container_len;
    struct container **containers;
    struct wordbank *wb;
    struct tree *first_item;
    struct tree *last_item;
};

struct forest *forest_init();
void dump_tree(struct forest *f);
//struct tree *tree_insert(struct forest *f, const char *data, size_t data_len, size_t depth, struct tree *prev_tree, struct tree *parent_tree);
void tree_insert(struct forest *f, const char *data, size_t data_len, size_t depth);
struct tree *tree_init(struct word *w, size_t depth);
void tree_add_parent(struct tree *t, struct tree *parent);

#endif
