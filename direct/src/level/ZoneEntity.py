"""ZoneEntity module: contains the ZoneEntity class"""

import ZoneEntityBase
import BasicEntities

class ZoneEntity(ZoneEntityBase.ZoneEntityBase, BasicEntities.NodePathAttribs):
    def __init__(self, level, entId):
        ZoneEntityBase.ZoneEntityBase.__init__(self, level, entId)

        self.nodePath = self.level.getZoneNode(self.modelZoneNum)
        BasicEntities.NodePathAttribs.initNodePathAttribs(self, doReparent=0)

    def destroy(self):
        BasicEntities.NodePathAttribs.destroy(self)
        ZoneEntityBase.ZoneEntityBase.destroy(self)

    def getNodePath(self):
        return self.nodePath

    def setVisibility(self, visibility):
        self.visibility = visibility
        self.level.handleVisChange()
