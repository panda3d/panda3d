from PandaModules import *
from DirectNotifyGlobal import *
import Interval
import types

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
            CIntervalManager.__init__(self, None)
            cObj = CIntervalManager.getGlobalPtr()
            self.this = cObj.this
            self.userManagesMemory = 0

        else:
            CIntervalManager.__init__(self)

        self.ivals = []

    def addInterval(self, interval):
        index = self.addCInterval(interval, 1)
        self.__storeInterval(interval, index)

    def removeInterval(self, interval):
        index = self.findCInterval(interval.getName())
        if index >= 0:
            self.removeCInterval(index)
            self.ivals[index] = None
            return 1
        return 0

    def getInterval(self, name):
        index = self.findCInterval(name)
        if index >= 0:
            return self.ivals[index]
        return None

    def step(self):
        # Call C++ step, then do all the required Python post-processing.
        CIntervalManager.step(self)

        # It is important to call all of the python callbacks on the
        # just-removed intervals before we call any of the callbacks
        # on the still-running intervals.
        index = self.getNextRemoval()
        while index >= 0:
            self.ivals[index].privPostEvent()
            self.ivals[index] = None
            index = self.getNextRemoval()

        index = self.getNextEvent()
        while index >= 0:
            self.ivals[index].privPostEvent()
            index = self.getNextEvent()
        
    def __storeInterval(self, interval, index):
        while index >= len(self.ivals):
            self.ivals.append(None)
        self.ivals[index] = interval

    def __repr__(self):
        return self.__str__()

# The global IntervalManager object.
ivalMgr = IntervalManager(1)
