"""LerpInterval module: contains the LerpInterval class"""

from Interval import *

import Lerp

class LerpInterval(Interval):

    # special methods
    
    def __init__(self, name, duration, functor, blendType='noBlend'):
        """__init__(name, duration, functor, blendType)
        """
	self.lerp = Lerp.Lerp(functor, duration, self.__getBlend(blendType))
	Interval.__init__(self, name, duration)

    def setT(self, t, entry=0):
	""" setT(t)
	"""
	assert(t >= 0.0)
	if (entry == 1) and (t > self.duration):
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

    def __init__(self, name, node, pos, hpr, duration, startPos=None,
		startHpr=None, other=None, blendType='noBlend'): 
	""" __init__(name, node, pos, hpr, duration, other, blendType)
	"""
        import PosHprLerpFunctor

	assert(not node.isEmpty())
        if (other != None):
            # lerp wrt other
	    if (startPos == None):
            	startPos = node.getPos(other)
	    if (startHpr == None):
            	startHpr = node.getHpr(other)
            functor = PosHprLerpFunctor.PosHprLerpFunctor(
                    node, startPos, pos,
                    startHpr, hpr, other)
        else:
	    if (startPos == None):
            	startPos = node.getPos()
	    if (startHpr == None):
            	startHpr = node.getHpr()
            functor = PosHprLerpFunctor.PosHprLerpFunctor(
                    node, startPos, pos,
                    startHpr, hpr)

	LerpInterval.__init__(self, name, duration, functor, blendType)


class LerpPosInterval(LerpInterval):

    def __init__(self, name, node, pos, duration, startPos=None,
					other=None, blendType='noBlend'):
	""" __init__(name, node, pos, duration, other, blendType)
	"""
        import PosLerpFunctor

	assert(not node.isEmpty())
        if (other != None):
            # lerp wrt other
	    if (startPos == None):
                startPos = node.getPos(other)
            functor = PosLerpFunctor.PosLerpFunctor(
                    node, startPos, pos, other)
        else:
	    if (startPos == None):
            	startPos = node.getPos()
            functor = PosLerpFunctor.PosLerpFunctor(
                    node, startPos, pos)

	LerpInterval.__init__(self, name, duration, functor, blendType) 

class LerpHprInterval(LerpInterval):

    def __init__(self, name, node, hpr, duration, startHpr=None,
					other=None, blendType='noBlend'):
	""" __init__(name, node, hpr, duration, other, blendType)
	"""
        import HprLerpFunctor

	assert(not node.isEmpty())
        if (other != None):
            # lerp wrt other
	    if (startHpr == None):
                startHpr = node.getHpr(other)
            functor = HprLerpFunctor.HprLerpFunctor(
                    node, startHpr, hpr, other)
        else:
	    if (startHpr == None):
            	startHpr = node.getHpr()
	    self.fhpr = startHpr
	    self.thpr = hpr
            functor = HprLerpFunctor.HprLerpFunctor(
                    node, startHpr, hpr)

	LerpInterval.__init__(self, name, duration, functor, blendType) 
