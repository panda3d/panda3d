"""MopathInterval module: contains the MopathInterval class"""

import Interval
import Mopath
import PosHprInterval

class MopathInterval(Interval.Interval):

    # special methods
    
    def __init__(self, name, mopath, node, t0=0.0, 
				type=Interval.PREVIOUS_END):
        """__init__(name, mopath, node, t0, type)
        """
	self.name = name
	self.node = node	
	self.mopath = mopath 
	self.duration = self.mopath.getMaxT()
	self.startTime = t0
	self.type = type

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	assert(t >= 0.0)
	if (t > self.duration):
	    self.mopath.goTo(self.node, self.duration)
	else:
	    self.mopath.goTo(self.node, t)
