#ifndef CONTAINER_H
#define CONTAINER_H 1

struct container {
    size_t count;
    struct sequence *seq;
    struct container *left;
    struct container *right;
};

struct container *container_init(void);
void dump_container(struct container *c, size_t depth, size_t indent);

#endif
