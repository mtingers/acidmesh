#ifndef CONTEXT_H
#define CONTEXT_H 1

struct context {
    size_t seqs_len;
    struct sequence **seqs;
    struct context *prev_ctx;
    struct context *next_ctx;
};

struct context *ctx_init(void);

#endif
