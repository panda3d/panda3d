"""ZoneEntityAI module: contains the ZoneEntityAI class"""

import ZoneEntityBase

class ZoneEntityAI(ZoneEntityBase.ZoneEntityBase):
    def __init__(self, level, entId):
        ZoneEntityBase.ZoneEntityBase.__init__(self, level, entId)

        if self.isUberZone():
            print 'uberZone'
            # the uberzone is already allocated
            self.setZoneId(self.level.uberZoneId)
        else:
            # allocate a network zoneId for this zone
            # there is error checking in air.allocateZone
            self.setZoneId(self.level.air.allocateZone())
        
    def destroy(self):
        if not self.isUberZone():
            self.level.air.deallocateZone(self.getZoneId())
        ZoneEntityBase.ZoneEntityBase.destroy(self)
