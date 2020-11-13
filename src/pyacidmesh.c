//
// Notes:
// https://docs.python.org/3/c-api/arg.html
//
#include <Python.h>
#include <time.h>
#include "util.h"
#include "mesh.h"
#include "datatree.h"
#include "container.h"
#include "context.h"

static size_t g_meshs_len = 0;
static struct mesh **g_meshs = NULL;
static const size_t MAX_DEPTH = 0;


size_t size_t_rand_range(size_t lower, size_t upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

static PyObject *pym_data_stats(PyObject *self, PyObject *args)
{
    size_t i = 0;
    if(!PyArg_ParseTuple(args, "k", &i)) {
        return NULL;
    }
    if(i >= g_meshs_len) {
        PyErr_SetString(PyExc_ValueError, "ERROR: Invalid mesh index");
        return NULL;
    }
    datatree_stats(g_meshs[i]);
    Py_RETURN_NONE;
}

static PyObject *pym_dump(PyObject *self, PyObject *args)
{
    size_t i = 0;
    if(!PyArg_ParseTuple(args, "k", &i)) {
        return NULL;
    }
    if(i >= g_meshs_len) {
        PyErr_SetString(PyExc_ValueError, "ERROR: Invalid mesh index");
        return NULL;
    }
    dump_sequence(g_meshs[i]);
    Py_RETURN_NONE;
}

static PyObject *pym_link_last_contexts(PyObject *self, PyObject *args)
{
    size_t i = 0;
    if(!PyArg_ParseTuple(args, "k", &i)) {
        return NULL;
    }
    if(i >= g_meshs_len) {
        PyErr_SetString(PyExc_ValueError, "ERROR: Invalid mesh index");
        return NULL;
    }
    link_last_contexts(g_meshs[i]);
    Py_RETURN_NONE;
}


static PyObject *pym_mesh_del(PyObject *self, PyObject *args)
{
    size_t i = 0;
    if(!PyArg_ParseTuple(args, "k", &i)) {
        return NULL;
    }
    if(g_meshs_len > i) {
        if(g_meshs[i]) {
            free(g_meshs[i]);
            g_meshs[i] = NULL;
        }
    }
    Py_RETURN_NONE;
}

static PyObject *pym_sequence_insert(PyObject *self, PyObject *args)
{
    size_t i = 0;
    char *s = NULL;
    size_t slen = 0;
    size_t index = 0;
    struct mesh *f = NULL;
    if(!PyArg_ParseTuple(args, "kskk", &i, &s, &slen, &index)) {
        return NULL;
    }
    if(i >= g_meshs_len) {
        PyErr_SetString(PyExc_ValueError, "ERROR: Invalid mesh index");
        return NULL;
    }
    f = g_meshs[i];
    sequence_insert(f, s, slen, index);
    Py_RETURN_NONE;
}

static PyObject *get_nexts_dict(struct sequence *s, size_t depth);
static PyObject *get_prevs_dict(struct sequence *s, size_t depth);

static size_t g_counter = 0;

size_t get_ancestors(PyObject *ancestors, struct container *parents, size_t index)
{
    if(!parents || !parents->seq || !parents->seq->data) {
        printf("E: !parents || !parents->seq)\n");
        exit(1);
    }
    struct container *cur = parents;
    g_counter++;
    PyObject *val = Py_BuildValue("s", cur->seq->data->data);
    PyList_SetItem(ancestors, index, val);
    index+=1;
    if(cur->left) {
        index = get_ancestors(ancestors, cur->left, index);
        index+=1;
    }
    if(cur->right) {
        index = get_ancestors(ancestors, cur->right, index);
        index+=1;
    }
    return index-1;
}

static PyObject *get_nexts_dict(struct sequence *s, size_t depth)
{
    struct sequence *cur;
    size_t i = 0, index = 0;
    PyObject *pseqs = PyList_New(s->next_len);
    if(s->next_len < 1 || depth > MAX_DEPTH) {
        Py_RETURN_NONE;
    }
    for(i = 0; i < s->next_len; i++) {
        cur = s->nexts[i];
        PyObject *ancestors = NULL, *sdict = NULL;
        if(cur->parents && cur->parents->count > 0) {
            ancestors = PyList_New(cur->parents->count);
            get_ancestors(ancestors, cur->parents, 0);
            //sdict = Py_BuildValue("{s:s, s:k, s:O, s:O, s:O}",
            sdict = Py_BuildValue("{s:s, s:k, s:O, s:O, s:O}",
                "data", cur->data->data,
                "depth", cur->depth,
                "nexts", get_nexts_dict(cur, depth+1),
                "prevs", get_prevs_dict(cur, depth+1),
                "ancestors", ancestors
            );
        } else {
            sdict = Py_BuildValue("{s:s, s:k, s:O, s:O, s:[]}",
                "data", cur->data->data,
                "depth", cur->depth,
                "nexts", get_nexts_dict(cur, depth+1),
                "prevs", get_prevs_dict(cur, depth+1),
                "ancestors"
            );
        }
        g_counter++;
        PyList_SetItem(pseqs, index, sdict);
        index++;
    }
    return pseqs;
}

static PyObject *get_prevs_dict(struct sequence *s, size_t depth)
{
    struct sequence *cur;
    size_t i = 0, index = 0;
    if(s->prev_len < 1 || depth > MAX_DEPTH) {
        Py_RETURN_NONE;
    }
    for(i = 0; i < s->prev_len; i++) {
        cur = s->prevs[i];
        if(cur != s) {
            index++;
        }
    }
    PyObject *pseqs = PyList_New(index);
    index = 0;
    for(i = 0; i < s->prev_len; i++) {
        cur = s->prevs[i];
        if(cur == s) {
            continue;
        }
        PyObject *ancestors = NULL, *sdict = NULL;
        if(cur->parents && cur->parents->count > 0) {
            ancestors = PyList_New(cur->parents->count);
            get_ancestors(ancestors, cur->parents, 0);
            sdict = Py_BuildValue("{s:s, s:k, s:O, s:O, s:O}",
                "data", cur->data->data,
                "depth", cur->depth,
                "prevs", get_prevs_dict(cur, depth+1),
                "nexts", get_nexts_dict(cur, depth+1),
                "ancestors", ancestors
            );
        } else {
            sdict = Py_BuildValue("{s:s, s:k, s:O, s:O, s:[]}",
                "data", cur->data->data,
                "depth", cur->depth,
                "prevs", get_prevs_dict(cur, depth+1),
                "nexts", get_nexts_dict(cur, depth+1),
                "ancestors"
            );

        }
        g_counter++;
        PyList_SetItem(pseqs, index, sdict);
        index++;
    }
    return pseqs;
}

void pym_generate_internal(struct mesh *m, const char *s, const char *s2, size_t slen, size_t s2len, int count, int max, size_t *not_rand, struct context **ctx_used, size_t ctx_used_n)
{
    size_t i = 0, k = 0;
    struct data *d = data_find(m, s, slen);
    if(!d) {
        return; //Py_RETURN_NONE;
    }
    size_t randn = 0;
    struct sequence *cur = NULL;
    struct context *ctx = NULL, *prev_ctx = NULL, *next_ctx = NULL;
    struct context *ctx_available[10];
    size_t ctx_available_n = 0;
    int found = 0;
    size_t j = 0;
    // search for s2 in d's associated sequences
    for(i = 0; i < d->seqs_len; i++) {
        if(d->seqs[i]) {
            for(k = 0; k < d->sub_seqs_len[i]; k++) {
                cur = d->seqs[i][k];
                if(cur) {
                    for(j = 0; j < cur->ctxs_len; j++) {
                        ctx = cur->contexts[j];
                        size_t h = 0;
                        for(h = 0; h < ctx->seqs_len; h++) {
                            if(s2len == ctx->seqs[h]->data->len) {
                                if(bncmp(ctx->seqs[h]->data->data, s2, s2len, s2len) == 0) {
                                    size_t z = 0;
                                    int is_used = 0;
                                    for(z = 0; z < ctx_used_n; z++) {
                                        if(ctx_used[z] == ctx) {
                                            is_used = 1;
                                            break;
                                        }
                                    }
                                    if(is_used < 1) {
                                        ctx_available[ctx_available_n] = ctx;
                                        ctx_used[ctx_used_n] = ctx;
                                        ctx_used_n++;
                                        ctx_available_n++;
                                        if(ctx_available_n > 9) {
                                            found = 1;
                                            break;
                                        }
                                    } else {
                                        printf("WAS_USED:\n");
                                    }
                                    //break;
                                }
                            }
                        }
                        if(found > 0)
                            break;
                    }
                }
                if(found > 0)
                    break;
            }
        }
        if(found > 0) {
            break;
        }
    }
    if(found < 1) {
        printf("NOT_FOUND\n");
    } else {
        printf("FOUND_FOR: %s\n", d->data);
        struct sequence *last_seq = NULL;
        size_t last_rand = 0;
        int rand_fail = 0;
        printf("\n\n");
        for(i = 0; i < 100; i++) {
            //randn = size_t_rand_range(0, cur->ctxs_len-1);
            randn = size_t_rand_range(0, ctx_available_n-1);
            for(j = 0; j < (size_t)count; j++) {
                last_rand = not_rand[j];
                if(randn == last_rand)
                    rand_fail = 1;
            }
            if(rand_fail < 1)
                break;
        }
        if(rand_fail > 0) {
            //printf("NO_MORE_RANDOM: 0-%lu\n", cur->ctxs_len-1);
            printf("NO_MORE_RANDOM: 0-%lu\n", ctx_available_n-1);
            return;
        }
        not_rand[count] = randn;
        //printf("randn: %lu of %lu\n", randn, cur->ctxs_len);
        printf("randn: %lu of %lu\n", randn, ctx_available_n);
        //ctx = cur->contexts[randn];
        ctx = ctx_available[randn]; //cur->contexts[randn];
        prev_ctx = ctx->prev_ctx;
        next_ctx = ctx->next_ctx;
        if(prev_ctx) {
            for(i = 0; i < prev_ctx->seqs_len; i++) {
                printf("%s ", prev_ctx->seqs[i]->data->data);
            }
        }
        for(i = 0; i < ctx->seqs_len; i++) {
            printf("%s ", ctx->seqs[i]->data->data);
        }
        if(next_ctx) {
            for(i = 0; i < next_ctx->seqs_len; i++) {
                printf("%s ", next_ctx->seqs[i]->data->data);
                last_seq = next_ctx->seqs[i];
            }
        }
        printf("\n\n");
        if(count < max)
            pym_generate_internal(m, s2, s, s2len, slen, count+1, max, not_rand, ctx_used, ctx_used_n);
    }
}

static PyObject *pym_generate(PyObject *self, PyObject *args)
{
    srand(time(0));
    size_t i = 0;
    char *s = NULL;
    char *s2 = NULL;
    size_t slen = 0, s2len = 0;
    struct mesh *m = NULL;
    struct context **ctx_used = safe_malloc(sizeof(*ctx_used)*10000, __LINE__);
    if(!PyArg_ParseTuple(args, "ksksk", &i, &s, &slen, &s2, &s2len)) {
        return NULL;
    }
    if(i >= g_meshs_len) {
        PyErr_SetString(PyExc_ValueError, "ERROR: Invalid mesh index");
        return NULL;
    }
    m = g_meshs[i];
    size_t *not_rand = safe_malloc(sizeof(*not_rand), __LINE__);
    not_rand[0] = 999999999999999999;
    pym_generate_internal(m, s, s2, slen, s2len, 0, 4, not_rand, ctx_used, 0);
    Py_RETURN_NONE;
}


static PyObject *pym_data_find(PyObject *self, PyObject *args)
{
    size_t i = 0, j = 0;
    char *s = NULL;
    size_t slen = 0;
    struct mesh *m = NULL;
    if(!PyArg_ParseTuple(args, "ksk", &i, &s, &slen)) {
        return NULL;
    }
    if(i >= g_meshs_len) {
        PyErr_SetString(PyExc_ValueError, "ERROR: Invalid mesh index");
        return NULL;
    }
    m = g_meshs[i];
    struct data *d = data_find(m, s, slen);
    if(!d) {
        Py_RETURN_NONE;
    }
    struct sequence *cur = NULL;
    size_t index = 0;
    for(i = 0; i < d->seqs_len; i++) {
        if(d->seqs[i])
            index++;
    }
    PyObject *pseqs = PyList_New(index);
    index = 0;
    g_counter = 0;
    for(i = 0; i < d->seqs_len; i++) {
        if(d->seqs[i]) {
            for(j = 0; j < d->sub_seqs_len[i]; j++) {
                cur = d->seqs[i][j];
                if(cur) {
                    PyObject *ancestors = NULL, *sdict = NULL;
                    if(cur->parents && cur->depth > 0) {
                        ancestors = PyList_New(cur->parents->count);
                        get_ancestors(ancestors, cur->parents, 0);
                        sdict = Py_BuildValue("{s:s, s:k, s:O, s:O, s:O}",
                            "data", cur->data->data,
                            "depth", cur->depth,
                            "nexts", get_nexts_dict(cur, 0),
                            "prevs", get_prevs_dict(cur, 0),
                            "ancestors", ancestors
                        );
                    } else {
                        sdict = Py_BuildValue("{s:s, s:k, s:O, s:O, s:[]}",
                            "data", cur->data->data,
                            "depth", cur->depth,
                            "nexts", get_nexts_dict(cur, 0),
                            "prevs", get_prevs_dict(cur, 0),
                            "ancestors"
                        );
                    }
                    g_counter++;
                    PyList_SetItem(pseqs, index, sdict);
                    index++;
                }
            }
        }
    }
    PyObject *found = Py_BuildValue("{s:s, s:O}",
        "data", d->data,
        "seqs", pseqs
    );
    g_counter++;
    printf("G_COUNTER: %lu\n", g_counter);
    printf("CTX_LEN:%lu\n", m->ctxs_len);
    return found;
}

static PyObject *pym_acidmesh(PyObject *self, PyObject *args)
{
    char *str = NULL;
    if(!PyArg_ParseTuple(args, "s", &str)) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *pym_mesh(PyObject *self, PyObject *args)
{
    if(!g_meshs) {
        g_meshs = safe_malloc(sizeof(*g_meshs), __LINE__);
        g_meshs[0] = mesh_init();
        g_meshs_len++;
        return PyLong_FromLong(0);
    }
    g_meshs = safe_realloc(g_meshs, sizeof(*g_meshs)*(g_meshs_len+1), __LINE__);
    g_meshs[g_meshs_len] = mesh_init();
    g_meshs_len++;
    return PyLong_FromLong(g_meshs_len-1);
}

//
// Initialization section
// Adds methods and module to python
//

static PyMethodDef acidmesh_methods[] = {
    {"acidmesh_test", pym_acidmesh, METH_VARARGS, "The AcidMesh Python module."},
    {"generate", pym_generate, METH_VARARGS, "Generate sentences from basic input keywords."},
    {"mesh", pym_mesh, METH_VARARGS, "Create a new mesh."},
    {"data_stats", pym_data_stats, METH_VARARGS, "Print stats on data word counts."},
    {"mesh_del", pym_mesh_del, METH_VARARGS, "Delete a mesh."},
    {"sequence_insert", pym_sequence_insert, METH_VARARGS, "Insert a sequence."},
    {"dump", pym_dump, METH_VARARGS, "Print/dump the mesh."},
    {"data_find", pym_data_find, METH_VARARGS, "Search for data reference in datatree and return associated data (nexts/prevs/ancestors)."},
    {"link_last_contexts", pym_link_last_contexts, METH_VARARGS, "Link previous and current visited contexts."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef acidmeshmodule = {
    PyModuleDef_HEAD_INIT,
    "cacidmesh",
    "The AcidMesh Python module",
    -1,
    acidmesh_methods
};

PyMODINIT_FUNC PyInit_cacidmesh(void)
{
    return PyModule_Create(&acidmeshmodule);
}
