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
        
        # If no GSG is specified, use the main GSG.
        if gsg is None:
            gsg = base.win.getGsg()
        elif isinstance(gsg, GraphicsOutput):
            # If we were passed a window, use that window's GSG.
            gsg = gsg.getGsg()

        self.gsg = gsg

        # Now open a new window just to render the output.
        self.winSize = (300, 300)
        name = 'Texture Memory'
        props = WindowProperties()
        props.setOrigin(100, 100)
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
        self.overflowing = False
        self.nextTexRecordKey = 0
        self.rollover = None
        self.isolate = None
        self.isolated = None
        self.needsRepack = False
        
        self.task = taskMgr.doMethodLater(0.5, self.updateTextures, 'TexMemWatcher')

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

    def setLimit(self, limit):
        self.limit = limit
        self.dynamicLimit = False
        
        if limit is None:
            # If no limit was specified, use the specified graphics
            # memory limit, if any.
            lruLimit = self.gsg.getPreparedObjects().getGraphicsMemoryLimit()
            if lruLimit < 2**32 - 1:
                # Got a real lruLimit.  Use it.
                self.limit = lruLimit

            else:
                # No LRU limit either, so there won't be a practical
                # limit to the TexMemWatcher.  We'll determine our
                # limit on-the-fly instead.
                
                self.dynamicLimit = True

        if not self.dynamicLimit:
            # Set our GSG to limit itself to no more textures than we
            # expect to display onscreen, so we don't go crazy with
            # texture memory.
            self.win.getGsg().getPreparedObjects().setGraphicsMemoryLimit(self.limit)

        # The actual height of the canvas, including the overflow
        # area.  The texture memory itself is restricted to (0..1)
        # vertically; anything higher than 1 is overflow.
        self.top = 1.25
        if self.dynamicLimit:
            # Actually, we'll never exceed texture memory, so never mind.
            self.top = 1
        self.makeCanvasBackground()
        
        self.canvasLens.setFilmSize(1, self.top)
        self.canvasLens.setFilmOffset(0.5, self.top / 2.0)  # lens covers 0..1 in x and y

        self.reconfigureWindow()

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
        self.gsg.clearFlashTexture()

        if not tr:
            return

        # Now isolate.
        
        self.canvas.hide()
        # Disable the red bar at the top.
        self.canvasBackground.setColor(1, 1, 1, 1, 1)

        # Show the texture in all its filtered glory.
        self.win.getGsg().setTextureQualityOverride(Texture.QLBest)

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
            if overflowCount:
                # Shouldn't be overflowing any more.  Better repack.
                self.repack()

            else:
                # Pack in just the newly-loaded textures.

                # Sort the regions from largest to smallest to maximize
                # packing effectiveness.
                texRecords.sort(key = lambda tr: (-tr.w, -tr.h))

                self.overflowing = False
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
        self.mw.clearRegions()
        self.setRollover(None, None)
        self.w = 1
        self.h = 1

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

        if self.dynamicLimit:
            # Choose a suitable limit by rounding to the next power of two.
            self.limit = Texture.upToPower2(self.totalSize)

            # Set our GSG to limit itself to no more textures than we
            # expect to display onscreen, so we don't go crazy with
            # texture memory.
            self.win.getGsg().getPreparedObjects().setGraphicsMemoryLimit(self.limit)

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
        self.w = w
        self.h = h

        self.canvas.setScale(1.0 / w, 1.0, 1.0 / h)
        self.mw.setFrame(0, w, 0, h * self.top)

        # Sort the regions from largest to smallest to maximize
        # packing effectiveness.
        texRecords = self.texRecordsByTex.values()
        texRecords.sort(key = lambda tr: (tr.w, tr.h), reverse = True)
        
        self.overflowing = False
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
        for tp in tr.placements:
            del self.texPlacements[tp]
        tr.placements = []
        tr.clearCard(self)

    def placeTexture(self, tr):
        """ Places the texture somewhere on the canvas where it will
        fit. """

        if not self.overflowing:
            tp = self.findHole(tr.w, tr.h)
            if tp:
                tr.placements = [tp]
                tr.makeCard(self)
                self.texPlacements[tp] = tr
                return

            # Couldn't find a hole; can we fit it if we rotate?
            tp = self.findHole(tr.h, tr.w)
            if tp:
                tp.rotated = True
                tr.placements = [tp]
                tr.makeCard(self)
                self.texPlacements[tp] = tr
                return

            # Couldn't find a hole of the right shape; can we find a
            # single rectangular hole of the right area, but of any shape?
            tp = self.findArea(tr.w, tr.h)
            if tp:
                texCmp = cmp(tr.w, tr.h)
                holeCmp = cmp(tp.p[1] - tp.p[0], tp.p[3] - tp.p[2])
                if texCmp != 0 and holeCmp != 0 and texCmp != holeCmp:
                    tp.rotated = True
                tr.placements = [tp]
                tr.makeCard(self)
                self.texPlacements[tp] = tr
                return

            # Couldn't find a single rectangular hole.  We'll have to
            # divide the texture up into several smaller pieces to cram it
            # in.
            tpList = self.findHolePieces(tr.h * tr.w)
            if tpList:
                texCmp = cmp(tr.w, tr.h)
                tr.placements = tpList
                for tp in tpList:
                    holeCmp = cmp(tp.p[1] - tp.p[0], tp.p[3] - tp.p[2])
                    if texCmp != 0 and holeCmp != 0 and texCmp != holeCmp:
                        tp.rotated = True
                    self.texPlacements[tp] = tr
                tr.makeCard(self)
                return

        # Just let it overflow.
        self.overflowing = True
        tp = self.findHole(tr.w, tr.h, allowOverflow = True)
        if tp:
            if tp.p[1] > self.w or tp.p[3] > self.h:
                tp.overflowed = 1
            tr.placements = [tp]
            tr.makeCard(self)
            self.texPlacements[tp] = tr
            return

        # Something went wrong.
        assert False

    def findHole(self, w, h, allowOverflow = False):
        """ Searches for a hole large enough for (w, h).  If one is
        found, returns an appropriate TexPlacement; otherwise, returns
        None. """

        if w > self.w:
            # It won't fit within the row at all.
            if not allowOverflow:
                return None
            # Just stack it on the top.
            y = 0
            if self.texPlacements:
                y = max(map(lambda tp: tp.p[3], self.texPlacements.keys()))
            tp = TexPlacement(0, w, y, y + h)
            return tp
            
        y = 0
        while y + h <= self.h or allowOverflow:
            nextY = None

            # Scan along the row at 'y'.
            x = 0
            while x + w <= self.w:
                # Consider the spot at x, y.
                tp = TexPlacement(x, x + w, y, y + h)
                overlap = self.findOverlap(tp)
                if not overlap:
                    # Hooray!
                    return tp

                nextX = overlap.p[1]
                if nextY is None:
                    nextY = overlap.p[3]
                else:
                    nextY = min(nextY, overlap.p[3])

                assert nextX > x
                x = nextX

            assert nextY > y
            y = nextY

        # Nope, wouldn't fit anywhere.
        return None

    def findArea(self, w, h):
        """ Searches for a rectangular hole that is at least area
        square units big, regardless of its shape, but attempt to find
        one that comes close to the right shape, at least.  If one is
        found, returns an appropriate TexPlacement; otherwise, returns
        None. """

        aspect = float(min(w, h)) / float(max(w, h))
        area = w * h
        holes = self.findAvailableHoles(area)

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
                nh = area / tw
                assert nh <= th
                th = nh
            elif th < h:
                # We'd have to make it narrower.
                nw = area / th
                assert nw <= tw
                tw = nw
            else:
                # We don't have to squish it?  Shouldn't have gotten
                # here.
                assert False

            # Make a new tp that has the right area.
            tp = TexPlacement(l, l + tw, b, b + th)
            ta = float(min(tw, th)) / float(max(tw, th))

            match = min(ta, aspect) / max(ta, aspect)
            matches.append((match, tp))

        if matches:
            return max(matches)[1]
        return None

    def findAllArea(self, area):
        """ Searches for a rectangular hole that is at least area
        square units big, regardless of its shape.  Returns a list of
        all such holes found. """

        result = []

        y = 0
        while y < self.h:
            nextY = self.h

            # Scan along the row at 'y'.
            x = 0
            while x < self.w:
                nextX = self.w

                # Consider the spot at x, y.

                # How wide can we go?  Start by trying to go all the
                # way to the edge of the region.
                tpw = self.w - x

                # Now, given this particular width, how tall do we
                # need to go?
                tph = area / tpw

                while y + tph < self.h:
                    tp = TexPlacement(x, x + tpw, y, y + tph)
                    overlap = self.findOverlap(tp)
                    if not overlap:
                        result.append(tp)

                    nextX = min(nextX, overlap.p[1])
                    nextY = min(nextY, overlap.p[3])

                    # Shorten the available region.
                    tpw0 = overlap.p[0] - x
                    if tpw0 <= 0.0:
                        break
                    if x + tpw0 == x + tpw:
                        tpw0 *= 0.999  # imprecision hack
                    tpw = tpw0
                    tph = area / tpw
                
                assert nextX > x
                x = nextX

            assert nextY > y
            y = nextY

        return result

    def findHolePieces(self, area):
        """ Returns a list of holes whose net area sums to the given
        area, or None if there are not enough holes. """

        # First, save the original value of self.texPlacements, since
        # we will be modifying that during this search.
        savedTexPlacements = copy.copy(self.texPlacements)

        result = []

        while area > 0:
            tp = self.findLargestHole()
            if not tp:
                break
            
            l, r, b, t = tp.p
            tpArea = (r - l) * (t - b)
            if tpArea >= area:
                # we're done.
                shorten = (tpArea - area) / (r - l)
                tp.p = (l, r, b, t - shorten)
                result.append(tp)
                self.texPlacements = savedTexPlacements
                return result

            # Keep going.
            area -= tpArea
            result.append(tp)
            self.texPlacements[tp] = None

        # Huh, not enough room, or no more holes.
        self.texPlacements = savedTexPlacements
        return None

    def findLargestHole(self):
        holes = self.findAvailableHoles(0)
        if holes:
            return max(holes)[1]
        return None

    def findAvailableHoles(self, area):
        """ Finds a list of available holes, of at least the indicated
        area.  Returns a list of tuples, where each tuple is of the
        form (area, tp)."""

        holes = []
            
        y = 0
        while y < self.h:
            nextY = self.h

            # Scan along the row at 'y'.
            x = 0
            while x < self.w:
                nextX = self.w

                # Consider the spot at x, y.

                # How wide can we go?  Start by trying to go all the
                # way to the edge of the region.
                tpw = self.w - x

                # And how tall can we go?  Start by trying to go to
                # the top of the region.
                tph = self.h - y

                while tpw > 0.0 and tph > 0.0:
                    tp = TexPlacement(x, x + tpw, y, y + tph)
                    overlap = self.findOverlap(tp)
                    if not overlap:
                        # Here's a hole.
                        tarea = tpw * tph
                        if tarea >= area:
                            holes.append((tarea, tp))
                        break

                    nextX = min(nextX, overlap.p[1])
                    nextY = min(nextY, overlap.p[3])

                    # We've been intersected either on the top or the
                    # right.  We need to shorten either width or
                    # height.  Which way results in the largest
                    # remaining area?

                    tpw0 = overlap.p[0] - x
                    tph0 = overlap.p[2] - y

                    if tpw0 * tph > tpw * tph0:
                        # Shortening width results in larger.
                        if x + tpw == x + tpw0:
                            tpw0 *= 0.999  # imprecision hack
                        tpw = tpw0
                    else:
                        # Shortening height results in larger.
                        if y + tph == y + tph0:
                            tph0 *= 0.999  # imprecision hack
                        tph = tph0
                    #print "x = %s, y = %s, tpw = %s, tph = %s" % (x, y, tpw, tph)
                
                assert nextX > x
                x = nextX

            assert nextY > y
            y = nextY

        return holes

    def findOverlap(self, tp):
        """ If there is another placement that overlaps the indicated
        TexPlacement, returns it.  Otherwise, returns None. """

        for other in self.texPlacements.keys():
            if other.intersects(tp):
                return other

        return None
            


class TexRecord:
    def __init__(self, key, tex, size, active):
        self.key = key
        self.tex = tex
        self.active = active
        self.root = None
        self.regions = []
        self.placements = []

        self.setSize(size)

    def setSize(self, size):
        self.size = size
        x = self.tex.getXSize()
        y = self.tex.getYSize()
        r = float(y) / float(x)

        # Card size
        w = math.sqrt(self.size) / math.sqrt(r)
        h = w * r

        self.w = w
        self.h = h


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
        self.rotated = False
        self.overflowed = 0

    def intersects(self, other):
        """ Returns True if the placements intersect, False
        otherwise. """

        ml, mr, mb, mt = self.p
        tl, tr, tb, tt = other.p

        return (tl < mr and tr > ml and
                tb < mt and tt > mb)

