"""ActorInterval module: contains the ActorInterval class"""

from PandaModules import *
from Interval import *
import math

import DirectNotifyGlobal

class ActorInterval(Interval):

    notify = DirectNotifyGlobal.directNotify.newCategory('ActorInterval')

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
                 startTime = 0.0, name=None):
        """__init__(name)
        """
        # Generate unique id
        id = 'Actor-%d' % ActorInterval.animNum
        ActorInterval.animNum += 1
        # Record class specific variables
	self.actor = actor
	self.animName = animName
        self.loop = loop
        self.frameRate = self.actor.getFrameRate(self.animName)
	self.numFrames = self.actor.getNumFrames(self.animName)
        # Compute start time
        self.startTime = startTime
        # If no name specified, use id as name
	if (name == None):
	    name = id
        # Compute duration if no duration specified
        if duration == 0.0:
	    duration = max(self.actor.getDuration(self.animName) - startTime,
                           0.0)
        # Initialize superclass
	Interval.__init__(self, name, duration)
        # Update stopEvent
        if self.loop:
            self.stopEvent = id + '_stopEvent'
            self.stopEventList = [self.stopEvent]

    def calcFrame(self, t):
        # Compute current frame based upon current time
        floatFrame = self.frameRate * (self.startTime + t)
        # Need max to avoid frame = -1 when t = 0
        frame = max(0, int(math.ceil(floatFrame)) - 1)
        # Modulo in case of looping anim
        return frame % self.numFrames

    def goToT(self, t):
        # Calc integer frame number
        frame = self.calcFrame(t)
        # Pose anim
        self.actor.pose(self.animName, frame)
        # Print debug information
        self.notify.debug('ActorInterval.goToT() - %s pose to frame: %d' %
                          (self.name,frame))
        return frame

    def updateFunc(self, t, event = IVAL_NONE):
	""" updateFunc(t, event)
	    Go to time t
	"""
	if (self.actor.isEmpty()):
	    self.notify.warning('updateFunc() - %s empty actor!' % self.name)
	    return
        # Update animation based upon current time
        # Pose or stop anim
        if (t >= (self.startTime + self.getDuration())):
            self.actor.stop()
            frame = self.goToT(self.getDuration())
            if self.loop:
                self.ignore(self.stopEvent)
            # Print debug information
	    self.notify.debug(
                'ActorInterval.updateFunc() - %s stoping at frame: ' %
                self.name +
                '%d Num frames: %d' % (frame, self.numFrames))
        elif self.loop == 1:
            if event == IVAL_INIT:
                # Pose anim
                self.goToT(t)
                # And start loop, restart flag says continue from current frame
                self.actor.loop(self.animName, restart=0)
                self.accept(self.stopEvent, self.actor.stop)
                # Print debug information
                self.notify.debug(
                    'ActorInterval.updateFunc() - IVAL_INIT %s looping anim' %
                    self.name)
        else:
            # Pose anim
            self.goToT(t)

