//
// Notes:
// https://docs.python.org/3/c-api/arg.html
//
#include <Python.h>
#include "util.h"
#include "mesh.h"
#include "datatree.h"
#include "container.h"
#include "context.h"

static size_t g_meshs_len = 0;
static struct mesh **g_meshs = NULL;
static const size_t MAX_DEPTH = 2;

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

size_t get_ancestors(PyObject *ancestors, struct container *parents, size_t index)
{
    if(!parents || !parents->seq || !parents->seq->data) {
        printf("E: !parents || !parents->seq)\n");
        exit(1);
    }
    struct container *cur = parents;
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
        PyList_SetItem(pseqs, index, sdict);
        index++;
    }
    return pseqs;
}

static PyObject *pym_data_find(PyObject *self, PyObject *args)
{
    size_t i = 0;
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
    for(i = 0; i < d->seqs_len; i++) {
        cur = d->seqs[i];
        if(cur) {
            PyObject *ancestors = NULL, *sdict = NULL;
            if(cur->parents && cur->depth > 0) {
                ancestors = PyList_New(cur->parents->count);
                get_ancestors(ancestors, cur->parents, 0);
                sdict = Py_BuildValue("{s:k, s:O, s:O, s:O}",
                    "depth", cur->depth,
                    "nexts", get_nexts_dict(cur, 0),
                    "prevs", get_prevs_dict(cur, 0),
                    "ancestors", ancestors
                );
            } else {
                sdict = Py_BuildValue("{s:k, s:O, s:O, s:[]}",
                    "depth", cur->depth,
                    "nexts", get_nexts_dict(cur, 0),
                    "prevs", get_prevs_dict(cur, 0),
                    "ancestors"
                );
            }
            PyList_SetItem(pseqs, index, sdict);
            index++;
        }
    }
    PyObject *found = Py_BuildValue("{s:s, s:O}",
        "data", d->data,
        "seqs", pseqs
    );
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
    {"acidmesh_test", pym_acidmesh, METH_VARARGS, "The AcidMesh Python module"},
    {"mesh", pym_mesh, METH_VARARGS, "Create a new mesh."},
    {"mesh_del", pym_mesh_del, METH_VARARGS, "Delete a mesh."},
    {"sequence_insert", pym_sequence_insert, METH_VARARGS, "Insert a sequence."},
    {"dump", pym_dump, METH_VARARGS, "Print/dump the mesh."},
    {"data_find", pym_data_find, METH_VARARGS, "Search for data reference in datatree."},
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
