"""EntityCreatorAI module: contains the EntityCreatorAI class"""

import EntityCreatorBase
import LogicGateAI
import EditMgrAI
import LevelMgrAI
import ZoneEntityAI
from PythonUtil import Functor

# some useful constructor functions
# ctor functions for entities must take
#  (level, entId, zoneId)

# this func creates distributed entities whose constructors take
#  (air, level doId, entId)
# and do not generate themselves
def createDistributedEntity(AIclass, level, entId, zoneId):
    """create a distributed entity and call generate"""
    ent = AIclass(level, entId)
    ent.generateWithRequired(zoneId)
    return ent

# this func creates local entities whose constructors take
#  (level, entId)
def createLocalEntity(AIclass, level, entId, zoneId):
    """create a local entity"""
    ent = AIclass(level, entId)

# take any number of args to support local and distributed entities
def nothing(*args):
    """Create entity that doesn't have a server side representation."""
    return None

class EntityCreatorAI(EntityCreatorBase.EntityCreatorBase):
    """This class is responsible for creating instances of Entities on the AI.
    It can be subclassed to handle more Entity types."""

    def __init__(self, level):
        EntityCreatorBase.EntityCreatorBase.__init__(self, level)
        
        # create short aliases for ctor funcs
        cLE = createLocalEntity

        self.privRegisterTypes({
            'cutScene': nothing,
            'editMgr': Functor(cLE, EditMgrAI.EditMgrAI),
            'levelMgr': Functor(cLE, LevelMgrAI.LevelMgrAI),
            'logicGate': Functor(cLE, LogicGateAI.LogicGateAI),
            'model' : nothing,
            'nodepath': nothing,
            'zone': Functor(cLE, ZoneEntityAI.ZoneEntityAI),
            })

    def doCreateEntity(self, ctor, entId):
        zoneId = self.level.getEntityZoneId(entId)
        self.notify.debug('creating entity %s in zone %s' % (entId, zoneId))
        return ctor(self.level, entId, zoneId)
