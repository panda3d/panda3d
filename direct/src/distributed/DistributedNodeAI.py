from AIBaseGlobal import *
import DistributedObjectAI
import Task

class DistributedNodeAI(DistributedObjectAI.DistributedObjectAI):
    def __init__(self, air):
        DistributedObjectAI.DistributedObjectAI.__init__(self, air)

    def delete(self):
        DistributedObjectAI.DistributedObjectAI.delete(self)

    ### setParent ###

    def d_setParent(self, parentString):
        if type(parentString) == type(''):
            self.sendUpdate("setParentStr", [parentString])
        else:
            self.sendUpdate("setParent", [parentString])

    def setParent(self, parentString):
        pass

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
        pass

    def d_setXY(self, x, y):
        self.sendUpdate("setXY", [x, y])

    # setPos provided by NodePath
    def d_setPos(self, x, y, z):
        self.sendUpdate("setPos", [x, y, z])

    # setHpr provided by NodePath
    def d_setHpr(self, h, p, r):
        self.sendUpdate("setHpr", [h, p, r])

    def setXYH(self, x, y, h):
        pass
        
    def d_setXYH(self, x, y, h):
        self.sendUpdate("setXYH", [x, y, h])

    def setXYZH(self, x, y, z, h):
        pass

    def d_setXYZH(self, x, y, z, h):
        self.sendUpdate("setXYZH", [x, y, z, h])

    # setPosHpr provided by NodePath
    def d_setPosHpr(self, x, y, z, h, p, r):
        self.sendUpdate("setPosHpr", [x, y, z, h, p, r])


