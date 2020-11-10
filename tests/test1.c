#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "mesh.h"
#include "datatree.h"
#include "container.h"

int main()
{
    struct mesh *mesh = mesh_init();

    // Example of creating a tree from sentence: "Have a good day!"
    // Internally this adds new words to the datatree
    sequence_insert(
        mesh,         // <- mesh
        "Have",         // <- input sequence item 0
        strlen("Have"), // <- length of input
        0,              // <- depth (item 0)
    );
    sequence_insert(mesh, "a", strlen("a"), 1);
    sequence_insert(mesh, "good", strlen("good"), 2);
    sequence_insert(mesh, "day!", strlen("day!"), 3);

    // Example of inserting a word into the datatree, detached from any sequence
    struct data *w;
    w = data_insert(mesh, "Test123", strlen("Test123"));
    // Example of searching for a word in the datatree
    w = data_find(mesh, "Test123", strlen("Test123"));
    if(w) {
        printf("Found word: %s\n", w->data);
    }
    return 0;
}
