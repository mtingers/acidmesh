//
// A deque of max size n
// if n is reached, delete first item
// To be used with struct tree:
//    context **ctx_prev
//    context **ctx_next
//

#ifndef CONTEXT_H
#define CONTEXT_H 1

struct ctx_nback_deque {
    size_t n;
    struct tree **trees;
};

struct ctx_nback_deque *ctx_nback_new(size_t n);
void ctx_nback_free(struct ctx_nback_deque *cnd);
void ctx_nback_append(struct ctx_nback_deque *cnd, struct tree *t);
#endif

