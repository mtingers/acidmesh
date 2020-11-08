#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "wordbank.h"
#include "forest.h"

struct word *word_init(const char *data, size_t len)
{
    struct word *w = safe_malloc(sizeof(*w), __LINE__);
    w->left = NULL;
    w->right = NULL;
    w->trees_len = 0;
    w->trees = NULL;
    w->len = len;
    w->data = safe_malloc(strlen(data)+1, __LINE__);
    memcpy(w->data, data, len);
    w->data[len] = '\0';
    return w;

}

struct wordbank *wordbank_init()
{
    struct wordbank *wb = safe_malloc(sizeof(*wb), __LINE__);
    wb->count = 1;
    wb->words = word_init("]", 1);
    return wb;
}

void word_add_tree(struct word *w, struct tree *t)
{
    size_t i = 0;
    if(t->depth+1 > w->trees_len) {
        if(!w->trees) {
            // no references exist at this depth, so allocate to this depth
            // then NULL out those that don't exist yet
            w->trees = safe_malloc(sizeof(*w->trees)*(t->depth+1), __LINE__);
            for(i = 0; i < t->depth; i++) {
                w->trees[i] = NULL;
            }
        } else {
            w->trees = safe_realloc(w->trees, sizeof(*w->trees)*(t->depth+1), __LINE__);
        }
        w->trees[t->depth] = t;
        w->trees_len = t->depth+1;
    } else {
        // this depth exists, so we must check if it's NULL and add if so
        if(!w->trees[t->depth]) {
            w->trees[t->depth] = t;
        }
    }
}

struct word *word_insert(struct forest *f, const char *data, size_t len)
{
    return _word_insert(f->wb, data, len);
}

struct word *_word_insert(struct wordbank *wb, const char *data, size_t len)
{
    int rc;
    struct word *cur = wb->words;
    assert(wb->words != NULL);
    while(cur) {
        rc = bncmp(data, cur->data, len, cur->len);
        if(rc < 0) {
            if(!cur->left) {
                cur->left = word_init(data, len);
                wb->count++;
                return cur->left;
            }
            cur = cur->left;
        } else if(rc > 0) {
            if(!cur->right) {
                cur->right = word_init(data, len);
                wb->count++;
                return cur->right;
            }
            cur = cur->right;
        } else {
            // exists, skip inserting new
            return cur;
        }
    }
    fprintf(stderr, "ERROR: word_insert() failed due to programmer error.\n");
    exit(1);
}

struct word *word_find(struct forest *f, const char *data, size_t len)
{
    int rc;
    struct word *cur = f->wb->words;
    assert(f->wb->words);
    while(cur) {
        rc = bncmp(data, cur->data, len, cur->len);
        if(rc < 0) {
            cur = cur->left;
        } else if(rc > 0) {
            cur = cur->right;
        } else {
            return cur;
        }
    }
    return NULL;
}

void dump_words(struct word *w, size_t indent)
{
    size_t i = 0;
    for(i = 0; i < indent*2; i++) {
        printf(" ");
    }
    printf("%s\n", w->data);
    if(w->left) {
        dump_words(w->left, indent+1);
    }
    if(w->right) {
        dump_words(w->right, indent+1);
    }
}

