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
	self.loop = loop
	self.isPlaying = 0
	self.startTime = t0
	self.type = type

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	print 'SoundInterval.setT(): t: %f' % t
	if (t > self.duration):
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
