"""MopathInterval module: contains the MopathInterval class"""

import Interval

# import Mopath

class MopathInterval(Interval.Interval):
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
        Interval.Interval.__init__(self, name, duration)

    def privStep(self, t):
        """ updateFunc(t, event)
            Go to time t
        """
        self.mopath.goTo(self.node, t)
        self.state = CInterval.SStarted
        self.currT = t

