#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include "util.h"
#include "forest.h"
#include "wordbank.h"
#include "container.h"

struct forest *forest_init()
{
    struct forest *f = safe_malloc(sizeof(*f), __LINE__);
    f->containers = safe_malloc(sizeof(*f->containers), __LINE__);
    f->containers[0] = container_init();
    f->container_len = 1;
    f->wb = wordbank_init();
    return f;
}

struct tree *tree_insert(struct forest *f, const char *data, size_t depth, struct tree *prev_tree)
{
    int rc;
    size_t i = 0;
    struct word *w = word_find(f, data);
    struct tree *t = NULL;
    struct tree *return_tree = NULL;
    struct container *cur = NULL;
    int has_been_linked = 0;

    DEBUG_PRINT(("tree_insert(): data=%s depth=%lu prev_tree:%s\n",
        data, depth, (prev_tree) ? "yes" : "no"));

    // make sure the word exists in the wordbank
    if(!w) {
        DEBUG_PRINT(("tree_insert(): create word\n"));
        w = word_insert(f, data);
    }

    // When a new depth is reached, expand
    if(f->container_len < depth+1) {
        DEBUG_PRINT(("tree_insert(): expand containers: %lu -> %lu\n", f->container_len, depth));
        f->container_len++;
        f->containers = safe_realloc(f->containers, sizeof(*f->containers)*f->container_len, __LINE__);
        f->containers[depth] = container_init();
    }

    t = tree_init(w, prev_tree, depth);
    DEBUG_PRINT(("tree_insert(): tree_init(): %s %lu\n", t->word->data, depth));

    if(!f->containers[depth]->tree) {
        DEBUG_PRINT(("tree_insert(): !f->containers[depth]->tree\n"));
        // The 1st item in this container, so simply add the tree
        f->containers[depth]->tree = t;
        return_tree = t;
        //return t;
    } else {
        DEBUG_PRINT(("tree_insert(): f->containers[depth]->tree\n"));
        // Not the first tree in this container at this depth, so insert into
        // the container tree by word. Traverse the binary tree.
        cur = f->containers[depth];
        while(cur) {
            rc = strcmp(cur->tree->word->data, data);
            if(rc < 0) {
                if(!cur->left) {
                    DEBUG_PRINT(("while(cur): new-left\n"));
                    cur->left = container_init();
                    cur->left->tree = t;
                    return_tree = cur->left->tree;
                    break;
                }
                cur = cur->left;
            } else if (rc > 0) {
                if(!cur->right) {
                    DEBUG_PRINT(("while(cur): new-right\n"));
                    cur->right = container_init();
                    cur->right->tree = t;
                    return_tree = cur->right->tree;
                    break;
                }
                cur = cur->right;
            } else {
                // this word exists at this depth in this container tree
                DEBUG_PRINT(("while(cur): found\n"));
                return_tree = cur->tree;
                break;
            }
        }
    }
    // link prevs and nexts
    if(prev_tree) {
        DEBUG_PRINT(("tree_insert(): prev_tree\n"));
        // Check if this link already exists in both directions
        for(i = 0; i < prev_tree->next_len; i++) {
            if(strcmp(prev_tree->nexts[i]->word->data, t->word->data) == 0) {
                has_been_linked = 1;
                break;
            }
        }
        if(has_been_linked < 1) {
            DEBUG_PRINT(("tree_insert(): !found-link\n"));
            prev_tree->nexts = safe_realloc(
                prev_tree->nexts,
                sizeof(*prev_tree->nexts)*(prev_tree->next_len+1),
                __LINE__
            );
            prev_tree->nexts[prev_tree->next_len] = t;
            prev_tree->next_len++;
            t->prevs = safe_realloc(
                t->prevs,
                sizeof(*t->prevs)*(t->prev_len+1),
                __LINE__
            );
            t->prevs[t->prev_len] = t;
            t->prev_len++;
            // copy prev parent, this inherets down the tree for quick lookup
            t->parent = prev_tree->parent;
        } else {
            DEBUG_PRINT(("tree_insert(): found-link\n"));
        }
    } else {
        DEBUG_PRINT(("tree_insert(): !!prev_tree\n"));
        // this inherets down the tree for quick lookup
        t->parent = t; // point to self
    }
    // Add this tree reference (word at this depth) to the word itself for
    // reverse lookups from a word.
    word_add_tree(w, return_tree);
    return return_tree;
}

struct tree *tree_init(struct word *w, struct tree *parent, size_t depth)
{
    struct tree *t = safe_malloc(sizeof(*t), __LINE__);
    t->depth = depth;
    t->parent = parent;
    t->word = w;
    t->prev_len = 0;
    t->next_len = 0;
    t->nexts = NULL;
    t->prevs = NULL;
    return t;
}

void dump_tree(struct forest *f)
{
    size_t i = 0;
    for(i = 0; i < f->container_len; i++) {
        dump_container(f->containers[i], i, 0);
    }
    dump_words(f->wb->words, 0);
    printf("Total word: %lu\n", f->wb->count);
}


#ifdef TEST_FOREST
#define BUF_SIZE 1024*1024*4
void test_forest(const char *data_directory)
{
    struct forest *f = forest_init();
    struct word *w;
    struct tree *t;
    char *token = NULL;
    char *rest = NULL;
    char *str = NULL;
    struct dirent **namelist;
    int n;
    char *find = NULL;
    char *txt = NULL;
    struct file *fd;
    char *buf = safe_malloc(sizeof(*buf)*BUF_SIZE, __LINE__);
    size_t buf_i = 0, x = 0;
    size_t i = 0;
    int nn = 0;

    n = scandir(data_directory, &namelist, NULL, alphasort);
    nn = n;
    chdir(data_directory);
    if(n == -1) {
        perror("scandir");
        exit(1);
    }
    DEBUG_PRINT(("Start file read loop...\n"));
    while (n--) {
        find = strstr(namelist[n]->d_name, ".dl");
        if(!find)
            continue;
        if(n % 1000 == 0) {
            printf("%d/%d: word-count:%lu\n", nn-n, nn, f->wb->count);
        }
        fd = file_open(namelist[n]->d_name, "rb", 1);
        if(fd->size < 5) {
            fd->close(fd);
            free(fd);
            continue;
        }
        txt = safe_malloc(sizeof(*txt)*fd->size+1, __LINE__);
        fd->read(fd, fd->size, txt);
        fd->close(fd);
        memset(buf, '\0', sizeof(*buf)*BUF_SIZE);
        for(x = 0; x < fd->size; x++) {
            buf[buf_i] = txt[x];
            buf_i++;
            if(txt[x] == '\0') {
                newline2space(buf, buf_i);
                token = NULL;
                buf_i = 0;
                token = NULL;
                rest = NULL;
                str = strdup(buf);
                i = 0;
                t = NULL;
                for(token = strtok_r(str, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest)) {
                    t = tree_insert(f, token, i, t);
                    i++;
                }
                free(str);
                str = NULL;
                memset(buf, '\0', sizeof(*buf)*BUF_SIZE);
            } else if(buf_i >= BUF_SIZE) {
                fprintf(stderr, "WARNING: Buffer overflow, skipping some data.\n");
                break;
            }
        }
        free(fd);
        free(txt);
    }
    free(namelist);
    // add words to wordbank
    w = word_insert(f, "What"); //1
    w = word_insert(f, "is");
    w = word_insert(f, "your");
    w = word_insert(f, "favorite");
    w = word_insert(f, "food?");
    w = word_insert(f, "Where"); //2
    w = word_insert(f, "are");
    w = word_insert(f, "we");
    w = word_insert(f, "going");
    w = word_insert(f, "tonight?");
    // (tree_insert does word find/create internally)
    t = tree_insert(f, "What", 0, NULL);
    t = tree_insert(f, "is", 1, t);
    t = tree_insert(f, "your", 2, t);
    t = tree_insert(f, "favorite", 3, t);
    t = tree_insert(f, "food?", 4, t);
    // example of having tree_insert create non-existent words
    t = tree_insert(f, "This", 0, NULL);
    t = tree_insert(f, "has", 1, t);
    t = tree_insert(f, "new", 2, t);
    t = tree_insert(f, "words", 3, t);
    t = tree_insert(f, "created", 4, t);
    t = tree_insert(f, "from", 5, t);
    t = tree_insert(f, "tree_insert.", 6, t);
    dump_tree(f);
}

int main(int argc, char **argv)
{
    if(argc != 2) {
        printf("usage:\n");
        printf("%s <data-directory>\n", argv[0]);
        return 1;
    }
    test_forest(argv[1]);
    return 0;
}
#endif
