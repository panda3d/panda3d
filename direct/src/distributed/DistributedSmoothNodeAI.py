from AIBaseGlobal import *
import DistributedNodeAI
import DistributedSmoothNodeBase

class DistributedSmoothNodeAI(DistributedNodeAI.DistributedNodeAI,
                              DistributedSmoothNodeBase.\
                              DistributedSmoothNodeBase):
    def __init__(self, air, name=None):
        DistributedNodeAI.DistributedNodeAI.__init__(self, air, name)
        DistributedSmoothNodeBase.DistributedSmoothNodeBase.__init__(self)

    def delete(self):
        DistributedSmoothNodeBase.DistributedSmoothNodeBase.delete(self)
        DistributedNodeAI.DistributedNodeAI.delete(self)

    # distributed set pos and hpr functions
    # these are invoked by the DC system
    # 'send' (d_set*) versions are inherited from DistributedSmoothNodeBase
    def setSmStop(self, t):
        self.setComponentT(t)
    def setSmH(self, h, t):
        self.setComponentH(h)
        self.setComponentT(t)
    def setSmXY(self, x, y, t):
        self.setComponentX(x)
        self.setComponentY(y)
        self.setComponentT(t)
    def setSmXZ(self, x, z, t):
        self.setComponentX(x)
        self.setComponentZ(z)
        self.setComponentT(t)
    def setSmPos(self, x, y, z, t):
        self.setComponentX(x)
        self.setComponentY(y)
        self.setComponentZ(z)
        self.setComponentT(t)
    def setSmHpr(self, h, p, r, t):
        self.setComponentH(h)
        self.setComponentP(p)
        self.setComponentR(r)
        self.setComponentT(t)
    def setSmXYH(self, x, y, h, t):
        self.setComponentX(x)
        self.setComponentY(y)
        self.setComponentH(h)
        self.setComponentT(t)
    def setSmXYZH(self, x, y, z, h, t):
        self.setComponentX(x)
        self.setComponentY(y)
        self.setComponentZ(z)
        self.setComponentH(h)
        self.setComponentT(t)
    def setSmPosHpr(self, x, y, z, h, p, r, t):
        self.setComponentX(x)
        self.setComponentY(y)
        self.setComponentZ(z)
        self.setComponentH(h)
        self.setComponentP(p)
        self.setComponentR(r)
        self.setComponentT(t)
        
    def clearSmoothing(self, bogus = None):
        pass
    
    ### component set pos and hpr functions ###

    ### These are the component functions that are invoked
    ### remotely by the above composite functions.

    # on the AI, the components are assigned immediately
    def setComponentX(self, x):
        self.setX(x)
    def setComponentY(self, y):
        self.setY(y)
    def setComponentZ(self, z):
        self.setZ(z)
    def setComponentH(self, h):
        self.setH(h)
    def setComponentP(self, p):
        self.setP(p)
    def setComponentR(self, r):
        self.setR(r)
    def setComponentT(self, t):
        pass
