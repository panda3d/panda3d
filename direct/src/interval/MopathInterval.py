"""MopathInterval module: contains the MopathInterval class"""

from Interval import *

import Mopath

class MopathInterval(Interval):
    # Name counter
    mopathNum = 1
    # create MopathInterval DirectNotify category
    notify = directNotify.newCategory('MopathInterval')
    # Class methods
    def __init__(self, mopath, node, name=None):
        """__init__(mopath, node, name)
        """
        self.mopath = mopath 
        self.node = node
        # Generate unique name if necessary
        if (name == None):
            name = 'Mopath-%d' % MopathInterval.mopathNum
            MopathInterval.mopathNum += 1
        # Compute duration
        duration = self.mopath.getMaxT()
        # Initialize superclass
        Interval.__init__(self, name, duration)

    def updateFunc(self, t, event = IVAL_NONE):
        """ updateFunc(t, event)
            Go to time t
        """
        self.mopath.goTo(self.node, t)
        # Print debug information
        self.notify.debug('updateFunc() - %s: t = %f' % (self.name, t))

