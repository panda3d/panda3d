"""LevelMgrBase module: contains the LevelMgrBase class"""

import Entity

class LevelMgrBase(Entity.Entity):
    """This class contains LevelMgr code shared by the AI and client"""
    def __init__(self, level, entId):
        Entity.Entity.__init__(self, level, entId)
        self.level.levelMgr = self

    def destroy(self):
        del self.level.levelMgr
        Entity.Entity.destroy(self)
        self.ignoreAll()
