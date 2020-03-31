"""OnscreenText module: contains the OnscreenText class.

See the :ref:`onscreentext` page in the programming manual for explanation of
this class.
"""

__all__ = ['OnscreenText', 'Plain', 'ScreenTitle', 'ScreenPrompt', 'NameConfirm', 'BlackOnWhite']

from panda3d.core import *
from . import DirectGuiGlobals as DGG
import sys

## These are the styles of text we might commonly see.  They set the
## overall appearance of the text according to one of a number of
## pre-canned styles.  You can further customize the appearance of the
## text by specifying individual parameters as well.
Plain = 1
ScreenTitle = 2
ScreenPrompt = 3
NameConfirm = 4
BlackOnWhite = 5

class OnscreenText(NodePath):

    def __init__(self, text = '',
                 style = Plain,
                 pos = (0, 0),
                 roll = 0,
                 scale = None,
                 fg = None,
                 bg = None,
                 shadow = None,
                 shadowOffset = (0.04, 0.04),
                 frame = None,
                 align = None,
                 wordwrap = None,
                 drawOrder = None,
                 decal = 0,
                 font = None,
                 parent = None,
                 sort = 0,
                 mayChange = True,
                 direction = None):
        """
        Make a text node from string, put it into the 2d sg and set it
        up with all the indicated parameters.

        Parameters:

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

          align: one of TextNode.ALeft, TextNode.ARight, or TextNode.ACenter.

          wordwrap: either the width to wordwrap the text at, or None
              to specify no automatic word wrapping.

          drawOrder: the drawing order of this text with respect to
              all other things in the 'fixed' bin within render2d.
              The text will actually use drawOrder through drawOrder +
              2.

          decal: if this is True, the text is decalled onto its
              background card.  Useful when the text will be parented
              into the 3-D scene graph.

          font: the font to use for the text.

          parent: the NodePath to parent the text to initially.

          mayChange: pass true if the text or its properties may need
              to be changed at runtime, false if it is static once
              created (which leads to better memory optimization).

          direction: this can be set to 'ltr' or 'rtl' to override the
              direction of the text.
        """
        if parent == None:
            parent = aspect2d

        # make a text node
        textNode = TextNode('')
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
            if align == None:
                align = TextNode.ACenter
        elif style == ScreenTitle:
            scale = scale or 0.15
            fg = fg or (1, 0.2, 0.2, 1)
            bg = bg or (0, 0, 0, 0)
            shadow = shadow or (0, 0, 0, 1)
            frame = frame or (0, 0, 0, 0)
            if align == None:
                align = TextNode.ACenter
        elif style == ScreenPrompt:
            scale = scale or 0.1
            fg = fg or (1, 1, 0, 1)
            bg = bg or (0, 0, 0, 0)
            shadow = shadow or (0, 0, 0, 1)
            frame = frame or (0, 0, 0, 0)
            if align == None:
                align = TextNode.ACenter
        elif style == NameConfirm:
            scale = scale or 0.1
            fg = fg or (0, 1, 0, 1)
            bg = bg or (0, 0, 0, 0)
            shadow = shadow or (0, 0, 0, 0)
            frame = frame or (0, 0, 0, 0)
            if align == None:
                align = TextNode.ACenter
        elif style == BlackOnWhite:
            scale = scale or 0.1
            fg = fg or (0, 0, 0, 1)
            bg = bg or (1, 1, 1, 1)
            shadow = shadow or (0, 0, 0, 0)
            frame = frame or (0, 0, 0, 0)
            if align == None:
                align = TextNode.ACenter
        else:
            raise ValueError

        if not isinstance(scale, tuple):
            # If the scale is already a tuple, it's a 2-d (x, y) scale.
            # Otherwise, it's a uniform scale--make it a tuple.
            scale = (scale, scale)

        # Save some of the parameters for posterity.
        self.__scale = scale
        self.__pos = pos
        self.__roll = roll
        self.__wordwrap = wordwrap

        if decal:
            textNode.setCardDecal(1)

        if font == None:
            font = DGG.getDefaultFont()

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
            # Can't use the *shadow interface because it might be a VBase4.
            #textNode.setShadowColor(*shadow)
            textNode.setShadowColor(shadow[0], shadow[1], shadow[2], shadow[3])
            textNode.setShadow(*shadowOffset)

        if frame[3] != 0:
            # If we have a frame color, create a frame.
            textNode.setFrameColor(frame[0], frame[1], frame[2], frame[3])
            textNode.setFrameAsMargin(0.1, 0.1, 0.1, 0.1)

        if direction is not None:
            if isinstance(direction, str):
                direction = direction.lower()
                if direction == 'rtl':
                    direction = TextProperties.D_rtl
                elif direction == 'ltr':
                    direction = TextProperties.D_ltr
                else:
                    raise ValueError('invalid direction')
            textNode.setDirection(direction)

        # Create a transform for the text for our scale and position.
        # We'd rather do it here, on the text itself, rather than on
        # our NodePath, so we have one fewer transforms in the scene
        # graph.
        self.updateTransformMat()

        if drawOrder != None:
            textNode.setBin('fixed')
            textNode.setDrawOrder(drawOrder)

        self.setText(text)
        if not text:
            # If we don't have any text, assume we'll be changing it later.
            self.mayChange = 1
        else:
            self.mayChange = mayChange

        # Ok, now update the node.
        if not self.mayChange:
            # If we aren't going to change the text later, we can
            # throw away the TextNode.
            self.textNode = textNode.generate()

        self.isClean = 0

        # Set ourselves up as the NodePath that points to this node.
        self.assign(parent.attachNewNode(self.textNode, sort))

    def cleanup(self):
        self.textNode = None
        if self.isClean == 0:
            self.isClean = 1
            self.removeNode()

    def destroy(self):
        self.cleanup()

    def freeze(self):
        pass

    def thaw(self):
        pass

    # Allow changing of several of the parameters after the text has
    # been created.  These should be used with caution; it is better
    # to set all the parameters up front.  These functions are
    # primarily intended for interactive placement of the initial
    # text, and for those rare occasions when you actually want to
    # change a text's property after it has been created.

    def setDecal(self, decal):
        self.textNode.setCardDecal(decal)

    def getDecal(self):
        return self.textNode.getCardDecal()

    decal = property(getDecal, setDecal)

    def setFont(self, font):
        self.textNode.setFont(font)

    def getFont(self):
        return self.textNode.getFont()

    font = property(getFont, setFont)

    def clearText(self):
        self.textNode.clearText()

    def setText(self, text):
        if sys.version_info >= (3, 0):
            assert not isinstance(text, bytes)
            self.unicodeText = True
        else:
            self.unicodeText = isinstance(text, unicode)

        if self.unicodeText:
            self.textNode.setWtext(text)
        else:
            self.textNode.setText(text)

    def appendText(self, text):
        if sys.version_info >= (3, 0):
            assert not isinstance(text, bytes)
            self.unicodeText = True
        else:
            self.unicodeText = isinstance(text, unicode)

        if self.unicodeText:
            self.textNode.appendWtext(text)
        else:
            self.textNode.appendText(text)

    def getText(self):
        if self.unicodeText:
            return self.textNode.getWtext()
        else:
            return self.textNode.getText()

    text = property(getText, setText)

    def setX(self, x):
        self.setPos(x, self.__pos[1])

    def setY(self, y):
        self.setPos(self.__pos[0], y)

    def setPos(self, x, y):
        """setPos(self, float, float)
        Position the onscreen text in 2d screen space
        """
        self.__pos = (x, y)
        self.updateTransformMat()

    def getPos(self):
        return self.__pos

    pos = property(getPos, setPos)

    def setRoll(self, roll):
        """setRoll(self, float)
        Rotate the onscreen text around the screen's normal
        """
        self.__roll = roll
        self.updateTransformMat()

    def getRoll(self):
        return self.__roll

    roll = property(getRoll, setRoll)

    def setScale(self, sx, sy = None):
        """setScale(self, float, float)
        Scale the text in 2d space.  You may specify either a single
        uniform scale, or two scales, or a tuple of two scales.
        """

        if sy == None:
            if isinstance(sx, tuple):
                self.__scale = sx
            else:
                self.__scale = (sx, sx)
        else:
            self.__scale = (sx, sy)
        self.updateTransformMat()

    def updateTransformMat(self):
        assert(isinstance(self.textNode, TextNode))
        mat = (
            Mat4.scaleMat(Vec3.rfu(self.__scale[0], 1, self.__scale[1])) *
            Mat4.rotateMat(self.__roll, Vec3.back()) *
            Mat4.translateMat(Point3.rfu(self.__pos[0], 0, self.__pos[1]))
            )
        self.textNode.setTransform(mat)

    def getScale(self):
        return self.__scale

    scale = property(getScale, setScale)

    def setWordwrap(self, wordwrap):
        self.__wordwrap = wordwrap

        if wordwrap:
            self.textNode.setWordwrap(wordwrap)
        else:
            self.textNode.clearWordwrap()

    def getWordwrap(self):
        return self.__wordwrap

    wordwrap = property(getWordwrap, setWordwrap)

    def __getFg(self):
        return self.textNode.getTextColor()

    def setFg(self, fg):
        self.textNode.setTextColor(fg[0], fg[1], fg[2], fg[3])

    fg = property(__getFg, setFg)

    def __getBg(self):
        if self.textNode.hasCard():
            return self.textNode.getCardColor()
        else:
            return LColor(0)

    def setBg(self, bg):
        if bg[3] != 0:
            # If we have a background color, create a card.
            self.textNode.setCardColor(bg[0], bg[1], bg[2], bg[3])
            self.textNode.setCardAsMargin(0.1, 0.1, 0.1, 0.1)
        else:
            # Otherwise, remove the card.
            self.textNode.clearCard()

    bg = property(__getBg, setBg)

    def __getShadow(self):
        return self.textNode.getShadowColor()

    def setShadow(self, shadow):
        if shadow[3] != 0:
            # If we have a shadow color, create a shadow.
            self.textNode.setShadowColor(shadow[0], shadow[1], shadow[2], shadow[3])
            self.textNode.setShadow(0.04, 0.04)
        else:
            # Otherwise, remove the shadow.
            self.textNode.clearShadow()

    shadow = property(__getShadow, setShadow)

    def __getFrame(self):
        return self.textNode.getFrameColor()

    def setFrame(self, frame):
        if frame[3] != 0:
            # If we have a frame color, create a frame.
            self.textNode.setFrameColor(frame[0], frame[1], frame[2], frame[3])
            self.textNode.setFrameAsMargin(0.1, 0.1, 0.1, 0.1)
        else:
            # Otherwise, remove the frame.
            self.textNode.clearFrame()

    frame = property(__getFrame, setFrame)

    def configure(self, option=None, **kw):
        # These is for compatibility with DirectGui functions
        if not self.mayChange:
            print('OnscreenText.configure: mayChange == 0')
            return
        for option, value in kw.items():
            # Use option string to access setter function
            try:
                setter = getattr(self, 'set' + option[0].upper() + option[1:])
                if setter == self.setPos:
                    setter(value[0], value[1])
                else:
                    setter(value)
            except AttributeError:
                print('OnscreenText.configure: invalid option: %s' % option)

    # Allow index style references
    def __setitem__(self, key, value):
        self.configure(*(), **{key: value})

    def cget(self, option):
        # Get current configuration setting.
        # This is for compatibility with DirectGui functions
        getter = getattr(self, 'get' + option[0].upper() + option[1:])
        return getter()

    def __getAlign(self):
        return self.textNode.getAlign()

    def setAlign(self, align):
        self.textNode.setAlign(align)

    align = property(__getAlign, setAlign)

    # Allow index style refererences
    __getitem__ = cget

