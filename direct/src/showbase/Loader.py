"""Loader module: contains the Loader class"""

__all__ = ['Loader']

from pandac.PandaModules import *
from direct.directnotify.DirectNotifyGlobal import *
from direct.showbase.DirectObject import DirectObject
import types

# You can specify a phaseChecker callback to check
# a modelPath to see if it is being loaded in the correct
# phase
phaseChecker = None

class Loader(DirectObject):
    """
    Load models, textures, sounds, and code.
    """
    notify = directNotify.newCategory("Loader")
    loaderIndex = 0

    class Callback:
        def __init__(self, numObjects, callback, extraArgs):
            self.objects = [None] * numObjects
            self.callback = callback
            self.extraArgs = extraArgs
            self.numRemaining = numObjects

        def gotObject(self, index, object):
            self.objects[index] = object
            self.numRemaining -= 1

            if self.numRemaining == 0:
                self.callback(*(self.objects + self.extraArgs))

    # special methods
    def __init__(self, base):
        self.base = base
        self.loader = PandaLoader()

        self.hook = "async_loader_%s" % (Loader.loaderIndex)
        Loader.loaderIndex += 1
        self.accept(self.hook, self.__gotAsyncObject)

    def destroy(self):
        self.ignore(self.hook)
        del self.base
        del self.loader

    # model loading funcs
    def loadModel(self, modelPath, loaderOptions = None, noCache = None,
                  callback = None, extraArgs = []):
        """
        Attempts to load a model or models from one or more relative
        pathnames.  If the input modelPath is a string (a single model
        pathname), the return value will be a NodePath to the model
        loaded if the load was successful, or None otherwise.  If the
        input modelPath is a list of pathnames, the return value will
        be a list of NodePaths and/or Nones.

        loaderOptions may optionally be passed in to control details
        about the way the model is searched and loaded.  See the
        LoaderOptions class for more.

        The default is to look in the ModelPool (RAM) cache first, and
        return a copy from that if the model can be found there.  If
        the bam cache is enabled (via the model-cache-dir config
        variable), then that will be consulted next, and if both
        caches fail, the file will be loaded from disk.  If noCache is
        True, then neither cache will be consulted or updated.

        If callback is not None, then the model load will be performed
        asynchronously.  In this case, loadModel() will initiate a
        background load and return immediately, with no return value.
        At some later point, when the requested model(s) have finished
        loading, the callback function will be invoked with the n
        loaded models passed as its parameter list.  It is possible
        that the callback will be invoked immediately, even before
        loadModel() returns.

        True asynchronous model loading requires Panda to have been
        compiled with threading support enabled (you can test
        Thread.isThreadingSupported()).  In the absence of threading
        support, the asynchronous interface still exists and still
        behaves exactly as described, except that loadModel() might
        not return immediately.
        
        """
        
        assert Loader.notify.debug("Loading model: %s" % (modelPath))
        if phaseChecker:
            phaseChecker(modelPath)
        if loaderOptions == None:
            loaderOptions = LoaderOptions()
        else:
            loaderOptions = LoaderOptions(loaderOptions)

        if noCache is not None:
            if noCache:
                loaderOptions.setFlags(loaderOptions.getFlags() | LoaderOptions.LFNoCache)
            else:
                loaderOptions.setFlags(loaderOptions.getFlags() & ~LoaderOptions.LFNoCache)

        if isinstance(modelPath, types.StringTypes) or \
           isinstance(modelPath, Filename):
            # We were given a single model pathname.
            modelList = [modelPath]
            gotList = False
        else:
            # Assume we were given a list of model pathnames.
            modelList = modelPath
            gotList = True

        if callback is None:
            # We got no callback, so it's a synchronous load.

            result = []
            for modelPath in modelList:
                node = self.loader.loadSync(Filename(modelPath), loaderOptions)
                if (node != None):
                    nodePath = NodePath(node)
                else:
                    nodePath = None

                result.append(nodePath)

            if gotList:
                return result
            else:
                return result[0]

        else:
            # We got a callback, so we want an asynchronous (threaded)
            # load.  We'll return immediately, but when all of the
            # requested models have been loaded, we'll invoke the
            # callback (passing it the models on the parameter list).
            
            cb = Loader.Callback(len(modelList), callback, extraArgs)
            for i in range(len(modelList)):
                modelPath = modelList[i]
                if loaderOptions.allowRamCache() and ModelPool.hasModel(modelPath):
                    # If it's already in the model pool, we won't
                    # bother bouncing the load request through the
                    # thread; and maybe we can just make the callback
                    # immediately.
                    node = ModelPool.loadModel(modelPath)
                    nodePath = NodePath(node)
                    cb.gotObject(i, nodePath)

                else:
                    # We do need to go to the thread to load this model.
                    request = ModelLoadRequest(Filename(modelPath), loaderOptions)
                    request.setDoneEvent(self.hook)
                    request.setPythonObject((cb, i))
                    self.loader.loadAsync(request)

    def loadModelOnce(self, modelPath):
        """
        modelPath is a string.

        Attempt to load a model from modelPool, if not present
        then attempt to load it from disk. Return a nodepath to
        the model if successful or None otherwise
        """
        Loader.notify.debug("loader.loadModelOnce() is deprecated; use loader.loadModel() instead.")

        return self.loadModel(modelPath, noCache = False)

    def loadModelCopy(self, modelPath):
        """loadModelCopy(self, string)
        Attempt to load a model from modelPool, if not present
        then attempt to load it from disk. Return a nodepath to
        a copy of the model if successful or None otherwise
        """
        Loader.notify.debug("loader.loadModelCopy() is deprecated; use loader.loadModel() instead.")

        return self.loadModel(modelPath, noCache = False)

    def loadModelNode(self, modelPath):
        """
        modelPath is a string.

        This is like loadModelOnce in that it loads a model from the
        modelPool, but it does not then instance it to hidden and it
        returns a Node instead of a NodePath.  This is particularly
        useful for special models like fonts that you don't care about
        where they're parented to, and you don't want a NodePath
        anyway--it prevents accumulation of instances of the font
        model under hidden.

        However, if you're loading a font, see loadFont(), below.
        """
        Loader.notify.debug("loader.loadModelNode() is deprecated; use loader.loadModel() instead.")

        model = self.loadModel(modelPath, noCache = False)
        if model is not None:
            model = model.node()

        return model

    def unloadModel(self, modelPath):
        """
        modelPath is a string.
        """
        assert Loader.notify.debug("Unloading model: %s" % (modelPath))
        ModelPool.releaseModel(modelPath)

    # font loading funcs
    def loadFont(self, modelPath,
                 spaceAdvance = None, pointSize = None,
                 pixelsPerUnit = None, scaleFactor = None,
                 textureMargin = None, polyMargin = None,
                 minFilter = None, magFilter = None,
                 anisotropicDegree = None,
                 lineHeight = None):
        """
        modelPath is a string.

        This loads a special model as a TextFont object, for rendering
        text with a TextNode.  A font file must be either a special
        egg file (or bam file) generated with egg-mkfont, or a
        standard font file (like a TTF file) that is supported by
        FreeType.
        """
        assert Loader.notify.debug("Loading font: %s" % (modelPath))
        if phaseChecker:
            phaseChecker(modelPath)

        font = FontPool.loadFont(modelPath)
        if font == None:
            # If we couldn't load the model, at least return an
            # empty font.
            font = StaticTextFont(PandaNode("empty"))

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

        if lineHeight is not None:
            # If the line height is specified, it overrides whatever
            # the font itself thinks the line height should be.  This
            # and spaceAdvance should be set last, since some of the
            # other parameters can cause these to be reset to their
            # default.
            font.setLineHeight(lineHeight)

        if spaceAdvance is not None:
            font.setSpaceAdvance(spaceAdvance)

        return font

    # texture loading funcs
    def loadTexture(self, texturePath, alphaPath = None,
                    readMipmaps = False):
        """
        texturePath is a string.

        Attempt to load a texture from the given file path using
        TexturePool class. Returns None if not found
        """
        if alphaPath is None:
            assert Loader.notify.debug("Loading texture: %s" % (texturePath))
            if phaseChecker:
                phaseChecker(texturePath)
            texture = TexturePool.loadTexture(texturePath, 0, readMipmaps)
        else:
            assert Loader.notify.debug("Loading texture: %s %s" % (texturePath, alphaPath))
            if phaseChecker:
                phaseChecker(texturePath)
            texture = TexturePool.loadTexture(texturePath, alphaPath, 0, 0, readMipmaps)
        return texture

    def load3DTexture(self, texturePattern, readMipmaps = False):
        """
        texturePattern is a string that contains a sequence of one or
        more '#' characters, which will be filled in with the sequence
        number.

        Returns a 3-D Texture object, suitable for rendering
        volumetric textures, if successful, or None if not.
        """
        assert Loader.notify.debug("Loading 3-D texture: %s" % (texturePattern))
        if phaseChecker:
            phaseChecker(texturePattern)
        texture = TexturePool.load3dTexture(texturePattern, readMipmaps)
        return texture

    def loadCubeMap(self, texturePattern, readMipmaps = False):
        """
        texturePattern is a string that contains a sequence of one or
        more '#' characters, which will be filled in with the sequence
        number.

        Returns a six-face cube map Texture object if successful, or
        None if not.

        """
        assert Loader.notify.debug("Loading cube map: %s" % (texturePattern))
        if phaseChecker:
            phaseChecker(texturePattern)
        texture = TexturePool.loadCubeMap(texturePattern, readMipmaps)
        return texture

    def unloadTexture(self, texture):

        """
        Removes the previously-loaded texture from the cache, so
        that when the last reference to it is gone, it will be
        released.  This also means that the next time the same texture
        is loaded, it will be re-read from disk (and duplicated in
        texture memory if there are still outstanding references to
        it).

        The texture parameter may be the return value of any previous
        call to loadTexture(), load3DTexture(), or loadCubeMap().
        """
        assert Loader.notify.debug("Unloading texture: %s" % (texture))
        TexturePool.releaseTexture(texture)

    # sound loading funcs
    def loadSfx(self, *args, **kw):
        """Loads one or more sound files, specifically designated as a
        "sound effect" file (that is, uses the sfxManager to load the
        sound).  There is no distinction between sound effect files
        and music files other than the particular AudioManager used to
        load the sound file, but this distinction allows the sound
        effects and/or the music files to be adjusted as a group,
        independently of the other group."""
        
        # showbase-created sfxManager should always be at front of list
        return self.loadSound(base.sfxManagerList[0], *args, **kw)
    
    def loadMusic(self, *args, **kw):
        """Loads one or more sound files, specifically designated as a
        "music" file (that is, uses the musicManager to load the
        sound).  There is no distinction between sound effect files
        and music files other than the particular AudioManager used to
        load the sound file, but this distinction allows the sound
        effects and/or the music files to be adjusted as a group,
        independently of the other group."""

        return self.loadSound(base.musicManager, *args, **kw)

    def loadSound(self, manager, soundPath, positional = False,
                  callback = None, extraArgs = []):

        """Loads one or more sound files, specifying the particular
        AudioManager that should be used to load them.  The soundPath
        may be either a single filename, or a list of filenames.  If a
        callback is specified, the loading happens in the background,
        just as in loadModel(); otherwise, the loading happens before
        loadSound() returns."""
    
        if isinstance(soundPath, types.StringTypes) or \
           isinstance(soundPath, Filename):
            # We were given a single sound pathname.
            soundList = [soundPath]
            gotList = False
        else:
            # Assume we were given a list of sound pathnames.
            soundList = soundPath
            gotList = True

        if callback is None:
            # We got no callback, so it's a synchronous load.

            result = []
            for soundPath in soundList:
                # should return a valid sound obj even if musicMgr is invalid
                sound = manager.getSound(soundPath)
                result.append(sound)

            if gotList:
                return result
            else:
                return result[0]

        else:
            # We got a callback, so we want an asynchronous (threaded)
            # load.  We'll return immediately, but when all of the
            # requested sounds have been loaded, we'll invoke the
            # callback (passing it the sounds on the parameter list).
            
            cb = Loader.Callback(len(soundList), callback, extraArgs)
            for i in range(len(soundList)):
                soundPath = soundList[i]
                request = AudioLoadRequest(manager, soundPath, positional)
                request.setDoneEvent(self.hook)
                request.setPythonObject((cb, i))
                self.loader.loadAsync(request)

##     def makeNodeNamesUnique(self, nodePath, nodeCount):
##         if nodeCount == 0:
##             Loader.modelCount += 1
##         nodePath.setName(nodePath.getName() +
##                          ('_%d_%d' % (Loader.modelCount, nodeCount)))
##         for i in range(nodePath.getNumChildren()):
##             nodeCount += 1
##             self.makeNodeNamesUnique(nodePath.getChild(i), nodeCount)

    def loadShader (self, shaderPath):
        shader = ShaderPool.loadShader (shaderPath)
        if (shader == None):
            Loader.notify.warning("Could not load shader file %s." % shaderPath)
        return shader

    def unloadShader(self, shaderPath):
        if (shaderPath != None):
            ShaderPool.releaseShader(shaderPath)

    def __gotAsyncObject(self, request):
        """A model or sound file or some such thing has just been
        loaded asynchronously by the sub-thread.  Add it to the list
        of loaded objects, and call the appropriate callback when it's
        time."""

        cb, i = request.getPythonObject()

        object = None
        if hasattr(request, "getModel"):
            node = request.getModel()
            if (node != None):
                object = NodePath(node)

        elif hasattr(request, "getSound"):
            object = request.getSound()

        cb.gotObject(i, object)
