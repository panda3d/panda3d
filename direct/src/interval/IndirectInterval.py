"""IndirectInterval module: contains the IndirectInterval class"""

from PandaModules import *
from DirectNotifyGlobal import *
import LerpInterval

class IndirectInterval(LerpInterval.LerpFunctionInterval):
    """
    This class can be used to play samples of another interval, so
    that only a subset of the interval is played, or the time is
    compressed, or some such nonsense.

    It keeps a reference to the interval itself and repeatedly calls
    setT() on it, rather than actually starting the interval or
    copying its members like Sequence() or Parallel().  This means two
    IndirectIntervals that operate on the same nested interval may
    have some interaction that you should be aware of.
    """

    # Interval counter
    indirectIntervalNum = 1

    notify = directNotify.newCategory('IndirectInterval')

    # Class methods
    def __init__(self, interval,
                 startT = 0, endT = None, playRate = 1,
                 duration = None, blendType = 'noBlend', name = None):
        self.interval = interval
        
        if endT == None:
            endT = interval.getDuration()

        if duration == None:
            duration = abs(endT - startT) / playRate

        if (name == None):
            name = ('IndirectInterval-%d' %
                    IndirectInterval.indirectIntervalNum)
            IndirectInterval.indirectIntervalNum += 1

        LerpInterval.LerpFunctionInterval.__init__(
            self, self.__step, fromData = startT, toData = endT,
            duration = duration, blendType = blendType, name = name)

    def __step(self, t):
        self.interval.setT(t)
        
