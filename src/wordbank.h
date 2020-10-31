
#ifndef WORD_BANK_H
#define WORK_BANK_H 1


struct wb_word {
    size_t len;             /* the length of the word (word->data) */
    size_t trees_len;       /* the number of trees in the list */
    char *data;             /* the actual word stored */
    struct tree **trees;    /* points to all trees referencing this word */
    struct wb_word *left;   /* binary tree left/right paths for auto-sorting */
    struct wb_word *right;
};

struct word_bank {
    size_t count;           /* the number of words tracked */
    struct wb_word *words;     /* entry point into all the words */
};

struct wb_word *word_new(const char *word, size_t word_len);
struct word_bank *wb_new();
void wb_free(struct word_bank *wb);
void wb_free_branch(struct wb_word *w);
struct wb_word *wb_find(struct word_bank *wb, const char *word, size_t word_len);
struct tree *wb_insert_tree(struct wb_word *w, struct tree *t, const char *word, size_t word_len);
struct wb_word *wb_insert(struct word_bank *wb, const char *word, size_t word_len);
void wb_dump_tree(struct wb_word *w, int depth);
void wb_test();

#endif

