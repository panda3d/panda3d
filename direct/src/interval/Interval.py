"""Interval module: contains the Interval class"""

from DirectObject import *

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

    def getName(self):
	""" getName()
	"""
	return self.name

    def getDuration(self):
	""" getDuration()
	"""
	return self.duration

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	pass
