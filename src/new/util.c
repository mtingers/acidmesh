#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

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

void newline2space(char *s, size_t s_len)
{
    size_t i = 0;
    for(; i < s_len; i++) {
        if(s[i] == '\n') {
            s[i] = ' ';
        }
    }
}

size_t file_read(struct file *f, size_t size, void *ptr)
{
    if(fread(ptr, size, 1, f->fd) != 1) {
        perror("fread");
        if(f->exit_on_error > 0) {
            f->close(f);
            exit(1);
        }
    }
    return 1;
}

void file_close(struct file *f)
{
    if(f->fd) {
        fclose(f->fd);
    }
}

struct file *file_open(const char *path, const char *mode, int exit_on_error)
{
    struct file *f = safe_malloc(sizeof(*f), __LINE__);
    f->exit_on_error = exit_on_error;
    f->fd = fopen(path, mode);
    if(!f->fd) {
        fprintf(stderr, "ERROR: Failed to open file: '%s'\n", path);
        free(f);
        if(exit_on_error > 0)
            exit(1);
        return NULL;
    }
    fseek(f->fd, 0, SEEK_END);
    f->size = ftell(f->fd);
    rewind(f->fd);
    f->read = &file_read;
    f->close = &file_close;
    return f;
}


