"""ActorInterval module: contains the ActorInterval class"""

from PandaModules import *
import Interval
import math
import LerpBlendHelpers

class ActorInterval(Interval.Interval):

    # create ActorInterval DirectNotify category
    notify = directNotify.newCategory('ActorInterval')

    # Name counter
    animNum = 1
    # Class methods
    # Interval used to play an animation.  If loop = 0, animation is
    # played only once and the pose of the anim depends on t passed
    # into the setT method every frame.  If loop = 1, the animation is
    # started and plays on its own and stopped when t >= duration or
    # playback stop event occurs.  If no duration is specified, interval
    # duration defaults to be equal to the length of the animation.
    # Note: if loop == 0 and duration > anim duration then the animation
    # will play once and then nothing will happen for the remainder of the
    # interval
    def __init__(self, actor, animName, loop=0, duration=0.0,
                 startTime=0.0, endTime=None, playRate=1.0, name=None):
        """__init__(name)
        """
        # Generate unique id
        id = 'Actor-%d' % ActorInterval.animNum
        ActorInterval.animNum += 1
        # Record class specific variables
        self.actor = actor
        self.animName = animName
        self.controls = self.actor.getAnimControls(self.animName)
        assert(len(self.controls) > 0)
        self.loopAnim = loop
        self.frameRate = self.controls[0].getAnim().getBaseFrameRate() * playRate
        self.numFrames = self.controls[0].getNumFrames()
        # Compute start time
        self.startTime = startTime
        # If no name specified, use id as name
        if (name == None):
            name = id
        # Compute duration if no duration specified
        self.reverse = 0
        if duration == 0.0:
            if (endTime == None):
                duration = max(self.actor.getDuration(self.animName) - \
                                startTime, 0.0)
            else:
                duration = endTime - startTime
                if (duration < 0.0):
                    duration = -duration
        if (endTime == None):
            self.finishTime = self.startTime + duration
        else:
            self.finishTime = endTime
        if (self.startTime > self.finishTime):
            self.reverse = 1

        # Initialize superclass
        Interval.Interval.__init__(self, name, duration)

    def calcFrame(self, t):
        segmentLength = abs(self.finishTime - self.startTime)
        if segmentLength == 0:
            offset = 0
        else:
            offset = t % segmentLength
        # Handle boundary case where we want to set the final frame
        if (t == self.getDuration() and offset < 0.0001):
            offset = segmentLength
        # Compute current frame based upon current time
        if (self.reverse == 0):
            floatFrame = self.frameRate * (self.startTime + offset)
        else:
            negOffset = (self.startTime - self.finishTime) - offset
            floatFrame = self.frameRate * (self.finishTime + negOffset)
        # Need max to avoid frame = -1 when t = 0
        frame = max(0, int(math.ceil(floatFrame)) - 1)
        # Modulo in case of looping anim
        return frame % self.numFrames

    def goToT(self, t):
        # Calc integer frame number
        frame = self.calcFrame(t)
        # Pose anim

        # We use our pre-computed list of animControls for
        # efficiency's sake, rather than going through the relatively
        # expensive Actor interface every frame.
        for control in self.controls:
            control.pose(frame)
            
        return frame

    def privInitialize(self, t):
        self.state = CInterval.SStarted
        self.goToT(t)
        if self.loopAnim:
            self.actor.loop(self.animName, restart = 0)
        self.currT = t

    def privFinalize(self):
        if self.loopAnim:
            self.actor.stop()
        else:
            self.goToT(self.getDuration())
        self.currT = self.getDuration()
        self.state = CInterval.SFinal
            
    def privStep(self, t):
        if not self.loopAnim:
            self.goToT(t)

        self.state = CInterval.SStarted
        self.currT = t

    def privInterrupt(self):
        if self.loopAnim:
            self.actor.stop
        

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
