from otp.ai.AIBaseGlobal import *
import DistributedNodeAI
import DistributedSmoothNodeBase

class DistributedSmoothNodeAI(DistributedNodeAI.DistributedNodeAI,
                              DistributedSmoothNodeBase.DistributedSmoothNodeBase):

    def __init__(self, air, name=None):
        DistributedNodeAI.DistributedNodeAI.__init__(self, air, name)
        DistributedSmoothNodeBase.DistributedSmoothNodeBase.__init__(self)

    def generate(self):
        DistributedNodeAI.DistributedNodeAI.generate(self)
        DistributedSmoothNodeBase.DistributedSmoothNodeBase.generate(self)
        self.cnode.setRepository(self.air, 1, self.air.ourChannel)

    def disable(self):
        DistributedSmoothNodeBase.DistributedSmoothNodeBase.disable(self)
        DistributedNodeAI.DistributedNodeAI.disable(self)

    def delete(self):
        DistributedSmoothNodeBase.DistributedSmoothNodeBase.delete(self)
        DistributedNodeAI.DistributedNodeAI.delete(self)

    # distributed set pos and hpr functions
    # these are invoked by the DC system
    # 'send' (d_set*) versions are inherited from DistributedSmoothNodeBase
    def setSmStop(self, t=None):
        pass

    # These have their FFI functions exposed for efficiency
    def setSmH(self, h, t=None):
        self.setH(h)

    def setSmZ(self, z, t=None):
        self.setZ(z)

    def setSmXY(self, x, y, t=None):
        self.setX(x)
        self.setY(y)

    def setSmXZ(self, x, z, t=None):
        self.setX(x)
        self.setZ(z)

    def setSmPos(self, x, y, z, t=None):
        self.setPos(x, y, z)

    def setSmHpr(self, h, p, r, t=None):
        self.setHpr(h, p, r)

    def setSmXYH(self, x, y, h, t=None):
        self.setX(x)
        self.setY(y)
        self.setH(h)

    def setSmXYZH(self, x, y, z, h, t=None):
        self.setPos(x, y, z)
        self.setH(h)

    def setSmPosHpr(self, x, y, z, h, p, r, t=None):
        self.setPosHpr(x, y, z, h, p, r)

    def setSmPosHprL(self, l, x, y, z, h, p, r, t=None):
        self.setPosHpr(x, y, z, h, p, r)

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
    def setComponentL(self, l):
        pass
    def setComponentT(self, t):
        pass

    def getComponentX(self):
        return self.getX()
    def getComponentY(self):
        return self.getY()
    def getComponentZ(self):
        return self.getZ()
    def getComponentH(self):
        return self.getH()
    def getComponentP(self):
        return self.getP()
    def getComponentR(self):
        return self.getR()
    def getComponentL(self):
        if (self.zoneId):
            return self.zoneId
        else:
            # we can't send None over the wire which self.zoneId can sometimes be
            return 0
    def getComponentT(self):
        return 0

