from pandac.PandaModules import *
from direct.showbase.DirectObject import DirectObject
import math
import copy

class TexMemWatcher(DirectObject):
    """
    This class creates a separate graphics window that displays an
    approximation of the current texture memory, showing the textures
    that are resident and/or active, and an approximation of the
    amount of texture memory consumed by each one.  It's intended as a
    useful tool to help determine where texture memory is being spent.

    Although it represents the textures visually in a 2-d space, it
    doesn't actually have any idea how textures are physically laid
    out in memory--but it has to lay them out somehow, so it makes
    something up.  It occasionally rearranges the texture display when
    it feels it needs to, without regard to what the graphics card is
    actually doing.  This tool can't be used to research texture
    memory fragmentation issues.
    """

    NextIndex = 1

    StatusHeight = 20  # in pixels

    def __init__(self, gsg = None, limit = None):
        DirectObject.__init__(self)

        # First, we'll need a name to uniquify the object.
        self.name = 'tex-mem%s' % (TexMemWatcher.NextIndex)
        TexMemWatcher.NextIndex += 1

        self.cleanedUp = False
        self.top = 1.0

        # The textures managed by the TexMemWatcher are packed
        # arbitrarily into the canvas, which is the viewable region
        # that represents texture memory allocation.  The packing
        # arrangement has no relation to actual layout within texture
        # memory (which we have no way to determine).

        # The visual size of each texture is chosen in proportion to
        # the total number of bytes of texture memory the texture
        # consumes.  This includes mipmaps, and accounts for texture
        # compression.  Visually, a texture with mipmaps will be
        # represented by a rectangle 33% larger than an
        # equivalent-sized texture without mipmaps.  Of course, this
        # once again has little bearing to the way the textures are
        # actually arranged in memory; but it serves to give a visual
        # indication of how much texture memory each texture consumes.

        # There is an arbitrary limit, self.limit, which may have been
        # passed to the constructor, or which may be arbitrarily
        # determined.  This represents the intended limit to texture
        # memory utilization.  We (generously) assume that the
        # graphics card will implement a perfect texture packing
        # algorithm, so that as long as our total utilization <=
        # self.limit, it must fit within texture memory.  We represent
        # this visually by aggressively packing textures within the
        # self.limit block so that they are guaranteed to fit, as long
        # as we do not exceed the total utilization.  This may
        # sometimes mean distorting a texture block or even breaking
        # it into multiple pieces to get it to fit, clearly
        # fictionalizing whatever the graphics driver is actually
        # doing.

        # Internally, textures are packed into an integer grid of
        # Q-units.  Q-units are in proportion to texture bytes.
        # Specifically, each Q-unit corresponds to a block of
        # self.quantize * self.quantize texture bytes in the Texture
        # Memory window.  The Q-units are the smallest packable unit;
        # increasing self.quantize therefore reduces the visual
        # packing resolution correspondingly.  Q-units very roughly
        # correspond to pixels onscreen (they may be larger, sometimes
        # considerably larger, than 1 pixel, depending on the window
        # size).


        # This number defines the size of a Q-unit square, in texture
        # bytes.  It is automatically adjusted in repack() based on
        # the window size and the texture memory size.
        self.quantize = 1

        # This is the maximum number of bitmask rows (within
        # self.limit) to allocate for packing.  This controls the
        # value assigned to self.quantize in repack().
        self.maxHeight = base.config.GetInt('tex-mem-max-height', 300)
        
        # The total number of texture bytes tracked, including overflow.
        self.totalSize = 0

        # The total number of texture bytes placed, not including
        # overflow (that is, within self.limit).
        self.placedSize = 0

        # The total number of Q-units placed, not including overflow.
        self.placedQSize = 0
        
        # If no GSG is specified, use the main GSG.
        if gsg is None:
            gsg = base.win.getGsg()
        elif isinstance(gsg, GraphicsOutput):
            # If we were passed a window, use that window's GSG.
            gsg = gsg.getGsg()

        self.gsg = gsg

        # Now open a new window just to render the output.
        size = ConfigVariableInt('tex-mem-win-size', '300 300')
        origin = ConfigVariableInt('tex-mem-win-origin', '100 100')
        self.winSize = (size[0], size[1])
        name = 'Texture Memory'
        props = WindowProperties()
        props.setOrigin(origin[0], origin[1])
        props.setSize(*self.winSize)
        props.setTitle(name)
        props.setFullscreen(False)
        props.setUndecorated(False)

        fbprops = FrameBufferProperties.getDefault()
        flags = GraphicsPipe.BFFbPropsOptional | GraphicsPipe.BFRequireWindow

        self.pipe = None

        # Set this to tinydisplay if you're running on a machine with
        # limited texture memory.  That way you won't compete for
        # texture memory with the main scene.
        moduleName = base.config.GetString('tex-mem-pipe', '')
        if moduleName:
            self.pipe = base.makeModulePipe(moduleName)

        # If the requested pipe fails for some reason, we'll use the
        # regular pipe.
        if not self.pipe:
            self.pipe = base.pipe
        
        self.win = base.graphicsEngine.makeOutput(self.pipe, name, 0, fbprops,
                                                  props, flags)
        assert self.win

        # We should render at the end of the frame.
        self.win.setSort(10000)

        # We don't need to clear the color buffer, since we'll be
        # filling it with a texture.  We also don't need to clear the
        # depth buffer, since we won't be using it.
        self.win.setClearColorActive(False)
        self.win.setClearDepthActive(False)

        eventName = '%s-window' % (self.name)
        self.win.setWindowEvent(eventName)
        self.accept(eventName, self.windowEvent)

        # Listen for this event so we can update appropriately, if
        # anyone changes the window's graphics memory limit,
        self.accept('graphics_memory_limit_changed',
                    self.graphicsMemoryLimitChanged)

        # We'll need a mouse object to get mouse events.
        self.mouse = base.dataRoot.attachNewNode(MouseAndKeyboard(self.win, 0, '%s-mouse' % (self.name)))
        bt = ButtonThrower('%s-thrower' % (self.name))
        self.mouse.attachNewNode(bt)
        bt.setPrefix('button-%s-' % (self.name))
        self.accept('button-%s-mouse1' % (self.name), self.mouseClick)

        self.setupGui()
        self.setupCanvas()

        # Now start handling up the actual stuff in the scene.
        
        self.background = None
        self.nextTexRecordKey = 0
        self.rollover = None
        self.isolate = None
        self.isolated = None
        self.needsRepack = False

        # How frequently should the texture memory window check for
        # state changes?
        updateInterval = base.config.GetDouble("tex-mem-update-interval", 0.5)
        self.task = taskMgr.doMethodLater(updateInterval, self.updateTextures, 'TexMemWatcher')

        self.setLimit(limit)

    def setupGui(self):
        """ Creates the gui elements and supporting structures. """

        self.render2d = NodePath('render2d')
        self.render2d.setDepthTest(False)
        self.render2d.setDepthWrite(False)
        self.render2d.setTwoSided(True)
        self.render2d.setBin('unsorted', 0)
        
        # Create a DisplayRegion and an associated camera.
        dr = self.win.makeDisplayRegion()
        cam = Camera('cam2d')
        self.lens = OrthographicLens()
        self.lens.setNearFar(-1000, 1000)
        self.lens.setFilmSize(2, 2)
        cam.setLens(self.lens)

        np = self.render2d.attachNewNode(cam)
        dr.setCamera(np)

        self.aspect2d = self.render2d.attachNewNode('aspect2d')

        cm = CardMaker('statusBackground')
        cm.setColor(0.85, 0.85, 0.85, 1)
        cm.setFrame(0, 2, 0, 2)
        self.statusBackground = self.render2d.attachNewNode(cm.generate(), -1)
        self.statusBackground.setPos(-1, 0, -1)

        self.status = self.aspect2d.attachNewNode('status')
        self.statusText = TextNode('statusText')
        self.statusText.setTextColor(0, 0, 0, 1)
        self.statusTextNP = self.status.attachNewNode(self.statusText)
        self.statusTextNP.setScale(1.5)

        self.sizeText = TextNode('sizeText')
        self.sizeText.setTextColor(0, 0, 0, 1)
        self.sizeText.setAlign(TextNode.ARight)
        self.sizeText.setCardAsMargin(0.25, 0, 0, -0.25)
        self.sizeText.setCardColor(0.85, 0.85, 0.85, 1)
        self.sizeTextNP = self.status.attachNewNode(self.sizeText)
        self.sizeTextNP.setScale(1.5)

    def setupCanvas(self):
        """ Creates the "canvas", which is the checkerboard area where
        texture memory is laid out.  The canvas has its own
        DisplayRegion. """
        
        self.canvasRoot = NodePath('canvasRoot')
        self.canvasRoot.setDepthTest(False)
        self.canvasRoot.setDepthWrite(False)
        self.canvasRoot.setTwoSided(True)
        self.canvasRoot.setBin('unsorted', 0)

        self.canvas = self.canvasRoot.attachNewNode('canvas')
        
        # Create a DisplayRegion and an associated camera.
        self.canvasDR = self.win.makeDisplayRegion()
        self.canvasDR.setSort(-10)
        cam = Camera('cam2d')
        self.canvasLens = OrthographicLens()
        self.canvasLens.setNearFar(-1000, 1000)
        cam.setLens(self.canvasLens)

        np = self.canvasRoot.attachNewNode(cam)
        self.canvasDR.setCamera(np)

        # Create a MouseWatcher so we can interact with the various
        # textures.
        self.mw = MouseWatcher('%s-watcher' % (self.name))
        self.mw.setDisplayRegion(self.canvasDR)
        mwnp = self.mouse.attachNewNode(self.mw)

        eventName = '%s-enter' % (self.name)
        self.mw.setEnterPattern(eventName)
        self.accept(eventName, self.enterRegion)

        eventName = '%s-leave' % (self.name)
        self.mw.setLeavePattern(eventName)
        self.accept(eventName, self.leaveRegion)

        # Create a checkerboard background card for the canvas.
        p = PNMImage(2, 2, 1)
        p.setGray(0, 0, 0.40)
        p.setGray(1, 1, 0.40)
        p.setGray(0, 1, 0.75)
        p.setGray(1, 0, 0.75)

        self.checkTex = Texture('checkTex')
        self.checkTex.load(p)
        self.checkTex.setMagfilter(Texture.FTNearest)

        self.canvasBackground = None

        self.makeCanvasBackground()

    def makeCanvasBackground(self):
        if self.canvasBackground:
            self.canvasBackground.removeNode()
            
        self.canvasBackground = self.canvasRoot.attachNewNode('canvasBackground', -100)

        cm = CardMaker('background')
        cm.setFrame(0, 1, 0, 1)
        cm.setUvRange((0, 0), (1, 1))
        self.canvasBackground.attachNewNode(cm.generate())

        cm.setFrame(0, 1, 1, self.top)
        cm.setUvRange((0, 1), (1, self.top))
        bad = self.canvasBackground.attachNewNode(cm.generate())
        bad.setColor((0.8, 0.2, 0.2, 1))

        self.canvasBackground.setTexture(self.checkTex)

    def setLimit(self, limit = None):
        """ Indicates the texture memory limit.  If limit is None or
        unspecified, the limit is taken from the GSG, if any; or there
        is no limit. """
        
        self.__doSetLimit(limit)
        self.reconfigureWindow()
        
    def __doSetLimit(self, limit):
        """ Internal implementation of setLimit(). """
        self.limit = limit
        self.lruLimit = False
        self.dynamicLimit = False
        
        if not limit:
            # If no limit was specified, use the specified graphics
            # memory limit, if any.
            lruSize = self.gsg.getPreparedObjects().getGraphicsMemoryLimit()
            if lruSize and lruSize < 2**32 - 1:
                # Got a real lruSize.  Use it.
                self.limit = lruSize
                self.lruLimit = True

            else:
                # No LRU limit either, so there won't be a practical
                # limit to the TexMemWatcher.  We'll determine our
                # limit on-the-fly instead.
                self.dynamicLimit = True

        if self.dynamicLimit:
            # Choose a suitable limit by rounding to the next power of two.
            self.limit = Texture.upToPower2(self.totalSize)

        # Set our GSG to limit itself to no more textures than we
        # expect to display onscreen, so we don't go crazy with
        # texture memory.
        self.win.getGsg().getPreparedObjects().setGraphicsMemoryLimit(self.limit)

        # The actual height of the canvas, including the overflow
        # area.  The texture memory itself is restricted to (0..1)
        # vertically; anything higher than 1 is overflow.
        top = 1.25
        if self.dynamicLimit:
            # Actually, we'll never exceed texture memory, so never mind.
            top = 1
        if top != self.top:
            self.top = top
            self.makeCanvasBackground()
        
        self.canvasLens.setFilmSize(1, self.top)
        self.canvasLens.setFilmOffset(0.5, self.top / 2.0)  # lens covers 0..1 in x and y

    def cleanup(self):
        if not self.cleanedUp:
            self.cleanedUp = True

            # Remove the window.
            base.graphicsEngine.removeWindow(self.win)
            self.win = None
            self.gsg = None
            self.pipe = None

            # Remove the mouse.
            self.mouse.detachNode()

            taskMgr.remove(self.task)
            self.ignoreAll()

            self.canvas.getChildren().detach()
            self.texRecordsByTex = {}
            self.texRecordsByKey = {}
            self.texPlacements = {}

    def graphicsMemoryLimitChanged(self):
        if self.dynamicLimit or self.lruLimit:
            self.__doSetLimit(None)
            self.reconfigureWindow()

    def windowEvent(self, win):
        if win == self.win:
            props = win.getProperties()
            if not props.getOpen():
                # User closed window.
                self.cleanup()
                return
            
            size = (props.getXSize(), props.getYSize())
            if size != self.winSize:
                self.winSize = size
                self.reconfigureWindow()

    def enterRegion(self, region, buttonName):
        """ the mouse has rolled over a texture. """
        key, pi = map(int, region.getName().split(':'))
        tr = self.texRecordsByKey.get(key)
        if not tr:
            return

        self.setRollover(tr, pi)

    def leaveRegion(self, region, buttonName):
        """ the mouse is no longer over a texture. """
        key, pi = map(int, region.getName().split(':'))
        tr = self.texRecordsByKey.get(key)
        if tr != self.rollover:
            return

        self.setRollover(None, None)

    def mouseClick(self):
        """ Received a mouse-click within the window.  This isolates
        the currently-highlighted texture into a full-window
        presentation. """

        if self.isolate:
            # We're already isolating a texture; the click undoes this.
            self.isolateTexture(None)
            return

        if self.rollover:
            self.isolateTexture(self.rollover)

    def setRollover(self, tr, pi):
        """ Sets the highlighted texture (due to mouse rollover) to
        the indicated texture, or None to clear it. """
        
        self.rollover = tr
        if self.rollover:
            self.statusText.setText(tr.tex.getName())
        else:
            self.statusText.setText('')

    def isolateTexture(self, tr):
        """ Isolates the indicated texture onscreen, or None to
        restore normal mode. """

        if self.isolate:
            self.isolate.removeNode()
            self.isolate = None

        self.isolated = tr

        # Undo the previous call to isolate.
        self.canvas.show()
        self.canvasBackground.clearColor()
        self.win.getGsg().setTextureQualityOverride(Texture.QLDefault)
        if hasattr(self.gsg, 'clearFlashTexture'):
            self.gsg.clearFlashTexture()

        if not tr:
            return

        # Now isolate.
        
        self.canvas.hide()
        # Disable the red bar at the top.
        self.canvasBackground.setColor(1, 1, 1, 1, 1)

        # Show the texture in all its filtered glory.
        self.win.getGsg().setTextureQualityOverride(Texture.QLBest)

        if hasattr(self.gsg, 'setFlashTexture'):
            # Start the texture flashing in the main window.
            self.gsg.setFlashTexture(tr.tex)

        self.isolate = self.render2d.attachNewNode('isolate')

        wx, wy = self.winSize

        # Put a label on the bottom of the screen.
        tn = TextNode('tn')
        tn.setText('%s\n%s x %s\n%s' % (
            tr.tex.getName(), tr.tex.getXSize(), tr.tex.getYSize(),
            self.formatSize(tr.size)))
        tn.setAlign(tn.ACenter)
        tn.setCardAsMargin(100.0, 100.0, 0.1, 0.1)
        tn.setCardColor(0.1, 0.2, 0.4, 1)
        tnp = self.isolate.attachNewNode(tn)
        scale = 30.0 / wy
        tnp.setScale(scale * wy / wx, scale, scale)
        tnp.setPos(render2d, 0, 0, -1 - tn.getBottom() * scale)

        labelTop = tn.getHeight() * scale

        # Make a card that shows the texture in actual pixel size, but
        # don't let it exceed the screen size.
        tw = tr.tex.getXSize()
        th = tr.tex.getYSize()

        wx = float(wx)
        wy = float(wy) * (2.0 - labelTop) * 0.5

        w = min(tw, wx)
        h = min(th, wy)

        sx = w / tw
        sy = h / th
        s = min(sx, sy)

        w = tw * s / float(self.winSize[0])
        h = th * s / float(self.winSize[1])

        cx = 0.0
        cy = 1.0 - (2.0 - labelTop) * 0.5

        l = cx - w
        r = cx + w
        b = cy - h
        t = cy + h

        cm = CardMaker('card')
        cm.setFrame(l, r, b, t)
        c = self.isolate.attachNewNode(cm.generate())
        c.setTexture(tr.tex)
        c.setTransparency(TransparencyAttrib.MAlpha)

        ls = LineSegs('frame')
        ls.setColor(0, 0, 0, 1)
        ls.moveTo(l, 0, b)
        ls.drawTo(r, 0, b)
        ls.drawTo(r, 0, t)
        ls.drawTo(l, 0, t)
        ls.drawTo(l, 0, b)
        self.isolate.attachNewNode(ls.create())


    def reconfigureWindow(self):
        """ Resets everything for a new window size. """

        wx, wy = self.winSize
        if wx <= 0 or wy <= 0:
            return

        self.aspect2d.setScale(float(wy) / float(wx), 1, 1)

        # Reserve self.StatusHeight pixels for the status bar;
        # everything else is for the canvas.

        statusScale = float(self.StatusHeight) / float(wy)
        self.statusBackground.setScale(1, 1, statusScale)
        self.status.setScale(statusScale)
        self.statusTextNP.setPos(self.statusBackground, 0, 0, 0.5)
        self.sizeTextNP.setPos(self.statusBackground, 2, 0, 0.5)

        self.canvasDR.setDimensions(0, 1, statusScale, 1)

        w = self.canvasDR.getPixelWidth()
        h = self.canvasDR.getPixelHeight()
        self.canvasBackground.setTexScale(TextureStage.getDefault(),
                                          w / 20.0, h / (20.0 * self.top))

        if self.isolate:
            # If we're currently showing an isolated texture, refresh
            # that display so we get its size right.  And when we come
            # back to the main window (but not now), repack it.
            self.needsRepack = True
            self.isolateTexture(self.isolated)

        else:
            # If we're showing the main window, just repack it
            # immediately.
            self.repack()

    def updateTextures(self, task):
        """ Gets the current list of resident textures and adds new
        textures or removes old ones from the onscreen display, as
        necessary. """

        if self.isolate:
            # never mind for now.
            return task.again

        if self.needsRepack:
            self.needsRepack = False
            self.repack()
            return task.again

        pgo = self.gsg.getPreparedObjects()
        totalSize = 0

        texRecords = []
        neverVisited = copy.copy(self.texRecordsByTex)
        for tex in self.gsg.getPreparedTextures():
            # We have visited this texture; remove it from the
            # neverVisited list.
            if tex in neverVisited:
                del neverVisited[tex]
                
            size = 0
            if tex.getResident(pgo):
                size = tex.getDataSizeBytes(pgo)

            tr = self.texRecordsByTex.get(tex, None)

            if size:
                totalSize += size
                active = tex.getActive(pgo)
                if not tr:
                    # This is a new texture; need to record it.
                    key = self.nextTexRecordKey
                    self.nextTexRecordKey += 1
                    tr = TexRecord(key, tex, size, active)
                    texRecords.append(tr)
                else:
                    tr.setActive(active)
                    if tr.size != size or not tr.placements:
                        # The size has changed; reapply it.
                        tr.setSize(size)
                        self.unplaceTexture(tr)
                        texRecords.append(tr)
            else:
                if tr:
                    # This texture is no longer resident; need to remove it.
                    self.unplaceTexture(tr)

        # Now go through and make sure we unplace (and remove!) any
        # textures that we didn't visit at all this pass.
        for tex, tr in neverVisited.items():
            self.unplaceTexture(tr)
            del self.texRecordsByTex[tex]
            del self.texRecordsByKey[tr.key]

        self.totalSize = totalSize
        self.sizeText.setText(self.formatSize(self.totalSize))
        if totalSize > self.limit and self.dynamicLimit:
            # Actually, never mind on the update: we have exceeded the
            # dynamic limit computed before, and therefore we need to
            # repack.
            self.repack()

        else:
            overflowCount = sum(map(lambda tp: tp.overflowed, self.texPlacements.keys()))
            if totalSize <= self.limit and overflowCount:
                # Shouldn't be overflowing any more.  Better repack.
                self.repack()

            else:
                # Pack in just the newly-loaded textures.

                # Sort the regions from largest to smallest to maximize
                # packing effectiveness.
                texRecords.sort(key = lambda tr: (tr.tw, tr.th), reverse = True)

                for tr in texRecords:
                    self.placeTexture(tr)
                    self.texRecordsByTex[tr.tex] = tr
                    self.texRecordsByKey[tr.key] = tr

        return task.again
                

    def repack(self):
        """ Repacks all of the current textures. """

        self.canvas.getChildren().detach()
        self.texRecordsByTex = {}
        self.texRecordsByKey = {}
        self.texPlacements = {}
        self.bitmasks = []
        self.mw.clearRegions()
        self.setRollover(None, None)
        self.w = 1
        self.h = 1
        self.placedSize = 0
        self.placedQSize = 0

        pgo = self.gsg.getPreparedObjects()
        totalSize = 0
        
        for tex in self.gsg.getPreparedTextures():
            if tex.getResident(pgo):
                size = tex.getDataSizeBytes(pgo)
                if size:
                    active = tex.getActive(pgo)
                    key = self.nextTexRecordKey
                    self.nextTexRecordKey += 1
                    tr = TexRecord(key, tex, size, active)
                    self.texRecordsByTex[tr.tex] = tr
                    self.texRecordsByKey[tr.key] = tr
                    totalSize += size

        self.totalSize = totalSize
        self.sizeText.setText(self.formatSize(self.totalSize))
        if not self.totalSize:
            return

        if self.dynamicLimit or self.lruLimit:
            # Adjust the limit to ensure we keep tracking the lru size.
            self.__doSetLimit(None)

        # Now make that into a 2-D rectangle of the appropriate shape,
        # such that w * h == limit.

        # Window size
        x, y = self.winSize

        # There should be a little buffer on the top so we can see if
        # we overflow.
        y /= self.top
        
        r = float(y) / float(x)

        # Region size
        w = math.sqrt(self.limit) / math.sqrt(r)
        h = w * r

        # Now choose self.quantize so that we don't exceed
        # self.maxHeight.
        if h > self.maxHeight:
            self.quantize = int(math.ceil(h / self.maxHeight))
        else:
            self.quantize = 1
        
        w = max(int(w / self.quantize + 0.5), 1)
        h = max(int(h / self.quantize + 0.5), 1)
        self.w = w
        self.h = h
        self.area = self.w * self.h

        # We store a bitarray for each row, for fast lookup for
        # unallocated space on the canvas.  Each Q-unit on the row
        # corresponds to a bit in the bitarray, where bit 0 is Q-unit
        # 0, bit 1 is Q-unit 1, and so on.  If the bit is set, the
        # space is occupied.
        self.bitmasks = []
        for i in range(self.h):
            self.bitmasks.append(BitArray())

        self.canvas.setScale(1.0 / w, 1.0, 1.0 / h)
        self.mw.setFrame(0, w, 0, h * self.top)

        # Sort the regions from largest to smallest to maximize
        # packing effectiveness.
        texRecords = self.texRecordsByTex.values()
        texRecords.sort(key = lambda tr: (tr.tw, tr.th), reverse = True)
        
        for tr in texRecords:
            self.placeTexture(tr)

    def formatSize(self, size):
        """ Returns a size in MB, KB, GB, whatever. """
        if size < 1000:
            return '%s bytes' % (size)
        size /= 1024.0
        if size < 1000:
            return '%0.1f kb' % (size)
        size /= 1024.0
        if size < 1000:
            return '%0.1f MB' % (size)
        size /= 1024.0
        return '%0.1f GB' % (size)

    def unplaceTexture(self, tr):
        """ Removes the texture from its place on the canvas. """
        if tr.placements:
            for tp in tr.placements:
                tp.clearBitmasks(self.bitmasks)
                if not tp.overflowed:
                    self.placedQSize -= tp.area
                    assert self.placedQSize >= 0
                del self.texPlacements[tp]
            tr.placements = []
            tr.clearCard(self)
            if not tr.overflowed:
                self.placedSize -= tr.size
                assert self.placedSize >= 0
        tr.overflowed = 0

    def placeTexture(self, tr):
        """ Places the texture somewhere on the canvas where it will
        fit. """

        tr.computePlacementSize(self)
        tr.overflowed = 0

        shouldFit = False
        availableSize = self.limit - self.placedSize
        if availableSize >= tr.size:
            shouldFit = True
            availableQSize = self.area - self.placedQSize
            if availableQSize < tr.area:
                # The texture should fit, but won't, due to roundoff
                # error.  Make it correspondingly smaller, so we can
                # place it anyway.
                tr.area = availableQSize
            
        if shouldFit:
            # Look for a single rectangular hole to hold this piece.
            tp = self.findHole(tr.area, tr.w, tr.h)
            if tp:
                texCmp = cmp(tr.w, tr.h)
                holeCmp = cmp(tp.p[1] - tp.p[0], tp.p[3] - tp.p[2])
                if texCmp != 0 and holeCmp != 0 and texCmp != holeCmp:
                    tp.rotated = True
                tr.placements = [tp]
                tr.makeCard(self)
                tp.setBitmasks(self.bitmasks)
                self.placedQSize += tp.area
                self.texPlacements[tp] = tr
                self.placedSize += tr.size
                return

            # Couldn't find a single rectangular hole.  We'll have to
            # divide the texture up into several smaller pieces to cram it
            # in.
            tpList = self.findHolePieces(tr.area)
            if tpList:
                texCmp = cmp(tr.w, tr.h)
                tr.placements = tpList
                for tp in tpList:
                    holeCmp = cmp(tp.p[1] - tp.p[0], tp.p[3] - tp.p[2])
                    if texCmp != 0 and holeCmp != 0 and texCmp != holeCmp:
                        tp.rotated = True
                    tp.setBitmasks(self.bitmasks)
                    self.placedQSize += tp.area
                    self.texPlacements[tp] = tr
                self.placedSize += tr.size
                tr.makeCard(self)
                return

        # Just let it overflow.
        tr.overflowed = 1
        tp = self.findOverflowHole(tr.area, tr.w, tr.h)
        tp.overflowed = 1
        while len(self.bitmasks) <= tp.p[3]:
            self.bitmasks.append(BitArray())

        tr.placements = [tp]
        tr.makeCard(self)
        tp.setBitmasks(self.bitmasks)
        self.texPlacements[tp] = tr


    def findHole(self, area, w, h):
        """ Searches for a rectangular hole that is at least area
        square units big, regardless of its shape, but attempt to find
        one that comes close to the right shape, at least.  If one is
        found, returns an appropriate TexPlacement; otherwise, returns
        None. """

        if area == 0:
            tp = TexPlacement(0, 0, 0, 0)
            return tp

        # Rotate the hole to horizontal first.
        w, h = max(w, h), min(w, h)

        aspect = float(w) / float(h)
        holes = self.findAvailableHoles(area, w, h)

        # Walk through the list and find the one with the best aspect
        # match.
        matches = []
        for tarea, tp in holes:
            l, r, b, t = tp.p
            tw = r - l
            th = t - b

            # To constrain our area within this rectangle, how would
            # we have to squish it?
            if tw < w:
                # We'd have to make it taller.
                nh = min(area / tw, th)
                th = nh
            elif th < h:
                # We'd have to make it narrower.
                nw = min(area / th, tw)
                tw = nw
            else:
                # Hey, we don't have to squish it after all!  Just
                # return this hole.
                tw = w
                th = h

            # Make a new tp that has the right area.
            tp = TexPlacement(l, l + tw, b, b + th)
            
            ta = float(max(tw, th)) / float(min(tw, th))
            if ta == aspect:
                return tp

            match = min(ta, aspect) / max(ta, aspect)
            matches.append((match, tp))

        if matches:
            return max(matches)[1]
        return None

    def findHolePieces(self, area):
        """ Returns a list of holes whose net area sums to the given
        area, or None if there are not enough holes. """

        # First, save the original value of self.texPlacements, since
        # we will be modifying that during this search.
        savedTexPlacements = copy.copy(self.texPlacements)
        savedBitmasks = []
        for ba in self.bitmasks:
            savedBitmasks.append(BitArray(ba))

        result = []

        while area > 0:

            # We have to call findLargestHole() each time through this
            # loop, instead of just walking through
            # findAvailableHoles() in order, because
            # findAvailableHoles() might return a list of overlapping
            # holes.
            tp = self.findLargestHole()
            if not tp:
                break
            
            l, r, b, t = tp.p
            tpArea = (r - l) * (t - b)
            if tpArea >= area:
                # we're done.
                shorten = (tpArea - area) / (r - l)
                t -= shorten
                tp.p = (l, r, b, t)
                tp.area = (r - l) * (t - b)
                result.append(tp)
                self.texPlacements = savedTexPlacements
                self.bitmasks = savedBitmasks
                return result

            # Keep going.
            area -= tpArea
            result.append(tp)
            tp.setBitmasks(self.bitmasks)
            self.texPlacements[tp] = None

        # Huh, not enough room, or no more holes.
        self.texPlacements = savedTexPlacements
        self.bitmasks = savedBitmasks
        return None

    def findLargestHole(self):
        holes = self.findAvailableHoles(0)
        if holes:
            return max(holes)[1]
        return None

    def findAvailableHoles(self, area, w = None, h = None):
        """ Finds a list of available holes, of at least the indicated
        area.  Returns a list of tuples, where each tuple is of the
        form (area, tp).

        If w and h are non-None, this will short-circuit on the first
        hole it finds that fits w x h, and return just that hole in a
        singleton list.
        """

        holes = []
        lastTuples = set()
        lastBitmask = None
        b = 0
        while b < self.h:
            # Separate this row into (l, r) tuples.
            bm = self.bitmasks[b]
            if bm == lastBitmask:
                # This row is exactly the same as the row below; no
                # need to reexamine.
                b += 1
                continue

            lastBitmask = bm
                
            tuples = self.findEmptyRuns(bm)
            newTuples = tuples.difference(lastTuples)

            for l, r in newTuples:
                # Find out how high we can go with this bitmask. 
                mask = BitArray.range(l, r - l)
                t = b + 1
                while t < self.h and (self.bitmasks[t] & mask).isZero():
                    t += 1

                tpw = (r - l)
                tph = (t - b)
                tarea = tpw * tph
                assert tarea > 0
                if tarea >= area:
                    tp = TexPlacement(l, r, b, t)
                    if w and h and \
                       ((tpw >= w and tph >= h) or \
                        (tph >= w and tpw >= h)):
                        # This hole is big enough; short circuit.
                        return [(tarea, tp)]

                    holes.append((tarea, tp))

            lastTuples = tuples
            b += 1

        return holes

    def findOverflowHole(self, area, w, h):
        """ Searches for a hole large enough for (w, h), in the
        overflow space.  Since the overflow space is infinite, this
        will always succeed. """

        if w > self.w:
            # It won't fit within the margins at all; just stack it on
            # the top.

            # Scan down past all of the empty bitmasks that may be
            # stacked on top.
            b = len(self.bitmasks)
            while b > self.h and self.bitmasks[b - 1].isZero():
                b -= 1
                
            tp = TexPlacement(0, w, b, b + h)
            return tp

        # It fits within the margins; find the first row with enough
        # space for it.

        lastTuples = set()
        lastBitmask = None
        b = self.h
        while True:
            if b >= len(self.bitmasks):
                # Off the top.  Just leave it here.
                tp = TexPlacement(0, w, b, b + h)
                return tp
                
            # Separate this row into (l, r) tuples.
            bm = self.bitmasks[b]
            if bm == lastBitmask:
                # This row is exactly the same as the row below; no
                # need to reexamine.
                b += 1
                continue

            lastBitmask = bm

            tuples = self.findEmptyRuns(bm)
            newTuples = tuples.difference(lastTuples)

            for l, r in newTuples:
                # Is this region wide enough?
                if r - l < w:
                    continue
                
                # Is it tall enough?
                r = l + w
                mask = BitArray.range(l, r - l)

                t = b + 1
                while t < b + h and \
                      (t >= len(self.bitmasks) or (self.bitmasks[t] & mask).isZero()):
                    t += 1

                if t < b + h:
                    # Not tall enough.
                    continue

                tp = TexPlacement(l, r, b, t)
                return tp

            lastTuples = tuples
            b += 1

    def findEmptyRuns(self, bm):
        """ Separates a bitmask into a list of (l, r) tuples,
        corresponding to the empty regions in the row between 0 and
        self.w. """

        tuples = set()
        l = bm.getLowestOffBit()
        assert l != -1
        if l < self.w:
            r = bm.getNextHigherDifferentBit(l)
            if r == l or r >= self.w:
                r = self.w
            tuples.add((l, r))
            l = bm.getNextHigherDifferentBit(r)
            while l != r and l < self.w:
                r = bm.getNextHigherDifferentBit(l)
                if r == l or r >= self.w:
                    r = self.w
                tuples.add((l, r))
                l = bm.getNextHigherDifferentBit(r)

        return tuples


class TexRecord:
    def __init__(self, key, tex, size, active):
        self.key = key
        self.tex = tex
        self.active = active
        self.root = None
        self.regions = []
        self.placements = []
        self.overflowed = 0

        self.setSize(size)

    def setSize(self, size):
        self.size = size
        x = self.tex.getXSize()
        y = self.tex.getYSize()
        r = float(y) / float(x)

        # Card size, in unscaled texel units.
        self.tw = math.sqrt(self.size) / math.sqrt(r)
        self.th = self.tw * r

    def computePlacementSize(self, tmw):
        self.w = max(int(self.tw / tmw.quantize + 0.5), 1)
        self.h = max(int(self.th / tmw.quantize + 0.5), 1)
        self.area = self.w * self.h


    def setActive(self, flag):
        self.active = flag
        if self.active:
            self.backing.clearColor()
            self.matte.clearColor()
            self.card.clearColor()
        else:
            self.backing.setColor((0.2, 0.2, 0.2, 1), 2)
            self.matte.setColor((0.2, 0.2, 0.2, 1), 2)
            self.card.setColor((0.4, 0.4, 0.4, 1), 2)

    def clearCard(self, tmw):
        if self.root:
            self.root.detachNode()
            self.root = None

        for r in self.regions:
            tmw.mw.removeRegion(r)
        self.regions = []

    def makeCard(self, tmw):
        self.clearCard(tmw)
        root = NodePath('root')

        # A matte to frame the texture and indicate its status.
        matte = root.attachNewNode('matte', 0)

        # A backing to put behind the card.
        backing = root.attachNewNode('backing', 10)

        # A card to display the texture.
        card = root.attachNewNode('card', 20)

        # A wire frame to ring the matte and separate the card from
        # its neighbors.
        frame = root.attachNewNode('frame', 30)


        for p in self.placements:
            l, r, b, t = p.p
            cx = (l + r) * 0.5
            cy = (b + t) * 0.5
            shrinkMat = Mat4.translateMat(-cx, 0, -cy) * Mat4.scaleMat(0.9) * Mat4.translateMat(cx, 0, cy)
            
            cm = CardMaker('backing')
            cm.setFrame(l, r, b, t)
            cm.setColor(0.1, 0.3, 0.5, 1)
            c = backing.attachNewNode(cm.generate())
            c.setMat(shrinkMat)
            
            cm = CardMaker('card')
            cm.setFrame(l, r, b, t)
            if p.rotated:
                cm.setUvRange((0, 1), (0, 0), (1, 0), (1, 1))
            c = card.attachNewNode(cm.generate())
            c.setMat(shrinkMat)

            cm = CardMaker('matte')
            cm.setFrame(l, r, b, t)
            matte.attachNewNode(cm.generate())

            ls = LineSegs('frame')
            ls.setColor(0, 0, 0, 1)
            ls.moveTo(l, 0, b)
            ls.drawTo(r, 0, b)
            ls.drawTo(r, 0, t)
            ls.drawTo(l, 0, t)
            ls.drawTo(l, 0, b)
            f1 = frame.attachNewNode(ls.create())
            f2 = f1.copyTo(frame)
            f2.setMat(shrinkMat)

        #matte.flattenStrong()
        self.matte = matte

        #backing.flattenStrong()
        self.backing = backing

        card.setTransparency(TransparencyAttrib.MAlpha)
        card.setTexture(self.tex)
        #card.flattenStrong()
        self.card = card

        #frame.flattenStrong()
        self.frame = frame

        root.reparentTo(tmw.canvas)

        self.root = root

        # Also, make one or more clickable MouseWatcherRegions.
        assert self.regions == []
        for pi in range(len(self.placements)):
            p = self.placements[pi]
            r = MouseWatcherRegion('%s:%s' % (self.key, pi), *p.p)
            tmw.mw.addRegion(r)
            self.regions.append(r)

class TexPlacement:
    def __init__(self, l, r, b, t):
        self.p = (l, r, b, t)
        self.area = (r - l) * (t - b)
        self.rotated = False
        self.overflowed = 0

    def intersects(self, other):
        """ Returns True if the placements intersect, False
        otherwise. """

        ml, mr, mb, mt = self.p
        tl, tr, tb, tt = other.p

        return (tl < mr and tr > ml and
                tb < mt and tt > mb)

    def setBitmasks(self, bitmasks):
        """ Sets all of the appropriate bits to indicate this region
        is taken. """

        l, r, b, t = self.p
        mask = BitArray.range(l, r - l)
        
        for yi in range(b, t):
            assert (bitmasks[yi] & mask).isZero()
            bitmasks[yi] |= mask

    def clearBitmasks(self, bitmasks):
        """ Clears all of the appropriate bits to indicate this region
        is available. """

        l, r, b, t = self.p
        mask = ~BitArray.range(l, r - l)
        
        for yi in range(b, t):
            assert (bitmasks[yi] | mask).isAllOn()
            bitmasks[yi] &= mask

    def hasOverlap(self, bitmasks):
        """ Returns true if there is an overlap with this region and
        any other region, false otherwise. """

        l, r, b, t = self.p
        mask = BitArray.range(l, r - l)
        
        for yi in range(b, t):
            if not (bitmasks[yi] & mask).isZero():
                return True
        return False
