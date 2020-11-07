#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "forest.h"
#include "wordbank.h"
#include "container.h"

struct container *container_init()
{
    struct container *c = safe_malloc(sizeof(*c), __LINE__);
    c->tree = NULL;
    c->left = NULL;
    c->right = NULL;
    return c;
}

void dump_container(struct container *c, size_t depth, size_t indent)
{
    size_t i = 0;
    printf("depth:%lu| ", depth);
    for(i = 0; i < indent*2; i++) {
        printf(" ");
    }
    printf("%s\n", c->tree->word->data);
    if(c->left) {
        dump_container(c->left, depth, indent+1);
    }
    if(c->right) {
        dump_container(c->right, depth, indent+1);
    }
}


