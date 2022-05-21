"""Contains the BufferViewer class, which is used as a debugging aid
when debugging render-to-texture effects.  It shows different views at
the bottom of the screen showing the various render targets.

When using ShowBase, the normal way to enable the BufferViewer is using the
following code::

    base.bufferViewer.toggleEnable()

Or, you can enable the following variable in your Config.prc::

    show-buffers true
"""

__all__ = ['BufferViewer']

from panda3d.core import *
from direct.task import Task
from direct.task.TaskManagerGlobal import taskMgr
from direct.directnotify.DirectNotifyGlobal import *
from direct.showbase.DirectObject import DirectObject
import math


class BufferViewer(DirectObject):
    notify = directNotify.newCategory('BufferViewer')

    def __init__(self, win, parent):
        """Access: private.  Constructor."""
        self.enabled = 0
        size = ConfigVariableDouble('buffer-viewer-size', '0 0')
        self.sizex = size[0]
        self.sizey = size[1]
        self.position = ConfigVariableString('buffer-viewer-position', "lrcorner").getValue()
        self.layout = ConfigVariableString('buffer-viewer-layout', "hline").getValue()
        self.include = "all"
        self.exclude = "none"
        self.cullbin = "fixed"
        self.cullsort = 10000
        self.win = win
        self.engine = GraphicsEngine.getGlobalPtr()
        self.renderParent = parent
        self.cards = []
        self.cardindex = 0
        self.cardmaker = CardMaker("cubemaker")
        self.cardmaker.setFrame(-1,1,-1,1)
        self.task = 0
        self.dirty = 1
        self.accept("render-texture-targets-changed", self.refreshReadout)
        if (ConfigVariableBool("show-buffers", 0).getValue()):
            self.enable(1)

    def refreshReadout(self):
        """Force the readout to be refreshed.  This is usually invoked
        by GraphicsOutput::add_render_texture (via an event handler).
        However, it is also possible to invoke it manually.  Currently,
        the only time I know of that this is necessary is after a
        window resize (and I ought to fix that)."""
        self.dirty = 1

        # Call enabled again, mainly to ensure that the task has been
        # started.
        self.enable(self.enabled)

    def isValidTextureSet(self, x):
        """Access: private. Returns true if the parameter is a
        list of GraphicsOutput and Texture, or the keyword 'all'."""
        if (isinstance(x, list)):
            for elt in x:
                if (self.isValidTextureSet(elt)==0):
                    return 0
        else:
            return (x=="all") or (isinstance(x, Texture)) or (isinstance(x, GraphicsOutput))

    def isEnabled(self):
        """Returns true if the buffer viewer is currently enabled."""
        return self.enabled

    def enable(self, x):
        """Turn the buffer viewer on or off.  The initial state of the
        buffer viewer depends on the Config variable 'show-buffers'."""
        if (x != 0) and (x != 1):
            BufferViewer.notify.error('invalid parameter to BufferViewer.enable')
            return
        self.enabled = x
        self.dirty = 1
        if (x and self.task == 0):
            self.task = taskMgr.add(self.maintainReadout, "buffer-viewer-maintain-readout",
                                    priority=1)

    def toggleEnable(self):
        """Toggle the buffer viewer on or off.  The initial state of the
        enable flag depends on the Config variable 'show-buffers'."""
        self.enable(1-self.enabled)

    def setCardSize(self, x, y):
        """Set the size of each card.  The units are relative to
        render2d (ie, 1x1 card is not square).  If one of the
        dimensions is zero, then the viewer will choose a value
        for that dimension so as to ensure that the aspect ratio
        of the card matches the aspect ratio of the source-window.
        If both dimensions are zero, the viewer uses a heuristic
        to choose a reasonable size for the card.  The initial
        value is (0, 0)."""
        if (x < 0) or (y < 0):
            BufferViewer.notify.error('invalid parameter to BufferViewer.setCardSize')
            return
        self.sizex = x
        self.sizey = y
        self.dirty = 1

    def setPosition(self, pos):
        """Set the position of the cards.  The valid values are:

        - *llcorner* - put them in the lower-left  corner of the window
        - *lrcorner* - put them in the lower-right corner of the window
        - *ulcorner* - put them in the upper-left  corner of the window
        - *urcorner* - put them in the upper-right corner of the window
        - *window* - put them in a separate window

        The initial value is 'lrcorner'."""
        valid=["llcorner","lrcorner","ulcorner","urcorner","window"]
        if (valid.count(pos)==0):
            BufferViewer.notify.error('invalid parameter to BufferViewer.setPosition')
            BufferViewer.notify.error('valid parameters are: llcorner, lrcorner, ulcorner, urcorner, window')
            return
        if (pos == "window"):
            BufferViewer.notify.error('BufferViewer.setPosition - "window" mode not implemented yet.')
            return
        self.position = pos
        self.dirty = 1

    def setLayout(self, lay):
        """Set the layout of the cards.  The valid values are:

        - *vline* - display them in a vertical line
        - *hline* - display them in a horizontal line
        - *vgrid* - display them in a vertical grid
        - *hgrid* - display them in a horizontal grid
        - *cycle* - display one card at a time, using selectCard/advanceCard

        The default value is 'hline'."""
        valid=["vline","hline","vgrid","hgrid","cycle"]
        if (valid.count(lay)==0):
            BufferViewer.notify.error('invalid parameter to BufferViewer.setLayout')
            BufferViewer.notify.error('valid parameters are: vline, hline, vgrid, hgrid, cycle')
            return
        self.layout = lay
        self.dirty = 1

    def selectCard(self, i):
        """Only useful when using setLayout('cycle').  Sets the index
        that selects which card to display.  The index is taken modulo
        the actual number of cards."""
        self.cardindex = i
        self.dirty = 1

    def advanceCard(self):
        """Only useful when using setLayout('cycle').  Increments the index
        that selects which card to display.  The index is taken modulo
        the actual number of cards."""
        self.cardindex += 1
        self.dirty = 1

    def setInclude(self, x):
        """Set the include-set for the buffer viewer.  The include-set
        specifies which of the render-to-texture targets to display.
        Valid inputs are the string 'all' (display every render-to-texture
        target), or a list of GraphicsOutputs or Textures.  The initial
        value is 'all'."""
        if (self.isValidTextureSet(x)==0):
            BufferViewer.notify.error('setInclude: must be list of textures and buffers, or "all"')
            return
        self.include = x
        self.dirty = 1

    def setExclude(self, x):
        """Set the exclude-set for the buffer viewer.  The exclude-set
        should be a list of GraphicsOutputs and Textures to ignore.
        The exclude-set is subtracted from the include-set (so the excludes
        effectively override the includes.)  The initial value is the
        empty list."""
        if (self.isValidTextureSet(x)==0):
            BufferViewer.notify.error('setExclude: must be list of textures and buffers')
            return
        self.exclude = x
        self.dirty = 1

    def setSort(self, bin, sort):
        """Set the cull-bin and sort-order for the output cards.  The
        default value is 'fixed', 10000."""
        self.cullbin = bin
        self.cullsort = sort
        self.dirty = 1

    def setRenderParent(self, renderParent):
        """Set the scene graph root to which the output cards should
        be parented.  The default is render2d. """
        self.renderParent = renderParent
        self.dirty = 1

    def analyzeTextureSet(self, x, set):
        """Access: private.  Converts a list of GraphicsObject,
        GraphicsEngine, and Texture into a table of Textures."""

        if (isinstance(x, list)):
            for elt in x:
                self.analyzeTextureSet(elt, set)
        elif (isinstance(x, Texture)):
            set[x] = 1
        elif (isinstance(x, GraphicsOutput)):
            for itex in range(x.countTextures()):
                tex = x.getTexture(itex)
                set[tex] = 1
        elif (isinstance(x, GraphicsEngine)):
            for iwin in range(x.getNumWindows()):
                win = x.getWindow(iwin)
                self.analyzeTextureSet(win, set)
        elif (x=="all"):
            self.analyzeTextureSet(self.engine, set)
        else: return


    def makeFrame(self, sizex, sizey):
        """Access: private.  Each texture card is displayed with
        a two-pixel wide frame (a ring of black and a ring of white).
        This routine builds the frame geometry.  It is necessary to
        be precise so that the frame exactly aligns to pixel
        boundaries, and so that it doesn't overlap the card at all."""

        format=GeomVertexFormat.getV3cp()
        vdata=GeomVertexData('card-frame', format, Geom.UHDynamic)

        vwriter=GeomVertexWriter(vdata, 'vertex')
        cwriter=GeomVertexWriter(vdata, 'color')

        ringoffset = [0, 1, 1, 2]
        ringbright = [0, 0, 1, 1]
        for ring in range(4):
            offsetx = (ringoffset[ring]*2.0) / float(sizex)
            offsety = (ringoffset[ring]*2.0) / float(sizey)
            bright = ringbright[ring]
            vwriter.addData3f(Vec3F.rfu(-1 - offsetx, 0, -1 - offsety))
            vwriter.addData3f(Vec3F.rfu( 1 + offsetx, 0, -1 - offsety))
            vwriter.addData3f(Vec3F.rfu( 1 + offsetx, 0,  1 + offsety))
            vwriter.addData3f(Vec3F.rfu(-1 - offsetx, 0,  1 + offsety))
            cwriter.addData3f(bright, bright, bright)
            cwriter.addData3f(bright, bright, bright)
            cwriter.addData3f(bright, bright, bright)
            cwriter.addData3f(bright, bright, bright)

        triangles=GeomTriangles(Geom.UHStatic)
        for i in range(2):
            delta = i*8
            triangles.addVertices(0+delta, 4+delta, 1+delta)
            triangles.addVertices(1+delta, 4+delta, 5+delta)
            triangles.addVertices(1+delta, 5+delta, 2+delta)
            triangles.addVertices(2+delta, 5+delta, 6+delta)
            triangles.addVertices(2+delta, 6+delta, 3+delta)
            triangles.addVertices(3+delta, 6+delta, 7+delta)
            triangles.addVertices(3+delta, 7+delta, 0+delta)
            triangles.addVertices(0+delta, 7+delta, 4+delta)
        triangles.closePrimitive()

        geom=Geom(vdata)
        geom.addPrimitive(triangles)
        geomnode=GeomNode("card-frame")
        geomnode.addGeom(geom)
        return NodePath(geomnode)


    def maintainReadout(self, task):
        """Access: private.  Whenever necessary, rebuilds the entire
        display from scratch.  This is only done when the configuration
        parameters have changed."""

        # If nothing has changed, don't update.
        if (self.dirty==0):
            return Task.cont
        self.dirty = 0

        # Delete the old set of cards.
        for card in self.cards:
            card.removeNode()
        self.cards = []

        # If not enabled, return.
        if (self.enabled == 0):
            self.task = 0
            return Task.done

        # Generate the include and exclude sets.
        exclude = {}
        include = {}
        self.analyzeTextureSet(self.exclude, exclude)
        self.analyzeTextureSet(self.include, include)

        # Use a custom sampler when applying the textures.  This fixes
        # wrap issues and prevents depth compare on shadow maps.
        sampler = SamplerState()
        sampler.setWrapU(SamplerState.WM_clamp)
        sampler.setWrapV(SamplerState.WM_clamp)
        sampler.setWrapW(SamplerState.WM_clamp)
        sampler.setMinfilter(SamplerState.FT_linear)
        sampler.setMagfilter(SamplerState.FT_nearest)

        # Generate a list of cards and the corresponding windows.
        cards = []
        wins = []
        for iwin in range(self.engine.getNumWindows()):
            win = self.engine.getWindow(iwin)
            for itex in range(win.countTextures()):
                tex = win.getTexture(itex)
                if (tex in include) and (tex not in exclude):
                    if (tex.getTextureType() == Texture.TTCubeMap):
                        for face in range(6):
                            self.cardmaker.setUvRangeCube(face)
                            card = NodePath(self.cardmaker.generate())
                            card.setTexture(tex, sampler)
                            cards.append(card)
                    elif (tex.getTextureType() == Texture.TT2dTextureArray):
                        for layer in range(tex.getZSize()):
                            self.cardmaker.setUvRange((0, 1, 1, 0), (0, 0, 1, 1),\
                                                      (layer, layer, layer, layer))
                            card = NodePath(self.cardmaker.generate())
                            # 2D texture arrays are not supported by
                            # the fixed-function pipeline, so we need to
                            # enable the shader generator to view them.
                            card.setShaderAuto()
                            card.setTexture(tex, sampler)
                            cards.append(card)
                    else:
                        card = win.getTextureCard()
                        card.setTexture(tex, sampler)
                        cards.append(card)
                    wins.append(win)
                    exclude[tex] = 1
        self.cards = cards
        if (len(cards)==0):
            self.task = 0
            return Task.done
        ncards = len(cards)

        # Decide how many rows and columns to use for the layout.
        if (self.layout == "hline"):
            rows = 1
            cols = ncards
        elif (self.layout == "vline"):
            rows = ncards
            cols = 1
        elif (self.layout == "hgrid"):
            rows = int(math.sqrt(ncards))
            cols = rows
            if (rows * cols < ncards): cols += 1
            if (rows * cols < ncards): rows += 1
        elif (self.layout == "vgrid"):
            rows = int(math.sqrt(ncards))
            cols = rows
            if (rows * cols < ncards): rows += 1
            if (rows * cols < ncards): cols += 1
        elif (self.layout == "cycle"):
            rows = 1
            cols = 1
        else:
            BufferViewer.notify.error('shouldnt ever get here in BufferViewer.maintainReadout')

        # Choose an aspect ratio for the cards.  All card size
        # calculations are done in pixel-units, using integers,
        # in order to ensure that everything ends up neatly on
        # a pixel boundary.

        aspectx = wins[0].getXSize()
        aspecty = wins[0].getYSize()
        for win in wins:
            if (win.getXSize()*aspecty) != (win.getYSize()*aspectx):
                aspectx = 1
                aspecty = 1

        # Choose a card size.  If the user didn't specify a size,
        # use a heuristic, otherwise, just follow orders.  The
        # heuristic uses an initial card size of 42.66666667% of
        # the screen vertically, which comes to 256 pixels on
        # an 800x600 display.  Then, it double checks that the
        # readout will fit on the screen, and if not, it shrinks it.

        bordersize = 4.0

        if (float(self.sizex)==0.0) and (float(self.sizey)==0.0):
            sizey = int(0.4266666667 * self.win.getYSize())
            sizex = (sizey * aspectx) // aspecty
            v_sizey = (self.win.getYSize() - (rows-1) - (rows*2)) // rows
            v_sizex = (v_sizey * aspectx) // aspecty
            if (v_sizey < sizey) or (v_sizex < sizex):
                sizey = v_sizey
                sizex = v_sizex

            adjustment = 2
            h_sizex = float (self.win.getXSize() - adjustment) / float (cols)

            h_sizex -= bordersize
            if (h_sizex < 1.0):
                h_sizex = 1.0

            h_sizey = (h_sizex * aspecty) // aspectx
            if (h_sizey < sizey) or (h_sizex < sizex):
                sizey = h_sizey
                sizex = h_sizex
        else:
            sizex = int(self.sizex * 0.5 * self.win.getXSize())
            sizey = int(self.sizey * 0.5 * self.win.getYSize())
            if (sizex == 0): sizex = (sizey*aspectx) // aspecty
            if (sizey == 0): sizey = (sizex*aspecty) // aspectx

        # Convert from pixels to render2d-units.
        fsizex = (2.0 * sizex) / float(self.win.getXSize())
        fsizey = (2.0 * sizey) / float(self.win.getYSize())
        fpixelx = 2.0 / float(self.win.getXSize())
        fpixely = 2.0 / float(self.win.getYSize())

        # Choose directional offsets
        if (self.position == "llcorner"):
            dirx = -1.0
            diry = -1.0
        elif (self.position == "lrcorner"):
            dirx =  1.0
            diry = -1.0
        elif (self.position == "ulcorner"):
            dirx = -1.0
            diry =  1.0
        elif (self.position == "urcorner"):
            dirx =  1.0
            diry =  1.0
        else:
            BufferViewer.notify.error('window mode not implemented yet')

        # Create the frame
        frame = self.makeFrame(sizex, sizey)

        # Now, position the cards on the screen.
        # For each card, create a frame consisting of eight quads.

        for r in range(rows):
            for c in range(cols):
                index = c + r*cols
                if (index < ncards):
                    index = (index + self.cardindex) % len(cards)

                    posx = dirx * (1.0 - ((c + 0.5) * (fsizex + fpixelx * bordersize))) - (fpixelx * dirx)
                    posy = diry * (1.0 - ((r + 0.5) * (fsizey + fpixely * bordersize))) - (fpixely * diry)
                    placer = NodePath("card-structure")
                    placer.setPos(Point3.rfu(posx, 0, posy))
                    placer.setScale(Vec3.rfu(fsizex*0.5, 1.0, fsizey*0.5))
                    placer.setBin(self.cullbin, self.cullsort)
                    placer.reparentTo(self.renderParent)
                    frame.instanceTo(placer)
                    cards[index].reparentTo(placer)
                    cards[index] = placer

        return Task.cont
