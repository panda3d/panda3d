__all__ = ['DirectEntryScroll']

from pandac.PandaModules import *
import DirectGuiGlobals as DGG
from DirectScrolledFrame import *
from DirectFrame import *
from DirectEntry import *

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
              
        self.entry = entry
        self.canvas = NodePath(self.guiItem.getCanvasNode())
        self.entry.reparentTo(self.canvas)
        self.canvas.setPos(0,0,0)
        
        self.entry.bind(DGG.CURSORMOVE,self.cursorMove)
        
        self.canvas.node().setBounds(OmniBoundingVolume())
        self.canvas.node().setFinal(1)
        self.resetCanvas()
        
        base.scroller = self
        
 
        
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
        
        
        