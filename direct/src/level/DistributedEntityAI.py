import DistributedObjectAI
import Entity
import DirectNotifyGlobal

class DistributedEntityAI(DistributedObjectAI.DistributedObjectAI,
                          Entity.Entity):
    notify = DirectNotifyGlobal.directNotify.newCategory(
        'DistributedEntityAI')

    def __init__(self, air, levelDoId, entId):
        DistributedObjectAI.DistributedObjectAI.__init__(self, air)
        self.levelDoId = levelDoId
        level = self.air.doId2do[self.levelDoId]
        Entity.Entity.__init__(self, level, entId)
        # get our spec data
        self.initializeEntity()

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
