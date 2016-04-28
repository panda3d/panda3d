"""DistributedSmoothNodeBase module: contains the DistributedSmoothNodeBase class"""

from .ClockDelta import *
from direct.task import Task
from direct.showbase.PythonUtil import randFloat, Enum
from panda3d.direct import CDistributedSmoothNodeBase

class DummyTaskClass:
    def setDelay(self, blah):
        pass

DummyTask = DummyTaskClass()

class DistributedSmoothNodeBase:
    """common base class for DistributedSmoothNode and DistributedSmoothNodeAI
    """
    BroadcastTypes = Enum('FULL, XYH, XY')

    def __init__(self):
        self.__broadcastPeriod = None

    def generate(self):
        self.cnode = CDistributedSmoothNodeBase()
        self.cnode.setClockDelta(globalClockDelta)
        self.d_broadcastPosHpr = None

    def disable(self):
        del self.cnode
        # make sure our task is gone
        self.stopPosHprBroadcast()

    def delete(self):
        pass

    def b_clearSmoothing(self):
        self.d_clearSmoothing()
        self.clearSmoothing()
    def d_clearSmoothing(self):
        self.sendUpdate("clearSmoothing", [0])

    ### posHprBroadcast ###

    def getPosHprBroadcastTaskName(self):
        # presumably, we have a doId at this point
        return "sendPosHpr-%s" % self.doId

    def setPosHprBroadcastPeriod(self, period):
        # call this at any time to change the delay between broadcasts
        self.__broadcastPeriod = period

    def getPosHprBroadcastPeriod(self):
        # query the current delay between broadcasts
        return self.__broadcastPeriod

    def stopPosHprBroadcast(self):
        taskMgr.remove(self.getPosHprBroadcastTaskName())
        # Delete this callback because it maintains a reference to self
        self.d_broadcastPosHpr = None

    def posHprBroadcastStarted(self):
        return self.d_broadcastPosHpr != None

    def wantSmoothPosBroadcastTask(self):
        return True

    def startPosHprBroadcast(self, period=.2, stagger=0, type=None):
        if self.cnode == None:
            self.initializeCnode()

        BT = DistributedSmoothNodeBase.BroadcastTypes
        if type is None:
            type = BT.FULL
        # set the broadcast type
        self.broadcastType = type

        broadcastFuncs = {
            BT.FULL: self.cnode.broadcastPosHprFull,
            BT.XYH:  self.cnode.broadcastPosHprXyh,
            BT.XY:  self.cnode.broadcastPosHprXy,
            }
        # this comment is here so it will show up in a grep for 'def d_broadcastPosHpr'
        self.d_broadcastPosHpr = broadcastFuncs[self.broadcastType]

        # Set stagger to non-zero to randomly delay the initial task execution
        # over 'period' seconds, to spread out task processing over time
        # when a large number of SmoothNodes are created simultaneously.
        taskName = self.getPosHprBroadcastTaskName()

        # Set up telemetry optimization variables
        self.cnode.initialize(self, self.dclass, self.doId)

        self.setPosHprBroadcastPeriod(period)
        # Broadcast our initial position
        self.b_clearSmoothing()
        self.cnode.sendEverything()

        # remove any old tasks
        taskMgr.remove(taskName)
        # spawn the new task
        delay = 0.
        if stagger:
            delay = randFloat(period)
        if self.wantSmoothPosBroadcastTask():
            taskMgr.doMethodLater(self.__broadcastPeriod + delay,
                                  self._posHprBroadcast, taskName)

    def _posHprBroadcast(self, task=DummyTask):
        # TODO: we explicitly stagger the initial task timing in
        # startPosHprBroadcast; we should at least make an effort to keep
        # this task accurately aligned with its period and starting time.
        self.d_broadcastPosHpr()
        task.setDelay(self.__broadcastPeriod)
        return Task.again

    def sendCurrentPosition(self):
        # if we're not currently broadcasting, make sure things are set up
        if self.d_broadcastPosHpr is None:
            self.cnode.initialize(self, self.dclass, self.doId)
        self.cnode.sendEverything()

