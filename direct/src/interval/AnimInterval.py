"""AnimInterval module: contains the AnimInterval class"""

from PandaModules import *
from Interval import *

class AnimInterval(Interval):
    # Name counter
    animNum = 1
    # Class methods
    # Interval used to play an animation.  If loop = 0, animation is
    # played only once and the pose of the anim depends on t passed
    # into the setT method every frame.  If loop = 1, the animation is
    # started and plays on its own and stopped when t > duration or
    # IVAL_STOP event occurs.  If no duration is specified, interval
    # duration defaults to be equal to the length of the animation.
    # Note: if loop == 0 and duration > anim duration then the animation
    # will play once and then nothing will happen for the remainder of the
    # interval
    def __init__(self, animControl, loop = 0, duration = 0.0, name=None):
        """__init__(name)
        """
        # Record class specific variables
	self.animControl = animControl
        self.loop = loop
        # Generate unique name if necessary
	if (name == None):
	    name = 'Anim-%d' % AnimInterval.animNum
	    AnimInterval.animNum += 1
        # Compute anim duration
        self.numFrames = self.animControl.getNumFrames()
        if duration == 0.0:
            duration = (float(self.numFrames)/animControl.getFrameRate())
        # Initialize superclass
	Interval.__init__(self, name, duration)
    def updateFunc(self, t, event = IVAL_NONE):
	""" updateFunc(t, event)
	    Go to time t
	"""
        # Update animation based upon current time
        # Pose or stop anim
        if (t >= self.getDuration()) or (event == IVAL_STOP):
            self.animControl.stop()
        elif self.loop == 1:
            if event == IVAL_INIT:
                # Determine the current frame
                frame = (int(self.animControl.getFrameRate() * t) %
                         self.numFrames)
                # Pose anim
                self.animControl.pose(frame)
                # And start loop
                self.animControl.loop(0)
        else:
            # Determine the current frame
            frame = (int(self.animControl.getFrameRate() * t) %
                     self.numFrames)
            # Pose anim
            self.animControl.pose(frame)
            


