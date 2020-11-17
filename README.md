# AcidMesh

AcidMesh is a network of data that is related by input order. It builds a
memory of data sequences to create an N-back context. The memory works as a way
to hold the context of a
sequence of input or output events. An example of this would be keeping the
same topic while generating output in response to input while allowing it to
drift the more it generates, depending on the N-back setting.


# Example Usage

## Python

Setup and install module from source:
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


# Todo

- [ ] API: Daemonize and provide a basic HTTP access REST API.
- [ ] Clustering: compile dataset over a network of worker nodes.
- [ ] Config file format, options, and parsing. -OR- program arguments may suffice.
- [ ] Serialize and store on-disk to avoid high memory consumption and long
re-compile times. Requires custom internal database engine. Example: convert C
data structures to on-disk format (e.g. pointers become encoded integers and
reference offsets of a file). Make this a selectable option (select: memory
and/or disk). Might cause a headache moving pointers in other locations.
- [ ] Swap out basic binary tree for self-balancing or something more performant.
- [x] Direct Python module integration.


