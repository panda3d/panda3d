"""EntityCreator module: contains the EntityCreator class"""

import BasicEntities
import DirectNotifyGlobal

# some useful constructor functions
# ctor functions must take (level, entId)
def nothing(level, entId):
    """For entities that don't need to be created by the client"""
    return None

class EntityCreator:
    """This class is responsible for creating instances of Entities on the
    client. It can be subclassed to handle more Entity types."""
    notify = DirectNotifyGlobal.directNotify.newCategory('EntityCreator')
    
    def __init__(self):
        self.entType2Ctor = {}
        self.privRegisterTypes({
            'logicGate': nothing,
            'nodepath': BasicEntities.NodePathEntity,
            })

    def privRegisterType(self, entType, ctor):
        assert(not self.entType2Ctor.has_key(entType))
        self.entType2Ctor[entType] = ctor

    def privRegisterTypes(self, type2ctor):
        for entType, ctor in type2ctor.items():
            self.privRegisterType(entType, ctor)

    def createEntity(self, entType, level, entId):
        if not self.entType2Ctor.has_key(entType):
            EntityCreator.notify.warning(
                'createEntity(entType=%s) not found' % entType)
            return None
        return self.entType2Ctor[entType](level, entId)
