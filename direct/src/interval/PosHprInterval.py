"""PosHprInterval module: contains the PosHprInterval class"""

from PandaModules import *
from Interval import *

class PosHprInterval(Interval):

    posHprNum = 1

    # special methods
    
    def __init__(self, node, pos, hpr, duration, name=None):
        """__init__(node, pos, hpr, duration, name)
        """
	self.node = node
	self.pos = pos
	self.hpr = hpr
	if (name == None):
	    n = 'PosHpr-%d' % self.posHprNum
	    self.posHprNum = self.posHprNum + 1
	else:
	    n = name
	Interval.__init__(self, n, duration)

    def setT(self, t, entry=0):
	""" setT(t, entry)
	    Go to time t
	"""
	if (t < 0):
	    return
	elif (entry == 1):
	    self.node.setPosHpr(self.pos, self.hpr)

class PosInterval(Interval):

    posNum = 1

    # special methods
    
    def __init__(self, node, pos, duration, name=None):
        """__init__(node, pos, duration, name)
        """
	self.node = node
	self.pos = pos
	if (name == None):
	    n = 'Pos-%d' % self.posNum
	    self.posNum = self.posNum + 1
	else:
	    n = name
	Interval.__init__(n, duration)

    def setT(self, t, entry=0):
	""" setT(t, entry)
	    Go to time t
	"""
	if (t < 0):
	    return
	elif (entry == 1):
	    self.node.setPos(self.pos)

class HprInterval(Interval):

    hprNum = 1

    # special methods
    
    def __init__(self, node, hpr, duration, name=None):
        """__init__(node, hpr, duration, name)
        """
	self.node = node
	self.hpr = hpr 
	if (name == None):
	    n = 'Hpr-%d' % self.hprNum
	    self.hprNum = self.hprNum + 1
	else:
	    n = name
	Interval.__init__(n, duration)

    def setT(self, t, entry=0):
	""" setT(t, entry)
	    Go to time t
	"""
	if (t < 0):
	    return
	elif (entry == 1):
	    self.node.setHpr(self.hpr)
