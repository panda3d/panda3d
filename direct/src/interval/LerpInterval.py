"""LerpInterval module: contains the LerpInterval class"""

from Interval import *
from PandaModules import *

import LerpBlendHelpers

class LerpInterval(Interval):
    # Class methods
    def __init__(self, name, duration, functorFunc, blendType='noBlend'):
        """__init__(name, duration, functorFunc, blendType)
        """
        # Record instance variables
        self.lerp = None
	self.functorFunc = functorFunc
	self.blendType = self.getBlend(blendType)
        # Initialize superclass
	Interval.__init__(self, name, duration)

    def updateFunc(self, t, event = IVAL_NONE):
	""" updateFunc(t, event)
	"""
        # Check to see if we need to create the lerp
        if (event == IVAL_INIT):
	    self.lerp = Lerp(self.functorFunc(), self.duration, 
                                  self.blendType)
        # Make sure lerp exists
	if (not self.lerp):
	    self.lerp = Lerp(self.functorFunc(), self.duration, 
                                  self.blendType)
        # Evaluate the lerp
        self.lerp.setT(t)

    def getBlend(self, blendType):
        """__getBlend(self, string)
        Return the C++ blend class corresponding to blendType string
        """
        if (blendType == "easeIn"):
            return LerpBlendHelpers.easeIn
        elif (blendType == "easeOut"):
            return LerpBlendHelpers.easeOut
        elif (blendType == "easeInOut"):
            return LerpBlendHelpers.easeInOut
        elif (blendType == "noBlend"):
            return LerpBlendHelpers.noBlend
        else:
            raise Exception(
		'Error: LerpInterval.__getBlend: Unknown blend type')

class LerpPosInterval(LerpInterval):
    # Name counter
    lerpPosNum = 1
    # Class methods
    def __init__(self, node, duration, pos, startPos=None,
				other=None, blendType='noBlend', name=None):
	""" __init__(node, duration, pos, startPos, other, blendType, name)
	"""
	def functorFunc(node=node, pos=pos, startPos=startPos,
			other=other):
	    assert(not node.isEmpty())
            if callable(pos):
                # This may be a thunk that returns a point.
                pos = pos()
            if (other != None):
            	# lerp wrt other
	    	if (startPos == None):
                    startPos = node.getPos(other)
            	functor = PosLerpFunctor(node, startPos, pos, other)
            else:
	    	if (startPos == None):
            	    startPos = node.getPos()
            	functor = PosLerpFunctor(node, startPos, pos)
	    return functor

        # Generate unique name if necessary
	if (name == None):
	    name = 'LerpPosInterval-%d' % LerpPosInterval.lerpPosNum
	    LerpPosInterval.lerpPosNum += 1
        # Initialize superclass
	LerpInterval.__init__(self, name, duration, functorFunc, blendType) 

class LerpHprInterval(LerpInterval):
    # Name counter
    lerpHprNum = 1
    # Class methods
    def __init__(self, node, duration, hpr, startHpr=None,
				other=None, blendType='noBlend', name=None):
	""" __init__(node, duration, hpr, startHpr, other, blendType, name)
	"""
	def functorFunc(node=node, hpr=hpr, startHpr=startHpr,
			other=other):
	    assert(not node.isEmpty())
            if callable(hpr):
                # This may be a thunk that returns a point.
                hpr = hpr()
            if (other != None):
            	# lerp wrt other
	    	if (startHpr == None):
                    startHpr = node.getHpr(other)
            	functor = HprLerpFunctor(node, startHpr, hpr, other)
            else:
	    	if (startHpr == None):
            	    startHpr = node.getHpr()
            	functor = HprLerpFunctor(node, startHpr, hpr)
	    return functor

        # Generate unique name if necessary
	if (name == None):
	    name = 'LerpHprInterval-%d' % LerpHprInterval.lerpHprNum
	    LerpHprInterval.lerpHprNum += 1
        # Initialize superclass
	LerpInterval.__init__(self, name, duration, functorFunc, blendType) 

class LerpScaleInterval(LerpInterval):

    # Interval counter
    lerpScaleNum = 1
    # Class methods
    def __init__(self, node, duration, scale, startScale=None,
				other=None, blendType='noBlend', name=None):
	""" __init__(node, duration, scale, startScale, other, blendType, name)
	"""
	def functorFunc(node=node, scale=scale,
                        startScale=startScale, other=other):
	    assert(not node.isEmpty())
            if callable(scale):
                # This may be a thunk that returns a point.
                scale = scale()
            if (other != None):
            	# lerp wrt other
	    	if (startScale == None):
                    startScale = node.getScale(other)
            	functor = ScaleLerpFunctor(node, startScale, scale, other)
            else:
	    	if (startScale == None):
            	    startScale = node.getScale()
            	functor = ScaleLerpFunctor(node, startScale, scale)
	    return functor

        # Generate unique name if necessary
	if (name == None):
	    name = 'LerpScaleInterval-%d' % LerpScaleInterval.lerpScaleNum
	    LerpScaleInterval.lerpScaleNum += 1
        # Initialize superclass
	LerpInterval.__init__(self, name, duration, functorFunc, blendType) 

class LerpPosHprInterval(LerpInterval):
    # Interval counter

    lerpPosHprNum = 1

    def __init__(self, node, duration, pos, hpr, startPos=None,
		startHpr=None, other=None, blendType='noBlend', name=None): 
	""" __init__(node, duration, pos, hpr, startPos, startHpr,
						other, blendType, name)
	"""
	def functorFunc(node=node, pos=pos, hpr=hpr, 
			startPos=startPos, startHpr=startHpr, other=other):
	    assert(not node.isEmpty())
            if callable(pos):
                # This may be a thunk that returns a point.
                pos = pos()
            if callable(hpr):
                # This may be a thunk that returns a point.
                hpr = hpr()
            if (other != None):
            	# lerp wrt other
	    	if (startPos == None):
            	    startPos = node.getPos(other)
	    	if (startHpr == None):
            	    startHpr = node.getHpr(other)
            	functor = PosHprLerpFunctor(
                    node, startPos, pos,
                    startHpr, hpr, other)
            else:
	    	if (startPos == None):
            	    startPos = node.getPos()
	    	if (startHpr == None):
            	    startHpr = node.getHpr()
            	functor = PosHprLerpFunctor(
                    node, startPos, pos,
                    startHpr, hpr)
	    return functor

        # Generate unique name if necessary
	if (name == None):
	    name = 'LerpPosHpr-%d' % LerpPosHprInterval.lerpPosHprNum
	    LerpPosHprInterval.lerpPosHprNum += 1
        # Initialize superclass
	LerpInterval.__init__(self, name, duration, functorFunc, blendType)

class LerpPosHprScaleInterval(LerpInterval):
    # Interval counter
    lerpPosHprScaleNum = 1
    # Class methods
    def __init__(self, node, duration, pos, hpr, scale,
                 startPos=None, startHpr=None, startScale=None,
                 other=None, blendType='noBlend', name=None): 
	""" __init__(node, duration, pos, hpr, scale,
                     startPos, startHpr, startScale, 
                     other, blendType, name)
	"""
	def functorFunc(node=node, pos=pos, hpr=hpr, scale=scale,
			startPos=startPos, startHpr=startHpr,
                        startScale=startScale, other=other):
	    assert(not node.isEmpty())
            if callable(pos):
                # This may be a thunk that returns a point.
                pos = pos()
            if callable(hpr):
                # This may be a thunk that returns a point.
                hpr = hpr()
            if callable(scale):
                # This may be a thunk that returns a point.
                scale = scale()
            if (other != None):
            	# lerp wrt other
	    	if (startPos == None):
            	    startPos = node.getPos(other)
	    	if (startHpr == None):
            	    startHpr = node.getHpr(other)
	    	if (startScale == None):
            	    startScale = node.getScale(other)
            	functor = PosHprScaleLerpFunctor(
                    node, startPos, pos, startHpr, hpr,
                    startScale, scale, other)
            else:
	    	if (startPos == None):
            	    startPos = node.getPos()
	    	if (startHpr == None):
            	    startHpr = node.getHpr()
	    	if (startScale == None):
            	    startScale = node.getScale()
            	functor = PosHprScaleLerpFunctor(
                    node, startPos, pos, startHpr, hpr, startScale, scale)
	    return functor

        # Generate unique name if necessary
	if (name == None):
	    name = ('LerpPosHprScale-%d' %
                    LerpPosHprScaleInterval.lerpPosHprScaleNum)
	    LerpPosHprScaleInterval.lerpPosHprScaleNum += 1
        # Initialize superclass
	LerpInterval.__init__(self, name, duration, functorFunc, blendType)

# Class used to execute a function over time.  Function can access fromData
# and toData to perform blend.  If fromData and toData not specified, will
# execute the given function passing in values ranging from 0 to 1
class LerpFunctionInterval(Interval):
    # Interval counter
    lerpFunctionIntervalNum = 1
    # Class methods

    def __init__(self, function, fromData = 0, toData = 1, duration = 0.0,
                 blendType = 'noBlend', extraArgs = [], name = None):
        """__init__(function, duration, fromData, toData, name)
        """
        # Record instance variables
	self.function = function
        self.fromData = fromData
        self.toData = toData
	self.blendType = self.getBlend(blendType)
        self.extraArgs = extraArgs
        # Generate unique name if necessary
	if (name == None):
	    name = ('LerpFunctionInterval-%d' %
                    LerpFunctionInterval.lerpFunctionIntervalNum)
            LerpFunctionInterval.lerpFunctionIntervalNum += 1
        # Initialize superclass
	Interval.__init__(self, name, duration)

    def updateFunc(self, t, event = IVAL_NONE):
	""" updateFunc(t, event)
	"""
        # Evaluate the function
        if (t >= self.duration):
            # Set to end value
	    apply(self.function, [self.toData] + self.extraArgs)
        elif self.duration == 0.0:
            # Zero duration, just use endpoint
            apply(self.function, [self.toData] + self.extraArgs)
        else:
            # In the middle of the lerp, compute appropriate blended value
            bt = self.blendType(t/self.duration)
            data = (self.fromData * (1 - bt)) + (self.toData * bt)
            # Evaluate function
            apply(self.function, [data] + self.extraArgs)
    def getBlend(self, blendType):
        """__getBlend(self, string)
        Return the C++ blend class corresponding to blendType string
        """
        # Note, this is temporary until blend functions get exposed
        if (blendType == "easeIn"):
            return LerpBlendHelpers.easeIn
        elif (blendType == "easeOut"):
            return LerpBlendHelpers.easeOut
        elif (blendType == "easeInOut"):
            return LerpBlendHelpers.easeInOut
        elif (blendType == "noBlend"):
            return LerpBlendHelpers.noBlend
        else:
            raise Exception(
		'Error: LerpInterval.__getBlend: Unknown blend type')

