"""EntityCreator module: contains the EntityCreator class"""

import CutScene
import EntityCreatorBase
import BasicEntities
import DirectNotifyGlobal
import EditMgr
import EntrancePoint
import LevelMgr
import LogicGate
import ZoneEntity
import ModelEntity
import PathEntity
import VisibilityExtender

# some useful constructor functions
# ctor functions must take (level, entId)
# and they must return the entity that was created, or 'nothing'
def nothing(*args):
    """For entities that don't need to be created by the client or don't
    exist on the client at all"""
    return 'nothing'

class EntityCreator(EntityCreatorBase.EntityCreatorBase):
    """
    This class is responsible for creating instances of Entities on the
    client. It can be subclassed to handle more Entity types.
    """
    
    def __init__(self, level):
        EntityCreatorBase.EntityCreatorBase.__init__(self, level)
        self.level = level
        self.privRegisterTypes({
            'cutScene': CutScene.CutScene,
            'editMgr': EditMgr.EditMgr,
            'entrancePoint': EntrancePoint.EntrancePoint,
            'levelMgr': LevelMgr.LevelMgr,
            'logicGate': LogicGate.LogicGate,
            'model' : ModelEntity.ModelEntity,
            'nodepath': BasicEntities.NodePathEntity,
            'path' : PathEntity.PathEntity,
            'visibilityExtender': VisibilityExtender.VisibilityExtender,
            'zone': ZoneEntity.ZoneEntity,
            })

    def doCreateEntity(self, ctor, entId):
        return ctor(self.level, entId)
