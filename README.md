# AcidMesh

AcidMesh is a network of data that is related by input order. It builds a
memory of data sequences to create an N-back context. The memory works as a way
to hold the context of a
sequence of input or output events. An example of this would be keeping the
same topic while generating output in response to input while allowing it to
drift the more it generates, depending on the N-back setting.


# Example Usage

## Python

Setup and install from source:
```bash
python3 -m venv venv
. venv/bin/activate
cd src/
make py
```

Example script:
```python
from acidmesh import Mesh

mesh = Mesh()

sentences = (
    ("Hello,", "world!"),
    ("Hello,", "how", "are", "you?"),
    ('What', 'time', 'is', 'it?'),
)

for sentence in sentences:
    depth = 0
    for word in sentence:
        mesh.sequence_insert(word, len(word), depth)
        depth += 1
    mesh.link_last_contexts()

mesh.dump()

finds = ('Hello,', 'What', 'are', 'is')
for find in finds:
    f = mesh.data_find(find, len(find));
    pprint(f)
```


## Example C

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


# Todo

* API: Daemonize and provide a basic HTTP access REST API.
* Clustering: compile dataset over a network of worker nodes.
* Config file format, options, and parsing. -OR- program arguments may suffice.
* Serialize and store on-disk to avoid high memory consumption and long
re-compile times. Requires custom internal database engine. Example: convert C
data structures to on-disk format (e.g. pointers become encoded integers and
reference offsets of a file). Make this a selectable option (select: memory
and/or disk).
* Swap out basic binary tree for self-balancing or something more performant.
* Direct Python module integration.

