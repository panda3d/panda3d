"""BasicEntities module: contains fundamental entity types and base classes"""

import Entity
import DistributedEntity
import NodePath

# this is an internal class, do not instantiate.
class privNodePathImpl(NodePath.NodePath):
    def __init__(self, name):
        node = hidden.attachNewNode(name)
        NodePath.NodePath.__init__(self, node)

    def initializeEntity(self):
        self.callSetters(('pos','x','y','z',
                          'hpr','h','p','r',
                          'scale','sx','sx','sz'))

        if hasattr(self, 'parent'):
            self.level.requestReparent(self, self.parent)
        else:
            self.level.requestReparent(self, 'zone')
        
    def destroy(self):
        self.removeNode()

class NodePathEntity(Entity.Entity, privNodePathImpl):
    """This is an entity that represents a NodePath on the client.
    It may be instantiated directly or used as a base class for other
    entity types."""
    def __init__(self, level, entId):
        Entity.Entity.__init__(self, level, entId)
        privNodePathImpl.__init__(self, str(self))
        self.initializeEntity()

    def initializeEntity(self):
        Entity.Entity.initializeEntity(self)
        privNodePathImpl.initializeEntity(self)

    def destroy(self):
        Entity.Entity.destroy(self)
        privNodePathImpl.destroy(self)

class DistributedNodePathEntity(DistributedEntity.DistributedEntity,
                                privNodePathImpl):
    """This is a distributed version of NodePathEntity. It should not
    be instantiated directly; derive your client-side distEntity from
    this class instead of DistributedEntity."""
    def __init__(self, cr):
        DistributedEntity.DistributedEntity.__init__(self, cr)
        privNodePathImpl.__init__(self, 'DistributedNodePathEntity')
        
    def initializeEntity(self):
        DistributedEntity.DistributedEntity.initializeEntity(self)
        privNodePathImpl.initializeEntity(self)

    def destroy(self):
        DistributedEntity.DistributedEntity.destroy(self)
        privNodePathImpl.destroy(self)
