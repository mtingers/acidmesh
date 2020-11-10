#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "forest.h"
#include "wordbank.h"
#include "container.h"

int main()
{
    struct forest *forest = forest_init();

    // Example of creating a tree from sentence: "Have a good day!"
    // Internally this adds new words to the wordbank
    tree_insert(
        forest,         // <- forest
        "Have",         // <- input sequence item 0
        strlen("Have"), // <- length of input
        0,              // <- depth (item 0)
    );
    tree_insert(forest, "a", strlen("a"), 1);
    tree_insert(forest, "good", strlen("good"), 2);
    tree_insert(forest, "day!", strlen("day!"), 3);

    // Example of inserting a word into the wordbank, detached from any tree
    struct word *w;
    w = word_insert(forest, "Test123", strlen("Test123"));
    // Example of searching for a word in the wordbank
    w = word_find(forest, "Test123", strlen("Test123"));
    if(w) {
        printf("Found word: %s\n", w->data);
    }
    return 0;
}
