"""LevelMgr module: contains the LevelMgr class"""

import Entity

class LevelMgr(Entity.Entity):
    """This class manages editable client-side level attributes"""
    def __init__(self, level, entId):
        Entity.Entity.__init__(self, level, entId)
        self.level.levelMgr = self
        
        # load the model
        self.geom = loader.loadModel(self.modelFilename)

    def destroy(self):
        del self.level.levelMgr
        self.geom.removeNode()
        del self.geom
        Entity.Entity.destroy(self)
