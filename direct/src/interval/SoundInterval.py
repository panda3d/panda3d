"""SoundInterval module: contains the SoundInterval class"""

from PandaModules import *

import Interval
import WaitInterval

class SoundInterval(Interval.Interval):

    # special methods
    
    def __init__(self, name, sound, loop=0, t0=0.0, 
				type=Interval.Interval.PrevEndRelative):
        """__init__(name, sound, loop, t0, type)
        """
	self.name = name
	self.sound = sound
	self.duration = self.sound.length() 
	if (self.duration == 0.0):
	    Interval.Interval.notify.warning(
		'SoundInterval(): zero length sound - setting duration = 1.0')
	self.duration = 1.0
	self.loop = loop
	self.isPlaying = 0
	self.startTime = t0
	self.type = type

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	if (t > self.duration):
	    if (self.isPlaying == 1):
		AudioManager.stop(self.sound)
	    return
	assert(t >= 0)
	if (t == 0):
	    AudioManager.play(self.sound)
	    if (self.loop):
		AudioManager.setLoop(self.sound, 1)
	elif (self.loop) and (self.isPlaying == 0):
	    AudioManager.play(self.sound)
	    AudioManager.setLoop(self.sound, 1)
	    self.isPlaying = 1
