"""VisibilityExtender module: contains the VisibilityExtender class"""

import Entity

class VisibilityExtender(Entity.Entity):
    def __init__(self, level, entId):
        Entity.Entity.__init__(self, level, entId)
        self.initVisExt()

    def initVisExt(self):
        self.extended = 0
        self.zoneEntId = self.getZoneEntId()
        self.eventName = 'switch-%s' % self.event
        self.accept(self.eventName, self.handleEvent)

    def destroyVisExt(self):
        self.ignore(self.eventName)
        if self.extended:
            self.retract()

    def handleEvent(self, doExtend):
        if doExtend:
            if not self.extended:
                self.extend()
        else:
            if self.extended:
                self.retract()

    def extend(self):
        """extend the visibility list"""
        assert not self.extended
        zoneEnt = self.level.getEntity(self.getZoneEntId())
        zoneEnt.incrementRefCounts(self.newZones)
        self.extended = 1
        self.level.handleVisChange()
        
    def retract(self):
        """un-extend the visibility list"""
        assert self.extended
        zoneEnt = self.level.getEntity(self.getZoneEntId())
        zoneEnt.decrementRefCounts(self.newZones)
        self.extended = 0
        self.level.handleVisChange()

    def destroy(self):
        self.destroyVisExt()
        Entity.Entity.destroy(self)

    if __dev__:
        def setNewZones(self, newZones):
            # we need to call destroyVisExt before accepting the new zone set
            self.destroyVisExt()
            self.newZones = newZones
            self.initVisExt()
            
        def attribChanged(self, *args):
            self.destroyVisExt()
            self.initVisExt()
