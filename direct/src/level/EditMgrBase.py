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

    if __dev__:
        def setInsertEntity(self, data):
            # tell the level who created this entity
            self.level.setEntityCreatorUsername(data['entId'], data['username'])
            # create the entity
            self.level.levelSpec.insertEntity(data['entId'],
                                              data['entType'],
                                              data['parentEntId'],
                                              )
            # clear out the attrib, it shouldn't be kept in the spec
            self.level.levelSpec.doSetAttrib(self.entId, 'insertEntity',
                                             None)

        def setRemoveEntity(self, data):
            self.level.levelSpec.removeEntity(data['entId'],
                                              )
            # clear out the attrib, it shouldn't be kept in the spec
            self.level.levelSpec.doSetAttrib(self.entId, 'removeEntity',
                                             None)
