"""LevelMgr module: contains the LevelMgr class"""

from PythonUtil import Functor
import LevelMgrBase

class LevelMgr(LevelMgrBase.LevelMgrBase):
    """This class manages editable client-side level attributes"""

    def __init__(self, level, entId):
        LevelMgrBase.LevelMgrBase.__init__(self, level, entId)
        
        # load the model
        self.geom = loader.loadModel(self.modelFilename)

        # modelZoneNum -> zone entId
        self.level.zoneNum2entId = {}
        # modelZoneNum -> network zoneId
        self.level.zoneNum2zoneId = {}
        # zone entId -> network zoneId
        self.level.zoneEntId2zoneId = {}

        # listen for every zone creation
        self.accept(self.level.getEntityOfTypeCreateEvent('zone'),
                    self.handleZoneCreated)

    def destroy(self):
        del self.level.zoneIds
        del self.level.zoneEntId2zoneId
        del self.level.zoneNum2zoneId
        del self.level.zoneNum2entId
        self.geom.removeNode()
        del self.geom
        LevelMgrBase.LevelMgrBase.destroy(self)

    def handleZoneCreated(self, entId):
        zoneEnt = self.level.getEntity(entId)

        # register the zone's info in the tables
        # right off the bat, we have the zone's modelZoneNum and entId
        self.level.zoneNum2entId[zoneEnt.modelZoneNum] = entId

        # we can assume that we have a complete list of zoneIds in
        # self.level.zoneIds. As each zone entity is created, set up
        # as if we have all of the zone entities. This allows dynamic
        # zone entity creation and deletion during editing.
        # TODO: we should delay this until all zone entities have been
        # created on level init
        self.privAssignZoneIds()

        # listen for the zone's destruction
        self.accept(self.level.getEntityDestroyEvent(entId),
                    Functor(self.handleZoneDestroy, entId))

    def handleZoneDestroy(self, entId):
        zoneEnt = self.level.getEntity(entId)
        # unregister the zone from the maps
        del self.level.zoneNum2entId[zoneEnt.modelZoneNum]
        del self.level.zoneNum2zoneId[zoneEnt.modelZoneNum]
        del self.level.zoneEntId2zoneId[entId]
        # reassign the zoneIds (we may not need to do this, if all of the
        # other entities already have their correct zoneId...?)
        self.privAssignZoneIds()

    def privAssignZoneIds(self):
        """assign zoneIds from self.level.zoneIds, according to the zones
        that are registered so far in self.level.zoneNum2entId"""
        # sort the model zoneNums
        modelZoneNums = self.level.zoneNum2entId.keys()
        modelZoneNums.sort()

        # dole out the zoneIds, in increasing order of zoneNum
        for i in range(len(modelZoneNums)):
            zoneNum = modelZoneNums[i]
            entId = self.level.zoneNum2entId[zoneNum]
            zoneEnt = self.level.getEntity(entId)
            zoneEnt.setZoneId(self.level.zoneIds[i])

        # the zoneIds have shifted. update the tables
        for entId in self.level.zoneNum2entId.values():
            zoneEnt = self.level.getEntity(entId)
            zoneId = zoneEnt.getZoneId()
            self.level.zoneNum2zoneId[zoneEnt.modelZoneNum] = zoneId
            self.level.zoneEntId2zoneId[entId] = zoneId
