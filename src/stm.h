#ifndef STM_H
#define STM_H 1

struct stm {
    size_t nback;
    size_t items_pos;
    size_t items_inserted;
    int direction;
    char **items;
    char **items_len;
};

struct stm *stm_init(size_t nback);
void stm_insert(struct stm *s, const char *item, size_t item_len);

#endif
