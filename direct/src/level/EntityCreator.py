"""EntityCreator.py: contains methods for creation of Entities"""

from PythonUtil import Functor
#import PlatformEntity
import BasicEntities

def nothing(level, entId):
    """For entities that don't need to be created by the client"""
    return None

# Client-side entity ctors:
EntityType2Ctor = {
    # Map entity type name to constructor function that takes
    # (level, entId)
    'beanBarrel': nothing,
    'door': nothing,
    'gagBarrel': nothing,
    'lift': nothing,
    'nodepath': BasicEntities.NodePathEntity,
    #'platform': PlatformEntity.PlatformEntity,
    'stomper': nothing,
    'switch': nothing,

    'andLogicObject': nothing,
    'orLogicObject': nothing,
    'xorLogicObject': nothing, #XorLoEntity.XorLoEntity,
    }

# TODO: what other args will be required?
def createEntity(entType, level, entId):
    if not EntityType2Ctor.has_key(entType):
        print "createEntity(entType=%s, entId=%s) not found"%(
                entType, entId)
        return None
    return EntityType2Ctor[entType](level, entId)
