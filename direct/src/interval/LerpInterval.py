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

    lerpPosHprNum = 1

    def __init__(self, node, duration, pos, hpr, startPos=None,
		startHpr=None, other=None, blendType='noBlend', name=None): 
	""" __init__(node, duration, pos, hpr, startPos, startHpr,
						other, blendType, name)
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

	if (name == None):
	    n = 'LerpPosHpr-%d' % self.lerpPosHprNum
	    self.lerpPosHprNum = self.lerpPosHprNum + 1
	else:
	    n = name

	LerpInterval.__init__(self, n, duration, functor, blendType)


class LerpPosInterval(LerpInterval):

    lerpPosNum = 1

    def __init__(self, node, duration, pos, startPos=None,
				other=None, blendType='noBlend', name=None):
	""" __init__(node, duration, pos, startPos, other, blendType, name)
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

	if (name == None):
	    n = 'LerpPos-%d' % self.lerpPosNum
	    self.lerpPosNum = self.lerpPosNum + 1
	else:
	    n = name

	LerpInterval.__init__(self, n, duration, functor, blendType) 

class LerpHprInterval(LerpInterval):

    lerpHprNum = 1

    def __init__(self, node, duration, hpr, startHpr=None,
				other=None, blendType='noBlend', name=None):
	""" __init__(node, duration, hpr, startHpr, other, blendType, name)
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

	if (name == None):
	    n = 'LerpHpr-%d' % self.lerpHprNum
	    self.lerpHprNum = self.lerpHprNum + 1
	else:
	    n = name

	LerpInterval.__init__(self, n, duration, functor, blendType) 
