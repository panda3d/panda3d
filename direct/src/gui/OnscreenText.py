"""OnscreenText module: contains the OnscreenText class"""

from PandaObject import *
from GuiGlobals import *

class OnscreenText(PandaObject, NodePath):

    def __init__(self, string, x=0.0, y=0.0, font=getDefaultFont()):
        """__init__(self, string, float=0.0, float=0.0,
        font=GuiGlobals.getDefaultFont())
        Make a text node from string, put it into the 2d sg and place
        it at x, y in screen space
        """
        # become one with our NodePath-ness
        NodePath.__init__(self)

        # make a text node
        self.textNode = textNode = TextNode()
        # Freeze the node while we set all the properties
        textNode.freeze()
        textNode.setBillboard(0)
        textNode.setTextColor(0.0, 0.0, 0.0, 1.0)
        textNode.setCardColor(1.0, 1.0, 1.0, 1.0)
        textNode.setCardAsMargin(0.1, 0.1, 0.1, 0.1)
        # textNode.setFrameColor(0.0, 0.0, 0.0, 1.0)
        # textNode.setFrameAsMargin(0.15, 0.15, 0.15, 0.15)
        textNode.setFont(font)
        textNode.setText(string)
        textNode.clearCardBorder()
        textNode.clearFrame()
        # Ok, now update the node
        textNode.thaw()

        # put the text node into the 2d scene graph
        textNodePath = render2d.attachNewNode(textNode)

        # we ARE this node path
        self.assign(textNodePath)

        # position ourselves
        self.setXY(x, y)

        # assume 4:3 aspect ratio
        self.setScale( 0.069, 1.0, 0.069)
	self.isClean = 0
	return None

    def cleanup(self):
	"""cleanup(self)
	"""
	self.textNode = None
	if self.isClean == 0:
	    self.isClean = 1
       	    NodePath.__del__(self) 
	return None

    def freeze(self):
        self.textNode.freeze()

    def thaw(self):
        self.textNode.thaw()
 
    def setText(self, string):
        """setText(self, string)
        Set the text of the onscreen text
        """
        self.node().setText(string)


    def setXY(self, x, y):
        """setPos(self, float, float)
        Position the onscreen text in 2d screen space
        """
        # render2d has x across and z up
        self.setPos(x, 0.0, y)

    def setColor(self, color):
        self.textNode.setCardColor(color[0],color[1],color[2],color[3])
