"""DistributedNode module: contains the DistributedNode class"""

from ShowBaseGlobal import *
import NodePath
import DistributedObject
import Task

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
        self.reparentTo(hidden)
        DistributedObject.DistributedObject.disable(self)

    def delete(self):
        try:
            self.DistributedNode_deleted
        except:
            self.DistributedNode_deleted = 1
            if not self.isEmpty():
                self.removeNode()
            DistributedObject.DistributedObject.delete(self)

    def generate(self):
        DistributedObject.DistributedObject.generate(self)

    ### setParent ###

    def b_setParent(self, parentString):
        self.setParent(parentString)
        self.d_setParent(parentString)
        return None

    def d_setParent(self, parentString):
        self.sendUpdate("setParent", [parentString])
        return None

    def setParent(self, parentString):
        # print "setting parent of %s to %s" % (self.getName(), parentString)
        assert(self.cr.name2nodePath.has_key(parentString))
        parent = self.cr.name2nodePath[parentString]
        self.wrtReparentTo(parent)
        return None

    ###### set pos and hpr functions #######

    # setX provided by NodePath
    def d_setX(self, x):
        self.sendUpdate("setX", [x])

    # setY provided by NodePath
    def d_setY(self, y):
        self.sendUpdate("setY", [y])
    
    # setZ provided by NodePath
    def d_setZ(self, z):
        self.sendUpdate("setZ", [z])
    
    # setH provided by NodePath
    def d_setH(self, h):
        self.sendUpdate("setH", [h])
    
    # setP provided by NodePath
    def d_setP(self, p):
        self.sendUpdate("setP", [p])
    
    # setR provided by NodePath
    def d_setR(self, r):
        self.sendUpdate("setR", [r])

    def setXY(self, x, y):
        self.setX(x)
        self.setY(y)
    def d_setXY(self, x, y):
        self.sendUpdate("setXY", [x, y])

    def setXZ(self, x, z):
        self.setX(x)
        self.setZ(z)
    def d_setXZ(self, x, z):
        self.sendUpdate("setXZ", [x, z])

    # setPos provided by NodePath
    def d_setPos(self, x, y, z):
        self.sendUpdate("setPos", [x, y, z])

    # setHpr provided by NodePath
    def d_setHpr(self, h, p, r):
        self.sendUpdate("setHpr", [h, p, r])

    def setXYH(self, x, y, h):
        self.setX(x)
        self.setY(y)
        self.setH(h)
    def d_setXYH(self, x, y, h):
        self.sendUpdate("setXYH", [x, y, h])

    def setXYZH(self, x, y, z, h):
        self.setPos(x, y, z)
        self.setH(h)
    def d_setXYZH(self, x, y, z, h):
        self.sendUpdate("setXYZH", [x, y, z, h])

    # setPosHpr provided by NodePath
    def d_setPosHpr(self, x, y, z, h, p, r):
        self.sendUpdate("setPosHpr", [x, y, z, h, p, r])

