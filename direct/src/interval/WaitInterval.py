"""WaitInterval module: contains the Wait class"""

from PandaModules import *

import Interval

class Wait(Interval.Interval):

    waitNum = 1 

    # special methods
    
    def __init__(self, duration, name = None):
        """__init__(duration, name)
        """
	if (name == None):
	    self.name = 'wait-%d' % self.waitNum
	    self.waitNum = self.waitNum + 1
	else:
	    self.name = name
	self.duration = duration 
