"""AnimInterval module: contains the AnimInterval class"""

from PandaModules import *
from Interval import *

class AnimInterval(Interval):

    animNum = 1

    # special methods
    
    def __init__(self, animControl, loop=0, name=None):
        """__init__(name)
        """
	self.animControl = animControl
	duration = (float(animControl.getNumFrames()) / 
			animControl.getFrameRate())
	self.loop = loop
	if (name == None):
	    n = 'Anim-%d' % self.animNum
	    self.animNum = self.animNum + 1
	else:
	    n = name
	Interval.__init__(self, n, duration)

    def setT(self, t, entry=0):
	""" setT(t, entry)
	    Go to time t
	"""
	if (t < 0):
	    return
	elif (t > self.duration):
	    if (self.isPlaying == 1):
		self.isPlaying = 0
		if (self.loop):
		    self.animControl.stop()
	    return
	elif (entry == 1):
	    self.isPlaying = 1
	    # Determine the current frame
	    frame = int(self.animControl.getFrameRate() * t)
	    if (self.loop):
		self.animControl.pos(frame)	
		self.animControl.loop(0)
	    else:
		self.animControl.play(frame, self.animControl.getNumFrames())
