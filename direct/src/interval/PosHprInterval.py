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

    def setT(self, t, entry=0):
	""" setT(t)
	    Go to time t
	"""
	if (entry == 1):
	    self.node.setPosHpr(self.pos, self.hpr)

class PosInterval(Interval.Interval):

    # special methods
    
    def __init__(self, name, node, pos, duration):
        """__init__(name, node, pos, duration)
        """
	self.name = name
	self.node = node
	self.pos = pos
	self.duration = duration

    def setT(self, t, entry=0):
	""" setT(t)
	    Go to time t
	"""
	if (entry == 1):
	    self.node.setPos(self.pos)

class HprInterval(Interval.Interval):

    # special methods
    
    def __init__(self, name, node, hpr, duration):
        """__init__(name, node, hpr, duration)
        """
	self.name = name
	self.node = node
	self.hpr = hpr 
	self.duration = duration

    def setT(self, t, entry=0):
	""" setT(t)
	    Go to time t
	"""
	if (entry == 1):
	    self.node.setHpr(self.hpr)
