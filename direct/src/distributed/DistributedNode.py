"""DistributedNode module: contains the DistributedNode class"""

import NodePath
import DistributedObject

class DistributedNode(DistributedObject.DistributedObject, NodePath.NodePath):
    """Distributed Node class:"""

    def __init__(self):
        try:
            self.DistributedNode_initialized
        except:
            self.DistributedNode_initialized = 1
        return None

    def generateInit(self, di):
        DistributedObject.DistributedObject.generateInit(self, di)

    def d_setPos(self, x, y, z):
        self.sendUpdate("setPos", [x, y, z])

    def d_setHpr(self, h, p, r):
        self.sendUpdate("setHpr", [h, p, r])

    def d_setPosHpr(self):
        
        self.d_setPos_Hpr(self.getX(), self.getY(), self.getZ(),
                          self.getH(), self.getP(), self.getR())

    def d_setPosHpr(self, x, y, z, h, p, r):
        self.sendUpdate("setPosHpr", [x, y, z, h, p, r])
