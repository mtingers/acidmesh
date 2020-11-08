# WordMesh

WordMesh is a network of data that is related by input order. It creates a
memory of data chains to create an N-back context (N can be specified while
compiling the dataset). The memory works as a way to keep the context of a
sequence of input or output events. An example of this would be keeping the
same topic while generating output in response to input while allowing it to
drift the more it generates.

## Concepts
* Data is stored in a "wordbank" binary tree for quick lookup.
* Each wordbank contains a list of "trees" that reference it.
* A tree is a sequence of inputs that are stored in order of input and
reference a wordbank item.
* Each tree holds references to previous and next items, which are items that
were seen before (previous) or after (next) in an input sequence.
* A tree is stored in "containers", which provide a quick way to determine if a
word is in a tree at a given depth.
* A container is another binary tree referencing the tree's wordbank item.
* Depth refers to the index of the order of the input sequence. For example, a
chain of input: "Hello," "World!": "Hello," is at depth 0 while "World!" is at
depth 1.

![test](/doc/chart0.png)
