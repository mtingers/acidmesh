
#ifndef FOREST_H
#define FOREST_H 1


struct tree {
    size_t index;               /* indicates the depth of this node */
    size_t root_index;          /* indicates the depth of this node */
    size_t popularity;          /* track how many times it's been hit on insert */
    size_t ctx_next_len;        /* track the size of ctx next */
    size_t ctx_prev_len;        /* track the size of ctx prev */
    size_t prev_len;            /* size of prev  */
    size_t next_len;            /* size of next  */
    struct wb_word *word;       /* pointer to "struct word" */
    struct tree **prev;         /* nodes seen before this word */
    struct tree **next;         /* nodes seen after this word */
    struct context **ctx_prev;  /* tree series seen previously */
    struct context **ctx_next;  /* tree series seen after this series */
};

struct forest {
    size_t count;
    struct tree **trees;
};

struct forest *forest_new();
struct tree *tree_new(size_t index, size_t root_index);
void tree_insert_prev(struct tree *cur, struct tree *prev);
void tree_insert_next(struct tree *cur, struct tree *prev);
void forest_expand(struct forest *f, struct tree *tree_series);
/*struct tree *forest_find_depth_else_new(struct word_bank *wb,
        struct forest *f, const char *word, size_t word_len, size_t depth,
        struct tree *prev, size_t root_index);
*/
void forest_dump(struct forest *f);

#endif

