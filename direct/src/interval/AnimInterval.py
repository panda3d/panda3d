"""AnimInterval module: contains the AnimInterval class"""

from PandaModules import *
from Interval import *

class AnimInterval(Interval):
    # Name counter
    animNum = 1
    # Class methods
    def __init__(self, animControl, loop=0, name=None):
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
	duration = (float(animControl.getNumFrames()) /
			animControl.getFrameRate())
        # Initialize superclass
	Interval.__init__(self, name, duration)

    def updateFunc(self, t, event = IVAL_NONE):
	""" updateFunc(t, event)
	    Go to time t
	"""
        # Update animation based upon current time
        if (t == self.getDuration()):
	    if (self.isPlaying == 1):
		self.isPlaying = 0
		if (self.loop):
		    self.animControl.stop()
        else:
            # Set flag
	    self.isPlaying = 1
	    # Determine the current frame
	    frame = int(self.animControl.getFrameRate() * t)
            # Pose anim
	    if (self.loop):
		self.animControl.pos(frame)	
		self.animControl.loop(0)
	    else:
		self.animControl.play(frame, self.animControl.getNumFrames())



