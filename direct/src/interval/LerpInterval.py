"""LerpInterval module: contains the LerpInterval class"""

import Interval
import Lerp
import PosHprInterval

class LerpInterval(Interval.Interval):

    # special methods
    
    def __init__(self, name, node, duration, t0=0.0, 
				type=Interval.Interval.PrevEndRelative):
        """__init__(name, node, duration, t0, type)
        """
	self.name = name
	self.node = node	
	self.duration = duration
	self.startTime = t0
	self.type = type

class LerpPosHprInterval(LerpInterval):

    def __init__(self, name, node, pos, hpr, duration, t0=0.0, 
		type=Interval.Interval.PrevEndRelative, other=None,
			blendType='noBlend'):
	""" __init__(name, node, pos, hpr, duration, t0, type, other, 
					blendType)
	"""
        import PosHprLerpFunctor

	LerpInterval.__init__(self, name, node, duration, t0, type)
	self.pos = pos
	self.hpr = hpr

        if (other != None):
            # lerp wrt other
            startPos = self.node.getPos(other)
            startHpr = self.node.getHpr(other)
            functor = PosHprLerpFunctor.PosHprLerpFunctor(
                    self.node, startPos, pos,
                    startHpr, hpr, other)
        else:
            startPos = self.node.getPos()
            startHpr = self.node.getHpr()
            functor = PosHprLerpFunctor.PosHprLerpFunctor(
                    self.node, startPos, pos,
                    startHpr, hpr)

	self.blendType = self.__getBlend(blendType)
	self.lerp = Lerp.Lerp(functor, self.duration, self.blendType)

    def setT(self, t):
	""" setT(t)
	"""
	assert(t >= 0.0)
	if (t > self.duration):
	    self.lerp.setT(self.duration)
	else:
	    self.lerp.setT(t)

    def __getBlend(self, blendType):
        """__getBlend(self, string)
        Return the C++ blend class corresponding to blendType string
        """
        import LerpBlendHelpers

        if (blendType == "easeIn"):
            return LerpBlendHelpers.LerpBlendHelpers.easeIn
        elif (blendType == "easeOut"):
            return LerpBlendHelpers.LerpBlendHelpers.easeOut
        elif (blendType == "easeInOut"):
            return LerpBlendHelpers.LerpBlendHelpers.easeInOut
        elif (blendType == "noBlend"):
            return LerpBlendHelpers.LerpBlendHelpers.noBlend
        else:
            raise Exception("Error: NodePath.__getBlend: Unknown blend type")
