"""PosHprInterval module: contains the PosHprInterval class"""

from PandaModules import *
from Interval import *

class PosHprInterval(Interval):

    # special methods
    
    def __init__(self, name, node, pos, hpr, duration):
        """__init__(name, node, pos, hpr, duration)
        """
	self.node = node
	self.pos = pos
	self.hpr = hpr
	Interval.__init__(self, name, duration)

    def setT(self, t, entry=0):
	""" setT(t)
	    Go to time t
	"""
	if (entry == 1):
	    self.node.setPosHpr(self.pos, self.hpr)

class PosInterval(Interval):

    # special methods
    
    def __init__(self, name, node, pos, duration):
        """__init__(name, node, pos, duration)
        """
	self.node = node
	self.pos = pos
	Interval.__init__(name, duration)

    def setT(self, t, entry=0):
	""" setT(t)
	    Go to time t
	"""
	if (entry == 1):
	    self.node.setPos(self.pos)

class HprInterval(Interval):

    # special methods
    
    def __init__(self, name, node, hpr, duration):
        """__init__(name, node, hpr, duration)
        """
	self.node = node
	self.hpr = hpr 
	Interval.__init__(name, duration)

    def setT(self, t, entry=0):
	""" setT(t)
	    Go to time t
	"""
	if (entry == 1):
	    self.node.setHpr(self.hpr)
