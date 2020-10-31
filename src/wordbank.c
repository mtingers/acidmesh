#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "wordbank.h"
#include "forest.h"

struct wb_word *word_new(const char *word, size_t word_len)
{
    struct wb_word *w = safe_malloc(sizeof(*w), __LINE__);
    w->len = word_len;
    w->trees_len = 0;
    w->data = safe_malloc(word_len+1, __LINE__);
    w->trees = NULL;
    w->left = NULL;
    w->right = NULL;
    memcpy(w->data, word, word_len);
    w->data[word_len] = '\0';
    return w;
}

struct word_bank *wb_new()
{
    struct word_bank *wb = safe_malloc(sizeof(*wb), __LINE__);
    wb->count = 0;
    wb->words = NULL;
    return wb;
}

void wb_free_branch(struct wb_word *w)
{
    struct wb_word *cur;
    if(!w) {
        fprintf(stderr, "ERROR: wb_free_branch(): programmer error\n");
        return;
    }
    cur = w;
    if(cur->left) {
        wb_free_branch(cur->left);
    }
    if(cur->right) {
        wb_free_branch(cur->right);
    }
    safe_free(cur->data, __LINE__);
    if(cur->trees)
        safe_free(cur->trees, __LINE__);
    safe_free(cur, __LINE__);
    w = NULL;
}

// Free all words and then itself
// Keep forest intact
void wb_free(struct word_bank *wb)
{
    assert(wb);
    if(wb->words) {
        wb_free_branch(wb->words);
    }
    wb->words = NULL;
    safe_free(wb, __LINE__);
}

struct tree *wb_insert_tree(struct wb_word *w, struct tree *t, const char *word, size_t word_len)
{
    size_t i = 0;
    assert(t);
    // resize trees and append new tree
    if(!w->trees) {
        w->trees = safe_malloc(sizeof(*w->trees), __LINE__);
        w->trees_len = 1;
        w->trees[0] = t;
        w->trees[0]->word = w;
        return w->trees[0];
    // trees has items, so append to it
    } else {
        // need to lookup this word to see if it already exists in the trees
        // if not, then add it
        for(i = 0; i < w->trees_len; i++) {
            if(strcmp(w->trees[i]->word->data, word) == 0 && w->trees[i]->index == t->index && w->trees[i]->root_index == t->root_index) {
                return w->trees[i];
            }
        }
        w->trees = safe_realloc(w->trees, sizeof(*w->trees)*(w->trees_len+1), __LINE__);
        w->trees[w->trees_len] = t;
        w->trees[w->trees_len]->word = w;
        w->trees_len++;
        return w->trees[w->trees_len-1];
    }
    return NULL;
}

struct wb_word *wb_insert(struct word_bank *wb, const char *word, size_t word_len)
{
    struct wb_word *cur = NULL;
    int rc = 0;
    assert(wb);
    assert(word_len > 0);
    cur = wb->words;
    // find the next open slot
    if(!cur) {
        // this means it is the first word being inserted
        wb->words = word_new(word, word_len);
        wb->count++;
        return wb->words;
    }
    // not first item, so we must traverse the tree to find match or next
    // location to create one
    while(cur) {
        rc = strcmp(cur->data, word);
        if(rc == 0) {
            // this is a match, just return;
            return cur;
        }
        if(rc > 0) {
            if(!cur->right) {
                cur->right = word_new(word, word_len);
                wb->count++;
                return cur->right;
            }
            cur = cur->right;
        } else {
            if(!cur->left) {
                cur->left = word_new(word, word_len);
                wb->count++;
                return cur->left;
            }
            cur = cur->left;
        }
    }
    fprintf(stderr, "ERROR: wb_insert() programmer error.  Exiting.\n");
    exit(1);
}

struct wb_word *wb_find(struct word_bank *wb, const char *word, size_t word_len)
{
    struct wb_word *cur = NULL;
    int rc = 0;
    assert(wb);
    assert(word_len > 0);
    cur = wb->words;
    if(!cur) {
        return NULL;
    }
    while(cur) {
        rc = strcmp(cur->data, word);
        if(rc == 0) {
            return cur;
        }
        if(rc > 0) {
            cur = cur->right;
        } else {
            cur = cur->left;
        }
    }
    return NULL;
}

void wb_dump_tree(struct wb_word *w, int depth)
{
    int i = 0;
    for(; i < depth*2; i++) {
        printf(" ");
    }
    if(w->len > 0) {
        printf("%s\n", w->data);
    }
    if(w->right) {
        wb_dump_tree(w->right, depth+1);
    }
    if(w->left) {
        wb_dump_tree(w->left, depth+1);
    }
}

void wb_test()
{
    struct word_bank *wb = wb_new();
    size_t i = 0;
    char *words1[] = {
        "Hi,", "this", "is", "really", "the", "end", "of", "the", "world!"
    };
    char *words2[] = {
        "Kidney", "transplantation", "or", "renal", "transplantation", ""
        "is", "the", "organ", "transplant", "of", "a", "kidney", "into", "a", ""
        "patient", "with", "end-stage", "kidney", "disease.",
    };

    wb_insert(wb, "]", 1);
    for(i = 0; i < sizeof(words1)/sizeof(*words1); i++) {
        printf("i: %lu %s\n", i, words1[i]);
        wb_insert(wb, words1[i], strlen(words1[i]));
    }
    for(i = 0; i < sizeof(words2)/sizeof(*words2); i++) {
        printf("i: %lu %s\n", i, words2[i]);
        wb_insert(wb, words2[i], strlen(words2[i]));
    }
    wb_dump_tree(wb->words, 1);
    wb_free(wb);
}

#if TEST_WORDBANK
int main(void)
{
    wb_test();
    return 0;
}
#endif

