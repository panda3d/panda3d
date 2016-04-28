"""IndirectInterval module: contains the IndirectInterval class"""

__all__ = ['IndirectInterval']

from panda3d.core import *
from panda3d.direct import *
from direct.directnotify.DirectNotifyGlobal import *
from . import Interval
from . import LerpBlendHelpers

class IndirectInterval(Interval.Interval):
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

        self.startAtStart = (startT == 0)
        self.endAtEnd = (endT == None or endT == interval.getDuration())

        if endT == None:
            endT = interval.getDuration()

        if duration == None:
            duration = abs(endT - startT) / playRate

        if (name == None):
            name = ('IndirectInterval-%d' %
                    IndirectInterval.indirectIntervalNum)
            IndirectInterval.indirectIntervalNum += 1

        self.startT = startT
        self.endT = endT
        self.deltaT = endT - startT
        self.blendType = LerpBlendHelpers.getBlend(blendType)

        Interval.Interval.__init__(self, name, duration)

    def __calcT(self, t):
        return self.startT + self.deltaT * self.blendType(t / self.duration)

    def privInitialize(self, t):
        state = self.interval.getState()
        if state == CInterval.SInitial or state == CInterval.SFinal:
            self.interval.privInitialize(self.__calcT(t))
        else:
            self.interval.privStep(self.__calcT(t))
        self.currT = t
        self.state = CInterval.SStarted
        self.interval.privPostEvent()

    def privInstant(self):
        state = self.interval.getState()
        if (state == CInterval.SInitial or state == CInterval.SFinal) and \
           self.endAtEnd:
            self.interval.privInstant()
            self.currT = self.getDuration()
            self.interval.privPostEvent()
            self.intervalDone()
        else:
            if state == CInterval.SInitial or state == CInterval.SFinal:
                self.interval.privInitialize(self.startT)
            else:
                self.interval.privStep(self.startT)
            self.privFinalize()

    def privStep(self, t):
        self.interval.privStep(self.__calcT(t))
        self.currT = t
        self.state = CInterval.SStarted
        self.interval.privPostEvent()

    def privFinalize(self):
        if self.endAtEnd:
            self.interval.privFinalize()
        else:
            self.interval.privStep(self.endT)
            self.interval.privInterrupt()
        self.currT = self.getDuration()
        self.state = CInterval.SFinal
        self.interval.privPostEvent()
        self.intervalDone()

    def privReverseInitialize(self, t):
        state = self.interval.getState()
        if state == CInterval.SInitial or state == CInterval.SFinal:
            self.interval.privReverseInitialize(self.__calcT(t))
        else:
            self.interval.privStep(self.__calcT(t))
        self.currT = t
        self.state = CInterval.SStarted
        self.interval.privPostEvent()

    def privReverseInstant(self):
        state = self.interval.getState()
        if (state == CInterval.SInitial or state == CInterval.SFinal) and \
           self.startAtStart:
            self.interval.privReverseInstant()
            self.currT = 0
            self.interval.privPostEvent()
        else:
            if state == CInterval.SInitial or state == CInterval.SFinal:
                self.interval.privReverseInitialize(self.endT)
            else:
                self.interval.privStep(self.endT)
            self.privReverseFinalize()

    def privReverseFinalize(self):
        if self.startAtStart:
            self.interval.privReverseFinalize()
        else:
            self.interval.privStep(self.endT)
            self.interval.privInterrupt()
        self.currT = 0
        self.state = CInterval.SInitial
        self.interval.privPostEvent()

    def privInterrupt(self):
        self.interval.privInterrupt()
        self.interval.privPostEvent()
