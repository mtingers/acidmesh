//
// Notes:
// https://docs.python.org/3/c-api/arg.html
// Some example code to reference:
// ftp://ftp.netapp.com/frm-ntap/opensource/ClusterViewer/1.0/package_sources/Python-3.4.4/Python/import.c
// https://stackoverflow.com/questions/3253563/pass-list-as-argument-to-python-c-module
//
// TODO: Figure out the python INCREF/DECREF stuff
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

static double dabs(double d)
{
    if(d < 0.0) {
        return 0.0-d;
    }
    return d;
}

static int check_mesh_index(size_t i)
{
    if(i >= g_meshs_len) {
        PyErr_SetString(PyExc_KeyError, "Invalid mesh index");
        return 0;
    }
    return 1;
}

size_t size_t_rand_range(size_t lower, size_t upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

static PyObject *pym_datatree_stats(PyObject *self, PyObject *args)
{
    size_t i = 0;
    int print_top_bottom = 0;
    if(!PyArg_ParseTuple(args, "ki", &i, &print_top_bottom)) {
        return NULL;
    }
    if(i >= g_meshs_len) {
        PyErr_SetString(PyExc_ValueError, "Invalid mesh index");
        return NULL;
    }
    datatree_stats(g_meshs[i], print_top_bottom);
    Py_RETURN_NONE;
}

static PyObject *pym_dump(PyObject *self, PyObject *args)
{
    size_t i = 0;
    if(!PyArg_ParseTuple(args, "k", &i)) {
        return NULL;
    }
    if(!check_mesh_index(i)) { return NULL; }
    dump_sequence(g_meshs[i]);
    Py_RETURN_NONE;
}

static PyObject *pym_link_last_contexts(PyObject *self, PyObject *args)
{
    size_t i = 0;
    if(!PyArg_ParseTuple(args, "k", &i)) {
        return NULL;
    }
    if(!check_mesh_index(i)) { return NULL; }
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
    if(!check_mesh_index(i)) { return NULL; }
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

// Return input as output
PyObject *pym_echo(PyObject *self, PyObject *args)
{
    size_t i = 0;
    struct mesh *m = NULL;
    PyObject *o = NULL;

    if(!PyArg_ParseTuple(args, "kO", &i, &o)) {
        return NULL;
    }
    Py_INCREF(o);
    if(!check_mesh_index(i)) { return NULL; }
    m = g_meshs[i];
    return o;
}

/*
Take variable input and generate a response.
    Example: How are you?
        -> I am good.

Ideas:
    - Build a sorted list from low to high based from data->stats_percent
    - Select contexts that match X% of the words in the sorted list
    - -OR- in other words, that may not work and find contexts that match the
      most (maybe with a threshold)
    - The idea is to find a context that matches X% items on the lower
      stats_percent of the inputs. Once contexts are found, sort them by
      higehst match. Next,
      pick a context (randomly?) and take the next_ctx value from it, with up
      to N next_ctx value (can be random or specified how much you want to
      generate). (next_ctx's contains sequences that can be printed/returned)

    - Edge cases:
        - all words are the same or similar rating
        - a small number of words have a low percent rating (low stats_percent
          usually inidicates it is not a language structure word for building
          sentences).

    - Other things: Maybe add a explore_factor% to select words outside of a
      context sequence that have a same common ancestor (at the same depth?)

 */
struct tracker {
    size_t i, j, k, l, o, p;
    size_t context_id;
    double hits;
};

int tracker_sort_cmp(const void *p1, const void *p2)
{
    struct tracker *d1 = *(struct tracker **)p1;
    struct tracker *d2 = *(struct tracker **)p2;
    if(d1->hits < d2->hits) {
        return 1;
    } else if(d1->hits > d2->hits) {
        return -1;
    }
    return 0;
}
int ctx_stats_cmp_old(const void *p1, const void *p2, size_t n1, size_t n2)
{
    size_t s1 = *(size_t *)p1;
    size_t s2 = *(size_t *)p2;
    if(s1 > s2) {
        return -1;
    } else if(s1 < s2) {
        return 1;
    }
    return 0;
}
int ctx_stats_cmp(void *p1, void *p2, size_t n1, size_t n2)
{
    size_t s1 = *(size_t *)p1;
    size_t s2 = *(size_t *)p2;
    if(s1 > s2) {
        return -1;
    } else if(s1 < s2) {
        return 1;
    }
    return 0;
}

PyObject *pym_generate_response(PyObject *self, PyObject *args)
{
    // TODO: Break up into functions and track malloc/free
    size_t i = 0, j = 0, k = 0, l = 0, n = 0, o = 0, p = 0;
    struct mesh *m = NULL;
    char * line;
    int nlines;
    PyObject * list_obj = NULL;
    PyObject * str_obj = NULL;
    struct data *d = NULL;
    struct datatree_stat **dstat = NULL;
    size_t dstat_n = 0;
    struct datatree_stat *dts = NULL;
    if(!PyArg_ParseTuple(args, "kO!", &i, &PyList_Type, &list_obj)) {
        return NULL;
    }
    if(!check_mesh_index(i)) { return NULL; }
    m = g_meshs[i];
    datatree_stats(m, 0);
    nlines = PyList_Size(list_obj);
    dstat = safe_malloc(sizeof(*dstat)*(nlines+1), __LINE__);
    for(n = 0; n < (size_t)nlines; n++){
        str_obj = PyList_GetItem(list_obj, n);
        const char *p = Py_TYPE(str_obj)->tp_name;
        if(p[0] != 's' || p[1] != 't' || p[2] != 'r' || p[3] != '\0') {
            PyErr_SetString(PyExc_ValueError,
                "Invalid list item (must be string)!");
            goto error_cleanup;
        }
        PyObject *temp_bytes = PyUnicode_AsEncodedString(
            str_obj, "UTF-8", "strict"
        );
        line = PyBytes_AS_STRING(temp_bytes);
        line = strdup(line);
        d = data_find(m, line, strlen(line));
        //if(d) { // && d->stats_percent < 1.0) {
        if(d) { // && d->stats_percent < 0.5) {
            //printf("D:%s %.6f\n", d->data, d->stats_percent);
            dts = safe_malloc(sizeof(*dts), __LINE__);
            dts->data_ptr = d;
            dts->count = d->stats_count;
            dts->percent = d->stats_percent;
            dstat[dstat_n] = dts;
            dstat_n++;
        }
        free(line);
        line = NULL;
    }
    qsort(dstat, dstat_n, sizeof(*dstat), datatree_sort_cmp);
    // Got here, the matching words were found and ordered by stat_percent
    // Now let's look at all of the contexts and rate them by count of matching
    // words
    struct tracker *track = NULL;
    struct tracker **tracks = NULL; //safe_malloc(sizeof(*tracks), __LINE__);
    size_t tracks_n = 0;
    struct sequence *seq = NULL, *sub_seq = NULL;
    double percent = 0.0;
    size_t count = 0;
    double hits = 0.0;
    struct context *ctx = NULL;
    Tree *ctx_stats = NULL;

    d = NULL;
    // This is essentially rating a context by how many matches the words have
    // within it (then gives it a rating).
    //
    // seqs[depth][index]
    // lookup all data's associated sequence's context's sequence's
    for(i = 0; i < dstat_n; i++) {
        d = dstat[i]->data_ptr;
        count = dstat[i]->count;
        percent = dstat[i]->percent;
        for(j = 0; j < d->seqs_len; j++) {
            if(!d->seqs[j]) { continue; }
            // This is where it breaks down on lookup since k can be huge for
            // common words. Does not scale. How to fix?
            for(k = 0; k < d->sub_seqs_len[j]; k++) {
                if(!d->seqs[j][k]) { continue; }
                seq = d->seqs[j][k];
                // track by context and skip if already calculated.
                // I think this makes sense?
                if(!ctx_stats) {
                    ctx_stats = tree_init();
                    ctx_stats->p = d->seqs[j][k];
                } else {
                    if(tree_find(ctx_stats, d->seqs[j][k], 8, ctx_stats_cmp) > 0) {
                        continue;
                    }
                    //printf("_new_: %p\n", d->seqs[j][k]);
                    tree_insert(ctx_stats, d->seqs[j][k], 8, ctx_stats_cmp);
                }
                // This might actually be the slow part...?
                for(l = 0; l < seq->ctxs_len; l++) {
                    ctx = seq->contexts[l];
                    hits = 0.0;
                    // this is slow here:
                    // all of the sequences in this context
                    int missed_words = 0;
                    for(o = 0; o < ctx->seqs_len; o++) {
                        sub_seq = ctx->seqs[o];
                        // hmm this is the slow part?
                        int found = 0;
                        for(p = 0; p < dstat_n; p++) {
                            if(p == i) { continue; }
                            //hits -= 1.0;
                            if(sub_seq->data->len != dstat[p]->data_ptr->len) {
                                missed_words++;
                                continue;
                            }
                            if(bncmp(sub_seq->data->data, dstat[p]->data_ptr->data, sub_seq->data->len, dstat[p]->data_ptr->len) == 0) {
                                //hits += 1.0; // + (0.0-dstat[p]->percent);
                                hits += 1.0 + (10.0-dstat[p]->percent); // + (0.0-dstat[p]->percent);
                                if(sub_seq->depth == i) {
                                    hits += 1.5; //hits; //0.5; // how much is this worth?
                                }
                                found = 1;
                            } else {
                                //hits += -dstat[p]->percent; //missed_words++;
                            }
                        }
                        if(found < 1) {
                            hits -= 10.0-sub_seq->data->stats_percent;
                        }
                    }
                    //hits -= missed_words;
                    double d = dabs((double)dstat_n - (double)ctx->seqs_len); // / 1.25;
                    hits =  hits - d;
                    track = safe_malloc(sizeof(*track), __LINE__);
                    track->i = i;
                    track->j = j;
                    track->k = k;
                    track->l = l;
                    track->context_id = ctx->context_id;
                    track->hits = hits;
                    if(tracks) {
                        tracks = safe_realloc(tracks, sizeof(*tracks)*(tracks_n+1), __LINE__);
                    } else {
                        tracks = safe_malloc(sizeof(*tracks), __LINE__);
                    }
                    tracks[tracks_n] = track;
                    tracks_n++;
                }
            }
        }
    }

    qsort(tracks, tracks_n, sizeof(*tracks), tracker_sort_cmp);

    PyObject *return_list = PyList_New(0);
    Py_INCREF(return_list);
    char *buffer = NULL;
    size_t buffer_size = 0, buffer_pos = 0;
    size_t *seen = safe_malloc(sizeof(*seen)*tracks_n*2, __LINE__);
    size_t seen_pos = 0;
    int is_seen = 0;
    double min_rating = 0.0;
    if(tracks[0]->hits < 0.0) {
        min_rating = 2 * (tracks[0]->hits / 2.0 + tracks[0]->hits);
    } else {
        min_rating = 2 * (tracks[0]->hits  / 2.0 - tracks[0]->hits);
    }
    for(i = 0; i < tracks_n; i++) {
        struct tracker *t = tracks[i];
        if(t->hits < min_rating) {
            break;
        }
        //printf("track: [%lu][%lu][%lu][%lu] -> %.2f\n", t->i, t->j, t->k, t->l, t->hits);
        d = dstat[t->i]->data_ptr;
        ctx = d->seqs[t->j][t->k]->contexts[t->l];
        is_seen = 0;
        if(ctx->next_ctx) {
            for(j = 0; j < seen_pos; j++) {
                if(seen[j] == ctx->next_ctx->context_id) {
                    is_seen = 1;
                    break;
                }
            }
        }
        if(ctx->next_ctx && is_seen < 1) {
            seen[seen_pos] = ctx->next_ctx->context_id;
            seen_pos++;
            buffer_size = 0;
            for(o = 0; o < ctx->next_ctx->seqs_len; o++) {
                buffer_size += ctx->next_ctx->seqs[o]->data->len + 1;
                //printf("%s ", ctx->next_ctx->seqs[o]->data->data);
            }
            buffer_size++;
            buffer = safe_malloc(sizeof(*buffer)*(buffer_size+1), __LINE__);
            buffer_size = 0;
            k = 0;
            buffer_pos = 0;
            for(o = 0; o < ctx->next_ctx->seqs_len; o++) {
                for(k = 0; k < ctx->next_ctx->seqs[o]->data->len; k++) {
                    if(ctx->next_ctx->seqs[o]->data->data[k] == '\0') { break; }
                    buffer[buffer_pos] = ctx->next_ctx->seqs[o]->data->data[k];
                    buffer_pos++;
                }
                buffer[buffer_pos] = ' ';
                buffer_pos++;
            }
            buffer[buffer_pos] = '\0';
            PyObject *strs = Py_BuildValue("{s:s, s:d}",
                "data", buffer,
                "rating", t->hits
            );
            PyList_Append(return_list, strs);
            safe_free(buffer, __LINE__);
        }
        is_seen = 0;
        if(ctx->prev_ctx) {
            for(j = 0; j < seen_pos; j++) {
                if(seen[j] == ctx->prev_ctx->context_id) {
                    is_seen = 1;
                    break;
                }
            }
        }
        if(!ctx->next_ctx && ctx->prev_ctx && is_seen < 1) {
            seen[seen_pos] = ctx->next_ctx->context_id;
            seen_pos++;
            buffer_size = 0;
            for(o = 0; o < ctx->prev_ctx->seqs_len; o++) {
                buffer_size += ctx->prev_ctx->seqs[o]->data->len + 1;
                //printf("%s ", ctx->prev_ctx->seqs[o]->data->data);
            }
            buffer_size++;
            buffer = safe_malloc(sizeof(*buffer)*(buffer_size+1), __LINE__);
            buffer_size = 0;
            k = 0;
            buffer_pos = 0;
            for(o = 0; o < ctx->prev_ctx->seqs_len; o++) {
                for(k = 0; k < ctx->prev_ctx->seqs[o]->data->len; k++) {
                    if(ctx->prev_ctx->seqs[o]->data->data[k] == '\0') { break; }
                    buffer[buffer_pos] = ctx->prev_ctx->seqs[o]->data->data[k];
                    buffer_pos++;
                }
                buffer[buffer_pos] = ' ';
                buffer_pos++;
            }
            buffer[buffer_pos] = '\0';
            PyObject *strs = Py_BuildValue("{s:s, s:d}",
                "data", buffer,
                "rating", t->hits
            );
            PyList_Append(return_list, strs);
            safe_free(buffer, __LINE__);
        }
    }

    // OK, whew, now lets take the top one and use the next context and print
    // it as a response. if next doesn't exist, use the previous. if prev
    // doesn't exist, then try the next item in the tracker;
    safe_free(seen, __LINE__);
    for(i = 0; i < dstat_n; i++)
        safe_free(dstat[i], __LINE__);
    safe_free(dstat, __LINE__);
    return return_list;
    //Py_RETURN_NONE;

error_cleanup:
    for(i = 0; i < dstat_n; i++)
        safe_free(dstat[i], __LINE__);
    safe_free(dstat, __LINE__);
    return NULL;
}

void pym_generate_internal(
        PyObject *return_list,
        struct mesh *m,
        const char *s1, const char *s2,
        size_t slen1, size_t slen2,
        size_t *used_ctxs, size_t used_ctxs_n, size_t max_used_ctxs)
{
    // find the word in the datatree
    struct data *d = data_find(m, s1, slen1);
    size_t i = 0, j = 0, k = 0, l = 0, n = 0;
    struct context *ctx = NULL;
    struct sequence *seq = NULL;
    int found = 0;
    struct context *prev_ctx = NULL;
    struct context *next_ctx = NULL;
    if(!d) {
        //fprintf(stderr, "WARNING: No results for lookup: %s\n", s1);
        return;
    }
    // pick a sequence that has > N contexts.
    // NOTE: seqs first layer is by depth and can have NULLs
    for(i = 0; i < d->seqs_len; i++) {
        if(!d->seqs[i]) { continue; }
        for(j = 0; j < d->sub_seqs_len[i]; j++) {
            seq = d->seqs[i][j];
            // Check each seqs->ctx counts for a suitable match
            //if(seq->ctxs_len < 1) { continue; seq = NULL; }
            // Found a suitable > N contexts. Now we need to see if s2 also
            // exists in any of these seq->contexts
            // NOTE: a context has a list of sequences in order
            for(k = 0; k < seq->ctxs_len; k++) {
                if(!seq->contexts[k]->prev_ctx || !seq->contexts[k]->next_ctx) {
                    continue;
                }
                for(l = 0; l < seq->contexts[k]->seqs_len; l++) {
                    if(bncmp(seq->contexts[k]->seqs[l]->data->data, s2,
                            seq->contexts[k]->seqs[l]->data->len, slen2) == 0) {
                        // Found a match, use this context if we haven't before
                        // check used_ctxs to see if it's in it
                        found = 0;
                        for(n = 0; n < used_ctxs_n; n++) {
                            if(used_ctxs[n] == seq->contexts[k]->context_id) {
                                found = 1;
                                break;
                            }
                        }
                        if(found < 1) {
                            ctx = seq->contexts[k];
                            goto found_ctx;
                        }
                    }
                }
            }
        }
    }
    // Reached end of loop, meaning goto found; was not done and nothing was
    // found
    //fprintf(stderr, "WARNING: No context results for lookup: %s\n", s1);
    return;

found_ctx:
    prev_ctx = ctx->prev_ctx;
    next_ctx = ctx->next_ctx;
    size_t buffer_size = 0;
    char *buffer = NULL;
    for(i = 0; i < prev_ctx->seqs_len; i++) {
        buffer_size += prev_ctx->seqs[i]->data->len;
    }
    for(i = 0; i < ctx->seqs_len; i++) {
        buffer_size += ctx->seqs[i]->data->len;
    }
    for(i = 0; i < next_ctx->seqs_len; i++) {
        buffer_size += next_ctx->seqs[i]->data->len;
    }
    buffer = safe_malloc(
        sizeof(*buffer)*(buffer_size)+prev_ctx->seqs_len+ctx->seqs_len+next_ctx->seqs_len+1,
        __LINE__
    );

    k = 0;
    for(i = 0; i < prev_ctx->seqs_len; i++) {
        for(j = 0; j < prev_ctx->seqs[i]->data->len; j++) {
            if(prev_ctx->seqs[i]->data->data[j] == '\0') { break; }
            buffer[k] = prev_ctx->seqs[i]->data->data[j];
            k++;
        }
        buffer[k] = ' ';
        k++;
        //printf("%s ", prev_ctx->seqs[i]->data->data);
    }
    for(i = 0; i < ctx->seqs_len; i++) {
        for(j = 0; j < ctx->seqs[i]->data->len; j++) {
            if(ctx->seqs[i]->data->data[j] == '\0') { break; }
            buffer[k] = ctx->seqs[i]->data->data[j];
            k++;
        }
        buffer[k] = ' ';
        k++;
        //printf("%s ", ctx->seqs[i]->data->data);
    }
    for(i = 0; i < next_ctx->seqs_len; i++) {
        for(j = 0; j < next_ctx->seqs[i]->data->len; j++) {
            if(next_ctx->seqs[i]->data->data[j] == '\0') { break; }
            buffer[k] = next_ctx->seqs[i]->data->data[j];
            k++;
        }
        buffer[k] = ' ';
        k++;
        //printf("%s ", next_ctx->seqs[i]->data->data);
    }
    buffer[k] = '\0';
    /*
    for(i = 0; i < ctx->seqs_len; i++) {
        printf("%s ", ctx->seqs[i]->data->data);
    }
    for(i = 0; i < next_ctx->seqs_len; i++) {
        printf("%s ", next_ctx->seqs[i]->data->data);
    }*/
    PyObject *strs = Py_BuildValue("s", buffer);
    safe_free(buffer, __LINE__);
    PyList_Append(return_list, strs);
    if(used_ctxs_n+3 < max_used_ctxs) {
        used_ctxs[used_ctxs_n] = prev_ctx->context_id;
        used_ctxs[used_ctxs_n+1] = ctx->context_id;
        used_ctxs[used_ctxs_n+2] = next_ctx->context_id;
        used_ctxs_n += 3;
        pym_generate_internal(return_list, m, s1, s2, slen1, slen2, used_ctxs, used_ctxs_n, max_used_ctxs);
    }
}

static PyObject *pym_generate(PyObject *self, PyObject *args)
{
    size_t i = 0;
    char *s = NULL;
    char *s2 = NULL;
    size_t slen = 0, s2len = 0;
    struct mesh *m = NULL;
    size_t used_ctxs_n = 0;
    size_t max_used_ctxs = 0;

    //srand(time(0));
    if(!PyArg_ParseTuple(args, "kskskk", &i, &s, &slen, &s2, &s2len, &max_used_ctxs)) {
        return NULL;
    }
    if(!check_mesh_index(i)) { return NULL; }
    m = g_meshs[i];

    size_t *used_ctxs = safe_malloc(sizeof(*used_ctxs)*max_used_ctxs, __LINE__);
    PyObject *return_list = PyList_New(0);
    pym_generate_internal(return_list, m, s, s2, slen, s2len, used_ctxs, used_ctxs_n, max_used_ctxs);

    safe_free(used_ctxs, __LINE__);
    return return_list;
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
    if(!check_mesh_index(i)) { return NULL; }
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
    {"generate_response", pym_generate_response, METH_VARARGS, "Create a new generate_response."},
    {"mesh", pym_mesh, METH_VARARGS, "Create a new mesh."},
    {"datatree_stats", pym_datatree_stats, METH_VARARGS, "Print stats on data word counts."},
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
