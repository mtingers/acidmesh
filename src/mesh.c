#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <pthread.h> 
#include "util.h"
#include "mesh.h"
#include "sequence.h"
#include "datatree.h"
#include "container.h"
#include "context.h"

void mesh_lock(struct mesh *m) 
{
    pthread_mutex_lock(&m->lock);
}

void mesh_unlock(struct mesh *m)
{
    pthread_mutex_unlock(&m->lock);
}

struct mesh *mesh_init(void)
{
    struct mesh *m = safe_malloc(sizeof(*m), __LINE__);
    if(pthread_mutex_init(&m->lock, NULL) != 0) {
        fprintf(stderr, "ERROR: pthread_mutex_init() failed. Line:%d\n", __LINE__);
        exit(1);
    }
    m->containers = NULL;
    m->container_len = 0;
    m->dt = datatree_init();
    m->first_item = NULL;
    m->last_item = NULL;
    m->ctxs_len = 0;
    m->ctxs = NULL;
    m->total_sequence_data_refs = 0;
    m->dt_stats_top = 0.0;
    return m;
}

void link_last_contexts(struct mesh *m)
{
    if(m->ctxs_len > 1) {
        m->ctxs[m->ctxs_len-2]->next_ctx = m->ctxs[m->ctxs_len-1];
        m->ctxs[m->ctxs_len-1]->prev_ctx = m->ctxs[m->ctxs_len-2];
    }
}
void link_last_contexts_r(struct mesh *m)
{
    mesh_lock(m);
    link_last_contexts(m);
    mesh_unlock(m);
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
    // use partial:
    //n = (int)(n/10);
    while(n--) {
        find = strstr(namelist[n]->d_name, ".dl");
        if(!find) continue;
        if(strstr(namelist[n]->d_name, "List_of_")) continue;
        if(strstr(namelist[n]->d_name, "Index_of_")) continue;
        if(n % 1000 == 0) {
            printf("%d/%d: data-count:%lu\n", nn-n, nn, f->dt->count);
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
    datatree_stats(f, 0);
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
