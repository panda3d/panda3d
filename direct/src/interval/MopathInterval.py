"""MopathInterval module: contains the MopathInterval class"""

from Interval import *

import Mopath

class MopathInterval(Interval):

    # special methods
    
    def __init__(self, name, mopath, node):
        """__init__(name, mopath, node)
        """
	self.node = node	
	self.mopath = mopath 
	duration = self.mopath.getMaxT()
	Interval.__init__(self, name, duration)

    def setT(self, t, entry=0):
	""" setT(t)
	    Go to time t
	"""
	assert(t >= 0.0)
	if (entry == 1) and (t > self.duration):
	    self.mopath.goTo(self.node, self.duration)
	else:
	    self.mopath.goTo(self.node, t)
