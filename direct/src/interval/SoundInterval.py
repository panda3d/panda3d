"""SoundInterval module: contains the SoundInterval class"""

from PandaModules import *
from Interval import *

class SoundInterval(Interval):
    # Name counter
    soundNum = 1
    # Class methods
    def __init__(self, sound, loop=0, name=None):
        """__init__(sound, loop, name)
        """
	self.sound = sound
	self.loop = loop
	self.isPlaying = 0
        # Generate unique name if necessary
	if (name == None):
	    name = 'Sound-%d' % SoundInterval.soundNum
	    SoundInterval.soundNum += 1
        # Calculate duration
	duration = self.sound.length()
        # Initialize superclass
	Interval.__init__(self, name, duration)

    def updateFunc(self, t, event = IVAL_NONE):
	""" updateFunc(t, event)
        Go to time t
	"""
        if (t == self.duration) or (event == IVAL_STOP):
            # Stop sound if necessary
	    if (self.isPlaying == 1):
		self.isPlaying = 0
		AudioManager.stop(self.sound)
        elif (event == IVAL_INIT):
            # Set flag
	    self.isPlaying = 1
            # If its within a 20th of a second of the start,
            # start at the beginning
            if (t < 0.05):
                t = 0.0
            # Start sound
            AudioManager.play(self.sound, t)
            # Loop in necessary
	    if (self.loop):
		AudioManager.setLoop(self.sound, 1)
