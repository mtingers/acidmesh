#ifndef UTIL_H
#define UTIL_H 1

#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

struct file {
    FILE *fd;
    size_t size;
    size_t (*read)(struct file *, size_t size, void *ptr);
    void (*close)(struct file *);
    //size_t (*write)(struct file *, size_t size, void *ptr);
    int exit_on_error;
};
size_t file_read(struct file *f, size_t size, void *ptr);
void file_close(struct file *f);
struct file *file_open(const char *path, const char *mode, int exit_on_error);

void *safe_malloc(size_t size, int lineno);
void *safe_realloc(void *ptr, size_t size, int lineno);
void safe_free(void *p, int lineno);
void newline2space(char *s, size_t s_len);
int bncmp(const char *s1, const char *s2, size_t s1_n, size_t s2_n);

#endif
