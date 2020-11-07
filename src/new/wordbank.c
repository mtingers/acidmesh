#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "wordbank.h"
#include "forest.h"

struct word *word_init(const char *data)
{
    struct word *w = safe_malloc(sizeof(*w), __LINE__);
    w->left = NULL;
    w->right = NULL;
    w->trees_len = 0;
    w->trees = NULL;
    w->data = safe_malloc(strlen(data)+1, __LINE__);
    memcpy(w->data, data, strlen(data));
    w->data[strlen(data)] = '\0';
    return w;

}

struct wordbank *wordbank_init()
{
    struct wordbank *wb = safe_malloc(sizeof(*wb), __LINE__);
    wb->count = 1;
    wb->words = word_init("]");
    return wb;
}

struct tree_list {
    struct tree *t;
    struct tree_list *next;
};

void word_add_tree(struct word *w, struct tree *t)
{
    size_t i = 0;
    struct tree_list *tl = NULL;
    if(t->depth+1 > w->trees_len) {
        if(!w->trees) {
            w->trees = safe_malloc(sizeof(*w->trees)*(t->depth+1));
            for(i = 0; i < t->depth; i++) {
                w->trees[i] = NULL;
            }
        }
        tl = safe_malloc(sizeof(*tl));
        tl->t = t;
        tl->next = NULL;
        w->trees[t->depth] = t;
    }
}

struct word *word_insert(struct forest *f, const char *data)
{
    return _word_insert(f->wb, data);
}

struct word *_word_insert(struct wordbank *wb, const char *data)
{
    int rc;
    struct word *cur = wb->words;
    assert(wb->words != NULL);
    while(cur) {
        rc = strcmp(data, cur->data);
        if(rc < 0) {
            if(!cur->left) {
                cur->left = word_init(data);
                wb->count++;
                return cur->left;
            }
            cur = cur->left;
        } else if(rc > 0) {
            if(!cur->right) {
                cur->right = word_init(data);
                wb->count++;
                return cur->right;
            }
            cur = cur->right;
        } else {
            // exists, skip
            return cur;
        }
    }
    fprintf(stderr, "ERROR: word_insert() failed due to programmer error.\n");
    exit(1);
}

struct word *word_find(struct forest *f, const char *data)
{
    int rc;
    struct word *cur = f->wb->words;
    assert(f->wb->words);
    while(cur) {
        rc = strcmp(data, cur->data);
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

