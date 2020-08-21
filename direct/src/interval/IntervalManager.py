"""Defines the IntervalManager class as well as the global instance of
this class, ivalMgr."""

__all__ = ['IntervalManager', 'ivalMgr']

from panda3d.core import *
from panda3d.direct import *
from direct.directnotify.DirectNotifyGlobal import *
from direct.showbase import EventManager
import fnmatch

class IntervalManager(CIntervalManager):

    # This is a Python-C++ hybrid class.  IntervalManager is a Python
    # extension of the C++ class CIntervalManager; the main purpose of
    # the Python extensions is to add support for Python-based
    # intervals (like MetaIntervals).

    def __init__(self, globalPtr = 0):
        # Pass globalPtr == 1 to the constructor to trick it into
        # "constructing" a Python wrapper around the global
        # CIntervalManager object.
        if globalPtr:
            self.cObj = CIntervalManager.getGlobalPtr()
            Dtool_BorrowThisReference(self, self.cObj)
            self.dd = self
        else:
            CIntervalManager.__init__(self)
        self.eventQueue = EventQueue()
        self.MyEventmanager = EventManager.EventManager(self.eventQueue)
        self.setEventQueue(self.eventQueue)
        self.ivals = []
        self.removedIvals = {}

    def addInterval(self, interval):
        index = self.addCInterval(interval, 1)
        self.__storeInterval(interval, index)

    def removeInterval(self, interval):
        index = self.findCInterval(interval.getName())
        if index >= 0:
            self.removeCInterval(index)
            if index < len(self.ivals):
                self.ivals[index] = None
            return 1
        return 0

    def getInterval(self, name):
        index = self.findCInterval(name)
        if index >= 0:
            if index < len(self.ivals) and self.ivals[index]:
                return self.ivals[index]
            # It must be a C-only interval.
            return self.getCInterval(index)
        return None

    def getIntervalsMatching(self, pattern):
        ivals = []

        count = 0
        maxIndex = self.getMaxIndex()
        for index in range(maxIndex):
            ival = self.getCInterval(index)
            if ival and \
               fnmatch.fnmatchcase(ival.getName(), pattern):
                # Finish and remove this interval.  Finishing it
                # automatically removes it.
                count += 1
                if index < len(self.ivals) and self.ivals[index]:
                    # Get the python version if we have it
                    ivals.append(self.ivals[index])
                else:
                    # Otherwise, it's a C-only interval.
                    ivals.append(ival)

        return ivals

    def finishIntervalsMatching(self, pattern):
        ivals = self.getIntervalsMatching(pattern)
        for ival in ivals:
            ival.finish()
        return len(ivals)

    def pauseIntervalsMatching(self, pattern):
        ivals = self.getIntervalsMatching(pattern)
        for ival in ivals:
            ival.pause()
        return len(ivals)

    def step(self):
        # This method should be called once per frame to perform all
        # of the per-frame processing on the active intervals.
        # Call C++ step, then do the Python stuff.
        CIntervalManager.step(self)
        self.__doPythonCallbacks()

    def interrupt(self):
        # This method should be called during an emergency cleanup
        # operation, to automatically pause or finish all active
        # intervals tagged with autoPause or autoFinish set true.
        # Call C++ interrupt, then do the Python stuff.
        CIntervalManager.interrupt(self)
        self.__doPythonCallbacks()

    def __doPythonCallbacks(self):
        # This method does all of the required Python post-processing
        # after performing some C++-level action.
        # It is important to call all of the python callbacks on the
        # just-removed intervals before we call any of the callbacks
        # on the still-running intervals.
        index = self.getNextRemoval()
        while index >= 0:
            # We have to clear the interval first before we call
            # privPostEvent() on it, because the interval might itself
            # try to add a new interval.
            ival = self.ivals[index]
            self.ivals[index] = None
            ival.privPostEvent()
            index = self.getNextRemoval()

        index = self.getNextEvent()
        while index >= 0:
            self.ivals[index].privPostEvent()
            index = self.getNextEvent()

        # Finally, throw all the events on the custom event queue.
        # These are the done events that may have been generated in
        # C++.  We use a custom event queue so we can service all of
        # these immediately, rather than waiting for the global event
        # queue to be serviced (which might not be till next frame).
        self.MyEventmanager.doEvents()


    def __storeInterval(self, interval, index):
        while index >= len(self.ivals):
            self.ivals.append(None)
        assert self.ivals[index] == None or self.ivals[index] == interval
        self.ivals[index] = interval

#: The global IntervalManager object.
ivalMgr = IntervalManager(1)
