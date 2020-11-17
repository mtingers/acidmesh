#ifndef CONTEXT_H
#define CONTEXT_H 1

struct context {
    size_t seqs_len;
    size_t context_id;
    struct sequence **seqs;
    struct context *prev_ctx;
    struct context *next_ctx;
    // TODO: Might need to add a lookup tree by data to serve the purpose of
    // seeing if a data exists in this context. Otherwise, it's a loop through
    // each and slow, O(n)
    // -OR- add it to the sequence as a list for accessing it in reverse
};

struct context *ctx_init(void);
struct context *ctx_init_r(struct mesh *m);

#endif
