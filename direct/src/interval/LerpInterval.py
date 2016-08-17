"""LerpInterval module: contains the LerpInterval class"""

__all__ = [
    'LerpNodePathInterval', 'LerpPosInterval', 'LerpHprInterval',
    'LerpQuatInterval', 'LerpScaleInterval', 'LerpShearInterval',
    'LerpPosHprInterval', 'LerpPosQuatInterval',
    'LerpHprScaleInterval', 'LerpQuatScaleInterval',
    'LerpPosHprScaleInterval', 'LerpPosQuatScaleInterval',
    'LerpPosHprScaleShearInterval', 'LerpPosQuatScaleShearInterval',
    'LerpColorInterval', 'LerpColorScaleInterval',
    'LerpTexOffsetInterval', 'LerpTexRotateInterval', 'LerpTexScaleInterval',
    'LerpFunctionInterval', 'LerpFunc','LerpFunctionNoStateInterval','LerpFuncNS'
    ]

from panda3d.core import *
from panda3d.direct import *
from direct.directnotify.DirectNotifyGlobal import *
from . import Interval
from . import LerpBlendHelpers

#
# Most of the intervals defined in this module--the group up here at
# the front of the file--are now derived from a CInterval instead of
# an Interval, so they can run in the low-level C++ code at high
# performance.
#

class LerpNodePathInterval(CLerpNodePathInterval):
    # This is the base class for all of the lerps, defined below, that
    # affect a property on a NodePath, like pos or hpr.
    lerpNodePathNum = 1

    def __init__(self, name, duration, blendType, bakeInStart, fluid,
                 nodePath, other):
        if name == None:
            name = '%s-%d' % (self.__class__.__name__, self.lerpNodePathNum)
            LerpNodePathInterval.lerpNodePathNum += 1
        else:
            # Allow the user to pass in a %d in the name and we'll go ahead
            # and uniquify the name for them.
            if "%d" in name:
                name = name % LerpNodePathInterval.lerpNodePathNum
                LerpNodePathInterval.lerpNodePathNum += 1

        blendType = self.stringBlendType(blendType)
        assert blendType != self.BTInvalid

        if other == None:
            other = NodePath()

        CLerpNodePathInterval.__init__(self, name, duration, blendType,
                                       bakeInStart, fluid, nodePath, other)

    def anyCallable(self, *params):
        # Returns true if any of the parameters listed is a callable
        # functor, false if none of them are.  This is used by derived
        # classes to determine if a functor was passed in for a
        # parameter.

        for param in params:
            if callable(param):
                return 1
        return 0

    def setupParam(self, func, param):
        # Stores the indicated parameter by passing it to the given
        # function (probably a C++ setter function).  If the param is
        # a callable functor, calls it; otherwise, uses the param
        # directly.
        if param != None:
            if callable(param):
                func(param())
            else:
                func(param)

#####################################################################
##
##  It is legal to pass in a functor (a callable Python function that
##  returns an appropriate value) for the any of the pos, hpr, scale,
##  or shear parameters in the intervals below.  These will be
##  evaluated at the time the interval starts in order to determine
##  the actual final (or initial) position.  However, doing so forces
##  the entire LerpInterval to be executed up in Python, instead of
##  down in the low-level C++ code, at some performance cost.
##
##  If bakeInStart is true, the starting values (if unspecified) are
##  computed at the time the interval runs for the first time, and
##  then stored.  If bakeInStart is false, the starting values are
##  recomputed each frame based on the current value and the time
##  delta since the last time the interval ran, which allows show code
##  to manipulate the node even while it is being lerped.
##
##  If fluid is true for a LerpPos-style interval, then the pos is set
##  via NodePath.setFluidPos() instead of NodePath.setPos(), causing
##  the collision system to treat the motion as continuous and test
##  for collisions against the entire motion path, instead of as
##  discrete position updates.  This has no meaning for Lerp intervals
##  that do not adjust pos.
##
#####################################################################

class LerpPosInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, pos, startPos = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, fluid = 0, name = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, fluid, nodePath, other)

        # Check for functors in the input parameters.
        self.paramSetup = self.anyCallable(pos, startPos)
        if self.paramSetup:
            self.endPos = pos
            self.startPos = startPos
            self.inPython = 1
        else:
            self.setEndPos(pos)
            if startPos != None:
                self.setStartPos(startPos)

    def privDoEvent(self, t, event):
        # This function is only used if Python functors were passed in
        # for some of the input parameters.
        if self.paramSetup and event == CInterval.ETInitialize:
            self.setupParam(self.setEndPos, self.endPos)
            self.setupParam(self.setStartPos, self.startPos)
        LerpNodePathInterval.privDoEvent(self, t, event)


class LerpHprInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, hpr,
                 startHpr = None, startQuat = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, fluid = 0, name = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, fluid, nodePath, other)

        # Check for functors in the input parameters.
        self.paramSetup = self.anyCallable(hpr, startHpr, startQuat)
        if self.paramSetup:
            self.endHpr = hpr
            self.startHpr = startHpr
            self.startQuat = startQuat
            self.inPython = 1
        else:
            self.setEndHpr(hpr)
            if startHpr != None:
                self.setStartHpr(startHpr)
            if startQuat != None:
                self.setStartQuat(startQuat)

    def privDoEvent(self, t, event):
        # This function is only used if Python functors were passed in
        # for some of the input parameters.
        if self.paramSetup and event == CInterval.ETInitialize:
            self.setupParam(self.setEndHpr, self.endHpr)
            self.setupParam(self.setStartHpr, self.startHpr)
            self.setupParam(self.setStartQuat, self.startQuat)
        LerpNodePathInterval.privDoEvent(self, t, event)

class LerpQuatInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, quat = None,
                 startHpr = None, startQuat = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, fluid = 0, name = None, hpr = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, fluid, nodePath, other)

        if not quat:
            # If we didn't specify a quat, we're allowed to specify a
            # hpr instead, which is converted to the equivalent quat.
            assert hpr
            quat = LOrientationf()
            quat.setHpr(hpr)

        # Check for functors in the input parameters.
        self.paramSetup = self.anyCallable(quat, startHpr, startQuat)
        if self.paramSetup:
            self.endQuat = quat
            self.startHpr = startHpr
            self.startQuat = startQuat
            self.inPython = 1
        else:
            self.setEndQuat(quat)
            if startHpr != None:
                self.setStartHpr(startHpr)
            if startQuat != None:
                self.setStartQuat(startQuat)

    def privDoEvent(self, t, event):
        # This function is only used if Python functors were passed in
        # for some of the input parameters.
        if self.paramSetup and event == CInterval.ETInitialize:
            self.setupParam(self.setEndQuat, self.endQuat)
            self.setupParam(self.setStartHpr, self.startHpr)
            self.setupParam(self.setStartQuat, self.startQuat)
        LerpNodePathInterval.privDoEvent(self, t, event)

class LerpScaleInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, scale, startScale = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, fluid = 0, name = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, fluid, nodePath, other)
        # Check for functors in the input parameters.
        self.paramSetup = self.anyCallable(scale, startScale)
        if self.paramSetup:
            self.endScale = scale
            self.startScale = startScale
            self.inPython = 1
        else:
            self.setEndScale(scale)
            if startScale != None:
                self.setStartScale(startScale)

    def privDoEvent(self, t, event):
        # This function is only used if Python functors were passed in
        # for some of the input parameters.
        if self.paramSetup and event == CInterval.ETInitialize:
            self.setupParam(self.setEndScale, self.endScale)
            self.setupParam(self.setStartScale, self.startScale)
        LerpNodePathInterval.privDoEvent(self, t, event)

class LerpShearInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, shear, startShear = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, fluid = 0, name = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, fluid, nodePath, other)
        # Check for functors in the input parameters.
        self.paramSetup = self.anyCallable(shear, startShear)
        if self.paramSetup:
            self.endShear = shear
            self.startShear = startShear
            self.inPython = 1
        else:
            self.setEndShear(shear)
            if startShear != None:
                self.setStartShear(startShear)

    def privDoEvent(self, t, event):
        # This function is only used if Python functors were passed in
        # for some of the input parameters.
        if self.paramSetup and event == CInterval.ETInitialize:
            self.setupParam(self.setEndShear, self.endShear)
            self.setupParam(self.setStartShear, self.startShear)
        LerpNodePathInterval.privDoEvent(self, t, event)

class LerpPosHprInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, pos, hpr,
                 startPos = None, startHpr = None, startQuat = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, fluid = 0, name = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, fluid, nodePath, other)
        # Check for functors in the input parameters.
        self.paramSetup = self.anyCallable(pos, startPos, hpr, startHpr, startQuat)
        if self.paramSetup:
            self.endPos = pos
            self.startPos = startPos
            self.endHpr = hpr
            self.startHpr = startHpr
            self.startQuat = startQuat
            self.inPython = 1
        else:
            self.setEndPos(pos)
            if startPos != None:
                self.setStartPos(startPos)
            self.setEndHpr(hpr)
            if startHpr != None:
                self.setStartHpr(startHpr)
            if startQuat != None:
                self.setStartQuat(startQuat)

    def privDoEvent(self, t, event):
        # This function is only used if Python functors were passed in
        # for some of the input parameters.
        if self.paramSetup and event == CInterval.ETInitialize:
            self.setupParam(self.setEndPos, self.endPos)
            self.setupParam(self.setStartPos, self.startPos)
            self.setupParam(self.setEndHpr, self.endHpr)
            self.setupParam(self.setStartHpr, self.startHpr)
            self.setupParam(self.setStartQuat, self.startQuat)
        LerpNodePathInterval.privDoEvent(self, t, event)

class LerpPosQuatInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, pos, quat = None,
                 startPos = None, startHpr = None, startQuat = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, fluid = 0, name = None, hpr = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, fluid, nodePath, other)

        if not quat:
            # If we didn't specify a quat, we're allowed to specify a
            # hpr instead, which is converted to the equivalent quat.
            assert hpr
            quat = LOrientationf()
            quat.setHpr(hpr)

        # Check for functors in the input parameters.
        self.paramSetup = self.anyCallable(pos, startPos, quat, startHpr, startQuat)
        if self.paramSetup:
            self.endPos = pos
            self.startPos = startPos
            self.endQuat = quat
            self.startHpr = startHpr
            self.startQuat = startQuat
            self.inPython = 1
        else:
            self.setEndPos(pos)
            if startPos != None:
                self.setStartPos(startPos)
            self.setEndQuat(quat)
            if startHpr != None:
                self.setStartHpr(startHpr)
            if startQuat != None:
                self.setStartQuat(startQuat)

    def privDoEvent(self, t, event):
        # This function is only used if Python functors were passed in
        # for some of the input parameters.
        if self.paramSetup and event == CInterval.ETInitialize:
            self.setupParam(self.setEndPos, self.endPos)
            self.setupParam(self.setStartPos, self.startPos)
            self.setupParam(self.setEndQuat, self.endQuat)
            self.setupParam(self.setStartHpr, self.startHpr)
            self.setupParam(self.setStartQuat, self.startQuat)
        LerpNodePathInterval.privDoEvent(self, t, event)

class LerpHprScaleInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, hpr, scale,
                 startHpr = None, startQuat = None, startScale = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, fluid = 0, name = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, fluid, nodePath, other)

        # Check for functors in the input parameters.
        self.paramSetup = self.anyCallable(hpr, startHpr, startQuat, scale, startScale)
        if self.paramSetup:
            self.endHpr = hpr
            self.startHpr = startHpr
            self.startQuat = startQuat
            self.endScale = scale
            self.startScale = startScale
            self.inPython = 1
        else:
            self.setEndHpr(hpr)
            if startHpr != None:
                self.setStartHpr(startHpr)
            if startQuat != None:
                self.setStartQuat(startQuat)
            self.setEndScale(scale)
            if startScale != None:
                self.setStartScale(startScale)

    def privDoEvent(self, t, event):
        # This function is only used if Python functors were passed in
        # for some of the input parameters.
        if self.paramSetup and event == CInterval.ETInitialize:
            self.setupParam(self.setEndHpr, self.endHpr)
            self.setupParam(self.setStartHpr, self.startHpr)
            self.setupParam(self.setStartQuat, self.startQuat)
            self.setupParam(self.setEndScale, self.endScale)
            self.setupParam(self.setStartScale, self.startScale)
        LerpNodePathInterval.privDoEvent(self, t, event)

class LerpQuatScaleInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, quat = None, scale = None,
                 hpr = None,
                 startHpr = None, startQuat = None, startScale = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, fluid = 0, name = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, fluid, nodePath, other)

        if not quat:
            # If we didn't specify a quat, we're allowed to specify a
            # hpr instead, which is converted to the equivalent quat.
            assert hpr
            quat = LOrientationf()
            quat.setHpr(hpr)

        assert scale

        # Check for functors in the input parameters.
        self.paramSetup = self.anyCallable(quat, startHpr, startQuat, scale, startScale)
        if self.paramSetup:
            self.endQuat = quat
            self.startHpr = startHpr
            self.startQuat = startQuat
            self.endScale = scale
            self.startScale = startScale
            self.inPython = 1
        else:
            self.setEndQuat(quat)
            if startHpr != None:
                self.setStartHpr(startHpr)
            if startQuat != None:
                self.setStartQuat(startQuat)
            self.setEndScale(scale)
            if startScale != None:
                self.setStartScale(startScale)

    def privDoEvent(self, t, event):
        # This function is only used if Python functors were passed in
        # for some of the input parameters.
        if self.paramSetup and event == CInterval.ETInitialize:
            self.setupParam(self.setEndQuat, self.endQuat)
            self.setupParam(self.setStartHpr, self.startHpr)
            self.setupParam(self.setStartQuat, self.startQuat)
            self.setupParam(self.setEndScale, self.endScale)
            self.setupParam(self.setStartScale, self.startScale)
        LerpNodePathInterval.privDoEvent(self, t, event)

class LerpPosHprScaleInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, pos, hpr, scale,
                 startPos = None, startHpr = None, startQuat = None,
                 startScale = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, fluid = 0, name = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, fluid, nodePath, other)
        # Check for functors in the input parameters.
        self.paramSetup = self.anyCallable(pos, startPos, hpr, startHpr,
                                           startQuat, scale, startScale)
        if self.paramSetup:
            self.endPos = pos
            self.startPos = startPos
            self.endHpr = hpr
            self.startHpr = startHpr
            self.startQuat = startQuat
            self.endScale = scale
            self.startScale = startScale
            self.inPython = 1
        else:
            self.setEndPos(pos)
            if startPos != None:
                self.setStartPos(startPos)
            self.setEndHpr(hpr)
            if startHpr != None:
                self.setStartHpr(startHpr)
            if startQuat != None:
                self.setStartQuat(startQuat)
            self.setEndScale(scale)
            if startScale != None:
                self.setStartScale(startScale)

    def privDoEvent(self, t, event):
        # This function is only used if Python functors were passed in
        # for some of the input parameters.
        if self.paramSetup and event == CInterval.ETInitialize:
            self.setupParam(self.setEndPos, self.endPos)
            self.setupParam(self.setStartPos, self.startPos)
            self.setupParam(self.setEndHpr, self.endHpr)
            self.setupParam(self.setStartHpr, self.startHpr)
            self.setupParam(self.setStartQuat, self.startQuat)
            self.setupParam(self.setEndScale, self.endScale)
            self.setupParam(self.setStartScale, self.startScale)
        LerpNodePathInterval.privDoEvent(self, t, event)

class LerpPosQuatScaleInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, pos, quat = None, scale = None,
                 startPos = None, startHpr = None, startQuat = None,
                 startScale = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, fluid = 0, name = None, hpr = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, fluid, nodePath, other)

        if not quat:
            # If we didn't specify a quat, we're allowed to specify a
            # hpr instead, which is converted to the equivalent quat.
            assert hpr
            quat = LOrientationf()
            quat.setHpr(hpr)

        assert scale

        # Check for functors in the input parameters.
        self.paramSetup = self.anyCallable(pos, startPos, quat, startHpr,
                                           startQuat, scale, startScale)
        if self.paramSetup:
            self.endPos = pos
            self.startPos = startPos
            self.endQuat = quat
            self.startHpr = startHpr
            self.startQuat = startQuat
            self.endScale = scale
            self.startScale = startScale
            self.inPython = 1
        else:
            self.setEndPos(pos)
            if startPos != None:
                self.setStartPos(startPos)
            self.setEndQuat(quat)
            if startHpr != None:
                self.setStartHpr(startHpr)
            if startQuat != None:
                self.setStartQuat(startQuat)
            self.setEndScale(scale)
            if startScale != None:
                self.setStartScale(startScale)

    def privDoEvent(self, t, event):
        # This function is only used if Python functors were passed in
        # for some of the input parameters.
        if self.paramSetup and event == CInterval.ETInitialize:
            self.setupParam(self.setEndPos, self.endPos)
            self.setupParam(self.setStartPos, self.startPos)
            self.setupParam(self.setEndQuat, self.endQuat)
            self.setupParam(self.setStartHpr, self.startHpr)
            self.setupParam(self.setStartQuat, self.startQuat)
            self.setupParam(self.setEndScale, self.endScale)
            self.setupParam(self.setStartScale, self.startScale)
        LerpNodePathInterval.privDoEvent(self, t, event)

class LerpPosHprScaleShearInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, pos, hpr, scale, shear,
                 startPos = None, startHpr = None, startQuat = None,
                 startScale = None, startShear = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, fluid = 0, name = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, fluid, nodePath, other)
        # Check for functors in the input parameters.
        self.paramSetup = self.anyCallable(pos, startPos, hpr, startHpr,
                                           startQuat, scale, startScale,
                                           shear, startShear)
        if self.paramSetup:
            self.endPos = pos
            self.startPos = startPos
            self.endHpr = hpr
            self.startHpr = startHpr
            self.startQuat = startQuat
            self.endScale = scale
            self.startScale = startScale
            self.endShear = shear
            self.startShear = startShear
            self.inPython = 1
        else:
            self.setEndPos(pos)
            if startPos != None:
                self.setStartPos(startPos)
            self.setEndHpr(hpr)
            if startHpr != None:
                self.setStartHpr(startHpr)
            if startQuat != None:
                self.setStartQuat(startQuat)
            self.setEndScale(scale)
            if startScale != None:
                self.setStartScale(startScale)
            self.setEndShear(shear)
            if startShear != None:
                self.setStartShear(startShear)

    def privDoEvent(self, t, event):
        # This function is only used if Python functors were passed in
        # for some of the input parameters.
        if self.paramSetup and event == CInterval.ETInitialize:
            self.setupParam(self.setEndPos, self.endPos)
            self.setupParam(self.setStartPos, self.startPos)
            self.setupParam(self.setEndHpr, self.endHpr)
            self.setupParam(self.setStartHpr, self.startHpr)
            self.setupParam(self.setStartQuat, self.startQuat)
            self.setupParam(self.setEndScale, self.endScale)
            self.setupParam(self.setStartScale, self.startScale)
            self.setupParam(self.setEndShear, self.endShear)
            self.setupParam(self.setStartShear, self.startShear)
        LerpNodePathInterval.privDoEvent(self, t, event)

class LerpPosQuatScaleShearInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, pos, quat = None, scale = None,
                 shear = None,
                 startPos = None, startHpr = None, startQuat = None,
                 startScale = None, startShear = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, fluid = 0, name = None, hpr = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, fluid, nodePath, other)

        if not quat:
            # If we didn't specify a quat, we're allowed to specify a
            # hpr instead, which is converted to the equivalent quat.
            assert hpr
            quat = LOrientationf()
            quat.setHpr(hpr)

        assert scale
        assert shear

        # Check for functors in the input parameters.
        self.paramSetup = self.anyCallable(pos, startPos, quat, startHpr,
                                           startQuat, scale, startScale,
                                           shear, startShear)
        if self.paramSetup:
            self.endPos = pos
            self.startPos = startPos
            self.endQuat = quat
            self.startHpr = startHpr
            self.startQuat = startQuat
            self.endScale = scale
            self.startScale = startScale
            self.endShear = shear
            self.startShear = startShear
            self.inPython = 1
        else:
            self.setEndPos(pos)
            if startPos != None:
                self.setStartPos(startPos)
            self.setEndQuat(quat)
            if startHpr != None:
                self.setStartHpr(startHpr)
            if startQuat != None:
                self.setStartQuat(startQuat)
            self.setEndScale(scale)
            if startScale != None:
                self.setStartScale(startScale)
            self.setEndShear(shear)
            if startShear != None:
                self.setStartShear(startShear)

    def privDoEvent(self, t, event):
        # This function is only used if Python functors were passed in
        # for some of the input parameters.
        if self.paramSetup and event == CInterval.ETInitialize:
            self.setupParam(self.setEndPos, self.endPos)
            self.setupParam(self.setStartPos, self.startPos)
            self.setupParam(self.setEndQuat, self.endQuat)
            self.setupParam(self.setStartHpr, self.startHpr)
            self.setupParam(self.setStartQuat, self.startQuat)
            self.setupParam(self.setEndScale, self.endScale)
            self.setupParam(self.setStartScale, self.startScale)
            self.setupParam(self.setEndShear, self.endShear)
            self.setupParam(self.setStartShear, self.startShear)
        LerpNodePathInterval.privDoEvent(self, t, event)

class LerpColorInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, color, startColor = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, name = None, override = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, 0, nodePath, other)
        self.setEndColor(color)
        if startColor != None:
            self.setStartColor(startColor)
        if override != None:
            self.setOverride(override)

class LerpColorScaleInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, colorScale, startColorScale = None,
                 other = None, blendType = 'noBlend',
                 bakeInStart = 1, name = None, override = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, 0, nodePath, other)
        self.setEndColorScale(colorScale)
        if startColorScale != None:
            self.setStartColorScale(startColorScale)
        if override != None:
            self.setOverride(override)

class LerpTexOffsetInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, texOffset, startTexOffset = None,
                 other = None, blendType = 'noBlend',
                 textureStage = None,
                 bakeInStart = 1, name = None, override = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, 0, nodePath, other)
        self.setEndTexOffset(texOffset)
        if startTexOffset != None:
            self.setStartTexOffset(startTexOffset)
        if textureStage != None:
            self.setTextureStage(textureStage)
        if override != None:
            self.setOverride(override)

class LerpTexRotateInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, texRotate, startTexRotate = None,
                 other = None, blendType = 'noBlend',
                 textureStage = None,
                 bakeInStart = 1, name = None, override = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, 0, nodePath, other)
        self.setEndTexRotate(texRotate)
        if startTexRotate != None:
            self.setStartTexRotate(startTexRotate)
        if textureStage != None:
            self.setTextureStage(textureStage)
        if override != None:
            self.setOverride(override)

class LerpTexScaleInterval(LerpNodePathInterval):
    def __init__(self, nodePath, duration, texScale, startTexScale = None,
                 other = None, blendType = 'noBlend',
                 textureStage = None,
                 bakeInStart = 1, name = None, override = None):
        LerpNodePathInterval.__init__(self, name, duration, blendType,
                                      bakeInStart, 0, nodePath, other)
        self.setEndTexScale(texScale)
        if startTexScale != None:
            self.setStartTexScale(startTexScale)
        if textureStage != None:
            self.setTextureStage(textureStage)
        if override != None:
            self.setOverride(override)




#
# The remaining intervals defined in this module are the old-school
# Python-based intervals.
#


class LerpFunctionNoStateInterval(Interval.Interval):
    """
    Class used to execute a function over time.  Function can access fromData
    and toData to perform blend.  If fromData and toData not specified, will
    execute the given function passing in values ranging from 0 to 1

    This is different from a standard LerpFunction, in that it assumes
    the function is not modifying any state that needs to be kept; so
    that it will only call the function while the lerp is actually
    running, and will not be guaranteed to call the function with its
    final value of the lerp.  In particular, if the lerp interval
    happens to get skipped over completely, it will not bother to call
    the function at all.
    """

    # Interval counter
    lerpFunctionIntervalNum = 1
    # create LerpFunctionInterval DirectNotify category
    notify = directNotify.newCategory('LerpFunctionNoStateInterval')
    # Class methods
    def __init__(self, function, duration = 0.0, fromData = 0, toData = 1,
                 blendType = 'noBlend', extraArgs = [], name = None):
        """__init__(function, duration, fromData, toData, name)
        """
        # Record instance variables
        self.function = function
        self.fromData = fromData
        self.toData = toData
        self.blendType = LerpBlendHelpers.getBlend(blendType)
        self.extraArgs = extraArgs
        # Generate unique name if necessary
        if (name == None):
            name = ('LerpFunctionInterval-%d' %
                    LerpFunctionNoStateInterval.lerpFunctionIntervalNum)
            LerpFunctionNoStateInterval.lerpFunctionIntervalNum += 1
        else:
            # Allow the user to pass in a %d in the name and we'll go ahead
            # and uniquify the name for them.
            if "%d" in name:
                name = name % LerpFunctionNoStateInterval.lerpFunctionIntervalNum
                LerpFunctionNoStateInterval.lerpFunctionIntervalNum += 1

        # Initialize superclass
        Interval.Interval.__init__(self, name, duration)

    #def privDoEvent(self,t,event):

        #print "doing event",t,event
        #bt = self.blendType(t/self.duration)
        #data = (self.fromData * (1 - bt)) + (self.toData * bt)
        ## Evaluate function
        #apply(self.function, [data] + self.extraArgs)
        #self.state = CInterval.SStarted
        #self.currT = t

    def privStep(self, t):
        # Evaluate the function
        #print "doing priv step",t
        if (t >= self.duration):
            # Set to end value
            if (t > self.duration):
                print("after end")
            #apply(self.function, [self.toData] + self.extraArgs)
        elif self.duration == 0.0:
            # Zero duration, just use endpoint
            self.function(*[self.toData] + self.extraArgs)
        else:
            # In the middle of the lerp, compute appropriate blended value
            bt = self.blendType(t/self.duration)
            data = (self.fromData * (1 - bt)) + (self.toData * bt)
            # Evaluate function
            self.function(*[data] + self.extraArgs)

        # Print debug information
#        assert self.notify.debug('updateFunc() - %s: t = %f' % (self.name, t))

        self.state = CInterval.SStarted
        self.currT = t

# New interface
class LerpFuncNS(LerpFunctionNoStateInterval):
    def __init__(self, *args, **kw):
        LerpFunctionNoStateInterval.__init__(self, *args, **kw)


class LerpFunctionInterval(Interval.Interval):
    """
    Class used to execute a function over time.  Function can access fromData
    and toData to perform blend.  If fromData and toData not specified, will
    execute the given function passing in values ranging from 0 to 1
    """

    # Interval counter
    lerpFunctionIntervalNum = 1
    # create LerpFunctionInterval DirectNotify category
    notify = directNotify.newCategory('LerpFunctionInterval')
    # Class methods
    def __init__(self, function, duration = 0.0, fromData = 0, toData = 1,
                 blendType = 'noBlend', extraArgs = [], name = None):
        """__init__(function, duration, fromData, toData, name)
        """
        # Record instance variables
        self.function = function
        self.fromData = fromData
        self.toData = toData
        self.blendType = LerpBlendHelpers.getBlend(blendType)
        self.extraArgs = extraArgs
        # Generate unique name if necessary
        if (name == None):
            name = ('LerpFunctionInterval-%s-%d' %
                    (function.__name__,
                     LerpFunctionInterval.lerpFunctionIntervalNum))
            LerpFunctionInterval.lerpFunctionIntervalNum += 1
        else:
            # Allow the user to pass in a %d in the name and we'll go ahead
            # and uniquify the name for them.
            if "%d" in name:
                name = name % LerpFunctionInterval.lerpFunctionIntervalNum
                LerpFunctionInterval.lerpFunctionIntervalNum += 1

        # Initialize superclass
        Interval.Interval.__init__(self, name, duration)

    def privStep(self, t):
        # Evaluate the function
        #print "doing priv step",t
        if (t >= self.duration):
            # Set to end value
            self.function(*[self.toData] + self.extraArgs)
        elif self.duration == 0.0:
            # Zero duration, just use endpoint
            self.function(*[self.toData] + self.extraArgs)
        else:
            # In the middle of the lerp, compute appropriate blended value
            bt = self.blendType(t/self.duration)
            data = (self.fromData * (1 - bt)) + (self.toData * bt)
            # Evaluate function
            self.function(*[data] + self.extraArgs)

        # Print debug information
#        assert self.notify.debug('updateFunc() - %s: t = %f' % (self.name, t))

        self.state = CInterval.SStarted
        self.currT = t

# New interface
class LerpFunc(LerpFunctionInterval):
    def __init__(self, *args, **kw):
        LerpFunctionInterval.__init__(self, *args, **kw)
