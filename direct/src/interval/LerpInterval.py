"""LerpInterval module: contains the LerpInterval class"""

import Interval
import Lerp

class LerpInterval(Interval.Interval):

    # special methods
    
    def __init__(self, name, node, duration, functor, t0=0.0, 
			type=Interval.PREVIOUS_END, blendType='noBlend'):
        """__init__(name, node, duration, functor, t0, type, blendType)
        """
	self.name = name
	self.node = node	
	self.duration = duration
	self.startTime = t0
	self.type = type
	self.lerp = Lerp.Lerp(functor, duration, self.__getBlend(blendType))

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
            raise Exception(
		'Error: LerpInterval.__getBlend: Unknown blend type')

class LerpPosHprInterval(LerpInterval):

    def __init__(self, name, node, pos, hpr, duration, t0=0.0, 
		type=Interval.PREVIOUS_END, other=None, blendType='noBlend'):
	""" __init__(name, node, pos, hpr, duration, t0, type, other, 
					blendType)
	"""
        import PosHprLerpFunctor

	self.node = node
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

	LerpInterval.__init__(self, name, node, duration, functor, t0, 
						type, blendType)


class LerpPosInterval(LerpInterval):

    def __init__(self, name, node, pos, duration, t0=0.0, 
		type=Interval.PREVIOUS_END, other=None, blendType='noBlend'):
	""" __init__(name, node, pos, duration, t0, type, other, blendType)
	"""
        import PosLerpFunctor

	self.node = node
	self.pos = pos
        if (other != None):
            # lerp wrt other
            startPos = self.node.getPos(other)
            functor = PosLerpFunctor.PosLerpFunctor(
                    self.node, startPos, pos, other)
        else:
            startPos = self.node.getPos()
            functor = PosLerpFunctor.PosLerpFunctor(
                    self.node, startPos, pos)

	LerpInterval.__init__(self, name, node, duration, functor, t0, 
							type, blendType)

class LerpHprInterval(LerpInterval):

    def __init__(self, name, node, hpr, duration, t0=0.0, 
		type=Interval.PREVIOUS_END, other=None, blendType='noBlend'):
	""" __init__(name, node, hpr, duration, t0, type, other, blendType)
	"""
        import HprLerpFunctor

	self.node = node
	self.hpr = hpr
        if (other != None):
            # lerp wrt other
            startHpr = self.node.getHpr(other)
            functor = HprLerpFunctor.HprLerpFunctor(
                    self.node, startHpr, hpr, other)
        else:
            startHpr = self.node.getHpr()
            functor = HprLerpFunctor.HprLerpFunctor(
                    self.node, startHpr, hpr)

	LerpInterval.__init__(self, name, node, duration, functor, t0, 
							type, blendType)
