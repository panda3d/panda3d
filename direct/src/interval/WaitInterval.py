"""WaitInterval module: contains the Wait class"""

from PandaModules import *

import Interval

class Wait(Interval.Interval):

    waitNum = 1 

    # special methods
    
    def __init__(self, duration, name=None):
        """__init__(duration, name, t0, type)
        """
	if (name == None):
	    self.name = 'wait-%d' % Wait.waitNum
	    Wait.waitNum = Wait.waitNum + 1
	else:
	    self.name = name
	self.duration = duration 
