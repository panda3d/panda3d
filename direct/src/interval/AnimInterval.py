"""AnimInterval module: contains the AnimInterval class"""

from PandaModules import *
from Interval import *

class AnimInterval(Interval):
    # Name counter
    animNum = 1
    # Class methods
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
            


