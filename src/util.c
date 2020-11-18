#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "util.h"

double dround(double val, int dp)
{
    int charsNeeded = 1 + snprintf(NULL, 0, "%.*f", dp, val);
    char *buffer = malloc(charsNeeded);
    snprintf(buffer, charsNeeded, "%.*f", dp, val);
    double result = atof(buffer);
    free(buffer);
    return result;
}

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

double bncmp_ratio(const char *s1, const char *s2, size_t s1_n, size_t s2_n)
{
    double result = s1_n - s2_n;
    size_t i = 0;
    size_t min_n = (s1_n > s2_n) ? s2_n : s1_n;

    for(; i < min_n; i++) {
        result += s1[i] - s2[i];
    }
    return result;
}

int bncmp(const char *s1, const char *s2, size_t s1_n, size_t s2_n)
{
    size_t i = 0;
    size_t min_n = (s1_n > s2_n) ? s2_n : s1_n;
    for(; i < min_n; i++) {
        if(s1[i] > s2[i]) {
            return 1;
        } else if(s1[i] < s2[i]) {
            return -1;
        }
    }
    if(s2_n > s1_n) {
        return 1;
    } else if(s2_n < s1_n) {
        return -1;
    }
    return 0;
}


Tree *tree_init(void)
{
    Tree *t = safe_malloc(sizeof(*t), __LINE__);
    t->p = NULL;
    t->left = NULL;
    t->right = NULL;
    return t;
}

Tree *tree_find(Tree *t, void *p, size_t plen, int (*compar)(void *, void *, size_t, size_t))
{
    Tree *cur = t;
    int rc = 0;
    while(cur) {
        rc = compar(cur->p, p, cur->len, plen);
        if(rc < 0) {
            if(!cur->left) {
                return NULL;
            }
            cur = cur->left;
        } else if(rc > 0) {
            if(!cur->right) {
                return NULL;
            }
            cur = cur->right;
        } else {
            // Found;
            return cur;
        }
    }
    // Not found
    return NULL;
}

Tree *tree_insert_copy(Tree *t, void *p, size_t plen, int (*compar)(void *, void *, size_t, size_t))
{
    // NOTE: This version malloc's and copies p based off of plen
    Tree *cur = t;
    int rc = 0;
    while(cur) {
        rc = compar(cur->p, p, cur->len, plen);
        if(rc < 0) {
            if(!cur->left) {
                cur->left = tree_init();
                cur->left->len = plen;
                cur->left->p = safe_malloc(plen, __LINE__);
                memcpy(cur->left->p, p, plen);
                return cur->left;
            }
            cur = cur->left;
        } else if(rc > 0) {
            if(!cur->right) {
                cur->right = tree_init();
                cur->right->len = plen;
                cur->right->p = safe_malloc(plen, __LINE__);
                memcpy(cur->right->p, p, plen);
                return cur->right;
            }
            cur = cur->right;
        } else {
            // Found;
            return cur;
        }
    }
    fprintf(stderr, "ERROR: Programmer error in tree_insert()\n");
    exit(1);
}

Tree *tree_insert(Tree *t, void *p, size_t plen, int (*compar)(void *, void *, size_t, size_t))
{
    // NOTE: This version is only a pointer to p and plen is only added as len
    // reference.
    Tree *cur = t;
    int rc = 0;
    while(cur) {
        rc = compar(cur->p, p, cur->len, plen);
        if(rc < 0) {
            if(!cur->left) {
                cur->left = tree_init();
                cur->left->len = plen;
                cur->left->p = p; //safe_malloc(plen, __LINE__);
                //memcpy(cur->left->p, p, plen);
                return cur->left;
            }
            cur = cur->left;
        } else if(rc > 0) {
            if(!cur->right) {
                cur->right = tree_init();
                cur->right->len = plen;
                cur->right->p = p; //safe_malloc(plen, __LINE__);
                //memcpy(cur->right->p, p, plen);
                return cur->right;
            }
            cur = cur->right;
        } else {
            // Found;
            return cur;
        }
    }
    fprintf(stderr, "ERROR: Programmer error in tree_insert()\n");
    exit(1);
}

void tree_free_copied(Tree *t)
{
    if(t->left) {
        tree_free(t->left);
    }
    if(t->right) {
        tree_free(t->right);
    }
    safe_free(t->p, __LINE__);
    safe_free(t, __LINE__);
}

void tree_free(Tree *t)
{
    if(t->left) {
        tree_free(t->left);
    }
    if(t->right) {
        tree_free(t->right);
    }
    safe_free(t, __LINE__);
}

