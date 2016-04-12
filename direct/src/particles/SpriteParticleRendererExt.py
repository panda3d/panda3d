from panda3d.physics import SpriteParticleRenderer


class SpriteParticleRendererExt(SpriteParticleRenderer):
    """
    Contains methods to extend functionality
    of the SpriteParticleRenderer class
    """

    # Initialize class variables for texture, source file and node for texture and
    # node path textures to None.  These will be initialized to a hardcoded default
    # or whatever the user specifies in his/her Configrc variable the first time they
    # are accessed
    # Will use instance copy of this in functions below
    sourceTextureName = None
    sourceFileName = None
    sourceNodeName = None

    def getSourceTextureName(self):
        if self.sourceTextureName == None:
            SpriteParticleRendererExt.sourceTextureName = base.config.GetString(
                'particle-sprite-texture', 'maps/lightbulb.rgb')
        # Return instance copy of class variable
        return self.sourceTextureName

    def setSourceTextureName(self, name):
        # Set instance copy of class variable
        self.sourceTextureName = name

    def setTextureFromFile(self, fileName = None):
        if fileName == None:
            fileName = self.getSourceTextureName()

        t = loader.loadTexture(fileName)
        if (t != None):
            self.setTexture(t, t.getYSize())
            self.setSourceTextureName(fileName)
            return True
        else:
            print("Couldn't find rendererSpriteTexture file: %s" % fileName)
            return False

    def addTextureFromFile(self, fileName = None):
        if(self.getNumAnims() == 0):
            return self.setTextureFromFile(fileName)

        if fileName == None:
            fileName = self.getSourceTextureName()

        t = loader.loadTexture(fileName)
        if (t != None):
            self.addTexture(t, t.getYSize())
            return True
        else:
            print("Couldn't find rendererSpriteTexture file: %s" % fileName)
            return False

    def getSourceFileName(self):
        if self.sourceFileName == None:
            SpriteParticleRendererExt.sourceFileName = base.config.GetString(
                'particle-sprite-model', 'models/misc/smiley')
        # Return instance copy of class variable
        return self.sourceFileName

    def setSourceFileName(self, name):
        # Set instance copy of class variable
        self.sourceFileName = name

    def getSourceNodeName(self):
        if self.sourceNodeName == None:
            SpriteParticleRendererExt.sourceNodeName = base.config.GetString(
                'particle-sprite-node', '**/*')
        # Return instance copy of class variable
        return self.sourceNodeName

    def setSourceNodeName(self, name):
        # Set instance copy of class variable
        self.sourceNodeName = name

    def setTextureFromNode(self, modelName = None, nodeName = None, sizeFromTexels = False):
        if modelName == None:
            modelName = self.getSourceFileName()
            if nodeName == None:
                nodeName = self.getSourceNodeName()

        # Load model and get texture
        m = loader.loadModel(modelName)
        if (m == None):
            print("SpriteParticleRendererExt: Couldn't find model: %s!" % modelName)
            return False

        np = m.find(nodeName)
        if np.isEmpty():
            print("SpriteParticleRendererExt: Couldn't find node: %s!" % nodeName)
            m.removeNode()
            return False

        self.setFromNode(np, modelName, nodeName, sizeFromTexels)
        self.setSourceFileName(modelName)
        self.setSourceNodeName(nodeName)
        m.removeNode()
        return True

    def addTextureFromNode(self, modelName = None, nodeName = None, sizeFromTexels = False):
        if(self.getNumAnims() == 0):
            return self.setTextureFromNode(modelName, nodeName, sizeFromTexels)

        if modelName == None:
            modelName = self.getSourceFileName()
            if nodeName == None:
                nodeName = self.getSourceNodeName()

        # Load model and get texture
        m = loader.loadModel(modelName)
        if (m == None):
            print("SpriteParticleRendererExt: Couldn't find model: %s!" % modelName)
            return False

        np = m.find(nodeName)
        if np.isEmpty():
            print("SpriteParticleRendererExt: Couldn't find node: %s!" % nodeName)
            m.removeNode()
            return False

        self.addFromNode(np, modelName, nodeName, sizeFromTexels)
        m.removeNode()

        return True
