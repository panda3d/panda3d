"""EditMgrBase module: contains the EditMgrBase class"""

import Entity

class EditMgrBase(Entity.Entity):
    """This class contains EditMgr code shared between AI and client"""
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
