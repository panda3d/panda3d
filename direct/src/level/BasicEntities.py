"""BasicEntities module: contains fundamental entity types and base classes"""

import Entity
import DistributedEntity
import NodePath

# base class for entities that support NodePath attributes
# *** Don't derive directly from this class; derive from the appropriate
# specialized class from the classes defined below.
class NodePathEntityBase:
    # we don't call this __init__ because it doesn't have to be called
    # upon object init
    def initNodePathAttribs(self, doReparent=1):
        """Call this after the entity has been initialized"""
        self.callSetters('pos','x','y','z',
                         'hpr','h','p','r',
                         'scale','sx','sy','sz')
        if doReparent:
            self.callSetters('parentEntId')

        self.getNodePath().setName('%s-%s' %
                                   (self.__class__.__name__, self.entId))

        if __dev__:
            # for the editor
            self.getNodePath().setTag('entity', '1')

    def setParentEntId(self, parentEntId):
        self.parentEntId = parentEntId
        self.level.requestReparent(self, self.parentEntId)

    def destroy(self):
        if __dev__:
            # for the editor
            self.getNodePath().clearTag('entity')

# Entities that already derive from NodePath and Entity should derive
# from this class
class NodePathAttribs(NodePathEntityBase):
    def initNodePathAttribs(self, doReparent=1):
        NodePathEntityBase.initNodePathAttribs(self, doReparent)

    def destroy(self):
        NodePathEntityBase.destroy(self)

    def getNodePath(self):
        return self

# Entities that already derive from Entity, and do not derive from NodePath,
# but want to be a NodePath, should derive from this.
class NodePathAndAttribs(NodePathEntityBase, NodePath.NodePath):
    def __init__(self):
        node = hidden.attachNewNode('EntityNodePath')
        NodePath.NodePath.__init__(self, node)

    def initNodePathAttribs(self, doReparent=1):
        NodePathEntityBase.initNodePathAttribs(self, doReparent)

    def destroy(self):
        NodePathEntityBase.destroy(self)
        self.removeNode()

    def getNodePath(self):
        return self
        
# Entities that already derive from Entity, and do not derive from NodePath,
# but HAVE a NodePath that they want to represent them, should derive from
# this. They must define getNodePath(), which should return their 'proxy'
# NodePath instance.
class NodePathAttribsProxy(NodePathEntityBase):
    def initNodePathAttribs(self, doReparent=1):
        """Call this after the entity has been initialized"""
        NodePathEntityBase.initNodePathAttribs(self, doReparent)
        assert self.getNodePath() != self

    def destroy(self):
        NodePathEntityBase.destroy(self)

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
    
    def reparentTo(self, *args): self.getNodePath().reparentTo(*args)

# This is an entity that represents a NodePath on the client.
# It may be instantiated directly or used as a base class for other
# entity types that 'are' NodePaths.
class NodePathEntity(Entity.Entity, NodePath.NodePath, NodePathAttribs):
    def __init__(self, level, entId):
        node = hidden.attachNewNode('NodePathEntity')
        NodePath.NodePath.__init__(self, node)
        Entity.Entity.__init__(self, level, entId)
        self.initNodePathAttribs(self)

    def destroy(self):
        NodePathAttribs.destroy(self)
        Entity.Entity.destroy(self)
        self.removeNode()

# This is a distributed version of NodePathEntity. It should not
# be instantiated directly; distributed entities that are also NodePaths
# may derive from this instead of DistributedEntity.
class DistributedNodePathEntity(DistributedEntity.DistributedEntity,
                                NodePath.NodePath, NodePathAttribs):
    def __init__(self, cr):
        DistributedEntity.DistributedEntity.__init__(self, cr)

    def generateInit(self):
        DistributedEntity.DistributedEntity.generateInit(self)
        node = hidden.attachNewNode('DistributedNodePathEntity')
        NodePath.NodePath.__init__(self, node)

    def announceGenerate(self):
        DistributedEntity.DistributedEntity.announceGenerate(self)
        self.initNodePathAttribs(self)

    def delete(self):
        self.removeNode()
        DistributedEntity.DistributedEntity.delete(self)
