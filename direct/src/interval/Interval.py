"""Interval module: contains the Interval class"""

from DirectObject import *

class Interval(DirectObject):
    """Interval class: Base class for timeline functionality"""

    # create Interval DirectNotify category
    notify = directNotify.newCategory("Interval")
    #notify.setDebug(1)

    PrevEndRelative = 1
    PrevStartRelative = 2
    TrackStartRelative = 3

    # special methods
    
    def __init__(self, name, duration, t0 = 0.0, type = PrevEndRelative):
        """__init__(name, duration, t0, type)
        """
	self.name = name
	self.duration = duration
	assert(t >= 0.0)
	self.startTime = t0
	self.type = type

    def getName(self):
	""" getName()
	"""
	return self.name

    def getDuration(self):
	""" getDuration()
	"""
	return self.duration

    def getStartTime(self):
	""" getStartTime()
	"""
	return self.startTime

    def setStartTime(self, t, rel=PrevEndRelative):
	""" setStartTime()
	"""
	assert(t >= 0.0)
	self.startTime = t
	self.type = rel

    def getType(self):
	""" getType()
	"""
	return self.type

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	pass
