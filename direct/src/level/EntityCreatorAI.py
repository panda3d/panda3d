"""EntityCreatorAI module: contains the EntityCreatorAI class"""

import DirectNotifyGlobal
import LogicGateAI
import LevelMgrAI

# some useful constructor functions
# ctor functions for distributed entities must take
#  (air, level doId, entId, zoneId)
# ctor functions for non-distributed entities must take
#  (level, entId)
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
            'levelMgr': LevelMgrAI.LevelMgrAI,
            'logicGate': LogicGateAI.LogicGateAI,
            'nodepath': nothing,
            })

    def privRegisterType(self, entType, ctor):
        if self.entType2Ctor.has_key(entType):
            EntityCreatorAI.notify.warning(
                'replacing %s ctor %s with %s' %
                (entType, self.entType2Ctor[entType], ctor))
        self.entType2Ctor[entType] = ctor

    def privRegisterTypes(self, type2ctor):
        for entType, ctor in type2ctor.items():
            self.privRegisterType(entType, ctor)

    def createEntity(self, entType, level, entId, air=None, zoneId=None):
        """zoneId=None indicates a non-distributed entity"""
        if not self.entType2Ctor.has_key(entType):
            EntityCreatorAI.notify.warning(
                'createEntity(entType=%s, levelDoId=%s, '
                'entId=%s, zoneId=%s) not found' %
                (entType, level.doId, entId, zoneId))
            return None

        if zoneId is None:
            return self.entType2Ctor[entType](level, entId)
        else:
            levelDoId = level.doId
            return self.entType2Ctor[entType](air, levelDoId, entId, zoneId)
