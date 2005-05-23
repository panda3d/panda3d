from otp.ai.AIBaseGlobal import *
import DistributedNodeAI
import DistributedSmoothNodeBase

class DistributedSmoothNodeAI(DistributedNodeAI.DistributedNodeAI,
                              DistributedSmoothNodeBase.DistributedSmoothNodeBase):
    
    def __init__(self, air, name=None):
        DistributedNodeAI.DistributedNodeAI.__init__(self, air, name)
        DistributedSmoothNodeBase.DistributedSmoothNodeBase.__init__(self)

        self.cnode.setRepository(air, 1, air.ourChannel)

    def delete(self):
        DistributedSmoothNodeBase.DistributedSmoothNodeBase.delete(self)
        DistributedNodeAI.DistributedNodeAI.delete(self)

    # distributed set pos and hpr functions
    # these are invoked by the DC system
    # 'send' (d_set*) versions are inherited from DistributedSmoothNodeBase
    def setSmStop(self, t):
        pass

    # These have their FFI functions exposed for efficiency
    def setSmH(self, h, t):
        self.setH(h)

    def setSmZ(self, z, t):
        self.setZ(z)
        
    def setSmXY(self, x, y, t):
        self.setX(x)
        self.setY(y)
        
    def setSmXZ(self, x, z, t):
        self.setX(x)
        self.setZ(z)
        
    def setSmPos(self, x, y, z, t):
        self.setPos(x,y,z)
        
    def setSmHpr(self, h, p, r, t):
        self.setHpr(h,p,r)
        
    def setSmXYH(self, x, y, h, t):
        self.setX(x)
        self.setY(y)
        self.setH(h)
        
    def setSmXYZH(self, x, y, z, h, t):
        self.setPos(x,y,z)
        self.setH(h)
        
    def setSmPosHpr(self, x, y, z, h, p, r, t):
        self.setPosHpr(x,y,z,h,p,r)
        
    def clearSmoothing(self, bogus = None):
        pass
    

    # Do we use these on the AIx?
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
