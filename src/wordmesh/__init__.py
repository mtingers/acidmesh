VERSION = '1.0.1'
__version__ = '1.0.1'

import cwordmesh as cwm

class Forest:
    def __init__(self):
        self.forest_id = cwm.forest()

    def tree_insert(self, s, slen, index):
        cwm.tree_insert(self.forest_id, s, slen, index)

    def dump(self):
        cwm.dump(self.forest_id)

    def link_last_contexts(self):
        cwm.link_last_contexts(self.forest_id)

    def delete(self):
        cwm.forest_del(self.forest_id)

