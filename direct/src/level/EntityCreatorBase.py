"""EntityCreatorBase module: contains the EntityCreatorBase class"""

import DirectNotifyGlobal

class EntityCreatorBase:
    """This class is responsible for creating instances of Entities on the
    AI and on the client. It must be subclassed to specify what entity
    types it can create, and to provide the creation implementation."""
    notify = DirectNotifyGlobal.directNotify.newCategory('EntityCreator')

    def __init__(self, level):
        self.level = level
        self.entType2Ctor = {}

    def createEntity(self, entId):
        entType = self.level.getEntityType(entId)
        
        if not self.entType2Ctor.has_key(entType):
            self.notify.error('unknown entity type: %s (ent%s)' %
                              (entType, entId))

        # inheritor must define doCreateEntity
        return self.doCreateEntity(self.entType2Ctor[entType], entId)

    def privRegisterType(self, entType, ctor):
        if self.entType2Ctor.has_key(entType):
            self.notify.warning('replacing %s ctor %s with %s' %
                                (entType, self.entType2Ctor[entType], ctor))
        self.entType2Ctor[entType] = ctor

    def privRegisterTypes(self, type2ctor):
        for entType, ctor in type2ctor.items():
            self.privRegisterType(entType, ctor)
