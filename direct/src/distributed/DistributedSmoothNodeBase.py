"""DistributedSmoothNodeBase module: contains the DistributedSmoothNodeBase class"""

from ClockDelta import *
from direct.task import Task
from direct.showbase.PythonUtil import randFloat, Enum

class DistributedSmoothNodeBase:
    """common base class for DistributedSmoothNode and DistributedSmoothNodeAI
    """
    BroadcastTypes = Enum('FULL, XYH')
    
    def __init__(self):
        pass

    def delete(self):
        # make sure our task is gone
        self.stopPosHprBroadcast()

    ### distributed set pos and hpr functions ###

    ### These functions send the distributed update to set the
    ### appropriate values on the remote side.  These are
    ### composite fields, with all the likely combinations
    ### defined; each function maps (via the dc file) to one or
    ### more component operations on the remote client.

    def d_setSmStop(self):
        self.sendUpdate("setSmStop", [globalClockDelta.getFrameNetworkTime()])
    def d_setSmH(self, h):
        self.sendUpdate("setSmH", [h, globalClockDelta.getFrameNetworkTime()])
    def d_setSmXY(self, x, y):
        self.sendUpdate("setSmXY", [x, y,
                                    globalClockDelta.getFrameNetworkTime()])
    def d_setSmXZ(self, x, z):
        self.sendUpdate("setSmXZ", [x, z,
                                    globalClockDelta.getFrameNetworkTime()])
    def d_setSmPos(self, x, y, z):
        self.sendUpdate("setSmPos", [x, y, z,
                                     globalClockDelta.getFrameNetworkTime()])
    def d_setSmHpr(self, h, p, r):
        self.sendUpdate("setSmHpr", [h, p, r,
                                     globalClockDelta.getFrameNetworkTime()])
    def d_setSmXYH(self, x, y, h):
        self.sendUpdate("setSmXYH", [x, y, h,
                                     globalClockDelta.getFrameNetworkTime()])
    def d_setSmXYZH(self, x, y, z, h):
        self.sendUpdate("setSmXYZH", [x, y, z, h,
                                      globalClockDelta.getFrameNetworkTime()])
    def d_setSmPosHpr(self, x, y, z, h, p, r):
        self.sendUpdate("setSmPosHpr", [x, y, z, h, p, r,
                                        globalClockDelta.getFrameNetworkTime()])

    def b_clearSmoothing(self):
        self.d_clearSmoothing()
        self.clearSmoothing()
    def d_clearSmoothing(self):
        self.sendUpdate("clearSmoothing", [0])

    ### posHprBroadcast ###

    def getPosHprBroadcastTaskName(self):
        # presumably, we have a doId at this point
        return "sendPosHpr-%s" % self.doId

    def stopPosHprBroadcast(self):
        taskMgr.remove(self.getPosHprBroadcastTaskName())
        # Delete this callback because it maintains a reference to self
        self.d_broadcastPosHpr = None

    def startPosHprBroadcast(self, period=.2, stagger=0, type=None):
        BT = DistributedSmoothNodeBase.BroadcastTypes
        if type is None:
            type = BT.FULL
        # set the broadcast type
        self.broadcastType = type

        broadcastFuncs = {
            BT.FULL: self.d_broadcastPosHpr_FULL,
            BT.XYH:  self.d_broadcastPosHpr_XYH,
            }
        self.d_broadcastPosHpr = broadcastFuncs[self.broadcastType]
        
        # Set stagger to non-zero to randomly delay the initial task execution
        # over 'period' seconds, to spread out task processing over time
        # when a large number of SmoothNodes are created simultaneously.
        taskName = self.getPosHprBroadcastTaskName()
        # Set up telemetry optimization variables
        xyz = self.getPos()
        hpr = self.getHpr()

        self.__storeX = xyz[0]
        self.__storeY = xyz[1]
        self.__storeZ = xyz[2]
        self.__storeH = hpr[0]
        self.__storeP = hpr[1]
        self.__storeR = hpr[2]
        self.__storeStop = 0
        self.__epsilon = 0.01
        self.__broadcastPeriod = period
        # Broadcast our initial position
        self.b_clearSmoothing()
        self.d_setSmPosHpr(self.__storeX, self.__storeY, self.__storeZ,
                           self.__storeH, self.__storeP, self.__storeR)
        # remove any old tasks
        taskMgr.remove(taskName)
        # spawn the new task
        delay = 0.
        if stagger:
            delay = randFloat(period)
        taskMgr.doMethodLater(self.__broadcastPeriod + delay,
                              self.__posHprBroadcast, taskName)

    def __posHprBroadcast(self, task):
        # TODO: we explicitly stagger the initial task timing in
        # startPosHprBroadcast; we should at least make an effort to keep
        # this task accurately aligned with its period and starting time.
        self.d_broadcastPosHpr()
        taskName = self.taskName("sendPosHpr")
        taskMgr.doMethodLater(self.__broadcastPeriod,
                              self.__posHprBroadcast, taskName)
        return Task.done

    def d_broadcastPosHpr_FULL(self):
        # send out the minimal bits to describe our new position
        xyz = self.getPos()
        hpr = self.getHpr()

        if abs(self.__storeX - xyz[0]) > self.__epsilon:
            self.__storeX = xyz[0]
            newX = 1
        else:
            newX = 0

        if abs(self.__storeY - xyz[1]) > self.__epsilon:
            self.__storeY = xyz[1]
            newY = 1
        else:
            newY = 0

        if abs(self.__storeZ - xyz[2]) > self.__epsilon:
            self.__storeZ = xyz[2]
            newZ = 1
        else:
            newZ = 0

        if abs(self.__storeH - hpr[0]) > self.__epsilon:
            self.__storeH = hpr[0]
            newH = 1
        else:
            newH = 0

        if abs(self.__storeP - hpr[1]) > self.__epsilon:
            self.__storeP = hpr[1]
            newP = 1
        else:
            newP = 0

        if abs(self.__storeR - hpr[2]) > self.__epsilon:
            self.__storeR = hpr[2]
            newR = 1
        else:
            newR = 0

        # Check for changes:
        if not(newX or newY or newZ or newH or newP or newR):
            # No change
            # Send one and only one "stop" message.
            if not self.__storeStop:
                self.__storeStop = 1
                self.d_setSmStop()
            # print 'no change'
        elif (newH) and not(newX or newY or newZ or newP or newR):
            # Only change in H
            self.__storeStop = 0
            self.d_setSmH(self.__storeH)
            # print ("H change")
        elif (newX or newY) and not(newZ or newH or newP or newR):
            # Only change in X, Y
            self.__storeStop = 0
            self.d_setSmXY(self.__storeX, self.__storeY)
            # print ("XY change")
        elif (newX or newY or newZ) and not(newH or newP or newR):
            # Only change in X, Y, Z
            self.__storeStop = 0
            self.d_setSmPos(self.__storeX, self.__storeY, self.__storeZ)
            # print ("XYZ change")
        elif (newX or newY or newH) and not(newZ or newP or newR):
            # Only change in X, Y, H
            self.__storeStop = 0
            self.d_setSmXYH(self.__storeX, self.__storeY, self.__storeH)
            # print ("XYH change")
        elif (newX or newY or newZ or newH) and not(newP or newR):
            # Only change in X, Y, Z, H
            self.__storeStop = 0
            self.d_setSmXYZH(self.__storeX, self.__storeY, self.__storeZ, self.__storeH)
            # print ("XYZH change")
        else:
            # Other changes
            self.__storeStop = 0
            self.d_setSmPosHpr(self.__storeX, self.__storeY, self.__storeZ,
                               self.__storeH, self.__storeP, self.__storeR)
            # print ("XYZHPR change")

    def d_broadcastPosHpr_XYH(self):
        # send out the minimal bits to describe our new position
        assert not self.isEmpty(), 'DistributedSmoothNode %s has been removed from graph?' % self.doId
        xyz = self.getPos()
        h   = self.getH()

        if abs(self.__storeX - xyz[0]) > self.__epsilon:
            self.__storeX = xyz[0]
            newX = 1
        else:
            newX = 0

        if abs(self.__storeY - xyz[1]) > self.__epsilon:
            self.__storeY = xyz[1]
            newY = 1
        else:
            newY = 0

        if abs(self.__storeH - h) > self.__epsilon:
            self.__storeH = h
            newH = 1
        else:
            newH = 0

        # Check for changes:
        if not(newX or newY or newH):
            # No change
            # Send one and only one "stop" message.
            if not self.__storeStop:
                self.__storeStop = 1
                self.d_setSmStop()
            # print 'no change'
        elif (newH) and not(newX or newY):
            # Only change in H
            self.__storeStop = 0
            self.d_setSmH(self.__storeH)
            # print ("H change")
        elif (newX or newY) and not(newH):
            # Only change in X, Y
            self.__storeStop = 0
            self.d_setSmXY(self.__storeX, self.__storeY)
            # print ("XY change")
        else:
            # Only change in X, Y, H
            self.__storeStop = 0
            self.d_setSmXYH(self.__storeX, self.__storeY, self.__storeH)
            # print ("XYH change")
