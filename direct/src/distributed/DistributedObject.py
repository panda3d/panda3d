"""DistributedObject module: contains the DistributedObject class"""

from PandaObject import *

class DistributedObject(PandaObject):
    """Distributed Object class:"""
    def __init__(self, doId, di):
        self.doId=doId
        self.zone=di.getUint32()
        assert(di.getRemainingSize() == 0)
