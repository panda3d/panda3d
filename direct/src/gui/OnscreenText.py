"""OnscreenText module: contains the OnscreenText class"""

from PandaObject import *
import GuiGlobals
import types

## These are the styles of text we might commonly see.  They set the
## overall appearance of the text according to one of a number of
## pre-canned styles.  You can further customize the appearance of the
## text by specifying individual parameters as well.
Plain = 1
ScreenTitle = 2
ScreenPrompt = 3
NameConfirm = 4
BlackOnWhite = 5

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
                 drawOrder = GuiGlobals.getDefaultDrawOrder(),
                 font = GuiGlobals.getDefaultFont(),
                 parent = aspect2d,
                 mayChange = 0):
        """__init__(self, ...)

        Make a text node from string, put it into the 2d sg and set it
        up with all the indicated parameters.

        The parameters are as follows:

          text: the actual text to display.  This may be omitted and
              specified later via setText() if you don't have it
              available, but it is better to specify it up front.

          style: one of the pre-canned style parameters defined at the
              head of this file.  This sets up the default values for
              many of the remaining parameters if they are
              unspecified; however, a parameter may still be specified
              to explicitly set it, overriding the pre-canned style.

          pos: the x, y position of the text on the screen.

          scale: the size of the text.  This may either be a single
              float (and it will usually be a small number like 0.07)
              or it may be a 2-tuple of floats, specifying a different
              x, y scale.

          fg: the (r, g, b, a) foreground color of the text.  This is
              normally a 4-tuple of floats or ints.

          bg: the (r, g, b, a) background color of the text.  If the
              fourth value, a, is nonzero, a card is created to place
              behind the text and set to the given color.

          shadow: the (r, g, b, a) color of the shadow behind the text.
              If the fourth value, a, is nonzero, a little drop shadow
              is created and placed behind the text.

          frame: the (r, g, b, a) color of the frame drawn around the
              text.  If the fourth value, a, is nonzero, a frame is
              created around the text.

          align: one of TMALIGNLEFT, TMALIGNRIGHT, or TMALIGNCENTER.

          wordwrap: either the width to wordwrap the text at, or None
              to specify no automatic word wrapping.

          drawOrder: the drawing order of this text with respect to
              all other things in the 'fixed' bin within render2d.
              The text will actually use drawOrder through drawOrder +
              2.

          font: the font to use for the text.

          parent: the NodePath to parent the text to initially.

          mayChange: pass true if the text or its properties may need
              to be changed at runtime, false if it is static once
              created (which leads to better memory optimization).
        
        """

        # make a text node
        textNode = TextNode()
        self.textNode = textNode

        # We ARE a node path.  Initially, we're an empty node path.
        NodePath.__init__(self)

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

        elif style == BlackOnWhite:
            scale = scale or 0.1
            fg = fg or (0, 0, 0, 1)
            bg = bg or (1, 1, 1, 1)
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

        if not text:
            # If we don't have any text, assume we'll be changing it later.
            mayChange = 1

        # Ok, now update the node.
        if mayChange:
            # If we might change the text later, we have to keep the
            # TextNode around.
            textNode.thaw()
        else:
            # Otherwise, we can throw it away.
            self.textNode = textNode.generate()

            #*** Temporary for old Pandas.
            if self.textNode == None:
                self.textNode = NamedNode()
        
        
	self.isClean = 0

        # Set ourselves up as the NodePath that points to this node.
        self.assign(parent.attachNewNode(self.textNode))
    
    def cleanup(self):
	"""cleanup(self)
	"""
	self.textNode = None
	if self.isClean == 0:
	    self.isClean = 1
            if self.hasArcs():
                self.removeNode()

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

    def getText(self):
        return self.textNode.getText()
        
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
