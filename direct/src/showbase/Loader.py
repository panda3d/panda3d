"""Loader module: contains the Loader class"""

from PandaModules import *
from DirectNotifyGlobal import *


class Loader:

    """Loader class: contains method to load models, sounds and code"""

    notify = directNotify.newCategory("Loader")
    
    # special methods
    def __init__(self, base):
        """__init__(self)
        Loader constructor"""
        self.__base = base
        self.__loader = PandaLoader()
        #self.__texturePool = TexturePool()
        #self.__modelPool = ModelPool()
        #self.__audioPool = AudioPool()
        
    # model loading funcs
    def loadModel(self, modelPath):
        """loadModel(self, string)
        Attempt to load a model from given file path, return
        a nodepath to the model if successful or None otherwise."""
        Loader.notify.info("Loading model: %s" % (modelPath) )
        node = self.__loader.loadSync(Filename(modelPath))
        if (node != None):
            nodePath = self.__base.hidden.attachNewNode(node)
        else:
            nodePath = None
        return nodePath
    
    def loadModelOnce(self, modelPath):
        """loadModelOnce(self, string)
        Attempt to load a model from modelPool, if not present
        then attempt to load it from disk. Return a nodepath to
        the model if successful or None otherwise"""
        Loader.notify.info("Loading model once: %s" % (modelPath))
        node = ModelPool.loadModel(modelPath)
        if (node != None):
            nodePath = self.__base.hidden.attachNewNode(node)
        else:
            nodePath = None
        return nodePath
    
    def loadModelCopy(self, modelPath):
        """loadModelCopy(self, string)
        Attempt to load a model from modelPool, if not present
        then attempt to load it from disk. Return a nodepath to
        a copy of the model if successful or None otherwise"""
        Loader.notify.info("Loading model copy: %s" % (modelPath))
        # utilize load once goodness
        nodePath = self.loadModelOnce(modelPath)
        if (nodePath != None):
            return (nodePath.copyTo(self.__base.hidden))
        else:
            return None
            
    # texture loading funcs
    def loadTexture(self, texturePath):
        """loadTexture(self, string)
        Attempt to load a texture from the given file path using
        TexturePool class. Returns None if not found"""
        Loader.notify.info("Loading texture: %s" % (texturePath) )
        texture = TexturePool.loadTexture(texturePath)
        return texture

    # sound loading funcs
    def loadSound(self, soundPath):
        """loadSound(self, string)
        Attempt to load a sound from the given file path using
        Cary's sound class. Returns None if not found"""
        Loader.notify.info("Loading sound: %s" % (soundPath) )
        sound = AudioPool.loadSound(soundPath)
        return sound









