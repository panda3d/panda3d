"""OnscreenText module: contains the OnscreenText class"""

from PandaObject import *
from GuiGlobals import *
import types

## These are the styles of text we might commonly see.  They set the
## overall appearance of the text according to one of a number of
## pre-canned styles.  You can further customize the appearance of the
## text by specifying individual parameters as well.
Plain = 1
ScreenTitle = 2
ScreenPrompt = 3
NameConfirm = 4

class OnscreenText(PandaObject, NodePath):

    def __init__(self, text = '',
                 style = Plain,
                 pos = (0, 0),
                 scale = None,
                 fg = None,
                 bg = None,
                 shadow = None,
                 frame = None,
                 align = None,
                 wordwrap = None,
                 drawOrder = getDefaultDrawOrder(),
                 font = getDefaultFont(),
                 parent = aspect2d):
        """__init__(self, ...)

        Make a text node from string, put it into the 2d sg and set it
        up with all the indicated parameters.
        
        """

        # make a text node
        textNode = TextNode()
        self.textNode = textNode

        # We ARE the node path that references this text node.
        NodePath.__init__(self, parent.attachNewNode(textNode))

        # Choose the default parameters according to the selected
        # style.
        if style == Plain:
            scale = scale or 0.07
            fg = fg or (0, 0, 0, 1)
            bg = bg or (0, 0, 0, 0)
            shadow = shadow or (0, 0, 0, 0)
            frame = frame or (0, 0, 0, 0)
            align = align or TMALIGNCENTER

        elif style == ScreenTitle:
            scale = scale or 0.15
            fg = fg or (1, 0.2, 0.2, 1)
            bg = bg or (0, 0, 0, 0)
            shadow = shadow or (0, 0, 0, 1)
            frame = frame or (0, 0, 0, 0)
            align = align or TMALIGNCENTER

        elif style == ScreenPrompt:
            scale = scale or 0.1
            fg = fg or (1, 1, 0, 1)
            bg = bg or (0, 0, 0, 0)
            shadow = shadow or (0, 0, 0, 1)
            frame = frame or (0, 0, 0, 0)
            align = align or TMALIGNCENTER

        elif style == NameConfirm:
            scale = scale or 0.1
            fg = fg or (0, 1, 0, 1)
            bg = bg or (0, 0, 0, 0)
            shadow = shadow or (0, 0, 0, 0)
            frame = frame or (0, 0, 0, 0)
            align = align or TMALIGNCENTER

        else:
            raise ValueError

        if not isinstance(scale, types.TupleType):
            # If the scale is already a tuple, it's a 2-d (x,y) scale.
            # Otherwise, it's a uniform scale--make it a tuple.
            scale = (scale, scale)

        # Save some of the parameters for posterity.
        self.scale = scale
        self.pos = pos
        
        # Freeze the node while we set all the properties
        textNode.freeze()
        textNode.setFont(font)
        textNode.setTextColor(fg[0], fg[1], fg[2], fg[3])
        textNode.setAlign(align)

        if wordwrap:
            textNode.setWordwrap(wordwrap)

        if bg[3] != 0:
            # If we have a background color, create a card.
            textNode.setCardColor(bg[0], bg[1], bg[2], bg[3])
            textNode.setCardAsMargin(0.1, 0.1, 0.1, 0.1)

        if shadow[3] != 0:
            # If we have a shadow color, create a shadow.
            textNode.setShadowColor(shadow[0], shadow[1], shadow[2], shadow[3])
            textNode.setShadow(0.03, 0.03)

        if frame[3] != 0:
            # If we have a frame color, create a frame.
            textNode.setFrameColor(frame[0], frame[1], frame[2], frame[3])
            textNode.setFrameAsMargin(0.1, 0.1, 0.1, 0.1)

        # Create a transform for the text for our scale and position.
        # We'd rather do it here, on the text itself, rather than on
        # our NodePath, so we have one fewer transforms in the scene
        # graph.
        mat = Mat4.scaleMat(scale[0], 1, scale[1]) * Mat4.translateMat(pos[0], 0, pos[1])
        textNode.setTransform(mat)

        textNode.setBin('fixed')
        textNode.setDrawOrder(drawOrder)

        textNode.setText(text)

        # Ok, now update the node
        textNode.thaw()
	self.isClean = 0

    def __del__(self):
        # Make sure the node is removed when we delete the
        # OnscreenText object.  This means we don't have to explicitly
        # remove an OnscreenText object; it can do it by itself.
        # Maybe this will be too confusing because we *do* have to
        # explicitly remove other kinds of onscreen objects.
        self.cleanup()
    
    def cleanup(self):
	"""cleanup(self)
	"""
	self.textNode = None
	if self.isClean == 0:
	    self.isClean = 1
            if self.hasArcs():
                self.removeNode()
       	    NodePath.__del__(self) 

    def freeze(self):
        self.textNode.freeze()

    def thaw(self):
        self.textNode.thaw()

    # Allow changing of several of the parameters after the text has
    # been created.  These should be used with caution; it is better
    # to set all the parameters up front.  These functions are
    # primarily intended for interactive placement of the initial
    # text, and for those rare occasions when you actually want to
    # change a text's property after it has been created.

    # If you need to change several properties at the same time at
    # runtime, you should call freeze() first and thaw() afterward.

    def setText(self, text):
        self.textNode.setText(text)
        
    def setPos(self, x, y):
        """setPos(self, float, float)
        Position the onscreen text in 2d screen space
        """
        self.pos = (x, y)
        mat = Mat4.scaleMat(self.scale[0], 1, self.scale[1]) * Mat4.translateMat(self.pos[0], 0, self.pos[1])
        self.textNode.setTransform(mat)

    def getPos(self):
        return self.pos
        
    def setScale(self, sx, sy = None):
        """setScale(self, float, float)
        Scale the text in 2d space.  You may specify either a single
        uniform scale, or two scales, or a tuple of two scales.
        """

        if sy == None:
            if isinstance(sx, types.TupleType):
                self.scale = sx
            else:
                self.scale = (sx, sx)
        else:
            self.scale = (sx, sy)
        mat = Mat4.scaleMat(self.scale[0], 1, self.scale[1]) * Mat4.translateMat(self.pos[0], 0, self.pos[1])
        self.textNode.setTransform(mat)

    def getScale(self):
        if self.scale[0] == self.scale[1]:
            return self.scale[0]
        else:
            return self.scale
        
    def setWordwrap(self, wordwrap):
        if wordwrap:
            self.textNode.setWordwrap(wordwrap)
        else:
            self.textNode.clearWordwrap()
        
    def setFg(self, fg):
        self.textNode.setTextColor(fg[0], fg[1], fg[2], fg[3])

    def setBg(self, bg):
        self.textNode.freeze()
        if bg[3] != 0:
            # If we have a background color, create a card.
            self.textNode.setCardColor(bg[0], bg[1], bg[2], bg[3])
            self.textNode.setCardAsMargin(0.1, 0.1, 0.1, 0.1)
        else:
            # Otherwise, remove the card.
            self.textNode.clearCard()
        self.textNode.thaw()

    def setShadow(self, shadow):
        self.textNode.freeze()
        if shadow[3] != 0:
            # If we have a shadow color, create a shadow.
            self.textNode.setShadowColor(shadow[0], shadow[1], shadow[2], shadow[3])
            self.textNode.setShadow(0.03, 0.03)
        else:
            # Otherwise, remove the shadow.
            self.textNode.clearShadow()
        self.textNode.thaw()

    def setFrame(self, frame):
        self.textNode.freeze()
        if frame[3] != 0:
            # If we have a frame color, create a frame.
            self.textNode.setFrameColor(frame[0], frame[1], frame[2], frame[3])
            self.textNode.setFrameAsMargin(0.1, 0.1, 0.1, 0.1)
        else:
            # Otherwise, remove the frame.
            self.textNode.clearFrame()
        self.textNode.thaw()
