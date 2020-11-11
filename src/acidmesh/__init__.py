VERSION = '1.0.1'
__version__ = '1.0.1'

import cacidmesh as cwm

class Mesh:
    def __init__(self):
        self.mesh_id = cwm.mesh()

    def sequence_insert(self, data, data_len, index):
        cwm.sequence_insert(self.mesh_id, data, data_len, index)

    def dump(self):
        cwm.dump(self.mesh_id)

    def link_last_contexts(self):
        cwm.link_last_contexts(self.mesh_id)

    def delete(self):
        cwm.mesh_del(self.mesh_id)

    def data_find(self, data, data_len):
        return cwm.data_find(self.mesh_id, data, data_len)

