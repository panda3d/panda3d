__all__ = ['DirectEntryScroll']

from panda3d.core import *
from . import DirectGuiGlobals as DGG
from .DirectScrolledFrame import *
from .DirectFrame import *
from .DirectEntry import *

class DirectEntryScroll(DirectFrame):
    def __init__(self, entry, parent = None, **kw):
        optiondefs = (
            ('pgFunc',         PGVirtualFrame,      None),
            ('relief', None, None),
            ('clipSize',     (-1, 1, -1, 1),        self.setClipSize),
            )

        self.defineoptions(kw, optiondefs)
        DirectFrame.__init__(self, parent, **kw)
        self.canvas = None
        self.visXMin = 0.0
        self.visXMax = 0.0
        self.clipXMin = 0.0
        self.clipXMax = 0.0
        self.initialiseoptions(DirectEntryScroll)

        # don't set a scale on the entry
        # instead make it the correct size, use something like:
           # text_scale = 0.035,
           # frameSize = (-0.006, 3.2, -0.015, 0.036),
        # if you need to scale the entry scale it's parent instead

        self.canvas = NodePath(self.guiItem.getCanvasNode())
        self.canvas.setPos(0,0,0)

        self.entry = None
        if entry is not None:
            self.entry = entry
            self.entry.reparentTo(self.canvas)
            self.entry.bind(DGG.CURSORMOVE, self.cursorMove)

        self.canvas.node().setBounds(OmniBoundingVolume())
        self.canvas.node().setFinal(1)
        self.resetCanvas()

    def setEntry(self, entry):
        """
        Sets a DirectEntry element for this scroll frame. A DirectEntryScroll
        can only hold one entry at a time, so make sure to not call this
        function twice or call clearEntry before to make sure no entry
        is already set.
        """
        assert self.entry is None, "An entry was already set for this DirectEntryScroll element"
        self.entry = entry
        self.entry.reparentTo(self.canvas)

        self.entry.bind(DGG.CURSORMOVE, self.cursorMove)

    def clearEntry(self):
        """
        detaches and unbinds the entry from the scroll frame and its
        events. You'll be responsible for destroying it.
        """
        if self.entry is None: return
        self.entry.unbind(DGG.CURSORMOVE)
        self.entry.detachNode()
        self.entry = None

    def cursorMove(self, cursorX, cursorY):
        cursorX = self.entry.guiItem.getCursorX() * self.entry['text_scale'][0]
        canvasX = self.canvas.getX()
        visXMin = self.clipXMin - canvasX
        visXMax = self.clipXMax - canvasX
        visXCenter = (visXMin + visXMax) * 0.5
        distanceToCenter = visXCenter - cursorX
        clipExtent = self.clipXMax - self.clipXMin

        entryExtent = self.entry['text_scale'][0] * self.entry['width']
        entryWiggle = entryExtent - clipExtent

        if abs(distanceToCenter) > (clipExtent * 0.5):
            self.moveToCenterCursor()


    def moveToCenterCursor(self):
        cursorX = self.entry.guiItem.getCursorX() * self.entry['text_scale'][0]
        canvasX = self.canvas.getX()
        visXMin = self.clipXMin - canvasX
        visXMax = self.clipXMax - canvasX
        visXCenter = (visXMin + visXMax) * 0.5
        distanceToCenter = visXCenter - cursorX
        newX = canvasX + distanceToCenter

        clipExtent = self.clipXMax - self.clipXMin

        entryExtent = self.entry['text_scale'][0] * self.entry['width']
        entryWiggle = entryExtent - clipExtent

        if self.entry.guiItem.getCursorPosition() <= 0: #deals with the cursor jump bug
            newX = 0.0
        elif newX > 0.0:
            newX = 0.0
        elif newX < (-entryWiggle):
            newX = -entryWiggle

        #print("CursorX %s CanvasX %s VisCenter %s Distance %s NewX %s Wiggle %s" % (cursorX, canvasX, visXCenter, distanceToCenter, newX, entryWiggle))

        self.canvas.setX(newX)

    def destroy(self):
        # Destroy children of the canvas
        for child in self.canvas.getChildren():
            childGui = self.guiDict.get(child.getName())
            if childGui:
                childGui.destroy()
            else:
                parts = child.getName().split('-')
                simpleChildGui = self.guiDict.get(parts[-1])
                if simpleChildGui:
                    simpleChildGui.destroy()
        self.entry.destroy()
        self.entry = None
        DirectFrame.destroy(self)


    def getCanvas(self):
        return self.canvas

    def setClipSize(self):
        self.guiItem.setClipFrame(self['clipSize'])
        self.clipXMin = self['clipSize'][0]
        self.clipXMax = self['clipSize'][1]
        self.visXMin = self.clipXMin
        self.visXMax = self.clipXMax
        if self.canvas:
            self.resetCanvas()

    def resetCanvas(self):
        self.canvas.setPos(0,0,0)

