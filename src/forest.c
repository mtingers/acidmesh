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

struct forest *forest_init()
{
    struct forest *f = safe_malloc(sizeof(*f), __LINE__);
    f->containers = safe_malloc(sizeof(*f->containers), __LINE__);
    f->containers[0] = container_init();
    f->container_len = 1;
    f->wb = wordbank_init();
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

struct tree *tree_insert(struct forest *f, const char *data, size_t data_len,
        size_t depth, struct tree *prev_tree, struct tree *parent_tree)
{
    int rc;
    size_t i = 0;
    struct word *w = word_find(f, data, data_len);
    struct tree *t = NULL;
    struct tree *return_tree = NULL;
    struct container *cur = NULL;
    int has_been_linked = 0;

    DEBUG_PRINT(("tree_insert(): data=%s depth=%lu prev_tree:%s\n",
        data, depth, (prev_tree) ? "yes" : "no"));

    // make sure the word exists in the wordbank
    if(!w) {
        DEBUG_PRINT(("tree_insert(): create word\n"));
        w = word_insert(f, data, data_len);
    }

    // When a new depth is reached, expand
    if(f->container_len < depth+1) {
        DEBUG_PRINT(("tree_insert(): expand containers: %lu -> %lu\n",
            f->container_len, depth));
        f->container_len++;
        f->containers = safe_realloc(f->containers,
            sizeof(*f->containers)*f->container_len, __LINE__);
        f->containers[depth] = container_init();
        f->containers[depth]->count++;
    }

    t = tree_init(w, depth);
    DEBUG_PRINT(("tree_insert(): tree_init(): %s %lu\n", t->word->data, depth));

    if(!f->containers[depth]->tree) {
        DEBUG_PRINT(("tree_insert(): !f->containers[depth]->tree\n"));
        // The 1st item in this container, so simply add the tree
        f->containers[depth]->tree = t;
        return_tree = t;
        f->containers[depth]->count++;
        //return t;
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
            // copy prev parent, this inherets down the tree for quick lookup
            // note that this tree can have multiple parents on how it got to
            // this depth
            // (see below: tree_add_parent)
            //t->parent = prev_tree->parent;
        } else {
            DEBUG_PRINT(("tree_insert(): found-link\n"));
        }
    } else {
        DEBUG_PRINT(("tree_insert(): !!prev_tree\n"));
        // this inherets down the tree for quick lookup
        //t->parent = t; // point to self
    }
    if(parent_tree) {
        tree_add_parent(return_tree, parent_tree);
    } else {
        tree_add_parent(return_tree, return_tree);
    }
    // Add this tree reference (word at this depth) to the word itself for
    // reverse lookups from a word.
    word_add_tree(w, return_tree);
    return return_tree;
}

struct tree *tree_init(struct word *w, size_t depth)
{
    struct tree *t = safe_malloc(sizeof(*t), __LINE__);
    t->depth = depth;
    //t->parents_len = 0;
    //t->parents_depth_len = NULL;
    t->parents = NULL;
    //t->parents = safe_malloc(sizeof(*t->parents), __LINE__);
    //t->parents[0] = parent;
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
        printf("depth-word-count:%lu|%lu\n", i, f->containers[i]->count);
    }
    printf("Total words: %lu\n", f->wb->count);
}


#ifdef TEST_FOREST
#define BUF_SIZE 1024*1024*4
void test_forest(const char *data_directory)
{
    struct forest *f = forest_init();
    //struct word *w;
    struct tree *t, *parent_tree = NULL;
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
    clock_t start, end;
    double cpu_time_used;
    double cpu_time_avg = 0.0;
    double cpu_time_total = 0.0;
    double cpu_time_max = 0.0;
    double cpu_time_min = 9999999999999999.9;

    n = scandir(data_directory, &namelist, NULL, alphasort);
    nn = n;
    chdir(data_directory);
    if(n == -1) {
        perror("scandir");
        exit(1);
    }
    DEBUG_PRINT(("Start file read loop...\n"));
    // use partial: n = (int)(n/4);
    while (n--) {
        find = strstr(namelist[n]->d_name, ".dl");
        if(!find)
            continue;
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
                parent_tree = NULL;
                for(token = strtok_r(str, " ", &rest);
                    token != NULL;
                    token = strtok_r(NULL, " ", &rest)
                ) {
                    t = tree_insert(f, token, strlen(token), i, t, parent_tree);
                    i++;
                    if(!parent_tree) {
                        parent_tree = t;
                    }
                }
                // TODO: Some of these are List_of_ or Index_of_ with no \0
                // delimiter for sentences (only newline).
                // Maybe check name of file and filter out?
                //if(i > 500) {
                //    printf("i:%lu, %s\n", i, namelist[n]->d_name);
                //}
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
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        cpu_time_total += cpu_time_used;
        if(cpu_time_used > cpu_time_max)
            cpu_time_max = cpu_time_used;
        if(cpu_time_used < cpu_time_min)
            cpu_time_min = cpu_time_used;
    }
    cpu_time_avg = cpu_time_total/(double)nn;
    free(namelist);
    dump_tree(f);
    printf("total_time:%.2lf avg_time:%.6lf min_time:%.6lf max_time:%.6lf\n",
        cpu_time_total, cpu_time_avg, cpu_time_min, cpu_time_max);
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
