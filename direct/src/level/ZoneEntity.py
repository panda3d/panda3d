"""ZoneEntity module: contains the ZoneEntity class"""

import ZoneEntityBase
import BasicEntities

class ZoneEntity(ZoneEntityBase.ZoneEntityBase, BasicEntities.NodePathAttribs):
    def __init__(self, level, entId):
        ZoneEntityBase.ZoneEntityBase.__init__(self, level, entId)

        self.nodePath = self.level.getZoneNode(self.entId)
        if __dev__:
            if self.nodePath is None:
                self.level.reportModelSpecSyncError(
                    'unknown zoneNum %s; zone was removed from model?' %
                    self.entId)
        BasicEntities.NodePathAttribs.initNodePathAttribs(self, doReparent=0)

        # dict of zoneNum to 'visible' reference count
        self.visibleZoneNums = {}

        # inc ref counts for the zones that are always visible from this zone
        self.incrementRefCounts(self.visibility)

    def destroy(self):
        # no need to dec our reference counts
        BasicEntities.NodePathAttribs.destroy(self)
        ZoneEntityBase.ZoneEntityBase.destroy(self)

    def getNodePath(self):
        return self.nodePath

    def getVisibleZoneNums(self):
        return self.visibleZoneNums.keys()

    # call these with lists of zoneNums to increment or decrement their
    # 'visible' reference counts
    # zone is visible as long as its ref count is nonzero
    def incrementRefCounts(self, zoneNumList):
        for zoneNum in zoneNumList:
            self.visibleZoneNums.setdefault(zoneNum, 0)
            self.visibleZoneNums[zoneNum] += 1
    def decrementRefCounts(self, zoneNumList):
        for zoneNum in zoneNumList:
            self.visibleZoneNums[zoneNum] -= 1
            if self.visibleZoneNums[zoneNum] == 0:
                del self.visibleZoneNums[zoneNum]

    if __dev__:
        def setVisibility(self, visibility):
            self.decrementRefCounts(self.visibility)
            self.visibility = visibility
            self.incrementRefCounts(self.visibility)

            self.level.handleVisChange()
