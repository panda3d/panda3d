"""SoundInterval module: contains the SoundInterval class"""

from PandaModules import *
from Interval import *

class SoundInterval(Interval):

    soundNum = 1

    # special methods
    
    def __init__(self, sound, loop=0, name=None):
        """__init__(sound, loop, name)
        """
	self.sound = sound
	duration = self.sound.length() 
	if (duration == 0.0):
	    Interval.notify.warning(
		'SoundInterval(): zero length sound - setting duration = 1.0')
	    duration = 1.0
	self.loop = loop
	self.isPlaying = 0
	if (name == None):
	    n = 'Sound-%d' % self.soundNum
	    self.soundNum = self.soundNum + 1
	else:
	    n = name
	Interval.__init__(self, n, duration)

    def setT(self, t, entry=0):
	""" setT(t)
	    Go to time t
	"""
	if (t > self.duration):
	    if (self.isPlaying == 1):
		AudioManager.stop(self.sound)
		self.isPlaying = 0
	    return
	assert(t >= 0)
	if (entry == 1):
	    self.isPlaying = 1
	    AudioManager.play(self.sound)
	    if (self.loop):
		AudioManager.setLoop(self.sound, 1)
