"""ZoneEntity module: contains the ZoneEntity class"""

import Entity
import BasicEntities

class ZoneEntity(Entity.Entity, BasicEntities.NodePathAttribs):
    def __init__(self, level, entId):
        Entity.Entity.__init__(self, level, entId)

        self.nodePath = self.level.getZoneNode(self.modelZoneNum)
        self.initNodePathAttribs(doReparent=0)

    def getNodePath(self):
        return self.nodePath
