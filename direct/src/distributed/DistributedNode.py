"""DistributedNode module: contains the DistributedNode class"""

from ShowBaseGlobal import *
import NodePath
import DistributedObject

class DistributedNode(DistributedObject.DistributedObject, NodePath.NodePath):
    """Distributed Node class:"""

    def __init__(self, cr):
        try:
            self.DistributedNode_initialized
        except:
            self.DistributedNode_initialized = 1
            DistributedObject.DistributedObject.__init__(self, cr)
        return None

    def disable(self):
        self.reparent(hidden)
        DistributedObject.DistributedObject.disable(self)

    def d_setPos(self, x, y, z):
        self.sendUpdate("setPos", [x, y, z])

    def d_setHpr(self, h, p, r):
        self.sendUpdate("setHpr", [h, p, r])

    def d_broadcastPosHpr(self):
        
        self.d_setPosHpr(self.getX(), self.getY(), self.getZ(),
                         self.getH(), self.getP(), self.getR())

    def d_setPosHpr(self, x, y, z, h, p, r):
        self.sendUpdate("setPosHpr", [x, y, z, h, p, r])
