"""PosHprInterval module: contains the PosHprInterval class"""

from PandaModules import *

import Interval

class PosHprInterval(Interval.Interval):

    # special methods
    
    def __init__(self, name, node, pos, hpr, duration, t0=0.0,
				type=Interval.PREVIOUS_END):
        """__init__(name, node, pos, hpr, duration, t0, type)
        """
	self.name = name
	self.node = node
	self.pos = pos
	self.hpr = hpr
	self.duration = duration
	self.startTime = t0
	self.type = type

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	self.node.setPosHpr(self.pos, self.hpr)

class PosInterval(Interval.Interval):

    # special methods
    
    def __init__(self, name, node, pos, duration, t0=0.0,
				type=Interval.PREVIOUS_END):
        """__init__(name, node, pos, duration, t0, type)
        """
	self.name = name
	self.node = node
	self.pos = pos
	self.duration = duration
	self.startTime = t0
	self.type = type

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	self.node.setPos(self.pos)

class HprInterval(Interval.Interval):

    # special methods
    
    def __init__(self, name, node, hpr, duration, t0=0.0,
				type=Interval.PREVIOUS_END):
        """__init__(name, node, hpr, duration, t0, type)
        """
	self.name = name
	self.node = node
	self.hpr = hpr 
	self.duration = duration
	self.startTime = t0
	self.type = type

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	self.node.setHpr(self.hpr)
