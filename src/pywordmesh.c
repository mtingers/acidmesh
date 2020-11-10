#include <Python.h>
#include "util.h"
#include "forest.h"
#include "wordbank.h"
#include "container.h"
#include "context.h"

static size_t g_forests_len = 0;
static struct forest **g_forests = NULL;

static PyObject *pym_dump(PyObject *self, PyObject *args)
{
    size_t i = 0;
    if(!PyArg_ParseTuple(args, "k", &i)) {
        return NULL;
    }
    if(i >= g_forests_len) {
        PyErr_SetString(PyExc_ValueError, "ERROR: Invalid forest index");
        return NULL;
    }
    dump_tree(g_forests[i]);
    Py_RETURN_NONE;
}

static PyObject *pym_link_last_contexts(PyObject *self, PyObject *args)
{
    size_t i = 0;
    if(!PyArg_ParseTuple(args, "k", &i)) {
        return NULL;
    }
    if(i >= g_forests_len) {
        PyErr_SetString(PyExc_ValueError, "ERROR: Invalid forest index");
        return NULL;
    }
    link_last_contexts(g_forests[i]);
    Py_RETURN_NONE;
}


static PyObject *pym_forest_del(PyObject *self, PyObject *args)
{
    size_t i = 0;
    if(!PyArg_ParseTuple(args, "k", &i)) {
        return NULL;
    }
    if(g_forests_len > i) {
        if(g_forests[i]) {
            free(g_forests[i]);
            g_forests[i] = NULL;
        }
    }
    Py_RETURN_NONE;
}

static PyObject *pym_tree_insert(PyObject *self, PyObject *args)
{
    size_t i = 0;
    char *s = NULL;
    size_t slen = 0;
    size_t index = 0;
    struct forest *f = NULL;
    if(!PyArg_ParseTuple(args, "kskk", &i, &s, &slen, &index)) {
        return NULL;
    }
    if(i >= g_forests_len) {
        PyErr_SetString(PyExc_ValueError, "ERROR: Invalid forest index");
        return NULL;
    }
    f = g_forests[i];
    tree_insert(f, s, slen, index);
    Py_RETURN_NONE;
}


static PyObject *pym_wordmesh(PyObject *self, PyObject *args)
{
    char *str = NULL;
    if(!PyArg_ParseTuple(args, "s", &str)) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *pym_forest(PyObject *self, PyObject *args)
{
    if(!g_forests) {
        g_forests = safe_malloc(sizeof(*g_forests), __LINE__);
        g_forests[0] = forest_init();
        g_forests_len++;
        return PyLong_FromLong(0);
    }
    g_forests = safe_realloc(g_forests, sizeof(*g_forests)*(g_forests_len+1), __LINE__);
    g_forests[g_forests_len] = forest_init();
    g_forests_len++;
    return PyLong_FromLong(g_forests_len-1);
}

//
// Initialization section
// Adds methods and module to python
//

static PyMethodDef wordmesh_methods[] = {
    {"wordmesh_test", pym_wordmesh, METH_VARARGS, "The WordMesh Python module"},
    {"forest", pym_forest, METH_VARARGS, "Initialize a wordmesh forest"},
    {"forest_del", pym_forest_del, METH_VARARGS, "Initialize a wordmesh forest"},
    {"tree_insert", pym_tree_insert, METH_VARARGS, "Initialize a wordmesh forest"},
    {"dump", pym_dump, METH_VARARGS, "Initialize a wordmesh forest"},
    {"link_last_contexts", pym_link_last_contexts, METH_VARARGS, "Initialize a wordmesh forest"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef wordmeshmodule = {
    PyModuleDef_HEAD_INIT,
    "cwordmesh",
    "The WordMesh Python module",
    -1,
    wordmesh_methods
};

PyMODINIT_FUNC PyInit_cwordmesh(void)
{
    return PyModule_Create(&wordmeshmodule);
}
