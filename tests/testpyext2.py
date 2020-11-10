#
# Test the acidmesh python C extension via Python wrapper
#

from acidmesh import Mesh

f = Mesh()

words = [
    ("Hello,", "world!"),
    ("Hello,", "how", "are", "you?"),
    ('What', 'time', 'is', 'it?'),
]

for w in words:
    i = 0
    for x in w:
        f.sequence_insert(x, len(x), i)
        i += 1
    f.link_last_contexts()

f.dump()
