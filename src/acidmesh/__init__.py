VERSION = '1.0.1'
__version__ = '1.0.1'

import cacidmesh as cwm

class Mesh:
    def __init__(self):
        self.mesh_id = cwm.mesh()

    def sequence_insert(self, data, index):
        cwm.sequence_insert(self.mesh_id, data, len(data), index)

    def dump(self):
        cwm.dump(self.mesh_id)

    def link_last_contexts(self):
        cwm.link_last_contexts(self.mesh_id)

    def delete(self):
        cwm.mesh_del(self.mesh_id)

    def data_find(self, data):
        return cwm.data_find(self.mesh_id, data, len(data))

    def generate(self, s1, s2):
        cwm.generate(self.mesh_id, s1, len(s1), s2, len(s2))

    def data_stats(self):
        cwm.data_stats(self.mesh_id)
