"""LevelMgrAI module: contains the LevelMgrAI class"""

from PythonUtil import Functor
import LevelMgrBase

class LevelMgrAI(LevelMgrBase.LevelMgrBase):
    """This class manages editable AI level attributes"""
    def __init__(self, level, entId):
        LevelMgrBase.LevelMgrBase.__init__(self, level, entId)

        # modelZoneNum -> zone entId
        self.level.zoneNum2entId = {}
        # modelZoneNum -> network zoneId
        self.level.zoneNum2zoneId = {}
        # zone entId -> network zoneId
        self.level.zoneEntId2zoneId = {}
        # list of network zoneIDs, sorted by modelZoneNum
        self.level.zoneIds = []

        # listen for every zone creation
        self.accept(self.level.getEntityOfTypeCreateEvent('zone'),
                    self.handleZoneCreated)

    def destroy(self):
        del self.level.zoneIds
        del self.level.zoneEntId2zoneId
        del self.level.zoneNum2zoneId
        del self.level.zoneNum2entId
        LevelMgrBase.LevelMgrBase.destroy(self)

    def handleZoneCreated(self, entId):
        zoneEnt = self.level.getEntity(entId)

        # register the zone's info in the tables
        self.level.zoneNum2entId[zoneEnt.modelZoneNum] = entId
        self.level.zoneNum2zoneId[zoneEnt.modelZoneNum] = zoneEnt.getZoneId()
        self.level.zoneEntId2zoneId[entId] = zoneEnt.getZoneId()

        # TODO: we should delay this until all zone entities have been
        # created on level init
        self.privCreateSortedZoneIdList()

        # listen for the zone's destruction
        self.accept(self.level.getEntityDestroyEvent(entId),
                    Functor(self.handleZoneDestroy, entId))

    def handleZoneDestroy(self, entId):
        zoneEnt = self.level.getEntity(entId)
        # unregister the zone from the maps
        del self.level.zoneNum2entId[zoneEnt.modelZoneNum]
        del self.level.zoneNum2zoneId[zoneEnt.modelZoneNum]
        del self.level.zoneEntId2zoneId[entId]
        # recreate the sorted network zoneId list
        self.privCreateSortedZoneIdList()

    def privCreateSortedZoneIdList(self):
        # sort the model zoneNums
        modelZoneNums = self.level.zoneNum2entId.keys()
        modelZoneNums.sort()

        # create a list of network zoneIds, ordered by their corresponding
        # sorted model zoneNum values
        self.level.zoneIds = []
        for zoneNum in modelZoneNums:
            self.level.zoneIds.append(self.level.zoneNum2zoneId[zoneNum])

        # TODO: if we just added or removed a zone entity AFTER the level
        # was initialized, we need to re-send the zoneId list
