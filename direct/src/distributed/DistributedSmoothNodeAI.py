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
        self._NodePath__overloaded_setH_ptrNodePath_float(h)

    def setSmZ(self, z, t):
        self._NodePath__overloaded_setZ_ptrNodePath_float(z)
        
    def setSmXY(self, x, y, t):
        self._NodePath__overloaded_setX_ptrNodePath_float(x)
        self._NodePath__overloaded_setY_ptrNodePath_float(y)
        
    def setSmXZ(self, x, z, t):
        self._NodePath__overloaded_setX_ptrNodePath_float(x)
        self._NodePath__overloaded_setZ_ptrNodePath_float(z)
        
    def setSmPos(self, x, y, z, t):
        self._NodePath__overloaded_setPos_ptrNodePath_float_float_float(x,y,z)
        
    def setSmHpr(self, h, p, r, t):
        self._NodePath__overloaded_setHpr_ptrNodePath_float_float_float(h,p,r)
        
    def setSmXYH(self, x, y, h, t):
        self._NodePath__overloaded_setX_ptrNodePath_float(x)
        self._NodePath__overloaded_setY_ptrNodePath_float(y)
        self._NodePath__overloaded_setH_ptrNodePath_float(h)
        
    def setSmXYZH(self, x, y, z, h, t):
        self._NodePath__overloaded_setPos_ptrNodePath_float_float_float(x,y,z)
        self._NodePath__overloaded_setH_ptrNodePath_float(h)
        
    def setSmPosHpr(self, x, y, z, h, p, r, t):
        self._NodePath__overloaded_setPosHpr_ptrNodePath_float_float_float_float_float_float(x,y,z,h,p,r)
        
    def clearSmoothing(self, bogus = None):
        pass
    

    # Do we use these on the AIx?
    def setComponentX(self, x):
        self._NodePath__overloaded_setX_ptrNodePath_float(x)
    def setComponentY(self, y):
        self._NodePath__overloaded_setY_ptrNodePath_float(y)
    def setComponentZ(self, z):
        self._NodePath__overloaded_setZ_ptrNodePath_float(z)
    def setComponentH(self, h):
        self._NodePath__overloaded_setH_ptrNodePath_float(h)
    def setComponentP(self, p):
        self._NodePath__overloaded_setP_ptrNodePath_float(p)
    def setComponentR(self, r):
        self._NodePath__overloaded_setR_ptrNodePath_float(r)
    def setComponentT(self, t):
        pass
