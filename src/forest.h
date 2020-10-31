
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

#endif

