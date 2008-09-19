
from pandac.PandaModules import *

from direct.task import Task
from DistributedNodeAI import DistributedNodeAI
from CartesianGridBase import CartesianGridBase

class DistributedCartesianGridAI(DistributedNodeAI, CartesianGridBase):
    notify = directNotify.newCategory("DistributedCartesianGridAI")

    RuleSeparator = ":"

    def __init__(self, air, startingZone, gridSize, gridRadius, cellWidth,
            style="Cartesian"):
        DistributedNodeAI.__init__(self, air)
        self.style = style
        self.startingZone = startingZone
        self.gridSize = gridSize
        self.gridRadius = gridRadius
        self.cellWidth = cellWidth

        # Keep track of all AI objects added to the grid
        self.gridObjects = {}
        self.updateTaskStarted = 0

    def delete(self):
        DistributedNodeAI.delete(self)
        self.stopUpdateGridTask()

    def isGridParent(self):
        # If this distributed object is a DistributedGrid return 1.
        # 0 by default
        return 1

    def getCellWidth(self):
        return self.cellWidth
    
    def getParentingRules(self):
        self.notify.debug("calling getter")
        rule = ("%i%s%i%s%i" % (self.startingZone, self.RuleSeparator,
                                self.gridSize, self.RuleSeparator,
                                self.gridRadius))
        return [self.style, rule]

    # Reparent and setLocation on av to DistributedOceanGrid
    def addObjectToGrid(self, av, useZoneId=-1, startAutoUpdate=True):
        self.notify.debug("setting parent to grid %s" % self)
        avId = av.doId

        # Create a grid parent
        #gridParent = self.attachNewNode("gridParent-%s" % avId)
        #self.gridParents[avId] = gridParent
        self.gridObjects[avId] = av

        # Put the avatar on the grid
        self.handleAvatarZoneChange(av, useZoneId)

        if (not self.updateTaskStarted) and startAutoUpdate:
            self.startUpdateGridTask()

    def removeObjectFromGrid(self, av):
        # TODO: WHAT LOCATION SHOULD WE SET THIS TO?
        #av.wrtReparentTo(self.parentNP)
        #av.setLocation(self.air.districtId, 1000)

        # Remove grid parent for this av
        avId = av.doId
        if self.gridObjects.has_key(avId):
            del self.gridObjects[avId]

        # Stop task if there are no more av's being managed
        if len(self.gridObjects) == 0:
            self.stopUpdateGridTask()

    #####################################################################
    # updateGridTask
    # This task is similar to the processVisibility task for the local client.
    # A couple differences:
    #  - we are not doing setInterest on the AI (that is a local client
    #    specific call).
    #  - we assume that the moving objects on the grid are parented to a
    #    gridParent, and are broadcasting their position relative to that
    #    gridParent.  This makes the task's math easy.  Just check to see
    #    when our position goes out of the current grid cell.  When it does,
    #    call handleAvatarZoneChange

    def startUpdateGridTask(self):
        self.stopUpdateGridTask()
        self.updateTaskStarted = 1
        taskMgr.add(self.updateGridTask, self.taskName("updateGridTask"))

    def stopUpdateGridTask(self):
        taskMgr.remove(self.taskName("updateGridTask"))
        self.updateTaskStarted = 0

    def updateGridTask(self, task=None):
        # Run through all grid objects and update their parents if needed
        missingObjs = []
        for avId in self.gridObjects.keys():
            av = self.gridObjects[avId]
            # handle a missing object after it is already gone?
            if (av.isEmpty()):
                task.setDelay(1.0)
                del self.gridObjects[avId]
                continue
            pos = av.getPos()
            if ((pos[0] < 0 or pos[1] < 0) or
                (pos[0] > self.cellWidth or pos[1] > self.cellWidth)):
                # we are out of the bounds of this current cell
                self.handleAvatarZoneChange(av)
        # Do this every second, not every frame
        if (task):
            task.setDelay(1.0)
        return Task.again

    def handleAvatarZoneChange(self, av, useZoneId=-1):
        # Calculate zone id
        # Get position of av relative to this grid
        if (useZoneId == -1):
            pos = av.getPos(self)
            zoneId = self.getZoneFromXYZ(pos)
        else:
            # zone already calculated, position of object might not
            # give the correct zone
            pos = None
            zoneId = useZoneId

        if not self.isValidZone(zoneId):
            self.notify.warning(
                "%s handleAvatarZoneChange %s: not a valid zone (%s) for pos %s" %(self.doId, av.doId, zoneId, pos))                     
            return

        # Set the location on the server.
        # setLocation will update the gridParent
        av.b_setLocation(self.doId, zoneId)

    def handleSetLocation(self, av, parentId, zoneId):
        pass
        #if (av.parentId != parentId):
            # parent changed, need to look up instance tree
            # to see if avatar's named area location information
            # changed
            #av.requestRegionUpdateTask(regionegionUid)

