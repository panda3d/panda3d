"""BasicEntities module: contains fundamental entity types and base classes"""

import Entity
import DistributedEntity
import NodePath

# this is an abstract class, do not instantiate.
class NodePathAttribs:
    """Derive from this class to give an entity the behavior of a
    NodePath, without necessarily deriving from NodePath. Derived class
    must implement getNodePath()."""
    __attribs__ = (
        'parent', 'pos', 'hpr',
        )
    def initNodePathAttribs(self, doReparent=1):
        """Call this after the entity has been initialized"""
        self.callSetters('pos','x','y','z',
                         'hpr','h','p','r',
                         'scale','sx','sy','sz')
        if doReparent and hasattr(self, 'parent'):
            self.level.requestReparent(self.getNodePath(), self.parent)

    def reparentTo(self, *args): self.getNodePath().reparentTo(*args)

    def setPos(self, *args): self.getNodePath().setPos(*args)
    def setX(self, *args): self.getNodePath().setX(*args)
    def setY(self, *args): self.getNodePath().setY(*args)
    def setZ(self, *args): self.getNodePath().setZ(*args)

    def setHpr(self, *args): self.getNodePath().setHpr(*args)
    def setH(self, *args): self.getNodePath().setH(*args)
    def setP(self, *args): self.getNodePath().setP(*args)
    def setR(self, *args): self.getNodePath().setR(*args)

    def setScale(self, *args): self.getNodePath().setScale(*args)
    def setSx(self, *args): self.getNodePath().setSx(*args)
    def setSy(self, *args): self.getNodePath().setSy(*args)
    def setSz(self, *args): self.getNodePath().setSz(*args)
    
class privNodePathImpl(NodePath.NodePath):
    __attribs__ = (
        'parent', 'pos', 'hpr',
        )
    def __init__(self, name):
        node = hidden.attachNewNode(name)
        NodePath.NodePath.__init__(self, node)

    def initNodePathAttribs(self):
        """Call this after the entity has been initialized, and all
        of its attributes have been set"""
        self.callSetters('pos','x','y','z',
                         'hpr','h','p','r',
                         'scale','sx','sy','sz')
        self.level.requestReparent(self, self.parent)
        
    def destroy(self):
        self.removeNode()

    def getNodePath(self):
        return self

class NodePathEntity(Entity.Entity, privNodePathImpl):
    """This is an entity that represents a NodePath on the client.
    It may be instantiated directly or used as a base class for other
    entity types."""
    def __init__(self, level, entId):
        privNodePathImpl.__init__(self, '')
        Entity.Entity.__init__(self, level, entId)
        self.setName(str(self))
        privNodePathImpl.initNodePathAttribs(self)

    def destroy(self):
        Entity.Entity.destroy(self)
        privNodePathImpl.destroy(self)

    def getNodePath(self):
        return self

class DistributedNodePathEntity(DistributedEntity.DistributedEntity,
                                privNodePathImpl):
    """This is a distributed version of NodePathEntity. It should not
    be instantiated directly; derive your client-side distEntity from
    this class instead of DistributedEntity."""
    def __init__(self, cr):
        DistributedEntity.DistributedEntity.__init__(self, cr)
        privNodePathImpl.__init__(self, 'DistributedNodePathEntity')

    def announceGenerate(self):
        DistributedEntity.DistributedEntity.announceGenerate(self)
        privNodePathImpl.initNodePathAttribs(self)
        
    def destroy(self):
        DistributedEntity.DistributedEntity.destroy(self)
        privNodePathImpl.destroy(self)

    def getNodePath(self):
        return self
