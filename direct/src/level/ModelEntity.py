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

        # HACK SDN: special code for moving crate wall collisions down
        if self.modelPath == "phase_9/models/cogHQ/woodCrateB":
            cNode = self.find("**/wall")
            if not cNode.isEmpty():
                cNode.setZ(-.5)

    def setModelPath(self, path):
        self.modelPath = path
        self.loadModel()
    
