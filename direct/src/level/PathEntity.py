from ToontownGlobals import *
import DirectNotifyGlobal
import BasicEntities
import GoonPathData

class PathEntity(BasicEntities.NodePathEntity):
    def __init__(self, level, entId):
        BasicEntities.NodePathEntity.__init__(self, level, entId)
        self.path = GoonPathData.Paths[self.pathIndex]
            
    def destroy(self):
        BasicEntities.NodePathEntity.destroy(self)

    def setPathIndex(self, pathIndex):
        self.pathIndex = pathIndex
        self.path = GoonPathData.Paths[self.pathIndex]
    
    
        
