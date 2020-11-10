#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <time.h>
#include "util.h"
#include "forest.h"
#include "wordbank.h"
#include "container.h"
#include "context.h"

struct forest *forest_init()
{
    struct forest *f = safe_malloc(sizeof(*f), __LINE__);
    f->containers = NULL; //safe_malloc(sizeof(*f->containers), __LINE__);
    //f->containers[0] = container_init();
    f->container_len = 0; //1;
    f->wb = wordbank_init();
    f->first_item = NULL;
    f->last_item = NULL;
    f->ctxs_len = 0;
    f->ctxs = NULL;
    return f;
}

void tree_add_parent(struct tree *t, struct tree *parent)
{
    int rc;
    struct container *cur;
    if(!t->parents) {
        t->parents = container_init();
        t->parents->tree = parent;
        return;
    }
    cur = t->parents;
    while(cur) {
        rc = bncmp(cur->tree->word->data, parent->word->data,
            cur->tree->word->len, parent->word->len);
        if(rc > 0) {
            if(!cur->right) {
                cur->right = container_init();
                cur->right->tree = parent;
                return;
            }
            cur = cur->right;
        } else if(rc < 0) {
            if(!cur->left) {
                cur->left = container_init();
                cur->left->tree = parent;
                return;
            }
            cur = cur->left;
        } else {
            //exists
            return;
        }
    }
    fprintf(stderr, "ERROR: tree_add_parent() failed due to programmer error.\n");
    exit(1);
}

void tree_insert(struct forest *f, const char *data, size_t data_len, size_t depth)
{
    int rc;
    size_t i = 0;
    struct word *w = word_find(f, data, data_len);
    struct tree *t = NULL;
    struct tree *return_tree = NULL;
    struct tree *prev_tree = NULL;
    struct tree *parent_tree = NULL;
    struct container *cur = NULL;
    int has_been_linked = 0;
    struct context *ctx = NULL;

    DEBUG_PRINT(("tree_insert(): data=%s depth=%lu prev_tree:%s\n",
        data, depth, (prev_tree) ? "yes" : "no"));

    // reset the forest item pointers on first item in sequence
    if(depth < 1) {
        f->last_item = NULL;
        f->first_item = NULL;
    } else {
        parent_tree = f->first_item;
        prev_tree = f->last_item;
    }

    // make sure the word exists in the wordbank
    if(!w) {
        DEBUG_PRINT(("tree_insert(): create word\n"));
        w = word_insert(f, data, data_len);
    }

    // When a new depth is reached, expand
    if(f->container_len < depth+1) {
        if(f->container_len+1 < depth) {
            fprintf(stderr,
                "ERROR: Programmer error: depth jumped greater than +1 from previous max depth (%lu->%lu).\n",
                f->container_len, depth);
            exit(1);
        }
        DEBUG_PRINT(("tree_insert(): expand containers: %lu -> %lu\n",
            f->container_len, depth));
        if(f->container_len < 1) { //!f->containers) {
            f->containers = safe_malloc(sizeof(*f->containers)*(depth+1), __LINE__);
            for(i = 0; i <= depth; i++) {
                f->containers[i] = NULL; //container_init();
            }
        } else {
            f->containers = safe_realloc(f->containers,
                sizeof(*f->containers)*(depth+1), __LINE__);
            for(i = f->container_len; i <= depth; i++) {
                f->containers[i] = NULL; //container_init();
            }
        }
        f->container_len = depth+1;
        f->containers[depth] = container_init();
        f->containers[depth]->count++;
    } else {
        //printf("DONT_EXPAND: %lu %lu\n", f->container_len, depth);
    }
    t = tree_init(w, depth);
    DEBUG_PRINT(("tree_insert(): tree_init(): %s %lu\n", t->word->data, depth));
    if(!f->containers[depth]) {
        f->containers[depth] = container_init();
    }
    if(!f->containers[depth]->tree) {
        DEBUG_PRINT(("tree_insert(): !f->containers[depth]->tree\n"));
        // The 1st item in this container, so simply add the tree
        f->containers[depth]->tree = t;
        return_tree = t;
        f->containers[depth]->count++;
    } else {
        DEBUG_PRINT(("tree_insert(): f->containers[depth]->tree\n"));
        // Not the first tree in this container at this depth, so insert into
        // the container tree by word. Traverse the binary tree.
        cur = f->containers[depth];
        while(cur) {
            rc = bncmp(cur->tree->word->data, data,
                cur->tree->word->len, data_len);
            if(rc < 0) {
                if(!cur->left) {
                    DEBUG_PRINT(("while(cur): new-left\n"));
                    cur->left = container_init();
                    cur->left->tree = t;
                    return_tree = cur->left->tree;
                    f->containers[depth]->count++;
                    break;
                }
                cur = cur->left;
            } else if (rc > 0) {
                if(!cur->right) {
                    DEBUG_PRINT(("while(cur): new-right\n"));
                    cur->right = container_init();
                    cur->right->tree = t;
                    return_tree = cur->right->tree;
                    f->containers[depth]->count++;
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
            if(bncmp(prev_tree->nexts[i]->word->data, t->word->data,
                    prev_tree->nexts[i]->word->len, t->word->len) == 0) {
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
        }
    }
    if(parent_tree) {
        DEBUG_PRINT(("tree_add_parent(): parent_tree\n"));
        tree_add_parent(return_tree, parent_tree);
    } else {
        DEBUG_PRINT(("tree_add_parent(): return_tree\n"));
        tree_add_parent(return_tree, return_tree);
    }
    // Add this tree reference (word at this depth) to the word itself for
    // reverse lookups from a word.
    word_add_tree(w, return_tree);

    // Set the first tree in the sequence if depth == 0
    if(depth < 1) {
        f->first_item = return_tree;
    }
    // Set the last item inserted in this sequence
    f->last_item = return_tree;

    // start of a new context that can be linked (new file being read and tokenized)
    if(!f->ctxs || depth < 1) {
        ctx = ctx_init();
        if(!f->ctxs) {
            f->ctxs = safe_malloc(sizeof(*f->ctxs), __LINE__);
        } else {
            f->ctxs = safe_realloc(f->ctxs, sizeof(*f->ctxs)*(f->ctxs_len+1), __LINE__);
        }
        f->ctxs[f->ctxs_len] = ctx;
        ctx->trees = safe_malloc(sizeof(*ctx->trees), __LINE__);
        ctx->trees[ctx->trees_len] = return_tree;
        f->ctxs_len++;
        ctx->trees_len++;
    } else {
        ctx = f->ctxs[f->ctxs_len-1];
        ctx->trees = safe_realloc(ctx->trees, sizeof(*ctx->trees)*(ctx->trees_len+1), __LINE__);
        ctx->trees[ctx->trees_len] = return_tree;
        ctx->trees_len++;
    }
}

void link_last_contexts(struct forest *f)
{
    if(f->ctxs_len > 1) {
        f->ctxs[f->ctxs_len-2]->next_ctx = f->ctxs[f->ctxs_len-1]; 
        f->ctxs[f->ctxs_len-1]->prev_ctx = f->ctxs[f->ctxs_len-2]; 
    }
}

struct tree *tree_init(struct word *w, size_t depth)
{
    struct tree *t = safe_malloc(sizeof(*t), __LINE__);
    t->depth = depth;
    t->parents = NULL;
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
    for(i = 0; i < f->container_len; i++) {
        printf("depth-word-count:%lu|%lu\n", i, f->containers[i]->count-1);
    }
    printf("Total words: %lu\n", f->wb->count-1);
    printf("Total contexts:%lu\n", f->ctxs_len-1);
    printf("Context samples:\n");
    size_t j = 0;
    for(i = 1; i < f->ctxs_len; i++) {
        printf("    ");
        for(j = 0; j < f->ctxs[i]->trees_len; j++) {
            printf("%s ", f->ctxs[i]->trees[j]->word->data);
        }
        printf("\n");
    }
}

#ifdef TEST_FOREST
#define BUF_SIZE 1024*1024*4
void test_forest(const char *data_directory)
{
    struct forest *f = forest_init();
    char *token = NULL, *rest = NULL, *str = NULL;
    struct dirent **namelist;
    int n = 0, nn = 0;
    size_t buf_i = 0, x = 0, i = 0;
    char *find = NULL, *txt = NULL;
    char *buf = safe_malloc(sizeof(*buf)*BUF_SIZE, __LINE__);
    clock_t start, end;
    double time_used;
    double time_avg = 0.0, time_total = 0.0, time_max = 0.0, time_min = 99999.9;
    struct file *fd;

    n = scandir(data_directory, &namelist, NULL, alphasort);
    nn = n;
    chdir(data_directory);
    if(n == -1) {
        perror("scandir");
        exit(1);
    }
    // use partial: n = (int)(n/4);
    while(n--) {
        find = strstr(namelist[n]->d_name, ".dl");
        if(!find) continue;
        if(n % 1000 == 0) {
            printf("%d/%d: word-count:%lu\n", nn-n, nn, f->wb->count);
        }
        start = clock();
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
        //f->ctx_use_prev = 0;
        for(x = 0; x < fd->size; x++) {
            buf[buf_i] = txt[x];
            buf_i++;
            if(txt[x] == '\0') {
                newline2space(buf, buf_i);
                str = strdup(buf);
                token = NULL;
                token = NULL;
                rest = NULL;
                buf_i = 0;
                i = 0;
                token = strtok_r(str, " ", &rest);
                while(token != NULL) {
                    tree_insert(f, token, strlen(token), i);
                    token = strtok_r(NULL, " ", &rest);
                    i++;
                    //f->ctx_use_prev = 1;
                }
                // TODO: Some of these are List_of_ or Index_of_ with no \0
                // delimiter for sentences (only newline).
                free(str);
                str = NULL;
                link_last_contexts(f);
            } else if(buf_i >= BUF_SIZE) {
                fprintf(stderr, "WARNING: Buffer overflow, skipping some data.\n");
                break;
            }
        }
        free(fd);
        free(txt);
        end = clock();
        time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        time_total += time_used;
        if(time_used > time_max)
            time_max = time_used;
        if(time_used < time_min)
            time_min = time_used;
    }
    time_avg = time_total/(double)nn;
    free(namelist);
    dump_tree(f);
    printf("total_time:%.2lf avg_time:%.6lf min_time:%.6lf max_time:%.6lf\n",
        time_total, time_avg, time_min, time_max);
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
