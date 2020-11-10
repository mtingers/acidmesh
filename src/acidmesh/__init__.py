VERSION = '1.0.1'
__version__ = '1.0.1'

import cacidmesh as cwm

class Mesh:
    def __init__(self):
        self.mesh_id = cwm.forest()

    def sequence_insert(self, s, slen, index):
        cwm.sequence_insert(self.mesh_id, s, slen, index)

    def dump(self):
        cwm.dump(self.mesh_id)

    def link_last_contexts(self):
        cwm.link_last_contexts(self.mesh_id)

    def delete(self):
        cwm.forest_del(self.mesh_id)

