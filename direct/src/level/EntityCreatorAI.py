"""EntityCreatorAI module: contains the EntityCreatorAI class"""

import DirectNotifyGlobal
import LogicGateAI

# some useful constructor functions
# ctor functions must take (air, level doId, entId, zoneId)
def createDistributedEntity(AIclass, air, levelDoId, entId, zoneId):
    """create a distributed entity and call generate"""
    ent = AIclass(air, levelDoId, entId)
    ent.generateWithRequired(zoneId)
    return ent

def nothing(air, levelDoId, entId, zoneId):
    """Create entity that doesn't have a server side representation."""
    return None

class EntityCreatorAI:
    """This class is responsible for creating instances of Entities on the AI.
    It can be subclassed to handle more Entity types."""
    notify = DirectNotifyGlobal.directNotify.newCategory('EntityCreatorAI')

    def __init__(self):
        self.entType2Ctor = {}
        self.privRegisterTypes({
            'logicGate': LogicGateAI.LogicGateAI,
            'nodepath': nothing,
            })

    def privRegisterType(self, entType, ctor):
        assert(not self.entType2Ctor.has_key(entType))
        self.entType2Ctor[entType] = ctor

    def privRegisterTypes(self, type2ctor):
        for entType, ctor in type2ctor.items():
            self.privRegisterType(entType, ctor)

    def createEntity(self, entType, air, levelDoId, entId, zoneId):
        if not self.entType2Ctor.has_key(entType):
            EntityCreatorAI.notify.warning(
                'createEntity(entType=%s, levelDoId=%s, '
                'entId=%s, zoneId=%s) not found' %
                (entType, levelDoId, entId, zoneId))
            return None
        return self.entType2Ctor[entType](air, levelDoId, entId, zoneId)
