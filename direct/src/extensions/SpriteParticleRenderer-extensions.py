
    """
    SpriteParticleRenderer-extensions module: contains methods to extend functionality
    of the SpriteParticleRenderer class
    """

    # Initialize class variable for source file and node for node path textures
    # Will use instance copy of this in functions below
    sourceFileName = 'models/directmodels/smiley'
    sourceNodeName = '**/Happy'

    def getSourceFileName(self):
        # Return instance copy of class variable
        return self.sourceFileName

    def setSourceFileName(self, name):
        # Set instance copy of class variable
        self.sourceFileName = name

    def getSourceNodeName(self):
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
        m = loader.loadModelOnce(modelName)
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
        

