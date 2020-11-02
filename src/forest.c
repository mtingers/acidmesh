#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#if TEST_FOREST
#include <dirent.h>
#endif
#include "forest.h"
#include "wordbank.h"
#include "util.h"


struct forest *forest_new()
{
    struct forest *f = safe_malloc(sizeof(*f), __LINE__);
    f->count = 0;
    f->trees = NULL;
    return f;
}

struct tree *tree_new(size_t index, size_t root_index)
{
    struct tree *t = safe_malloc(sizeof(*t), __LINE__);
    t->index = index;
    t->root_index = root_index;
    t->popularity = 1;
    t->ctx_next_len = 0;
    t->ctx_prev_len = 0;
    t->prev_len = 0;
    t->next_len = 0;
    t->word = NULL;
    t->prev = NULL;
    t->next = NULL;
    t->ctx_prev = NULL;
    t->ctx_next = NULL;
    return t;
}

void tree_insert_prev(struct tree *cur, struct tree *prev)
{
    size_t i = 0;
    assert(cur && cur->word);
    assert(prev && prev->word);
    if(!cur->prev) {
        cur->prev = safe_malloc(sizeof(*cur->prev), __LINE__);
    } else {
        // need to check if this relationship already exists to avoid dups
        for(i = 0; i < cur->prev_len; i++) {
            assert(cur->prev[i]);
            assert(cur->prev[i]->word);
            assert(cur->prev[i]->word->data);
            if(strcmp(cur->prev[i]->word->data, prev->word->data) == 0) {
                return;
            }
        }
        cur->prev = safe_realloc(cur->prev, sizeof(*cur->prev)*(cur->prev_len+1), __LINE__);
    }
    cur->prev[cur->prev_len] = prev;
    cur->prev_len++;
}
void tree_insert_next(struct tree *cur, struct tree *prev)
{
    size_t i = 0;
    assert(cur && cur->word);
    assert(prev && prev->word);
    if(!prev->next) {
        prev->next = safe_malloc(sizeof(*prev->next), __LINE__);
    } else {
        // need to check if this relationship already exists to avoid dups
        for(i = 0; i < prev->next_len; i++) {
            assert(prev->next[i]);
            assert(prev->next[i]->word);
            assert(prev->next[i]->word->data);
            if(strcmp(prev->next[i]->word->data, cur->word->data) == 0) {
                //prev->next[i] = cur;
                return;
            }
        }
        prev->next = safe_realloc(prev->next, sizeof(*prev->next)*(prev->next_len+1), __LINE__);
    }
    prev->next[prev->next_len] = cur;
    prev->next_len++;
}

void forest_expand(struct forest *f, struct tree *tree_series)
{
    f->trees = safe_realloc(f->trees, sizeof(*f->trees)*(f->count+1), __LINE__);
    f->trees[f->count] = tree_series;
    f->count++;
}

struct tree *forest_find_depth_else_new(struct word_bank *wb,
        struct forest *f, const char *word, size_t word_len, size_t depth,
        struct tree *prev, size_t root_index)
{
    struct wb_word *wbw = wb_find(wb, word, word_len);
    struct tree *root = NULL;
    size_t i = 0;
    if(!prev) {
        root_index = f->count;
    }
    if(!wbw) {
        root = tree_new(depth, root_index);
        wbw = wb_insert(wb, word, word_len);
        root = wb_insert_tree(wbw, root, word, word_len);
        if(depth < 1)
            forest_expand(f, root);
    } else {
        // found word, check if tree exists at this depth+root_index
        // for this word
        for(i = 0; i < wbw->trees_len; i++) {
            if(wbw->trees[i]->index == depth && (depth < 1 || wbw->trees[i]->root_index == root_index)) {
                wbw->trees[i]->popularity++;
                return wbw->trees[i];
            }
        }
        if(!prev)
            assert(depth == 0);
        if(!prev)
            assert(depth-1 == prev->index);
        root = tree_new(depth, root_index);
        root = wb_insert_tree(wbw, root, word, word_len);
        root->popularity++;
        if(depth < 1)
            forest_expand(f, root);
    }
    return root;
}

void tree_dump(struct tree *t)
{
    int i = 0;
    if(!t) {
        fprintf(stderr, "WARNING: tree_dump() had NULL tree.\n");
        return;
    }
    for(; i < t->index; i++) {
        printf(" ");
    }
    if(t->index == 0)
        printf("r:%lu ix:%lu tl:%lu w:%s p:%lu\n", t->root_index, t->index, t->next_len, t->word->data, t->popularity);
    if(t->next_len > 0) {
        for(i = 0; i < t->next_len; i++) {
            tree_dump(t->next[i]);
        }
    }
}

void forest_dump(struct forest *f)
{
    size_t i = 0;
    for(; i < f->count; i++) {
        printf("DEBUG: f->trees[%lu]->index: %lu\n", i, f->trees[i]->index);
        tree_dump(f->trees[i]);
    }
}

#if TEST_FOREST
#define BUF_SIZE 1024*1024*4
void forest_test(void)
{
    struct word_bank *wb = wb_new();
    struct forest *f = forest_new();
    struct tree *cur = NULL, *prev = NULL;
    size_t root_index = 0;
    int i = 0, j = 0;
    char *token = NULL;
    char *rest = NULL;
    char *str = NULL;
    struct dirent **namelist;
    int n;
    char *find = NULL;
    char *txt = NULL;
    size_t fsize = 0;
    FILE *fd = NULL;
    char *buf = safe_malloc(sizeof(*buf)*BUF_SIZE, __LINE__);
    size_t buf_i = 0, x = 0;
    char *tests[] = {
        "Kidney transplantation or renal transplantation "
        "is the organ transplant of a kidney into a "
        "patient with end-stage kidney disease.",

        "Kidney transplantation is typically classified "
        "as deceased-donor (formerly known as cadaveric) or "
        "living-donor transplantation depending if the source "
        "of the donor organ.",

        "Kidney transplantation is typically classified "
        "as deceased-donor (formerly known as cadaveric) or "
        "living-donor transplantation depending on the source "
        "of the donor organ.",

        "Kidney transplantation or renal transplantation "
        "is the organ transplant of a kidney onto a "
        "patient with end-stage kidney disease.",

        "Hi, this is really the end of the world!",
        
        "Where are we going with all of this?",
        
        "William Henry Kibby was born at Winlaton, County Durham, UK, on 15 "
        "April 1903.",
        
        "The second of three children, Kibby was born to John "
        "Robert Kibby, a draper's assistant, and Mary Isabella Kibby née "
        "Birnie.",
        
        "He had two sisters. In early 1914, the Kibby family emigrated "
        "to Adelaide, South Australia.",

        "Bill attended Mitcham Public School and "
        "then held various jobs before securing a position at the Perfection "
        "Fibrous Plaster Works in Edwardstown.",
        
        "There, he worked as an interior decorator, designing and fixing "
        "plaster decorations.",

        "He married Mabel Sarah Bidmead Morgan in 1926; they lived at "
        "Helmsdale (now Glenelg East) and had two daughters.",

        "The 2020 Aegean Sea earthquake was a magnitude 7.0 earthquake which "
        "struck on Friday, 30 October 2020, about 14 km (8.7 mi) northeast of "
        "the island of Samos, Greece.",
    };
    wb_insert(wb, "]", 1);
    printf("create forest:\n");
    for(j = 0; j < sizeof(tests)/sizeof(*tests); j++) {
        cur = NULL;
        prev = NULL;
        root_index = 0;
        token = NULL;
        str = strdup(tests[j]);
        i = 0;
        for(token = strtok_r(str, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest)) {
            cur = forest_find_depth_else_new(wb, f, token, strlen(token), i, prev, root_index);
            root_index = cur->root_index;
            // link the current and previous items in both directions
            if(prev) {
                tree_insert_prev(cur, prev);
                tree_insert_next(cur, prev);
            }
            prev = cur;
            i++;
        }
        free(str);
        str = NULL;
    }
    n = scandir("wiki-dl", &namelist, NULL, alphasort);
    if(n == -1) {
        perror("scandir");
        exit(1);
    }
    chdir("wiki-dl");
    while (n--) {
        find = strstr(namelist[n]->d_name, ".dl");
        if(!find)
            continue;
        printf("dirlist: %s\n", namelist[n]->d_name);
        fd = fopen(namelist[n]->d_name, "rb");
        if(!fd) {
            perror("fopen");
            exit(1);
        }
        fseek(fd, 0, SEEK_END);
        fsize = ftell(fd);
        if(fsize < 5) {
            fclose(fd);
            continue;
        }
        txt = safe_malloc(sizeof(*txt)*fsize+1, __LINE__);
        rewind(fd);
        if(fread(txt, fsize, 1, fd) != 1) {
            perror("fread");
            fclose(fd);
            exit(1);
        }
        fclose(fd);
        memset(buf, '\0', sizeof(*buf)*8192);
        for(x = 0; x < fsize; x++) {
            buf[buf_i] = txt[x];
            buf_i++;
            if(txt[x] == '\0') {
                newline2space(buf, buf_i);
                cur = NULL;
                prev = NULL;
                root_index = 0;
                token = NULL;
                buf_i = 0;
                token = NULL;
                rest = NULL;
                str = strdup(buf);
                i = 0;
                for(token = strtok_r(str, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest)) {
                    cur = forest_find_depth_else_new(wb, f, token, strlen(token), i, prev, root_index);
                    root_index = cur->root_index;
                    // link the current and previous items in both directions
                    if(prev) {
                        tree_insert_prev(cur, prev);
                        tree_insert_next(cur, prev);
                    }
                    prev = cur;
                    i++;
                }
                free(str);
                str = NULL;
            } else if(buf_i >= BUF_SIZE) {
                fprintf(stderr, "WARNING: Buffer overflow, skipping some data.\n");
                break;
            }
        }
        free(txt);
    }
    free(namelist);
    wb_dump_tree(wb->words, 1);
    //forest_dump(f);
    wb_free(wb);
    printf("STATS: word_bank_size:%lu forest_root_size:%lu\n", wb->count, f->count);
}
int main(void)
{
    int i = 0;
    for(; i < 1; i++)
        forest_test();
    return 0;
}
#endif


