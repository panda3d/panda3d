"""ZoneEntityAI module: contains the ZoneEntityAI class"""

import Entity

class ZoneEntityAI(Entity.Entity):
    def __init__(self, level, entId):
        Entity.Entity.__init__(self, level, entId)
        # allocate a network zoneId for each zone
        self.level.allocateZoneId()
