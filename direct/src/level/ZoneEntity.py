"""ZoneEntity module: contains the ZoneEntity class"""

import ZoneEntityBase
import BasicEntities

class ZoneEntity(ZoneEntityBase.ZoneEntityBase, BasicEntities.NodePathAttribs):
    __attribs = (
        'modelZoneNum',
        )
    
    def __init__(self, level, entId):
        ZoneEntityBase.ZoneEntityBase.__init__(self, level, entId)

        self.nodePath = self.level.getZoneNode(self.modelZoneNum)
        self.initNodePathAttribs(doReparent=0)

    def getNodePath(self):
        return self.nodePath
