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
	""" setT(t, entry)
	    Go to time t
	"""
	if (t < 0):
	    return
	elif (entry == 1) and (t > self.duration):
	    self.mopath.goTo(self.node, self.duration)
	else:
	    self.mopath.goTo(self.node, t)
