"""MopathInterval module: contains the MopathInterval class"""

import Interval
import Mopath

class MopathInterval(Interval.Interval):

    # special methods
    
    def __init__(self, name, mopath, node):
        """__init__(name, mopath, node)
        """
	self.name = name
	self.node = node	
	self.mopath = mopath 
	self.duration = self.mopath.getMaxT()

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	if (t > self.duration):
	    return
	assert(t >= 0)
	self.mopath.goTo(self.node, t)
