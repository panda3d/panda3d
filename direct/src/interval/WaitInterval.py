"""WaitInterval module: contains the WaitInterval class"""

from PandaModules import *
from Interval import *

class WaitInterval(Interval):
    # Name counter
    waitNum = 1
    # Class methods
    def __init__(self, duration, name=None):
        """__init__(duration, name)
        """
        # Generate unique name if necessary
        if (name == None):
            name = 'Wait-%d' % WaitInterval.waitNum
            WaitInterval.waitNum += 1
        # Initialize superclass
        Interval.__init__(self, name, duration)


class Wait(WaitInterval):
    def __init__(self, *args, **kw):
        WaitInterval.__init__(self, *args, **kw)
