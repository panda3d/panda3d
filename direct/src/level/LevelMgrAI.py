"""LevelMgrAI module: contains the LevelMgrAI class"""

import Entity

class LevelMgrAI(Entity.Entity):
    """This class manages editable AI level attributes"""
    def __init__(self, level, entId):
        Entity.Entity.__init__(self, level, entId)
        self.level.levelMgr = self
        
