"""PosHprInterval module: contains the PosHprInterval class"""

from PandaModules import *

import Interval

class PosHprInterval(Interval.Interval):

    # special methods
    
    def __init__(self, name, node, pos, hpr, duration):
        """__init__(name, node, pos, hpr, duration)
        """
	self.name = name
	self.node = node
	self.pos = pos
	self.hpr = hpr
	self.duration = duration

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	if (t > self.duration):
	    return
	assert(t >= 0)
	self.node.setPosHpr(self.pos, self.hpr)
