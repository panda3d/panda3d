"""EntityCreatorAI.py: contains methods for creation of Entities"""

from PythonUtil import Functor
import DistributedBeanBarrelAI
import DistributedLiftAI
import DistributedDoorEntityAI
import DistributedGagBarrelAI
#import DistributedStomperPairAI
import DistributedSwitchAI
import DistributedStomperAI
import AndLoEntityAI
import OrLoEntityAI
#import XorLoEntity


def cDE(AIclass, air, levelDoId, entId, zoneId):
    """create a distributed entity"""
    ent = AIclass(air, levelDoId, entId)
    ent.generateWithRequired(zoneId)
    return ent

def nothing(air, levelDoId, entId, zoneId):
    """Create entity that doesn't have a server side representation."""
    return None

# Server (AI) side factory functions:
EntityType2Ctor = {
    # Map entity type name to constructor function that takes
    # (air, level doId, entId, zoneId)
    'beanBarrel': Functor(cDE, DistributedBeanBarrelAI.DistributedBeanBarrelAI),
    'door': DistributedDoorEntityAI.DistributedDoorEntityAI,
    'gagBarrel': Functor(cDE, DistributedGagBarrelAI.DistributedGagBarrelAI),
    'lift': Functor(cDE, DistributedLiftAI.DistributedLiftAI),
    'nodepath': nothing,
    'platform': nothing,
    'stomper': Functor(cDE, DistributedStomperAI.DistributedStomperAI),
    'switch': DistributedSwitchAI.DistributedSwitchAI,

    'andLogicObject': AndLoEntityAI.AndLoEntityAI,
    'orLogicObject': OrLoEntityAI.OrLoEntityAI,
    'xorLogicObject': nothing, #XorLoEntity.XorLoEntity,
    }

def createEntity(entType, air, levelDoId, entId, zoneId):
    if not EntityType2Ctor.has_key(entType):
        print "createEntity(entType=%s, air=%s, levelDoId=%s, entId=%s, zoneId=%s) not found"%(
                entType, "the air", levelDoId, entId, zoneId)
        return None
    return EntityType2Ctor[entType](air, levelDoId, entId, zoneId)
