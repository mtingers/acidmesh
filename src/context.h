
#ifndef CONTEXT_H
#define CONTEXT_H 1

struct context {
    size_t popularity;          /* track how many times it's been hit */
    struct tree *tree;          /* pointer to the tree entry point */
};

#endif

