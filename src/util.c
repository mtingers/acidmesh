#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *safe_realloc(void *ptr, size_t size, int lineno)
{
    void *p = realloc(ptr, size);
    if(!p) {
        fprintf(stderr, "ERROR: safe_realloc() failed. Line: %d\n", lineno);
        exit(1);
    }
    return p;
}

void *safe_malloc(size_t size, int lineno)
{
    void *p = malloc(size);
    if(!p) {
        fprintf(stderr, "ERROR: safe_malloc() failed. Line: %d\n", lineno);
        exit(1);
    }
    return p;
}

void safe_free(void *p, int lineno)
{
    if(!p) {
        fprintf(stderr, "ERROR: safe_free() programmer error. Line: %d\n", lineno);
        exit(1);
    }
    free(p);
    p = NULL;
}

