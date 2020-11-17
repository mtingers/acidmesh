# Example C

See [src/mesh.c](/src/mesh.c#L220) `test_mesh()` function for a full example of
compiling a dataset. Or see [tests/test1.c](/tests/test1.c) for a basic
example.

# Concepts
* Data is stored in a `datatree` binary tree.
* Each `datatree` contains a list of `sequence` references back to it
(`sequence` references are internally stored as a binary tree too).
* A `sequence` is a series of inputs that are stored in order of input and
reference a `datatree` item.
* Each `sequence` holds references to previous and next items, which are items that
were seen before (previous) or after (next) in an input sequence.
* A `sequence` is held within a `container` binary tree, which provide a quick way
to determine if a word is in a `sequence` at a given depth.
* A `container` is sorted by the `sequence`'s `datatree` data reference.
* Depth refers to the index of the order of the input sequence. For example, a
sequence of input: `Hello,` `World!`: `Hello,` is at depth 0 while `World!` is
at depth 1.
* A `mesh` acts as a wrapper containing `datatree`, `sequence`, and `container`.
It simplifies the number of function arguments needed since they are contained
within it's structure. Works similar to a class with local public variables.


