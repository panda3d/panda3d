"""ActorInterval module: contains the ActorInterval class"""

from PandaModules import *
from Interval import *

class ActorInterval(Interval):
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

    def updateFunc(self, t, event = IVAL_NONE):
	""" updateFunc(t, event)
	    Go to time t
	"""
        # Update animation based upon current time
        # Pose or stop anim
        if (t >= self.getDuration()):
	    if (self.actor != None):
            	self.actor.stop()
	    else:
		self.notify.warning('updateFunc() - actor == None!')
            if self.loop:
                self.ignore(self.stopEvent)
        elif self.loop == 1:
            if event == IVAL_INIT:
                # Determine the current frame
                currT = (self.actor.getFrameRate(self.animName) *
                         (self.startTime + t))
                print t, currT
                frame = int(currT) % self.numFrames
                print frame
                # Pose anim
                self.actor.pose(self.animName, frame)
                # And start loop, restart flag says continue from current frame
                self.actor.loop(self.animName, restart=0)
                self.accept(self.stopEvent, self.actor.stop)
        else:
            # Determine the current frame
            currT = (self.actor.getFrameRate(self.animName) *
                     (self.startTime + t))
            frame = int(currT) % self.numFrames
            # Pose anim
            self.actor.pose(self.animName, frame)
            


