"""LevelMgrAI module: contains the LevelMgrAI class"""

from PythonUtil import Functor
import LevelMgrBase

class LevelMgrAI(LevelMgrBase.LevelMgrBase):
    """This class manages editable AI level attributes"""
    def __init__(self, level, entId):
        LevelMgrBase.LevelMgrBase.__init__(self, level, entId)

        # zoneNum -> network zoneId
        self.level.zoneNum2zoneId = {}
        # list of network zoneIDs, sorted by zoneNum
        self.level.zoneIds = []

        # listen for every zone creation
        self.accept(self.level.getEntityOfTypeCreateEvent('zone'),
                    self.handleZoneCreated)

    def destroy(self):
        del self.level.zoneIds
        del self.level.zoneNum2zoneId
        LevelMgrBase.LevelMgrBase.destroy(self)

    def handleZoneCreated(self, entId):
        zoneEnt = self.level.getEntity(entId)

        # register the zone's info in the tables
        self.level.zoneNum2zoneId[zoneEnt.entId] = zoneEnt.getZoneId()

        # TODO: we should delay this until all zone entities have been
        # created on level init
        self.privCreateSortedZoneIdList()

        # listen for the zone's destruction
        self.accept(self.level.getEntityDestroyEvent(entId),
                    Functor(self.handleZoneDestroy, entId))

    def handleZoneDestroy(self, entId):
        zoneEnt = self.level.getEntity(entId)
        # unregister the zone from the tables
        del self.level.zoneNum2zoneId[zoneEnt.entId]
        # recreate the sorted network zoneId list
        self.privCreateSortedZoneIdList()

    def privCreateSortedZoneIdList(self):
        # sort the zoneNums
        zoneNums = self.level.zoneNum2zoneId.keys()
        zoneNums.sort()

        # create a list of network zoneIds, ordered by their corresponding
        # sorted model zoneNum values
        self.level.zoneIds = []
        for zoneNum in zoneNums:
            self.level.zoneIds.append(self.level.zoneNum2zoneId[zoneNum])

        # TODO: if we ever allow dynamic insertion and removal of zone
        # entities, and we just added or removed a zone entity AFTER the level
        # was initialized, we would need to re-send the zoneId list
