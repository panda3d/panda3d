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
        self.base = base
        self.loader = PandaLoader()
        
    # model loading funcs
    def loadModel(self, modelPath):
        """loadModel(self, string)
        Attempt to load a model from given file path, return
        a nodepath to the model if successful or None otherwise."""
        Loader.notify.debug("Loading model: %s" % (modelPath) )
        node = self.loader.loadSync(Filename(modelPath))
        if (node != None):
            nodePath = self.base.hidden.attachNewNode(node)
        else:
            nodePath = None
        return nodePath

    def loadModelOnce(self, modelPath):
        """loadModelOnce(self, string)
        Attempt to load a model from modelPool, if not present
        then attempt to load it from disk. Return a nodepath to
        the model if successful or None otherwise"""
        Loader.notify.debug("Loading model once: %s" % (modelPath))
        node = ModelPool.loadModel(modelPath)
        if (node != None):
            nodePath = self.base.hidden.attachNewNode(node)
        else:
            nodePath = None
        return nodePath
    
    def loadModelCopy(self, modelPath):
        """loadModelCopy(self, string)
        Attempt to load a model from modelPool, if not present
        then attempt to load it from disk. Return a nodepath to
        a copy of the model if successful or None otherwise"""
        Loader.notify.debug("Loading model copy: %s" % (modelPath))
        node = ModelPool.loadModel(modelPath)
        if (node != None):
            return (NodePath(node).copyTo(self.base.hidden))
        else:
            return None

    def loadModelNode(self, modelPath):
        """loadModelNode(self, string)
        This is like loadModelOnce in that it loads a model from the
        modelPool, but it does not then instance it to hidden and it
        returns a Node instead of a NodePath.  This is particularly
        useful for special models like fonts that you don't care about
        where they're parented to, and you don't want a NodePath
        anyway--it prevents accumulation of instances of the font
        model under hidden."""
        
        Loader.notify.debug("Loading model once node: %s" % (modelPath))
        return ModelPool.loadModel(modelPath)

    def unloadModel(self, modelPath):
	"""unloadModel(self, string)
	"""
	Loader.notify.debug("Unloading model: %s" % (modelPath))
	ModelPool.releaseModel(modelPath)
            
    # texture loading funcs
    def loadTexture(self, texturePath, alphaPath = None):
        """loadTexture(self, string)
        Attempt to load a texture from the given file path using
        TexturePool class. Returns None if not found"""

        if alphaPath == None:
            Loader.notify.debug("Loading texture: %s" % (texturePath) )
            texture = TexturePool.loadTexture(texturePath)
        else:
            Loader.notify.debug("Loading texture: %s %s" % (texturePath, alphaPath) )
            texture = TexturePool.loadTexture(texturePath, alphaPath)
        return texture

    def unloadTexture(self, texture):
	"""unloadTexture(self, texture)
	"""
        Loader.notify.debug("Unloading texture: %s" % (texture) )
	TexturePool.releaseTexture(texture)

    # sound loading funcs
    def loadSound(self, soundPath):
        """loadSound(self, string)
        Attempt to load a sound from the given file path using
        Cary's sound class. Returns None if not found"""
        Loader.notify.debug("Loading sound: %s" % (soundPath) )
        sound = AudioPool.loadSound(soundPath)
        return sound

    def unloadSound(self, sound):
	"""unloadSound(self, sound)
	"""
        if sound:
            Loader.notify.debug("Unloading sound: %s" % (sound) )
            AudioPool.releaseSound(sound)









