"""LevelMgr module: contains the LevelMgr class"""

from direct.showbase.PythonUtil import Functor
import LevelMgrBase

class LevelMgr(LevelMgrBase.LevelMgrBase):
    """This class manages editable client-side level attributes"""

    def __init__(self, level, entId):
        LevelMgrBase.LevelMgrBase.__init__(self, level, entId)
        
        # load the model
        self.geom = loader.loadModel(self.modelFilename)

        # this will hold the zoneNums/entIds for our own bookkeeping
        self.zoneNums = []

        # zoneNum -> network zoneId
        self.level.zoneNum2zoneId = {}
        # network zoneId -> zoneNum
        self.level.zoneId2zoneNum = {}

        # listen for every zone creation
        self.accept(self.level.getEntityOfTypeCreateEvent('zone'),
                    self.handleZoneCreated)

    def destroy(self):
        del self.level.zoneIds
        del self.level.zoneId2zoneNum
        del self.level.zoneNum2zoneId
        self.geom.removeNode()
        del self.geom
        LevelMgrBase.LevelMgrBase.destroy(self)

    def handleZoneCreated(self, entId):
        zoneEnt = self.level.getEntity(entId)

        # register the zone's info in the tables

        assert (zoneEnt.entId not in self.zoneNums)
        self.zoneNums.append(zoneEnt.entId)
        
        # we can assume that we have a complete list of network zoneIds in
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
        del self.level.zoneId2zoneNum[
            self.level.zoneNum2zoneId[zoneEnt.entId]]
        del self.level.zoneNum2zoneId[zoneEnt.entId]
        self.zoneNums.remove(zoneEnt.entId)
        # reassign the zoneIds (we may not need to do this, if all of the
        # other entities already have their correct zoneId...?)
        self.privAssignZoneIds()

    def privAssignZoneIds(self):
        """assign network zoneIds from self.level.zoneIds, according to
        the zones that are registered so far"""
        # sort the zoneNums
        self.zoneNums.sort()

        # dole out the zoneIds, in increasing order of zoneNum
        for i in range(len(self.zoneNums)):
            zoneNum = self.zoneNums[i]
            zoneEnt = self.level.getEntity(zoneNum)
            zoneId = self.level.zoneIds[i]
            zoneEnt.setZoneId(zoneId)
            # the zoneIds have shifted. update the tables
            self.level.zoneNum2zoneId[zoneNum] = zoneId
            self.level.zoneId2zoneNum[zoneId] = zoneNum
