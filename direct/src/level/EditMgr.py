"""EditMgr module: contains the EditMgr class"""

import Entity

class EditMgr(Entity.Entity):
    """This class handles entity/level functionality used by the level editor"""
    def __init__(self, level, entId):
        Entity.Entity.__init__(self, level, entId)

    def destroy(self):
        Entity.Entity.destroy(self)
        self.ignoreAll()

    def setInsertEntity(self, data):
        self.level.levelSpec.insertEntity(data['entId'],
                                          data['entType'],
                                          data['parentEntId'],
                                          )

    def setRemoveEntity(self, data):
        self.level.levelSpec.removeEntity(data['entId'],
                                          )

    def getSpecSaveEvent(self):
        return 'requestSave-%s' % self.level.levelId
    def setRequestSave(self, data):
        messenger.send(self.getSpecSaveEvent())
