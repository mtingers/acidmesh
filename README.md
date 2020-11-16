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
from pprint import pprint
from acidmesh import Mesh

question_answer = (
	('Who are you?', 'Your mom.'),
	('Who are you?', 'Your dad.'),
	('How are you', 'I do not know anymore...'),
	('How are you really?',  'Fanstastic!'),
	('Where are you going?', 'I am lost!'),
	('Where are you going?', 'To the store.'),
    ('What is the answer to the ultimate question of life, the universe, and everything?',
        'The number 42.'),
)

mesh = Mesh()

# Insert question/answer pairs
for n, qa in enumerate(question_answer):
    question = qa[0]
    answer = qa[1]
    for depth, word in enumerate(question.strip().split(' ')):
        mesh.sequence_insert(word, depth)
    for depth, word in enumerate(answer.strip().split(' ')):
        mesh.sequence_insert(word, depth)
    mesh.link_last_contexts()

###########################################

# Ask a question and get a rated response list
questions = (
    'How are you?',
    'How are you',
    'Where you going?',
    'Who are you?',
    'Who are',
    'What is the answer to everything?',
)

for question in questions:
    print('-'*80)
    print('Question:', question)
    response = mesh.generate_response(question)
    if response:
        pprint(response)
    else:
        print('-- No results --')
```
```
--------------------------------------------------------------------------------
Question: How are you?
[{'data': 'I do not know anymore... ', 'rating': 5.8148},
 {'data': 'Your mom. ', 'rating': 3.6111},
 {'data': 'Your dad. ', 'rating': 3.6111},
 {'data': 'Fanstastic! ', 'rating': -3.3333}]
--------------------------------------------------------------------------------
Question: How are you
[{'data': 'I do not know anymore... ', 'rating': 13.5},
 {'data': 'Fanstastic! ', 'rating': 4.3519},
 {'data': 'To the store. ', 'rating': -7.3889},
 {'data': 'I am lost! ', 'rating': -7.3889},
 {'data': 'Your dad. ', 'rating': -11.4815},
 {'data': 'Your mom. ', 'rating': -11.4815}]
--------------------------------------------------------------------------------
Question: Where you going?
[{'data': 'To the store. ', 'rating': 12.1111},
 {'data': 'I am lost! ', 'rating': 12.1111},
 {'data': 'I do not know anymore... ', 'rating': -7.7778}]
--------------------------------------------------------------------------------
Question: Who are you?
[{'data': 'Your mom. ', 'rating': 17.2037},
 {'data': 'Your dad. ', 'rating': 17.2037},
 {'data': 'I do not know anymore... ', 'rating': -7.7778},
 {'data': 'I am lost! ', 'rating': -15.0741},
 {'data': 'To the store. ', 'rating': -15.0741},
 {'data': 'Fanstastic! ', 'rating': -16.9259}]
--------------------------------------------------------------------------------
Question: Who are
[{'data': 'Your dad. ', 'rating': 1.1111},
 {'data': 'Your mom. ', 'rating': 1.1111}]
--------------------------------------------------------------------------------
Question: What is the answer to everything?
[{'data': 'The number 42. ', 'rating': -16.1667}]
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

