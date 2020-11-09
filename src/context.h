#ifndef CONTEXT_H
#define CONTEXT_H 1

struct context {
    size_t trees_len;
    struct tree **trees;
    // not the prev item in this context but previous context sequence
    struct context *prev_ctx;
    struct context *next_ctx;
};

/*
struct contexts {
    size_t cs_len;
    struct context **cs;
};
*/

struct context *ctx_init();
//struct contexts *ctxs_init();

#endif
