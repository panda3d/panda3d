"""EventInterval module: contains the EventInterval class"""

from PandaModules import *
from Interval import *
from MessengerGlobal import *

class EventInterval(Interval):

    # special methods
    
    def __init__(self, name):
        """__init__(name)
        """
	duration = 0.0
	self.prevt = 0.0
	Interval.__init__(self, name, duration)

    def setT(self, t, entry=0):
	""" setT(t, entry)
	    Go to time t
	"""
	if (t < 0):
	    self.prevt = t
	    return
	elif (t == 0) or (self.prevt < 0):
	    messenger.send(self.name)
	    self.prevt = 0.0
	    
