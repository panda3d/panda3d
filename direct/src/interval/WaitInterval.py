"""WaitInterval module: contains the WaitInterval class"""

from PandaModules import *
from Interval import *

class WaitInterval(Interval):

    waitNum = 1

    # special methods
    
    def __init__(self, duration, name=None):
        """__init__(duration, name)
        """
	if (name == None):
	    n = 'Wait-%d' % self.waitNum
	    self.waitNum = self.waitNum + 1
	else:
	    n = name
	Interval.__init__(self, n, duration)
