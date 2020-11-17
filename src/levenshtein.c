// From: http://www.martinbroadhurst.com/levenshtein-distance-in-c.html
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "levenshtein.h" 

typedef enum {
    INSERTION,
    DELETION,
    SUBSTITUTION,
    NONE
} edit_type;
 
struct edit {
    unsigned int score;
    edit_type type;
    char arg1;
    char arg2;
    unsigned int pos;
    struct edit *prev;
};
typedef struct edit edit;
 
static int min3(int a, int b, int c)
{
    if (a < b && a < c) {
        return a;
    }
    if (b < a && b < c) {
        return b;
    }
    return c;
}
 
static unsigned int levenshtein_matrix_calculate(edit **mat, const char *str1, size_t len1,
        const char *str2, size_t len2)
{
    unsigned int i, j;
    for (j = 1; j <= len2; j++) {
        for (i = 1; i <= len1; i++) {
            unsigned int substitution_cost;
            unsigned int del = 0, ins = 0, subst = 0;
            unsigned int best;
            if (str1[i - 1] == str2[j - 1]) {
                substitution_cost = 0;
            }
            else {
                substitution_cost = 1;
            }
            del = mat[i - 1][j].score + 1; /* deletion */
            ins = mat[i][j - 1].score + 1; /* insertion */
            subst = mat[i - 1][j - 1].score + substitution_cost; /* substitution */
            best = min3(del, ins, subst);
            mat[i][j].score = best;                  
            mat[i][j].arg1 = str1[i - 1];
            mat[i][j].arg2 = str2[j - 1];
            mat[i][j].pos = i - 1;
            if (best == del) {
                mat[i][j].type = DELETION;
                mat[i][j].prev = &mat[i - 1][j];
            }
            else if (best == ins) {
                mat[i][j].type = INSERTION;
                mat[i][j].prev = &mat[i][j - 1];
            }
            else {
                if (substitution_cost > 0) {
                    mat[i][j].type = SUBSTITUTION;
                }
                else {
                    mat[i][j].type = NONE;
                }
                mat[i][j].prev = &mat[i - 1][j - 1];
            }
        }
    }
    return mat[len1][len2].score;
}
 
static edit **levenshtein_matrix_create(const char *str1, size_t len1, const char *str2,
        size_t len2)
{
    unsigned int i, j;
    edit **mat = malloc((len1 + 1) * sizeof(edit *));
    if (mat == NULL) {
        return NULL;
    }
    for (i = 0; i <= len1; i++) {
        mat[i] = malloc((len2 + 1) * sizeof(edit));
        if (mat[i] == NULL) {
            for (j = 0; j < i; j++) {
                free(mat[j]);
            }
            free(mat);
            return NULL;
        }
    }
    for (i = 0; i <= len1; i++) {
        mat[i][0].score = i;
        mat[i][0].prev = NULL;
        mat[i][0].arg1 = 0;
        mat[i][0].arg2 = 0;
    }
 
    for (j = 0; j <= len2; j++) {
        mat[0][j].score = j;
        mat[0][j].prev = NULL;
        mat[0][j].arg1 = 0;
        mat[0][j].arg2 = 0;
    }
    return mat; 
}
 
unsigned int levenshtein_distance(const char *str1, const char *str2, size_t len1, size_t len2)
{
    unsigned int i, distance;
    edit **mat;
 
    if(len1 == 0) {
        return len2;
    }
    if(len2 == 0) {
        return len1;
    }
    mat = levenshtein_matrix_create(str1, len1, str2, len2);
    if(!mat) {
        return 0;
    }
    distance = levenshtein_matrix_calculate(mat, str1, len1, str2, len2);
    for(i = 0; i <= len1; i++) {
        free(mat[i]);
    }
    free(mat);
    return distance;
}

#if 0
int main(void)
{
    printf("%d\n", levenshtein_distance("How", "how"));
    printf("%d\n", levenshtein_distance("How", "woh"));
    printf("%d\n", levenshtein_distance("How", "Howw"));
    printf("%d\n", levenshtein_distance("How", "Testing 123"));
    printf("%d\n", levenshtein_distance("How", "Where"));
    printf("%d\n", levenshtein_distance("How", "Hello!"));
    printf("%d\n", levenshtein_distance("How", "How"));
    return 0;
}
#endif 
