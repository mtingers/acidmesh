#
# Test the acidmesh python C extension directly
#

import cacidmesh as wm

f = wm.mesh()

words = [
    ("Hello,", "world!"),
    ("Hello,", "how", "are", "you?"),
    ('What', 'time', 'is', 'it?'),
]

for w in words:
    i = 0
    for x in w:
        wm.sequence_insert(f, x, len(x), i)
        i += 1
    wm.link_last_contexts(f)

wm.dump(f)
wm.dump(1000)
