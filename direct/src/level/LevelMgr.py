"""LevelMgr module: contains the LevelMgr class"""

import Entity

class LevelMgr(Entity.Entity):
    """This class manages editable client-side level attributes"""
    def __init__(self, level, entId):
        Entity.Entity.__init__(self, level, entId)
        self.callSetters('modelFilename')

    def setModelFilename(self, modelFilename):
        self.level.modelFilename = modelFilename
