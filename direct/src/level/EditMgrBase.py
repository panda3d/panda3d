"""EditMgrBase module: contains the EditMgrBase class"""

import Entity
import DirectNotifyGlobal

class EditMgrBase(Entity.Entity):
    """This class contains EditMgr code shared between AI and client"""
    notify = DirectNotifyGlobal.directNotify.newCategory("EditMgr")
    def __init__(self, level, entId):
        Entity.Entity.__init__(self, level, entId)

    def destroy(self):
        Entity.Entity.destroy(self)
        self.ignoreAll()

    if __debug__:
        def setInsertEntity(self, data):
            self.level.levelSpec.insertEntity(data['entId'],
                                              data['entType'],
                                              data['parentEntId'],
                                              )

        def setRemoveEntity(self, data):
            self.level.levelSpec.removeEntity(data['entId'],
                                              )
