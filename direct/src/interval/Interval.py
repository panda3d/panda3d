"""Interval module: contains the Interval class"""

from DirectObject import *

import ClockObject
import Task

class Interval(DirectObject):
    """Interval class: Base class for timeline functionality"""

    # create Interval DirectNotify category
    notify = directNotify.newCategory("Interval")
    #notify.setDebug(1)

    # special methods
    
    def __init__(self, name, duration):
        """__init__(name, duration)
        """
	self.name = name
	self.duration = duration
	self.clock = ClockObject.ClockObject.getGlobalClock()

    def getName(self):
	""" getName()
	"""
	return self.name

    def getDuration(self):
	""" getDuration()
	"""
	return self.duration

    def setT(self, t, entry=0):
	""" setT(t, entry)
	    Go to time t
	"""
	pass

    def setFinalT(self):
	""" setFinalT()
	"""
	self.setT(self.getDuration(), entry=1)

    def play(self, t0=0.0, duration=0.0, scale=1.0):
        """ play(t0, duration)
        """
        self.startT = self.clock.getFrameTime() - t0
	assert(scale > 0.0)
	self.scale = scale
	self.firstTime = 1
        if (duration == 0.0):
            self.playDuration = self.duration
        else:
            self.playDuration = duration
        taskMgr.spawnMethodNamed(self.__playTask, self.name + '-play')

    def stop(self):
        """ stop()
        """
        taskMgr.removeTasksNamed(self.name + '-play')

    def __playTask(self, task):
        """ __playTask(task)
        """
        t = self.clock.getFrameTime()
        te = (t - self.startT) * self.scale
        if (te <= self.playDuration):
	    if (self.firstTime):
		self.setT(te, entry=1)
		self.firstTime = 0
	    else:
            	self.setT(te)
            return Task.cont
        else:
            return Task.done

    def printParams(self, indent=0):
	""" printParams(indent)
	"""
	space = ''
	for l in range(indent):
	    space = space + ' '
	print (space + self.name + ' dur: %.2f' % self.duration)
