# WordMesh

WordMesh is a network of data that is related by input order. It builds a
memory of data sequences to create an N-back context (N can be specified while
compiling the dataset). The memory works as a way to keep the context of a
sequence of input or output events. An example of this would be keeping the
same topic while generating output in response to input while allowing it to
drift the more it generates, depending on the N-back setting.

## Concepts
* Data is stored in a `wordbank` binary tree for quick lookup.
* Each `wordbank` contains a list of `trees` that reference back to it
(`tree` references are internally stored as a binary tree for quick access).
* A `tree` is a sequence of inputs that are stored in order of input and
reference a `wordbank` item.
* Each `tree` holds references to previous and next items, which are items that
were seen before (previous) or after (next) in an input sequence.
* A `tree` is stored in `containers`, which provide a quick way to determine if a
word is in a `tree` at a given depth.
* A `container` is sorted by the `tree`'s `wordbank` data reference.
* Depth refers to the index of the order of the input sequence. For example, a
sequence of input: `Hello,` `World!`: `Hello,` is at depth 0 while `World!` is
at depth 1.
* A `forest` acts as a wrapper containing `wordbank`, `tree`, and `container`.
It simplifies the number of function arguments needed since they are contained
within it's structure. Works similar to a class with local public variables.


![test](/doc/chart0.png)

## Example Usage

See [src/forest.c](/src/forest.c#L220) `test_forest()` function for a full example of compiling
a dataset.

```C
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
    struct tree *tree = NULL;
    struct tree *start_tree = NULL;

    // Example of creating a tree from sentence: "Have a good day!"
    // Internally this adds new words to the wordbank
    tree_insert(
        forest,         // <- forest wrapper with internal tracking
        "Have",         // <- input sequence item 0
        strlen("Have"), // <- length of input
        0               // <- depth (item 0)
    );
    tree_insert(forest, "a", strlen("a"), 1);
    tree_insert(forest, "good", strlen("good"), 2);
    tree_insert(forest, "day!", strlen("day!"), 3);

    printf("last_item: %s\n", f->last_item->word->data);

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

```


# Todo

* API: Daemonize and provide a basic HTTP access REST API.
* Clustering: compile dataset over a network of worker nodes.
* Config file format, options, and parsing. -OR- program arguments may suffice.
* Serialize and store on-disk to avoid high memory consumption and long
re-compile times. Requires custom internal database engine. Example: convert C
data structures to on-disk format (e.g. pointers become encoded integers and
reference offsets of a file). Make this a selectable option (select: memory
and/or disk).
* Swap out basic binary tree for self-balancing or something more efficent.
* Direct Python module integration.

