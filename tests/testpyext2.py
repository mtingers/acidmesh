#
# Test the wordmesh python C extension via Python wrapper
#

from wordmesh import Forest

f = Forest()

words = [
    ("Hello,", "world!"),
    ("Hello,", "how", "are", "you?"),
    ('What', 'time', 'is', 'it?'),
]

for w in words:
    i = 0
    for x in w:
        f.tree_insert(x, len(x), i)
        i += 1
    f.link_last_contexts()

f.dump()
