"""MopathInterval module: contains the MopathInterval class"""

from Interval import *

import Mopath

class MopathInterval(Interval):

    mopathNum = 1

    # special methods
    
    def __init__(self, mopath, node, name=None):
        """__init__(mopath, node, name)
        """
	self.node = node	
	self.mopath = mopath 
	duration = self.mopath.getMaxT()
	if (name == None):
	    n = 'Mopath-%d' % self.mopathNum
	    self.mopathNum = self.mopathNum + 1
	else:
	    n = name
	Interval.__init__(self, n, duration)

    def setT(self, t, entry=0):
	""" setT(t)
	    Go to time t
	"""
	assert(t >= 0.0)
	if (entry == 1) and (t > self.duration):
	    self.mopath.goTo(self.node, self.duration)
	else:
	    self.mopath.goTo(self.node, t)
