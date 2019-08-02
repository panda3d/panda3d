"""Contains the DirectScrolledFrame class."""

__all__ = ['DirectScrolledFrame']

from panda3d.core import *
from . import DirectGuiGlobals as DGG
from .DirectFrame import *
from .DirectScrollBar import *

"""
import DirectScrolledFrame
d = DirectScrolledFrame(borderWidth=(0, 0))
"""

class DirectScrolledFrame(DirectFrame):
    """
    DirectScrolledFrame -- a special frame that uses DirectScrollBar to
    implement a small window (the frameSize) into a potentially much
    larger virtual canvas (the canvasSize, scrolledFrame.getCanvas()).

    Unless specified otherwise, scroll bars are automatically created
    and managed as needed, based on the relative sizes od the
    frameSize and the canvasSize.  You can also set manageScrollBars =
    0 and explicitly position and hide or show the scroll bars
    yourself.
    """
    def __init__(self, parent = None, **kw):
        optiondefs = (
            # Define type of DirectGuiWidget
            ('pgFunc',         PGScrollFrame,      None),
            ('frameSize',      (-0.5, 0.5, -0.5, 0.5), None),

            ('canvasSize',     (-1, 1, -1, 1),        self.setCanvasSize),
            ('manageScrollBars', 1,                self.setManageScrollBars),
            ('autoHideScrollBars', 1,              self.setAutoHideScrollBars),
            ('scrollBarWidth', 0.08,               self.setScrollBarWidth),
            ('borderWidth',    (0.01, 0.01),       self.setBorderWidth),
            )

        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        # The scrollBarWidth parameter is just used at scroll bar
        # construction time, and supplies a default frame.  It does
        # not override an explicit frame specified on the scroll bar.
        # If you want to change the frame width after construction,
        # you must specify their frameSize tuples explicitly.
        w = self['scrollBarWidth']

        self.verticalScroll = self.createcomponent(
            "verticalScroll", (), None,
            DirectScrollBar, (self,),
            borderWidth = self['borderWidth'],
            frameSize = (-w / 2.0, w / 2.0, -1, 1),
            orientation = DGG.VERTICAL)

        self.horizontalScroll = self.createcomponent(
            "horizontalScroll", (), None,
            DirectScrollBar, (self,),
            borderWidth = self['borderWidth'],
            frameSize = (-1, 1, -w / 2.0, w / 2.0),
            orientation = DGG.HORIZONTAL)

        self.guiItem.setVerticalSlider(self.verticalScroll.guiItem)
        self.guiItem.setHorizontalSlider(self.horizontalScroll.guiItem)

        self.canvas = NodePath(self.guiItem.getCanvasNode())

        # Call option initialization functions
        self.initialiseoptions(DirectScrolledFrame)

    def setScrollBarWidth(self):
        w = self['scrollBarWidth']
        self.verticalScroll["frameSize"] = (-w / 2.0, w / 2.0, -1, 1)
        self.horizontalScroll["frameSize"] = (-1, 1, -w / 2.0, w / 2.0)

    def setCanvasSize(self):
        f = self['canvasSize']
        self.guiItem.setVirtualFrame(f[0], f[1], f[2], f[3])

    def getCanvas(self):
        return self.canvas

    def setManageScrollBars(self):
        self.guiItem.setManagePieces(self['manageScrollBars'])

    def setAutoHideScrollBars(self):
        self.guiItem.setAutoHide(self['autoHideScrollBars'])

    def commandFunc(self):
        if self['command']:
            self['command'](*self['extraArgs'])

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
        if self.verticalScroll:
            self.verticalScroll.destroy()
        if self.horizontalScroll:
            self.horizontalScroll.destroy()
        self.verticalScroll = None
        self.horizontalScroll = None
        DirectFrame.destroy(self)
