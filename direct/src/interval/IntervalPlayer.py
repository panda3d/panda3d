"""IntervalPlayer module: contains the IntervalPlayer class"""

from DirectObject import *

import Interval
import Task

class IntervalPlayer(DirectObject):
    """IntervalPlayer class: Plays back Intervals (like a taskMgr)"""

    # special methods
    
    def __init__(self, clock):
        """__init__(clock)
        """
	self.clock = clock
	self.intervals = []

    def addInterval(self, interval):
	""" addInterval(interval)
	"""
	self.intervals.append(interval)

    def removeInterval(self, interval):
	""" removeInterval(interval)
	"""	
	self.intervals.remove(interval)

    def play(self):
	""" play()
	"""
	self.duration = 0.0
	for i in self.intervals:
	    dur = i.getDuration()
	    if (dur > self.duration):
		self.duration = dur		
	self.startT = self.clock.getFrameTime()
	taskMgr.spawnMethodNamed(self.__playTask, 'interval-player')

    def __playTask(self, task):
	t = self.clock.getFrameTime()	
	te = t - self.startT
	if (te <= self.duration):	
	    for i in self.intervals:
	    	i.setT(te)	
	    return Task.cont
	else:
	    return Task.done
