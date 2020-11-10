#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "mesh.h"
#include "datatree.h"
#include "container.h"

struct container *container_init(void)
{
    struct container *c = safe_malloc(sizeof(*c), __LINE__);
    c->count = 0;
    c->seq = NULL;
    c->left = NULL;
    c->right = NULL;
    return c;
}

void dump_container(struct container *c, size_t depth, size_t indent)
{
    size_t i = 0;
    assert(c);
    assert(c->seq);
    assert(c->seq->data);
    assert(c->seq->data->data);
    printf("depth:%lu| ", depth);
    for(i = 0; i < indent*2; i++) {
        printf(" ");
    }
    printf("%s\n", c->seq->data->data);
    if(c->left) {
        dump_container(c->left, depth, indent+1);
    }
    if(c->right) {
        dump_container(c->right, depth, indent+1);
    }
}


