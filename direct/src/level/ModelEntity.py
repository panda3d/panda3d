from ToontownGlobals import *
import DirectNotifyGlobal
import BasicEntities

class ModelEntity(BasicEntities.NodePathEntity):
    def __init__(self, level, entId):
        BasicEntities.NodePathEntity.__init__(self, level, entId)
        self.model = None
        self.loadModel()
        
    def destroy(self):
        if self.model:
            self.model.removeNode()
            del self.model
        BasicEntities.NodePathEntity.destroy(self)

    def loadModel(self):
        if self.model:
            self.model.removeNode()
            self.model = None
        if self.modelPath:
            self.model = loader.loadModel(self.modelPath)
            if self.model:
                self.model.reparentTo(self)

    def setModelPath(self, path):
        self.modelPath = path
        self.loadModel()
    
