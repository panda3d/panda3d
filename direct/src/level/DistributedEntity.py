import DistributedObject
import Entity
import DirectNotifyGlobal

class DistributedEntity(DistributedObject.DistributedObject, Entity.Entity):
    notify = DirectNotifyGlobal.directNotify.newCategory(
        'DistributedEntity')

    def __init__(self, cr):
        DistributedObject.DistributedObject.__init__(self, cr)

    def generateInit(self):
        DistributedEntity.notify.debug('generateInit')
        DistributedObject.DistributedObject.generateInit(self)
        # load stuff

    def generate(self):
        DistributedEntity.notify.debug('generate')
        DistributedObject.DistributedObject.generate(self)

    def setLevelDoId(self, levelDoId):
        DistributedEntity.notify.debug('setLevelDoId: %s' % levelDoId)
        self.levelDoId = levelDoId

    def setEntId(self, entId):
        DistributedEntity.notify.debug('setEntId: %s' % entId)
        self.entId = entId

    def announceGenerate(self):
        DistributedEntity.notify.debug('announceGenerate')

        # ask our level obj for our spec data
        level = toonbase.tcr.doId2do[self.levelDoId]
        Entity.Entity.__init__(self, level, self.entId)

        DistributedObject.DistributedObject.announceGenerate(self)

    def disable(self):
        DistributedEntity.notify.debug('disable')
        # stop things

    def delete(self):
        DistributedEntity.notify.debug('delete')
        # unload things
