"""ZoneEntityBase module: contains the ZoneEntityBase class"""

import Entity
import Level

class ZoneEntityBase(Entity.Entity):
    def __init__(self, level, entId):
        Entity.Entity.__init__(self, level, entId)
        self.zoneId = None

    def destroy(self):
        del self.zoneId
        Entity.Entity.destroy(self)

    def isUberZone(self):
        return self.entId == Level.Level.UberZoneEntId

    def setZoneId(self, zoneId):
        """set the network zoneId that this zone entity corresponds to"""
        self.zoneId = zoneId

    def getZoneId(self):
        return self.zoneId
