"""MopathInterval module: contains the MopathInterval class"""

import Interval
import Mopath
import PosHprInterval

class MopathInterval(Interval.Interval):

    # special methods
    
    def __init__(self, name, mopath, node):
        """__init__(name, mopath, node)
        """
	self.name = name
	self.node = node	
	self.mopath = mopath 
	self.duration = self.mopath.getMaxT()

    def setT(self, t, entry=0):
	""" setT(t)
	    Go to time t
	"""
	assert(t >= 0.0)
	if (entry == 1) and (t > self.duration):
	    self.mopath.goTo(self.node, self.duration)
	else:
	    self.mopath.goTo(self.node, t)
