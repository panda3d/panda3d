"""Loader module: contains the Loader class"""

from PandaModules import *
from DirectNotifyGlobal import *

# You can specify a phaseChecker callback to check
# a modelPath to see if it is being loaded in the correct
# phase
phaseChecker = None

class Loader:

    """Loader class: contains method to load models, sounds and code"""

    notify = directNotify.newCategory("Loader")
    modelCount = 0
    
    # special methods
    def __init__(self, base):
        """__init__(self)
        Loader constructor"""
        self.base = base
        self.loader = PandaLoader()
        
    # model loading funcs
    def loadModel(self, modelPath, fMakeNodeNamesUnique = 0):
        """loadModel(self, string)
        Attempt to load a model from given file path, return
        a nodepath to the model if successful or None otherwise."""
        assert(Loader.notify.debug("Loading model: %s" % (modelPath) ))
        if phaseChecker:
            phaseChecker(modelPath)
        node = self.loader.loadSync(Filename(modelPath))
        if (node != None):
            nodePath = NodePath(node)
            if fMakeNodeNamesUnique:
                self.makeNodeNamesUnique(nodePath, 0)
        else:
            nodePath = None
        return nodePath

    def loadModelOnce(self, modelPath):
        """loadModelOnce(self, string)
        Attempt to load a model from modelPool, if not present
        then attempt to load it from disk. Return a nodepath to
        the model if successful or None otherwise"""
        assert(Loader.notify.debug("Loading model once: %s" % (modelPath)))
        if phaseChecker:
            phaseChecker(modelPath)
        node = ModelPool.loadModel(modelPath)
        if (node != None):
            nodePath = NodePath.anyPath(node)
        else:
            nodePath = None
        return nodePath

    def loadModelOnceUnder(self, modelPath, underNode):
        """loadModelOnceUnder(self, string, string | node | NodePath)
        Behaves like loadModelOnce, but also implicitly creates a new
        node to attach the model under, which helps to differentiate
        different instances.

        underNode may be either a node name, or a NodePath or a Node
        to an already-existing node.

        This is useful when you want to load a model once several
        times before parenting each instance somewhere, or when you
        want to load a model and immediately set a transform on it.
        But also consider loadModelCopy().
        
        """

        assert(Loader.notify.debug("Loading model once: %s under %s" % (modelPath, underNode)))
        if phaseChecker:
            phaseChecker(modelPath)
        node = ModelPool.loadModel(modelPath)
        if (node != None):
            nodePath = NodePath(underNode)
            nodePath.attachNewNode(node)
        else:
            nodePath = None
        return nodePath
    
    def loadModelCopy(self, modelPath):
        """loadModelCopy(self, string)
        Attempt to load a model from modelPool, if not present
        then attempt to load it from disk. Return a nodepath to
        a copy of the model if successful or None otherwise"""
        assert(Loader.notify.debug("Loading model copy: %s" % (modelPath)))
        if phaseChecker:
            phaseChecker(modelPath)
        node = ModelPool.loadModel(modelPath)
        if (node != None):
            return (NodePath(node.copySubgraph()))
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
        model under hidden.

        However, if you're loading a font, see loadFont(), below.
        """
        assert(Loader.notify.debug("Loading model once node: %s" % (modelPath)))
        if phaseChecker:
            phaseChecker(modelPath)
        return ModelPool.loadModel(modelPath)

    def unloadModel(self, modelPath):
        """unloadModel(self, string)
        """
        assert(Loader.notify.debug("Unloading model: %s" % (modelPath)))
        ModelPool.releaseModel(modelPath)

    # font loading funcs
    def loadFont(self, modelPath, priority = 0, faceIndex = 0,
                 spaceAdvance = None, pointSize = None,
                 pixelsPerUnit = None, scaleFactor = None,
                 textureMargin = None, polyMargin = None,
                 minFilter = None, magFilter = None,
                 anisotropicDegree = None,
                 lineHeight = None):
        """loadFont(self, string)

        This loads a special model as a TextFont object, for rendering
        text with a TextNode.  A font file must be either a special
        egg file (or bam file) generated with egg-mkfont, or a
        standard font file (like a TTF file) that is supported by
        FreeType.
        """
        assert(Loader.notify.debug("Loading font: %s" % (modelPath)))
        if phaseChecker:
            phaseChecker(modelPath)

        # Check the filename extension.
        fn = Filename(modelPath)
        extension = fn.getExtension()

        if extension == "" or extension == "egg" or extension == "bam":
            # A traditional model file is probably an old-style,
            # static font.
            node = ModelPool.loadModel(modelPath)
            if node == None:
                # If we couldn't load the model, at least return an
                # empty font.
                font = StaticTextFont(PandaNode("empty"))
            else:
                # Create a temp node path so you can adjust priorities
                nodePath = NodePath(node)
                nodePath.adjustAllPriorities(priority)
                # Now create the text font from the node
                font = StaticTextFont(node)
        else:
            # Otherwise, it must be a new-style, dynamic font.  Maybe
            # it's just a TTF file or something.
            font = DynamicTextFont(fn, faceIndex)

        # The following properties may only be set for dynamic fonts.
        if hasattr(font, "setPointSize"):
            if pointSize != None:
                font.setPointSize(pointSize)
            if pixelsPerUnit != None:
                font.setPixelsPerUnit(pixelsPerUnit)
            if scaleFactor != None:
                font.setScaleFactor(scaleFactor)
            if textureMargin != None:
                font.setTextureMargin(textureMargin)
            if polyMargin != None:
                font.setPolyMargin(polyMargin)
            if minFilter != None:
                font.setMinFilter(minFilter)
            if magFilter != None:
                font.setMagFilter(magFilter)
            if anisotropicDegree != None:
                font.setAnisotropicDegree(anisotropicDegree)

        if lineHeight != None:
            # If the line height is specified, it overrides whatever
            # the font itself thinks the line height should be.  This
            # and spaceAdvance should be set last, since some of the
            # other parameters can cause these to be reset to their
            # default.
            font.setLineHeight(lineHeight)

        if spaceAdvance != None:
            font.setSpaceAdvance(spaceAdvance)

        return font

    # texture loading funcs
    def loadTexture(self, texturePath, alphaPath = None):
        """loadTexture(self, string)
        Attempt to load a texture from the given file path using
        TexturePool class. Returns None if not found"""

        if alphaPath == None:
            assert(Loader.notify.debug("Loading texture: %s" % (texturePath) ))
            if phaseChecker:
                phaseChecker(texturePath)
            texture = TexturePool.loadTexture(texturePath)
        else:
            assert(Loader.notify.debug("Loading texture: %s %s" % (texturePath, alphaPath) ))
            if phaseChecker:
                phaseChecker(texturePath)
            texture = TexturePool.loadTexture(texturePath, alphaPath)
        return texture

    def unloadTexture(self, texture):
        """unloadTexture(self, texture)
        """
        assert(Loader.notify.debug("Unloading texture: %s" % (texture) ))
        TexturePool.releaseTexture(texture)

    # sound loading funcs
    def loadSfx(self, name):
        assert(Loader.notify.debug("Loading sound: %s" % (name) ))
        if phaseChecker:
            phaseChecker(name)
        # should return a valid sound obj even if soundMgr is invalid
        sound = None
        if (name):
            # showbase-created sfxManager should always be at front of list
            sound=base.sfxManagerList[0].getSound(name)
        if sound == None:
            Loader.notify.warning("Could not load sound file %s." % name)
        return sound

    def loadMusic(self, name):
        assert(Loader.notify.debug("Loading sound: %s" % (name) ))
        # should return a valid sound obj even if musicMgr is invalid
        sound = None
        if (name):
            sound=base.musicManager.getSound(name)
        if sound == None:
            Loader.notify.warning("Could not load music file %s." % name)
        return sound


    def makeNodeNamesUnique(self, nodePath, nodeCount):
        if nodeCount == 0:
            Loader.modelCount += 1
        nodePath.setName(nodePath.getName() +
                         ('_%d_%d' % (Loader.modelCount, nodeCount)))
        for i in range(nodePath.getNumChildren()):
            nodeCount += 1
            self.makeNodeNamesUnique(nodePath.getChild(i), nodeCount)

