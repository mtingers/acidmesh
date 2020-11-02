#ifndef UTIL_H
#define UTIL_H 1

void *safe_malloc(size_t size, int lineno);
void *safe_realloc(void *ptr, size_t size, int lineno);
void safe_free(void *p, int lineno);
void newline2space(char *s, size_t s_len);

#endif
