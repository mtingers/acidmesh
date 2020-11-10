#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <time.h>
#include "util.h"
#include "mesh.h"
#include "datatree.h"
#include "container.h"
#include "context.h"

struct mesh *mesh_init(void)
{
    struct mesh *f = safe_malloc(sizeof(*f), __LINE__);
    f->containers = NULL; //safe_malloc(sizeof(*f->containers), __LINE__);
    //f->containers[0] = container_init();
    f->container_len = 0; //1;
    f->wb = datatree_init();
    f->first_item = NULL;
    f->last_item = NULL;
    f->ctxs_len = 0;
    f->ctxs = NULL;
    return f;
}

void sequence_add_parent(struct sequence *s, struct sequence *parent)
{
    int rc;
    struct container *cur;
    if(!s->parents) {
        s->parents = container_init();
        s->parents->seq = parent;
        return;
    }
    cur = s->parents;
    while(cur) {
        rc = bncmp(cur->seq->data->data, parent->data->data,
            cur->seq->data->len, parent->data->len);
        if(rc > 0) {
            if(!cur->right) {
                cur->right = container_init();
                cur->right->seq = parent;
                return;
            }
            cur = cur->right;
        } else if(rc < 0) {
            if(!cur->left) {
                cur->left = container_init();
                cur->left->seq = parent;
                return;
            }
            cur = cur->left;
        } else {
            //exists
            return;
        }
    }
    fprintf(stderr, "ERROR: sequence_add_parent() failed due to programmer error.\n");
    exit(1);
}

void sequence_insert(struct mesh *f, const char *data, size_t data_len, size_t depth)
{
    int rc;
    size_t i = 0;
    struct data *w = data_find(f, data, data_len);
    struct sequence *s = NULL;
    struct sequence *return_seq = NULL;
    struct sequence *prev_seq = NULL;
    struct sequence *parent_seq = NULL;
    struct container *cur = NULL;
    int has_been_linked = 0;
    struct context *ctx = NULL;

    DEBUG_PRINT(("sequence_insert(): data=%s depth=%lu prev_seq:%s\n",
        data, depth, (prev_seq) ? "yes" : "no"));

    // reset the mesh item pointers on first item in sequence
    if(depth < 1) {
        f->last_item = NULL;
        f->first_item = NULL;
    } else {
        parent_seq = f->first_item;
        prev_seq = f->last_item;
    }

    // make sure the data exists in the datatree
    if(!w) {
        DEBUG_PRINT(("sequence_insert(): create data\n"));
        w = data_insert(f, data, data_len);
    }

    // When a new depth is reached, expand
    if(f->container_len < depth+1) {
        if(f->container_len+1 < depth) {
            fprintf(stderr,
                "ERROR: Programmer error: depth jumped greater than +1 from previous max depth (%lu->%lu).\n",
                f->container_len, depth);
            exit(1);
        }
        DEBUG_PRINT(("sequence_insert(): expand containers: %lu -> %lu\n",
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
    s = sequence_init(w, depth);
    DEBUG_PRINT(("sequence_insert(): sequence_init(): %s %lu\n", s->data->data, depth));
    if(!f->containers[depth]) {
        f->containers[depth] = container_init();
    }
    if(!f->containers[depth]->seq) {
        DEBUG_PRINT(("sequence_insert(): !f->containers[depth]->seq\n"));
        // The 1st item in this container, so simply add the seq
        f->containers[depth]->seq = s;
        return_seq = s;
        f->containers[depth]->count++;
    } else {
        DEBUG_PRINT(("sequence_insert(): f->containers[depth]->seq\n"));
        // Not the first seq in this container at this depth, so insert into
        // the container seq by data. Traverse the binary seq.
        cur = f->containers[depth];
        while(cur) {
            rc = bncmp(cur->seq->data->data, data,
                cur->seq->data->len, data_len);
            if(rc < 0) {
                if(!cur->left) {
                    DEBUG_PRINT(("while(cur): new-left\n"));
                    cur->left = container_init();
                    cur->left->seq = s;
                    return_seq = cur->left->seq;
                    f->containers[depth]->count++;
                    break;
                }
                cur = cur->left;
            } else if (rc > 0) {
                if(!cur->right) {
                    DEBUG_PRINT(("while(cur): new-right\n"));
                    cur->right = container_init();
                    cur->right->seq = s;
                    return_seq = cur->right->seq;
                    f->containers[depth]->count++;
                    break;
                }
                cur = cur->right;
            } else {
                // this data exists at this depth in this container seq
                DEBUG_PRINT(("while(cur): found\n"));
                return_seq = cur->seq;
                break;
            }
        }
    }
    // link prevs and nexts
    if(prev_seq) {
        DEBUG_PRINT(("sequence_insert(): prev_seq\n"));
        // Check if this link already exists in both directions
        for(i = 0; i < prev_seq->next_len; i++) {
            if(bncmp(prev_seq->nexts[i]->data->data, s->data->data,
                    prev_seq->nexts[i]->data->len, s->data->len) == 0) {
                has_been_linked = 1;
                break;
            }
        }
        if(has_been_linked < 1) {
            DEBUG_PRINT(("sequence_insert(): !found-link\n"));
            prev_seq->nexts = safe_realloc(
                prev_seq->nexts,
                sizeof(*prev_seq->nexts)*(prev_seq->next_len+1),
                __LINE__
            );
            prev_seq->nexts[prev_seq->next_len] = s;
            prev_seq->next_len++;
            s->prevs = safe_realloc(
                s->prevs,
                sizeof(*s->prevs)*(s->prev_len+1),
                __LINE__
            );
            s->prevs[s->prev_len] = s;
            s->prev_len++;
        }
    }
    if(parent_seq) {
        DEBUG_PRINT(("sequence_add_parent(): parent_seq\n"));
        sequence_add_parent(return_seq, parent_seq);
    } else {
        DEBUG_PRINT(("sequence_add_parent(): return_seq\n"));
        sequence_add_parent(return_seq, return_seq);
    }
    // Add this seq reference (data at this depth) to the data itself for
    // reverse lookups from a data.
    data_add_sequence(w, return_seq);

    // Set the first seq in the sequence if depth == 0
    if(depth < 1) {
        f->first_item = return_seq;
    }
    // Set the last item inserted in this sequence
    f->last_item = return_seq;

    // start of a new context that can be linked (new file being read and tokenized)
    if(!f->ctxs || depth < 1) {
        ctx = ctx_init();
        if(!f->ctxs) {
            f->ctxs = safe_malloc(sizeof(*f->ctxs), __LINE__);
        } else {
            f->ctxs = safe_realloc(f->ctxs, sizeof(*f->ctxs)*(f->ctxs_len+1), __LINE__);
        }
        f->ctxs[f->ctxs_len] = ctx;
        ctx->seqs = safe_malloc(sizeof(*ctx->seqs), __LINE__);
        ctx->seqs[ctx->seqs_len] = return_seq;
        f->ctxs_len++;
        ctx->seqs_len++;
    } else {
        ctx = f->ctxs[f->ctxs_len-1];
        ctx->seqs = safe_realloc(ctx->seqs, sizeof(*ctx->seqs)*(ctx->seqs_len+1), __LINE__);
        ctx->seqs[ctx->seqs_len] = return_seq;
        ctx->seqs_len++;
    }
}

void link_last_contexts(struct mesh *f)
{
    if(f->ctxs_len > 1) {
        f->ctxs[f->ctxs_len-2]->next_ctx = f->ctxs[f->ctxs_len-1]; 
        f->ctxs[f->ctxs_len-1]->prev_ctx = f->ctxs[f->ctxs_len-2]; 
    }
}

struct sequence *sequence_init(struct data *w, size_t depth)
{
    struct sequence *s = safe_malloc(sizeof(*s), __LINE__);
    s->depth = depth;
    s->parents = NULL;
    s->data = w;
    s->prev_len = 0;
    s->next_len = 0;
    s->nexts = NULL;
    s->prevs = NULL;
    return s;
}

void dump_sequence(struct mesh *f)
{
    size_t i = 0;
    for(i = 0; i < f->container_len; i++) {
        dump_container(f->containers[i], i, 0);
    }
    dump_datas(f->wb->datas, 0);
    for(i = 0; i < f->container_len; i++) {
        printf("depth-data-count:%lu|%lu\n", i, f->containers[i]->count-1);
    }
    printf("Total datas: %lu\n", f->wb->count-1);
    printf("Total contexts:%lu\n", f->ctxs_len-1);
    printf("Context samples:\n");
    size_t j = 0;
    for(i = 1; i < f->ctxs_len; i++) {
        printf("    ");
        for(j = 0; j < f->ctxs[i]->seqs_len; j++) {
            printf("%s ", f->ctxs[i]->seqs[j]->data->data);
        }
        printf("\n");
    }
}

#ifdef TEST_FOREST
#define BUF_SIZE 1024*1024*4
void test_mesh(const char *data_directory)
{
    struct mesh *f = mesh_init();
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
            printf("%d/%d: data-count:%lu\n", nn-n, nn, f->wb->count);
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
                    sequence_insert(f, token, strlen(token), i);
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
    dump_sequence(f);
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
    test_mesh(argv[1]);
    return 0;
}
#endif
