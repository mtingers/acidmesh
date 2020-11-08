#ifndef WORDBANK_H
#define WORDBANK_H 1

#include "forest.h"

struct word {
    char *data;
    struct word *left;
    struct word *right;
    size_t trees_len;
    size_t len;
    struct tree **trees; // a list of trees with depth as first key
};

struct wordbank {
    size_t count;
    struct word *words;
};

struct word *word_init(const char *data, size_t len);
struct wordbank *wordbank_init();
struct word *word_insert(struct forest *f, const char *data, size_t len);
struct word *_word_insert(struct wordbank *wb, const char *data, size_t len);
struct word *word_find(struct forest *f, const char *data, size_t len);
void word_add_tree(struct word *w, struct tree *t);
void dump_words(struct word *w, size_t indent);

#endif
