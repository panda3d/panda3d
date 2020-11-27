"""This module contains a high-level interface for loading models, textures,
sound, music, shaders and fonts from disk.
"""

__all__ = ['Loader']

from panda3d.core import *
from panda3d.core import Loader as PandaLoader
from direct.directnotify.DirectNotifyGlobal import *
from direct.showbase.DirectObject import DirectObject

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

    _loadedPythonFileTypes = False

    class _Callback:
        """Returned by loadModel when used asynchronously.  This class is
        modelled after Future, and can be awaited."""

        # This indicates that this class behaves like a Future.
        _asyncio_future_blocking = False

        class _ResultAwaiter(object):
            """Reinvents generators because of PEP 479, sigh.  See #513."""

            __slots__ = 'requestList', 'index'

            def __init__(self, requestList):
                self.requestList = requestList
                self.index = 0

            def __await__(self):
                return self

            def __anext__(self):
                if self.index >= len(self.requestList):
                    raise StopAsyncIteration
                return self

            def __iter__(self):
                return self

            def __next__(self):
                i = self.index
                request = self.requestList[i]
                if not request.done():
                    return request

                self.index = i + 1

                result = request.result()
                if isinstance(result, PandaNode):
                    result = NodePath(result)

                exc = StopIteration(result)
                exc.value = result
                raise exc

        def __init__(self, loader, numObjects, gotList, callback, extraArgs):
            self._loader = loader
            self.objects = [None] * numObjects
            self.gotList = gotList
            self.callback = callback
            self.extraArgs = extraArgs
            self.requests = set()
            self.requestList = []

        def gotObject(self, index, object):
            self.objects[index] = object

            if not self.requests:
                self._loader = None
                if self.callback:
                    if self.gotList:
                        self.callback(self.objects, *self.extraArgs)
                    else:
                        self.callback(*(self.objects + self.extraArgs))

        def cancel(self):
            "Cancels the request.  Callback won't be called."
            if self._loader:
                for request in self.requests:
                    self._loader.loader.remove(request)
                    del self._loader._requests[request]
                self._loader = None
                self.requests = None
                self.requestList = None

        def cancelled(self):
            "Returns true if the request was cancelled."
            return self.requestList is None

        def done(self):
            "Returns true if all the requests were finished or cancelled."
            return not self.requests

        def result(self):
            "Returns the results, suspending the thread to wait if necessary."
            for r in list(self.requests):
                r.wait()
            if self.gotList:
                return self.objects
            else:
                return self.objects[0]

        def exception(self):
            assert self.done() and not self.cancelled()
            return None

        def __await__(self):
            """ Returns a generator that raises StopIteration when the loading
            is complete.  This allows this class to be used with 'await'."""

            if self.requests:
                self._asyncio_future_blocking = True

            if self.gotList:
                return self._ResultAwaiter([self])
            else:
                return self._ResultAwaiter(self.requestList)

        def __aiter__(self):
            """ This allows using `async for` to iterate asynchronously over
            the results of this class.  It does guarantee to return the
            results in order, though, even though they may not be loaded in
            that order. """
            requestList = self.requestList
            assert requestList is not None, "Request was cancelled."

            return self._ResultAwaiter(requestList)

    # special methods
    def __init__(self, base):
        self.base = base
        self.loader = PandaLoader.getGlobalPtr()

        self._requests = {}

        self.hook = "async_loader_%s" % (Loader.loaderIndex)
        Loader.loaderIndex += 1
        self.accept(self.hook, self.__gotAsyncObject)

        self._loadPythonFileTypes()

    def destroy(self):
        self.ignore(self.hook)
        self.loader.stopThreads()
        del self.base
        del self.loader

    @classmethod
    def _loadPythonFileTypes(cls):
        if cls._loadedPythonFileTypes:
            return

        if not ConfigVariableBool('loader-support-entry-points', True):
            return

        import importlib
        try:
            pkg_resources = importlib.import_module('pkg_resources')
        except ImportError:
            pkg_resources = None

        if pkg_resources:
            registry = LoaderFileTypeRegistry.getGlobalPtr()

            for entry_point in pkg_resources.iter_entry_points('panda3d.loaders'):
                registry.register_deferred_type(entry_point)

            cls._loadedPythonFileTypes = True

    # model loading funcs
    def loadModel(self, modelPath, loaderOptions = None, noCache = None,
                  allowInstance = False, okMissing = None,
                  callback = None, extraArgs = [], priority = None,
                  blocking = None):
        """
        Attempts to load a model or models from one or more relative
        pathnames.  If the input modelPath is a string (a single model
        pathname), the return value will be a NodePath to the model
        loaded if the load was successful, or None otherwise.  If the
        input modelPath is a list of pathnames, the return value will
        be a list of `.NodePath` objects and/or Nones.

        loaderOptions may optionally be passed in to control details
        about the way the model is searched and loaded.  See the
        `.LoaderOptions` class for more.

        The default is to look in the `.ModelPool` (RAM) cache first,
        and return a copy from that if the model can be found there.
        If the bam cache is enabled (via the `model-cache-dir` config
        variable), then that will be consulted next, and if both
        caches fail, the file will be loaded from disk.  If noCache is
        True, then neither cache will be consulted or updated.

        If allowInstance is True, a shared instance may be returned
        from the `.ModelPool`.  This is dangerous, since it is easy to
        accidentally modify the shared instance, and invalidate future
        load attempts of the same model.  Normally, you should leave
        allowInstance set to False, which will always return a unique
        copy.

        If okMissing is True, None is returned if the model is not
        found or cannot be read, and no error message is printed.
        Otherwise, an `IOError` is raised if the model is not found or
        cannot be read (similar to attempting to open a nonexistent
        file).  (If modelPath is a list of filenames, then `IOError`
        is raised if *any* of the models could not be loaded.)

        If callback is not None, then the model load will be performed
        asynchronously.  In this case, loadModel() will initiate a
        background load and return immediately.  The return value will
        be an object that can be used to check the status, cancel the
        request, or use it in an `await` expression.  Unless callback
        is the special value True, when the requested model(s) have
        finished loading, it will be invoked with the n
        loaded models passed as its parameter list.  It is possible
        that the callback will be invoked immediately, even before
        loadModel() returns.  If you use callback, you may also
        specify a priority, which specifies the relative importance
        over this model over all of the other asynchronous load
        requests (higher numbers are loaded first).

        True asynchronous model loading requires Panda to have been
        compiled with threading support enabled (you can test
        `.Thread.isThreadingSupported()`).  In the absence of threading
        support, the asynchronous interface still exists and still
        behaves exactly as described, except that loadModel() might
        not return immediately.

        """

        assert Loader.notify.debug("Loading model: %s" % (modelPath,))
        if loaderOptions is None:
            loaderOptions = LoaderOptions()
        else:
            loaderOptions = LoaderOptions(loaderOptions)

        if okMissing is not None:
            if okMissing:
                loaderOptions.setFlags(loaderOptions.getFlags() & ~LoaderOptions.LFReportErrors)
            else:
                loaderOptions.setFlags(loaderOptions.getFlags() | LoaderOptions.LFReportErrors)
        else:
            okMissing = ((loaderOptions.getFlags() & LoaderOptions.LFReportErrors) == 0)

        if noCache is not None:
            if noCache:
                loaderOptions.setFlags(loaderOptions.getFlags() | LoaderOptions.LFNoCache)
            else:
                loaderOptions.setFlags(loaderOptions.getFlags() & ~LoaderOptions.LFNoCache)

        if allowInstance:
            loaderOptions.setFlags(loaderOptions.getFlags() | LoaderOptions.LFAllowInstance)

        if not isinstance(modelPath, (tuple, list, set)):
            # We were given a single model pathname.
            modelList = [modelPath]
            if phaseChecker:
                phaseChecker(modelPath, loaderOptions)

            gotList = False
        else:
            # Assume we were given a list of model pathnames.
            modelList = modelPath
            gotList = True

        if blocking is None:
            blocking = callback is None

        if blocking:
            # We got no callback, so it's a synchronous load.

            result = []
            for modelPath in modelList:
                node = self.loader.loadSync(Filename(modelPath), loaderOptions)
                if node is not None:
                    nodePath = NodePath(node)
                else:
                    nodePath = None

                result.append(nodePath)

            if not okMissing and None in result:
                message = 'Could not load model file(s): %s' % (modelList,)
                raise IOError(message)

            if gotList:
                return result
            else:
                return result[0]

        else:
            # We got a callback, so we want an asynchronous (threaded)
            # load.  We'll return immediately, but when all of the
            # requested models have been loaded, we'll invoke the
            # callback (passing it the models on the parameter list).

            cb = Loader._Callback(self, len(modelList), gotList, callback, extraArgs)
            i = 0
            for modelPath in modelList:
                request = self.loader.makeAsyncRequest(Filename(modelPath), loaderOptions)
                if priority is not None:
                    request.setPriority(priority)
                request.setDoneEvent(self.hook)
                self.loader.loadAsync(request)
                cb.requests.add(request)
                cb.requestList.append(request)
                self._requests[request] = (cb, i)
                i += 1
            return cb

    def cancelRequest(self, cb):
        """Cancels an aysynchronous loading or flatten request issued
        earlier.  The callback associated with the request will not be
        called after cancelRequest() has been performed.

        This is now deprecated: call cb.cancel() instead. """

        cb.cancel()

    def isRequestPending(self, cb):
        """ Returns true if an asynchronous loading or flatten request
        issued earlier is still pending, or false if it has completed or
        been cancelled.

        This is now deprecated: call cb.done() instead. """

        return bool(cb.requests)

    def loadModelOnce(self, modelPath):
        """
        modelPath is a string.

        Attempt to load a model from modelPool, if not present
        then attempt to load it from disk. Return a nodepath to
        the model if successful or None otherwise
        """
        Loader.notify.info("loader.loadModelOnce() is deprecated; use loader.loadModel() instead.")

        return self.loadModel(modelPath, noCache = False)

    def loadModelCopy(self, modelPath, loaderOptions = None):
        """loadModelCopy(self, string)
        NOTE: This method is deprecated and should not be used.
        Attempt to load a model from modelPool, if not present
        then attempt to load it from disk. Return a nodepath to
        a copy of the model if successful or None otherwise
        """
        Loader.notify.info("loader.loadModelCopy() is deprecated; use loader.loadModel() instead.")

        return self.loadModel(modelPath, loaderOptions = loaderOptions, noCache = False)

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
        Loader.notify.info("loader.loadModelNode() is deprecated; use loader.loadModel() instead.")

        model = self.loadModel(modelPath, noCache = False)
        if model is not None:
            model = model.node()

        return model

    def unloadModel(self, model):
        """
        model is the return value of loadModel().  For backward
        compatibility, it may also be the filename that was passed to
        loadModel(), though this requires a disk search.
        """
        if isinstance(model, NodePath):
            # Maybe we were given a NodePath
            modelNode = model.node()

        elif isinstance(model, ModelNode):
            # Maybe we were given a node
            modelNode = model

        elif isinstance(model, (str, Filename)):
            # If we were given a filename, we have to ask the loader
            # to resolve it for us.
            options = LoaderOptions(LoaderOptions.LFSearch | LoaderOptions.LFNoDiskCache | LoaderOptions.LFCacheOnly)
            modelNode = self.loader.loadSync(Filename(model), options)
            if modelNode is None:
                # Model not found.
                assert Loader.notify.debug("Unloading model not loaded: %s" % (model))
                return

            assert Loader.notify.debug("%s resolves to %s" % (model, modelNode.getFullpath()))

        else:
            raise TypeError('Invalid parameter to unloadModel: %s' % (model))

        assert Loader.notify.debug("Unloading model: %s" % (modelNode.getFullpath()))
        ModelPool.releaseModel(modelNode)

    def saveModel(self, modelPath, node, loaderOptions = None,
                  callback = None, extraArgs = [], priority = None,
                  blocking = None):
        """ Saves the model (a `NodePath` or `PandaNode`) to the indicated
        filename path.  Returns true on success, false on failure.  If
        a callback is used, the model is saved asynchronously, and the
        true/false status is passed to the callback function. """

        if loaderOptions is None:
            loaderOptions = LoaderOptions()
        else:
            loaderOptions = LoaderOptions(loaderOptions)

        if not isinstance(modelPath, (tuple, list, set)):
            # We were given a single model pathname.
            modelList = [modelPath]
            nodeList = [node]
            if phaseChecker:
                phaseChecker(modelPath, loaderOptions)

            gotList = False
        else:
            # Assume we were given a list of model pathnames.
            modelList = modelPath
            nodeList = node
            gotList = True

        assert(len(modelList) == len(nodeList))

        # Make sure we have PandaNodes, not NodePaths.
        for i in range(len(nodeList)):
            if isinstance(nodeList[i], NodePath):
                nodeList[i] = nodeList[i].node()

        # From here on, we deal with a list of (filename, node) pairs.
        modelList = list(zip(modelList, nodeList))

        if blocking is None:
            blocking = callback is None

        if blocking:
            # We got no callback, so it's a synchronous save.

            result = []
            for modelPath, node in modelList:
                thisResult = self.loader.saveSync(Filename(modelPath), loaderOptions, node)
                result.append(thisResult)

            if gotList:
                return result
            else:
                return result[0]

        else:
            # We got a callback, so we want an asynchronous (threaded)
            # save.  We'll return immediately, but when all of the
            # requested models have been saved, we'll invoke the
            # callback (passing it the models on the parameter list).

            cb = Loader._Callback(self, len(modelList), gotList, callback, extraArgs)
            i = 0
            for modelPath, node in modelList:
                request = self.loader.makeAsyncSaveRequest(Filename(modelPath), loaderOptions, node)
                if priority is not None:
                    request.setPriority(priority)
                request.setDoneEvent(self.hook)
                self.loader.saveAsync(request)
                cb.requests.add(request)
                cb.requestList.append(request)
                self._requests[request] = (cb, i)
                i += 1
            return cb


    # font loading funcs
    def loadFont(self, modelPath,
                 spaceAdvance = None, lineHeight = None,
                 pointSize = None,
                 pixelsPerUnit = None, scaleFactor = None,
                 textureMargin = None, polyMargin = None,
                 minFilter = None, magFilter = None,
                 anisotropicDegree = None,
                 color = None,
                 outlineWidth = None,
                 outlineFeather = 0.1,
                 outlineColor = VBase4(0, 0, 0, 1),
                 renderMode = None,
                 okMissing = False):
        """
        modelPath is a string.

        This loads a special model as a `TextFont` object, for rendering
        text with a `TextNode`.  A font file must be either a special
        egg file (or bam file) generated with egg-mkfont, which is
        considered a static font, or a standard font file (like a TTF
        file) that is supported by FreeType, which is considered a
        dynamic font.

        okMissing should be True to indicate the method should return
        None if the font file is not found.  If it is False, the
        method will raise an exception if the font file is not found
        or cannot be loaded.

        Most font-customization parameters accepted by this method
        (except lineHeight and spaceAdvance) may only be specified for
        dynamic font files like TTF files, not for static egg files.

        lineHeight specifies the vertical distance between consecutive
        lines, in Panda units.  If unspecified, it is taken from the
        font information.  This parameter may be specified for static
        as well as dynamic fonts.

        spaceAdvance specifies the width of a space character (ascii
        32), in Panda units.  If unspecified, it is taken from the
        font information.  This may be specified for static as well as
        dynamic fonts.

        The remaining parameters may only be specified for dynamic
        fonts.

        pixelsPerUnit controls the visual quality of the rendered text
        characters.  It specifies the number of texture pixels per
        each Panda unit of character height.  Increasing this number
        increases the amount of detail that can be represented in the
        characters, at the expense of texture memory.

        scaleFactor also controls the visual quality of the rendered
        text characters.  It is the amount by which the characters are
        rendered bigger out of Freetype, and then downscaled to fit
        within the texture.  Increasing this number may reduce some
        artifacts of very small font characters, at a small cost of
        processing time to generate the characters initially.

        textureMargin specifies the number of pixels of the texture to
        leave between adjacent characters.  It may be a floating-point
        number.  This helps reduce bleed-through from nearby
        characters within the texture space.  Increasing this number
        reduces artifacts at the edges of the character cells
        (especially for very small text scales), at the expense of
        texture memory.

        polyMargin specifies the amount of additional buffer to create
        in the polygon that represents each character, in Panda units.
        It is similar to textureMargin, but it controls the polygon
        buffer, not the texture buffer.  Increasing this number
        reduces artifacts from letters getting chopped off at the
        edges (especially for very small text scales), with some
        increasing risk of adjacent letters overlapping and obscuring
        each other.

        minFilter, magFilter, and anisotropicDegree specify the
        texture filter modes that should be applied to the textures
        that are created to hold the font characters.

        If color is not None, it should be a VBase4 specifying the
        foreground color of the font.  Specifying this option breaks
        `TextNode.setColor()`, so you almost never want to use this
        option; the default (white) is the most appropriate for a
        font, as it allows text to have any arbitrary color assigned
        at generation time.  However, if you want to use a colored
        outline (below) with a different color for the interior, for
        instance a yellow letter with a blue outline, then you need
        this option, and then *all* text generated with this font will
        have to be yellow and blue.

        If outlineWidth is nonzero, an outline will be created at
        runtime for the letters, and outlineWidth will be the desired
        width of the outline, in points (most fonts are 10 points
        high, so 0.5 is often a good choice).  If you specify
        outlineWidth, you can also specify outlineFeather (0.0 .. 1.0)
        and outlineColor.  You may need to increase pixelsPerUnit to
        get the best results.

        if renderMode is not None, it may be one of the following
        symbols to specify a geometry-based font:

            TextFont.RMTexture - this is the default.  Font characters
              are rendered into a texture and applied to a polygon.
              This gives the best general-purpose results.

            TextFont.RMWireframe - Font characters are rendered as a
              sequence of one-pixel lines.  Consider enabling line or
              multisample antialiasing for best results.

            TextFont.RMPolygon - Font characters are rendered as a
              flat polygon.  This works best for very large
              characters, and generally requires polygon or
              multisample antialiasing to be enabled for best results.

            TextFont.RMExtruded - Font characters are rendered with a
              3-D outline made of polygons, like a cookie cutter.
              This is appropriate for a 3-D scene, but may be
              completely invisible when assigned to a 2-D scene and
              viewed normally from the front, since polygons are
              infinitely thin.

            TextFont.RMSolid - A combination of RMPolygon and
              RMExtruded: a flat polygon in front with a solid
              three-dimensional edge.  This is best for letters that
              will be tumbling in 3-D space.

        If the texture mode is other than RMTexture, most of the above
        parameters do not apply, though pixelsPerUnit still does apply
        and roughly controls the tightness of the curve approximation
        (and the number of vertices generated).

        """
        assert Loader.notify.debug("Loading font: %s" % (modelPath))
        if phaseChecker:
            loaderOptions = LoaderOptions()
            if(okMissing):
                loaderOptions.setFlags(loaderOptions.getFlags() & ~LoaderOptions.LFReportErrors)
            phaseChecker(modelPath, loaderOptions)

        font = FontPool.loadFont(modelPath)
        if font is None:
            if not okMissing:
                message = 'Could not load font file: %s' % (modelPath)
                raise IOError(message)
            # If we couldn't load the model, at least return an
            # empty font.
            font = StaticTextFont(PandaNode("empty"))

        # The following properties may only be set for dynamic fonts.
        if hasattr(font, "setPointSize"):
            if pointSize is not None:
                font.setPointSize(pointSize)
            if pixelsPerUnit is not None:
                font.setPixelsPerUnit(pixelsPerUnit)
            if scaleFactor is not None:
                font.setScaleFactor(scaleFactor)
            if textureMargin is not None:
                font.setTextureMargin(textureMargin)
            if polyMargin is not None:
                font.setPolyMargin(polyMargin)
            if minFilter is not None:
                font.setMinfilter(minFilter)
            if magFilter is not None:
                font.setMagfilter(magFilter)
            if anisotropicDegree is not None:
                font.setAnisotropicDegree(anisotropicDegree)
            if color:
                font.setFg(color)
                # This means we want the background to match the
                # foreground color, but transparent.
                font.setBg(VBase4(color[0], color[1], color[2], 0.0))
            if outlineWidth:
                font.setOutline(outlineColor, outlineWidth, outlineFeather)

                # This means we want the background to match the
                # outline color, but transparent.
                font.setBg(VBase4(outlineColor[0], outlineColor[1], outlineColor[2], 0.0))
            if renderMode:
                font.setRenderMode(renderMode)

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
                    readMipmaps = False, okMissing = False,
                    minfilter = None, magfilter = None,
                    anisotropicDegree = None, loaderOptions = None,
                    multiview = None):
        """
        texturePath is a string.

        Attempt to load a texture from the given file path using
        `TexturePool` class.  Returns a `Texture` object, or raises
        `IOError` if the file could not be loaded.

        okMissing should be True to indicate the method should return
        None if the texture file is not found.  If it is False, the
        method will raise an exception if the texture file is not
        found or cannot be loaded.

        If alphaPath is not None, it is the name of a grayscale image
        that is applied as the texture's alpha channel.

        If readMipmaps is True, then the filename string must contain
        a sequence of hash characters ('#') that are filled in with
        the mipmap index number, and n images will be loaded
        individually which define the n mipmap levels of the texture.
        The base level is mipmap level 0, and this defines the size of
        the texture and the number of expected mipmap images.

        If minfilter or magfilter is not None, they should be a symbol
        like `SamplerState.FTLinear` or `SamplerState.FTNearest`.
        (minfilter may be further one of the Mipmap filter type symbols.)
        These specify the filter mode that will automatically be applied
        to the texture when it is loaded.  Note that this setting may
        override the texture's existing settings, even if it has
        already been loaded.  See `egg-texture-cards` for a more robust
        way to apply per-texture filter types and settings.

        If anisotropicDegree is not None, it specifies the anisotropic degree
        to apply to the texture when it is loaded.  Like minfilter and
        magfilter, `egg-texture-cards` may be a more robust way to apply
        this setting.

        If multiview is true, it indicates to load a multiview or
        stereo texture.  In this case, the filename should contain a
        hash character ('#') that will be replaced with '0' for the
        left image and '1' for the right image.  Larger numbers are
        also allowed if you need more than two views.
        """
        if loaderOptions is None:
            loaderOptions = LoaderOptions()
        else:
            loaderOptions = LoaderOptions(loaderOptions)
        if multiview is not None:
            flags = loaderOptions.getTextureFlags()
            if multiview:
                flags |= LoaderOptions.TFMultiview
            else:
                flags &= ~LoaderOptions.TFMultiview
            loaderOptions.setTextureFlags(flags)

        if alphaPath is None:
            assert Loader.notify.debug("Loading texture: %s" % (texturePath))
            texture = TexturePool.loadTexture(texturePath, 0, readMipmaps, loaderOptions)
        else:
            assert Loader.notify.debug("Loading texture: %s %s" % (texturePath, alphaPath))
            texture = TexturePool.loadTexture(texturePath, alphaPath, 0, 0, readMipmaps, loaderOptions)
        if not texture and not okMissing:
            message = 'Could not load texture: %s' % (texturePath)
            raise IOError(message)

        if minfilter is not None:
            texture.setMinfilter(minfilter)
        if magfilter is not None:
            texture.setMagfilter(magfilter)
        if anisotropicDegree is not None:
            texture.setAnisotropicDegree(anisotropicDegree)

        return texture

    def load3DTexture(self, texturePattern, readMipmaps = False, okMissing = False,
                      minfilter = None, magfilter = None, anisotropicDegree = None,
                      loaderOptions = None, multiview = None, numViews = 2):
        """
        texturePattern is a string that contains a sequence of one or
        more hash characters ('#'), which will be filled in with the
        z-height number.  Returns a 3-D `Texture` object, suitable for
        rendering volumetric textures.

        okMissing should be True to indicate the method should return
        None if the texture file is not found.  If it is False, the
        method will raise an exception if the texture file is not
        found or cannot be loaded.

        If readMipmaps is True, then the filename string must contain
        two sequences of hash characters; the first group is filled in
        with the z-height number, and the second group with the mipmap
        index number.

        If multiview is true, it indicates to load a multiview or
        stereo texture.  In this case, numViews should also be
        specified (the default is 2), and the sequence of texture
        images will be divided into numViews views.  The total
        z-height will be (numImages / numViews).  For instance, if you
        read 16 images with numViews = 2, then you have created a
        stereo multiview image, with z = 8.  In this example, images
        numbered 0 - 7 will be part of the left eye view, and images
        numbered 8 - 15 will be part of the right eye view.
        """
        assert Loader.notify.debug("Loading 3-D texture: %s" % (texturePattern))
        if loaderOptions is None:
            loaderOptions = LoaderOptions()
        else:
            loaderOptions = LoaderOptions(loaderOptions)
        if multiview is not None:
            flags = loaderOptions.getTextureFlags()
            if multiview:
                flags |= LoaderOptions.TFMultiview
            else:
                flags &= ~LoaderOptions.TFMultiview
            loaderOptions.setTextureFlags(flags)
            loaderOptions.setTextureNumViews(numViews)

        texture = TexturePool.load3dTexture(texturePattern, readMipmaps, loaderOptions)
        if not texture and not okMissing:
            message = 'Could not load 3-D texture: %s' % (texturePattern)
            raise IOError(message)

        if minfilter is not None:
            texture.setMinfilter(minfilter)
        if magfilter is not None:
            texture.setMagfilter(magfilter)
        if anisotropicDegree is not None:
            texture.setAnisotropicDegree(anisotropicDegree)

        return texture

    def load2DTextureArray(self, texturePattern, readMipmaps = False, okMissing = False,
                      minfilter = None, magfilter = None, anisotropicDegree = None,
                      loaderOptions = None, multiview = None, numViews = 2):
        """
        texturePattern is a string that contains a sequence of one or
        more hash characters ('#'), which will be filled in with the
        z-height number.  Returns a 2-D `Texture` array object, suitable
        for rendering array of textures.

        okMissing should be True to indicate the method should return
        None if the texture file is not found.  If it is False, the
        method will raise an exception if the texture file is not
        found or cannot be loaded.

        If readMipmaps is True, then the filename string must contain
        two sequences of hash characters; the first group is filled in
        with the z-height number, and the second group with the mipmap
        index number.

        If multiview is true, it indicates to load a multiview or
        stereo texture.  In this case, numViews should also be
        specified (the default is 2), and the sequence of texture
        images will be divided into numViews views.  The total
        z-height will be (numImages / numViews).  For instance, if you
        read 16 images with numViews = 2, then you have created a
        stereo multiview image, with z = 8.  In this example, images
        numbered 0 - 7 will be part of the left eye view, and images
        numbered 8 - 15 will be part of the right eye view.
        """
        assert Loader.notify.debug("Loading 2-D texture array: %s" % (texturePattern))
        if loaderOptions is None:
            loaderOptions = LoaderOptions()
        else:
            loaderOptions = LoaderOptions(loaderOptions)
        if multiview is not None:
            flags = loaderOptions.getTextureFlags()
            if multiview:
                flags |= LoaderOptions.TFMultiview
            else:
                flags &= ~LoaderOptions.TFMultiview
            loaderOptions.setTextureFlags(flags)
            loaderOptions.setTextureNumViews(numViews)

        texture = TexturePool.load2dTextureArray(texturePattern, readMipmaps, loaderOptions)
        if not texture and not okMissing:
            message = 'Could not load 2-D texture array: %s' % (texturePattern)
            raise IOError(message)

        if minfilter is not None:
            texture.setMinfilter(minfilter)
        if magfilter is not None:
            texture.setMagfilter(magfilter)
        if anisotropicDegree is not None:
            texture.setAnisotropicDegree(anisotropicDegree)

        return texture

    def loadCubeMap(self, texturePattern, readMipmaps = False, okMissing = False,
                    minfilter = None, magfilter = None, anisotropicDegree = None,
                    loaderOptions = None, multiview = None):
        """
        texturePattern is a string that contains a sequence of one or
        more hash characters ('#'), which will be filled in with the
        face index number (0 through 6).  Returns a six-face cube map
        `Texture` object.

        okMissing should be True to indicate the method should return
        None if the texture file is not found.  If it is False, the
        method will raise an exception if the texture file is not
        found or cannot be loaded.

        If readMipmaps is True, then the filename string must contain
        two sequences of hash characters; the first group is filled in
        with the face index number, and the second group with the
        mipmap index number.

        If multiview is true, it indicates to load a multiview or
        stereo cube map.  For a stereo cube map, 12 images will be
        loaded--images numbered 0 - 5 will become the left eye view,
        and images 6 - 11 will become the right eye view.  In general,
        the number of images found on disk must be a multiple of six,
        and each six images will define a new view.
        """
        assert Loader.notify.debug("Loading cube map: %s" % (texturePattern))
        if loaderOptions is None:
            loaderOptions = LoaderOptions()
        else:
            loaderOptions = LoaderOptions(loaderOptions)
        if multiview is not None:
            flags = loaderOptions.getTextureFlags()
            if multiview:
                flags |= LoaderOptions.TFMultiview
            else:
                flags &= ~LoaderOptions.TFMultiview
            loaderOptions.setTextureFlags(flags)

        texture = TexturePool.loadCubeMap(texturePattern, readMipmaps, loaderOptions)
        if not texture and not okMissing:
            message = 'Could not load cube map: %s' % (texturePattern)
            raise IOError(message)

        if minfilter is not None:
            texture.setMinfilter(minfilter)
        if magfilter is not None:
            texture.setMagfilter(magfilter)
        if anisotropicDegree is not None:
            texture.setAnisotropicDegree(anisotropicDegree)

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
        and music files other than the particular `AudioManager` used
        to load the sound file, but this distinction allows the sound
        effects and/or the music files to be adjusted as a group,
        independently of the other group."""

        # showbase-created sfxManager should always be at front of list
        if(self.base.sfxManagerList):
            return self.loadSound(self.base.sfxManagerList[0], *args, **kw)
        return None

    def loadMusic(self, *args, **kw):
        """Loads one or more sound files, specifically designated as a
        "music" file (that is, uses the musicManager to load the
        sound).  There is no distinction between sound effect files
        and music files other than the particular `AudioManager` used
        to load the sound file, but this distinction allows the sound
        effects and/or the music files to be adjusted as a group,
        independently of the other group."""
        if(self.base.musicManager):
            return self.loadSound(self.base.musicManager, *args, **kw)
        else:
            return None

    def loadSound(self, manager, soundPath, positional = False,
                  callback = None, extraArgs = []):

        """Loads one or more sound files, specifying the particular
        AudioManager that should be used to load them.  The soundPath
        may be either a single filename, or a list of filenames.  If a
        callback is specified, the loading happens in the background,
        just as in loadModel(); otherwise, the loading happens before
        loadSound() returns."""

        if not isinstance(soundPath, (tuple, list, set)):
            # We were given a single sound pathname or a MovieAudio instance.
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
                sound = manager.getSound(soundPath, positional)
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

            cb = Loader._Callback(self, len(soundList), gotList, callback, extraArgs)
            for i, soundPath in enumerate(soundList):
                request = AudioLoadRequest(manager, soundPath, positional)
                request.setDoneEvent(self.hook)
                self.loader.loadAsync(request)
                cb.requests.add(request)
                cb.requestList.append(request)
                self._requests[request] = (cb, i)
            return cb

    def unloadSfx(self, sfx):
        if (sfx):
            if(self.base.sfxManagerList):
                self.base.sfxManagerList[0].uncacheSound (sfx.getName())

##     def makeNodeNamesUnique(self, nodePath, nodeCount):
##         if nodeCount == 0:
##             Loader.modelCount += 1
##         nodePath.setName(nodePath.getName() +
##                          ('_%d_%d' % (Loader.modelCount, nodeCount)))
##         for i in range(nodePath.getNumChildren()):
##             nodeCount += 1
##             self.makeNodeNamesUnique(nodePath.getChild(i), nodeCount)

    def loadShader(self, shaderPath, okMissing = False):
        shader = ShaderPool.loadShader (shaderPath)
        if not shader and not okMissing:
            message = 'Could not load shader file: %s' % (shaderPath)
            raise IOError(message)
        return shader

    def unloadShader(self, shaderPath):
        if shaderPath is not None:
            ShaderPool.releaseShader(shaderPath)

    def asyncFlattenStrong(self, model, inPlace = True,
                           callback = None, extraArgs = []):
        """ Performs a model.flattenStrong() operation in a sub-thread
        (if threading is compiled into Panda).  The model may be a
        single `.NodePath`, or it may be a list of NodePaths.

        Each model is duplicated and flattened in the sub-thread.

        If inPlace is True, then when the flatten operation completes,
        the newly flattened copies are automatically dropped into the
        scene graph, in place the original models.

        If a callback is specified, then it is called after the
        operation is finished, receiving the flattened model (or a
        list of flattened models)."""

        if isinstance(model, NodePath):
            # We were given a single model.
            modelList = [model]
            gotList = False
        else:
            # Assume we were given a list of models.
            modelList = model
            gotList = True

        if inPlace:
            extraArgs = [gotList, callback, modelList, extraArgs]
            callback = self.__asyncFlattenDone
            gotList = True

        cb = Loader._Callback(self, len(modelList), gotList, callback, extraArgs)
        i = 0
        for model in modelList:
            request = ModelFlattenRequest(model.node())
            request.setDoneEvent(self.hook)
            self.loader.loadAsync(request)
            cb.requests.add(request)
            cb.requestList.append(request)
            self._requests[request] = (cb, i)
            i += 1
        return cb

    def __asyncFlattenDone(self, models,
                           gotList, callback, origModelList, extraArgs):
        """ The asynchronous flatten operation has completed; quietly
        drop in the new models. """
        self.notify.debug("asyncFlattenDone: %s" % (models,))
        assert(len(models) == len(origModelList))
        for i in range(len(models)):
            origModelList[i].getChildren().detach()
            orig = origModelList[i].node()
            flat = models[i].node()
            orig.copyAllProperties(flat)
            flat.replaceNode(orig)

        if callback:
            if gotList:
                callback(origModelList, *extraArgs)
            else:
                callback(*(origModelList + extraArgs))

    def __gotAsyncObject(self, request):
        """A model or sound file or some such thing has just been
        loaded asynchronously by the sub-thread.  Add it to the list
        of loaded objects, and call the appropriate callback when it's
        time."""

        if request not in self._requests:
            return

        cb, i = self._requests[request]
        if cb.cancelled() or request.cancelled():
            # Shouldn't be here.
            del self._requests[request]
            return

        cb.requests.discard(request)
        if not cb.requests:
            del self._requests[request]

        result = request.result()
        if isinstance(result, PandaNode):
            result = NodePath(result)

        cb.gotObject(i, result)

    load_model = loadModel
    unload_model = unloadModel
    save_model = saveModel
    load_font = loadFont
    load_texture = loadTexture
    load_3d_texture = load3DTexture
    load_cube_map = loadCubeMap
    unload_texture = unloadTexture
    load_sfx = loadSfx
    load_music = loadMusic
    load_sound = loadSound
    unload_sfx = unloadSfx
    load_shader = loadShader
    unload_shader = unloadShader
    async_flatten_strong = asyncFlattenStrong
