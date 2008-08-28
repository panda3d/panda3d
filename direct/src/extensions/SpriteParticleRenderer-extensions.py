
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
            SpriteParticleRenderer.sourceTextureName = base.config.GetString(
                'particle-sprite-texture', 'phase_3/maps/feyes.jpg')
        # Return instance copy of class variable
        return self.sourceTextureName

    def setSourceTextureName(self, name):
        # Set instance copy of class variable
        self.sourceTextureName = name

    def setTextureFromFile(self, fileName = None):
        if fileName == None:
            fileName = self.getSourceTextureName()
        else:
            self.setSourceTextureName(fileName)
        t = loader.loadTexture(fileName)
        if (t != None):
            self.setTexture(t)
        else:
            print "Couldn't find rendererSpriteTexture file: %s" % fileName

    def getSourceFileName(self):
        if self.sourceFileName == None:
            SpriteParticleRenderer.sourceFileName = base.config.GetString(
                'particle-sprite-model', 'phase_3.5/models/props/suit-particles')
        # Return instance copy of class variable
        return self.sourceFileName

    def setSourceFileName(self, name):
        # Set instance copy of class variable
        self.sourceFileName = name

    def getSourceNodeName(self):
        if self.sourceNodeName == None:
            SpriteParticleRenderer.sourceNodeName = base.config.GetString(
                'particle-sprite-node', '**/fire')
        # Return instance copy of class variable
        return self.sourceNodeName

    def setSourceNodeName(self, name):
        # Set instance copy of class variable
        self.sourceNodeName = name

    def setTextureFromNode(self, modelName = None, nodeName = None):
        if modelName == None:
            modelName = self.getSourceFileName()
        else:
            self.setSourceFileName(modelName)
        if nodeName == None:
            nodeName = self.getSourceNodeName()
        else:
            self.setSourceNodeName(nodeName)
        # Load model and get texture
        m = loader.loadModel(modelName)
        if (m == None):
            print "SpriteParticleRenderer: Couldn't find model: %s!" % modelName 
            return None
        nodeName = self.getSourceNodeName()
        np = m.find(nodeName)
        if np.isEmpty():
            print "SpriteParticleRenderer: Couldn't find node: %s!" % nodeName
            m.removeNode()
            return None
        self.setFromNode(np)
        m.removeNode()
        

