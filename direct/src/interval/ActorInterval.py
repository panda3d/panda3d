"""ActorInterval module: contains the ActorInterval class"""

from PandaModules import *
from DirectNotifyGlobal import *
import Interval
import math
import LerpBlendHelpers

class ActorInterval(Interval.Interval):

    # create ActorInterval DirectNotify category
    notify = directNotify.newCategory('ActorInterval')

    # Name counter
    animNum = 1
    # Class methods

    # Plays an animation on an Actor.  The subrange of the animation
    # to be played may be specified via frames (startFrame up to and
    # including endFrame) or seconds (startTime up to and including
    # endTime).  If neither is specified, the default is the entire
    # range of the animation.

    # The duration may be implicit or explicit.  If it is omitted, it
    # is taken to be endTime - startTime.  There's not much point in
    # specifying otherwise unless you also specify loop=1, which will
    # loop the animation over its frame range during the duration of
    # the interval.
    
    # Note: if loop == 0 and duration > anim duration then the
    # animation will play once and then hold its final pose for the
    # remainder of the interval.
    
    def __init__(self, actor, animName, loop=0, duration=None,
                 startTime=None, endTime=None,
                 startFrame=None, endFrame=None,
                 playRate=1.0, name=None, forceUpdate=0,
                 partName=None, lodName=None):
        """__init__(name)
        """
        # Generate unique id
        id = 'Actor-%s-%d' % (animName, ActorInterval.animNum)
        ActorInterval.animNum += 1
        # Record class specific variables
        self.actor = actor
        self.animName = animName
        self.controls = self.actor.getAnimControls(
            self.animName, partName = partName, lodName = lodName)
        self.loopAnim = loop
        self.forceUpdate = forceUpdate

        # If no name specified, use id as name
        if (name == None):
            name = id

        if len(self.controls) == 0:
            self.notify.warning("Unknown animation for actor: %s" % (self.animName))
            self.frameRate = 1.0
            self.startFrame = 0
            self.endFrame = 0
        else:

            self.frameRate = self.controls[0].getAnim().getBaseFrameRate() * abs(playRate)
            # Compute start and end frames.
            if startFrame != None:
                self.startFrame = startFrame
            elif startTime != None:
                self.startFrame = int(math.floor(startTime * self.frameRate + 0.0001))
            else:
                self.startFrame = 0

            if endFrame != None:
                self.endFrame = endFrame
            elif endTime != None:
                self.endFrame = int(math.floor(endTime * self.frameRate + 0.0001))
            elif duration != None:
                if startTime == None:
                    startTime = float(self.startFrame) / float(self.frameRate)
                endTime = startTime + duration
                self.endFrame = int(math.floor(duration * self.frameRate + 0.0001))
            else:
                # No end frame specified.  Choose the maximum of all
                # of the controls' numbers of frames.
                maxFrames = self.controls[0].getNumFrames()
                warned = 0
                for i in range(1, len(self.controls)):
                    numFrames = self.controls[i].getNumFrames()
                    if numFrames != maxFrames and numFrames != 1 and not warned:
                        self.notify.warning("Animations '%s' on %s have an inconsistent number of frames." % (animName, actor.getName()))
                        warned = 1
                    maxFrames = max(maxFrames, numFrames)
                self.endFrame = maxFrames - 1

        # Must we play the animation backwards?  We play backwards if
        # either (or both) of the following is true: the playRate is
        # negative, or endFrame is before startFrame.
        self.reverse = (playRate < 0)
        if self.endFrame < self.startFrame:
            self.reverse = 1
            t = self.endFrame
            self.endFrame = self.startFrame
            self.startFrame = t

        numFrames = self.endFrame - self.startFrame + 1

        # Compute duration if no duration specified
        self.implicitDuration = 0
        if duration == None:
            self.implicitDuration = 1
            duration = float(numFrames) / self.frameRate

        # Initialize superclass
        Interval.Interval.__init__(self, name, duration)

    def privStep(self, t):
        if self.reverse:
            absFrame = self.endFrame - t * self.frameRate
        else:
            absFrame = self.startFrame + t * self.frameRate

        # Calc integer frame number
        absFrame = int(math.floor(absFrame + 0.0001))

        # Pose anim

        # We use our pre-computed list of animControls for
        # efficiency's sake, rather than going through the relatively
        # expensive Actor interface every frame.
        for control in self.controls:
            # Each animControl might have a different number of frames.
            numFrames = control.getNumFrames()
            if self.loopAnim:
                frame = absFrame % numFrames
            else:
                frame = max(min(absFrame, numFrames - 1), 0)

            control.pose(frame)

        if self.forceUpdate:
            self.actor.update()
        self.state = CInterval.SStarted
        self.currT = t

    def privFinalize(self):
        if self.implicitDuration:
            # As a special case, we ensure we end up posed to the last
            # frame of the animation if the original duration was
            # implicit.  This is necessary only to guard against
            # possible roundoff error in computing the final frame
            # from the duration.
            if self.reverse:
                for control in self.controls:
                    control.pose(self.startFrame)
            else:
                for control in self.controls:
                    control.pose(self.endFrame)
            if self.forceUpdate:
                self.actor.update()

        else:
            # Otherwise, the user-specified duration determines which
            # is our final frame.
            self.privStep(self.getDuration())
            
        self.state = CInterval.SFinal
        self.intervalDone()
        

class LerpAnimInterval(CLerpAnimEffectInterval):
    # Blends between two anims.  Start both anims first (or use
    # parallel ActorIntervals), then invoke LerpAnimInterval to
    # smoothly blend the control effect from the first to the second.
    lerpAnimNum = 1

    def __init__(self, actor, duration, startAnim, endAnim,
                 startWeight = 0.0, endWeight = 1.0,
                 blendType = 'noBlend', name = None):
        # Generate unique name if necessary
        if (name == None):
            name = 'LerpAnimInterval-%d' % LerpAnimInterval.lerpAnimNum
            LerpAnimInterval.lerpAnimNum += 1

        blendType = self.stringBlendType(blendType)
        assert(blendType != self.BTInvalid)

        # Initialize superclass
        CLerpAnimEffectInterval.__init__(self, name, duration, blendType)

        if startAnim != None:
            controls = actor.getAnimControls(startAnim)
            for control in controls:
                self.addControl(control, startAnim,
                                1.0 - startWeight, 1.0 - endWeight)
                
        if endAnim != None:
            controls = actor.getAnimControls(endAnim)
            for control in controls:
                self.addControl(control, endAnim,
                                startWeight, endWeight)
