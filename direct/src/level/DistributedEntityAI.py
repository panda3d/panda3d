import DistributedObjectAI
import Entity
import DirectNotifyGlobal

class DistributedEntityAI(DistributedObjectAI.DistributedObjectAI,
                          Entity.Entity):
    notify = DirectNotifyGlobal.directNotify.newCategory(
        'DistributedEntityAI')

    def __init__(self, level, entId):
        DistributedObjectAI.DistributedObjectAI.__init__(self, level.air)
        self.levelDoId = level.doId
        Entity.Entity.__init__(self, level, entId)

    def generate(self):
        self.notify.debug('generate')
        DistributedObjectAI.DistributedObjectAI.generate(self)

    def destroy(self):
        self.notify.debug('destroy')
        Entity.Entity.destroy(self)
        self.requestDelete()

    def delete(self):
        self.notify.debug('delete')
        DistributedObjectAI.DistributedObjectAI.delete(self)

    def getLevelDoId(self):
        return self.levelDoId

    def getEntId(self):
        return self.entId

    if __debug__:
        def setParentEntId(self, parentEntId):
            self.parentEntId = parentEntId
            # switch to new zone
            newZoneId = self.getZoneEntity().getZoneId()
            self.sendSetZone(newZoneId)
