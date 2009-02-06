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

    def __init__(self, gsg = None, limit = None):
        DirectObject.__init__(self)

        # First, we'll need a name to uniquify the object.
        self.name = 'tex-mem%s' % (TexMemWatcher.NextIndex)
        TexMemWatcher.NextIndex += 1

        self.cleanedUp = False
        
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

        # We should use a tinydisplay pipe, so we don't compete for the
        # graphics memory.
        moduleName = base.config.GetString('tex-mem-pipe', 'tinydisplay')
        if moduleName:
            self.pipe = base.makeModulePipe(moduleName)

        # If the requested pipe fails for some reason, I guess we'll
        # use the regular pipe.
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
        self.win.setClearColor(False)
        self.win.setClearDepth(False)

        eventName = '%s-window' % (self.name)
        self.win.setWindowEvent(eventName)
        self.accept(eventName, self.windowEvent)

        # Make a render2d in this new window.
        self.render2d = NodePath('render2d')
        self.render2d.setDepthTest(False)
        self.render2d.setDepthWrite(False)
        self.render2d.setTwoSided(True)
        
        # And a camera to view it.
        self.dr = self.win.makeDisplayRegion()
        cam = Camera('cam2d')
        self.lens = OrthographicLens()
        self.lens.setNearFar(-1000, 1000)
        cam.setLens(self.lens)

        self.cam = self.render2d.attachNewNode(cam)
        self.dr.setCamera(self.cam)

        # We'll need a Mouse and a MouseWatcher in the data graph, so
        # we can interact with the various textures.
        self.mouse = base.dataRoot.attachNewNode(MouseAndKeyboard(self.win, 0, '%s-mouse' % (self.name)))
        self.mw = MouseWatcher('%s-watcher' % (self.name))
        mwnp = self.mouse.attachNewNode(self.mw)
        bt = ButtonThrower('%s-thrower' % (self.name))
        mwnp.attachNewNode(bt)
        bt.setPrefix('button-%s-' % (self.name))
        self.accept('button-%s-mouse1' % (self.name), self.mouseClick)

        eventName = '%s-enter' % (self.name)
        self.mw.setEnterPattern(eventName)
        self.accept(eventName, self.enterRegion)

        eventName = '%s-leave' % (self.name)
        self.mw.setLeavePattern(eventName)
        self.accept(eventName, self.leaveRegion)

        # Now start handling up the actual stuff in the scene.

        self.canvas = self.render2d.attachNewNode('canvas')
        self.background = None
        self.overflowing = False
        self.nextTexRecordKey = 0
        self.rollover = None
        self.isolate = None
        
        self.task = taskMgr.doMethodLater(0.5, self.updateTextures, 'TexMemWatcher')

        self.setLimit(limit)

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

        self.lens.setFilmSize(1, self.top)
        self.lens.setFilmOffset(0.5, self.top / 2.0)  # lens covers 0..1 in x and y

        self.makeWindowBackground()
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

        if self.rollover:
            self.rollover.clearRollover()
        
        self.rollover = tr

        if self.rollover:
            self.rollover.showRollover(pi, self)

    def isolateTexture(self, tr):
        """ Isolates the indicated texture onscreen, or None to
        restore normal mode. """

        if self.isolate:
            self.isolate.removeNode()
            self.isolate = None

        self.canvas.show()
        self.background.clearColor()
        self.win.getGsg().setTextureQualityOverride(Texture.QLDefault)

        if not tr:
            return

        self.canvas.hide()
        # Disable the red bar at the top.
        self.background.setColor(1, 1, 1, 1, 1)

        # Show the texture in all its filtered glory.
        self.win.getGsg().setTextureQualityOverride(Texture.QLBest)

        self.isolate = self.render2d.attachNewNode('isolate')
        self.isolate.setBin('fixed', 0)

        # Put a label on the bottom of the screen.
        tn = TextNode('tn')
        tn.setText('%s\n%s x %s\n%s bytes' % (
            tr.tex.getName(), tr.tex.getXSize(), tr.tex.getYSize(),
            tr.size))
        tn.setAlign(tn.ACenter)
        tn.setCardAsMargin(100.0, 100.0, 0.1, 0.1)
        tn.setCardColor(0.1, 0.2, 0.4, 1)
        tnp = self.isolate.attachNewNode(tn)
        scale = 15.0 / self.winSize[1]
        tnp.setScale(scale * self.winSize[1] / self.winSize[0], scale, scale)
        tnp.setPos(0.5, 0, -tn.getBottom() * scale)

        labelTop = tn.getHeight() * scale

        # Make a card that shows the texture in actual pixel size, but
        # don't let it exceed the screen size.
        tw = tr.tex.getXSize()
        th = tr.tex.getYSize()

        wx = self.winSize[0] * 0.9
        wy = self.winSize[1] * (1.0 - labelTop) * 0.9

        w = min(tw, wx)
        h = min(th, wy)

        sx = w / tw
        sy = h / th
        s = min(sx, sy)

        w = tw * s / float(self.winSize[0])
        h = th * s / float(self.winSize[1])

        cx = 0.5
        cy = 1.0 - (1.0 - labelTop) * 0.5

        l = cx - w * 0.5
        r = cx + w * 0.5
        b = cy - h * 0.5
        t = cy + h * 0.5

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

        self.background.setTexScale(TextureStage.getDefault(),
                                    self.winSize[0] / 20.0, self.winSize[1] / (20.0 * self.top))
        self.repack()

    def makeWindowBackground(self):
        """ Creates a tile to use for coloring the background of the
        window, so we can tell what empty space looks like. """

        if self.background:
            self.background.detachNode()
            self.background = None

        # We start with a simple checkerboard texture image.
        p = PNMImage(2, 2, 1)
        p.setGray(0, 0, 0.40)
        p.setGray(1, 1, 0.40)
        p.setGray(0, 1, 0.80)
        p.setGray(1, 0, 0.80)

        tex = Texture('check')
        tex.load(p)
        tex.setMagfilter(tex.FTNearest)

        self.background = self.render2d.attachNewNode('background')

        cm = CardMaker('background')
        cm.setFrame(0, 1, 0, 1)
        cm.setUvRange((0, 0), (1, 1))
        self.background.attachNewNode(cm.generate())

        cm.setFrame(0, 1, 1, self.top)
        cm.setUvRange((0, 1), (1, self.top))
        bad = self.background.attachNewNode(cm.generate())
        bad.setColor((0.8, 0.2, 0.2, 1))

        self.background.setBin('fixed', -100)
        self.background.setTexture(tex)


    def updateTextures(self, task):
        """ Gets the current list of resident textures and adds new
        textures or removes old ones from the onscreen display, as
        necessary. """

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
        self.mw.setFrame(0, w, 0, h)

        # Sort the regions from largest to smallest to maximize
        # packing effectiveness.
        texRecords = self.texRecordsByTex.values()
        texRecords.sort(key = lambda tr: (-tr.w, -tr.h))
        
        self.overflowing = False
        for tr in texRecords:
            self.placeTexture(tr)

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
            tp = self.findArea(tr.h * tr.w)
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
                tr.placements = tpList
                tr.makeCard(self)
                for tp in tpList:
                    self.texPlacements[tp] = tr
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
        

    def findArea(self, area):
        """ Searches for a rectangular hole that is at least area
        square units big, regardless of its shape.  If one is found,
        returns an appropriate TexPlacement; otherwise, returns
        None. """

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
                        # Hooray!
                        return tp

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

        # Nope, wouldn't fit anywhere.
        return None

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
        """ Searches for the largest available hole. """

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
                        holes.append((tpw * tph, tp))
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

        if not holes:
            # No holes to be found.
            return None

        # Return the biggest hole
        return max(holes)[1]

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
        self.rollover = None

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

        # A backing to put behind the card.
        backing = root.attachNewNode('backing')

        # A card to display the texture.
        card = root.attachNewNode('card')

        # A matte to frame the texture and indicate its status.
        matte = root.attachNewNode('matte')

        # A wire frame to ring the matte and separate the card from
        # its neighbors.
        frame = root.attachNewNode('frame')


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

        matte.setBin('fixed', 0)
        #matte.flattenStrong()
        self.matte = matte

        backing.setBin('fixed', 10)
        #backing.flattenStrong()
        self.backing = backing

        card.setTransparency(TransparencyAttrib.MAlpha)
        card.setBin('fixed', 20)
        card.setTexture(self.tex)
        #card.flattenStrong()
        self.card = card

        frame.setBin('fixed', 30)
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
            self.regions.append(p)

    def showRollover(self, pi, tmw):
        self.clearRollover()
        try:
            p = self.placements[pi]
        except IndexError:
            return

        # Center the rollover rectangle over the placement
        l, r, b, t = p.p
        cx0 = (l + r) * 0.5
        cy0 = (b + t) * 0.5

        # Exaggerate its size a bit
        w = self.w
        h = self.h

        cx = cx0
        if cx + w > tmw.w:
            cx = tmw.w - w
        if cx - w < 0:
            cx = w

        cy = cy0
        if cy + h > tmw.h:
            cy = tmw.h - h
        if cy - h < 0:
            cy = h

        # But keep it within the window
        l = max(cx - w, 0)
        r = min(cx + w, tmw.w)
        b = max(cy - h, 0)
        t = min(cy + h, tmw.h)

        # If it needs to be shrunk to fit within the window, keep it
        # the same aspect ratio.
        sx = float(r - l) / float(w)
        sy = float(t - b) / float(h)
        if sx != sy:
            s = min(sx, sy)
            w *= s / sx
            h *= s / sy

            cx = cx0
            if cx + w > tmw.w:
                cx = tmw.w - w
            if cx - w < 0:
                cx = w

            cy = cy0
            if cy + h > tmw.h:
                cy = tmw.h - h
            if cy - h < 0:
                cy = h

            l = max(cx - w, 0)
            r = min(cx + w, tmw.w)
            b = max(cy - h, 0)
            t = min(cy + h, tmw.h)

        self.rollover = tmw.canvas.attachNewNode('rollover')

        cm = CardMaker('backing')
        cm.setFrame(l, r, b, t)
        cm.setColor(0.1, 0.3, 0.5, 1)
        c = self.rollover.attachNewNode(cm.generate())
        c.setBin('fixed', 100)
            
        cm = CardMaker('card')
        cm.setFrame(l, r, b, t)
        c = self.rollover.attachNewNode(cm.generate())
        c.setTexture(self.tex)
        c.setBin('fixed', 110)
        c.setTransparency(TransparencyAttrib.MAlpha)

        # Label the font too.
        tn = TextNode('tn')
        tn.setText('%s\n%s x %s' % (self.tex.getName(), self.tex.getXSize(), self.tex.getYSize()))
        tn.setAlign(tn.ACenter)
        tn.setShadow(0.05, 0.05)
        tnp = self.rollover.attachNewNode(tn)
        scale = 20.0 / tmw.winSize[1] * tmw.h
        tnp.setScale(scale)
        tx = (l + r) * 0.5
        ty = b + scale * 2
        tnp.setPos(tx, 0, ty)
        if tx + tn.getWidth() * scale * 0.5 > tmw.w:
            tn.setAlign(tn.ARight)
            tnp.setX(r)
        elif tx - tn.getWidth() * scale * 0.5 < 0:
            tn.setAlign(tn.ALeft)
            tnp.setX(l)
        tnp.setBin('fixed', 120)

    def clearRollover(self):
        if self.rollover:
            self.rollover.removeNode()
            self.rollover = None

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

