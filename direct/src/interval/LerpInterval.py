"""LerpInterval module: contains the LerpInterval class"""

import Interval
import Lerp

class LerpInterval(Interval.Interval):

    # special methods
    
    def __init__(self, name, node, duration):
        """__init__(name, node, duration)
        """
	self.name = name
	self.node = node	
	self.duration = duration

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	pass

class LerpPosHprInterval(LerpInterval):

    def __init__(self, name, node, pos, hpr, duration, other=None,
			blendType='noBlend'):
	""" __init__(name, node, pos, hpr, duration, other, blendType)
	"""
        import PosHprLerpFunctor

	LerpInterval.__init__(self, name, node, duration)
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
	if (t > self.duration):
	    return
	assert(t >= 0)
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
