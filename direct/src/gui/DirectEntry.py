"""Contains the DirectEntry class, a type of DirectGUI widget that accepts
text entered using the keyboard."""

__all__ = ['DirectEntry']

from panda3d.core import *
from direct.showbase import ShowBaseGlobal
from . import DirectGuiGlobals as DGG
from .DirectFrame import *
from .OnscreenText import OnscreenText
import sys
# import this to make sure it gets pulled into the publish
import encodings.utf_8
from direct.showbase.DirectObject import DirectObject

# DirectEntry States:
ENTRY_FOCUS_STATE    = PGEntry.SFocus      # 0
ENTRY_NO_FOCUS_STATE = PGEntry.SNoFocus    # 1
ENTRY_INACTIVE_STATE = PGEntry.SInactive   # 2

class DirectEntry(DirectFrame):
    """
    DirectEntry(parent) - Create a DirectGuiWidget which responds
    to keyboard buttons
    """

    directWtext = ConfigVariableBool('direct-wtext', 1)

    AllowCapNamePrefixes = ("Al", "Ap", "Ben", "De", "Del", "Della", "Delle", "Der", "Di", "Du",
                            "El", "Fitz", "La", "Las", "Le", "Les", "Lo", "Los",
                            "Mac", "St", "Te", "Ten", "Van", "Von", )
    ForceCapNamePrefixes = ("D'", "DeLa", "Dell'", "L'", "M'", "Mc", "O'", )

    def __init__(self, parent = None, **kw):
        # Inherits from DirectFrame
        # A Direct Frame can have:
        # - A background texture (pass in path to image, or Texture Card)
        # - A midground geometry item (pass in geometry)
        # - A foreground text Node (pass in text string or Onscreen Text)
        # For a direct entry:
        # Each button has 3 states (focus, noFocus, disabled)
        # The same image/geom/text can be used for all three states or each
        # state can have a different text/geom/image
        # State transitions happen automatically based upon mouse interaction
        optiondefs = (
            # Define type of DirectGuiWidget
            ('pgFunc',          PGEntry,          None),
            ('numStates',       3,                None),
            ('state',           DGG.NORMAL,       None),
            ('entryFont',       None,             DGG.INITOPT),
            ('width',           10,               self.updateWidth),
            ('numLines',        1,                self.updateNumLines),
            ('focus',           0,                self.setFocus),
            ('cursorKeys',      1,                self.setCursorKeysActive),
            ('obscured',        0,                self.setObscureMode),
            # Setting backgroundFocus allows the entry box to get keyboard
            # events that are not handled by other things (i.e. events that
            # fall through to the background):
            ('backgroundFocus', 0,                self.setBackgroundFocus),
            # Text used for the PGEntry text node
            # NOTE: This overrides the DirectFrame text option
            ('initialText',     '',               DGG.INITOPT),
            # Enable or disable text overflow scrolling
            ('overflow',        0,                self.setOverflowMode),
            # Command to be called on hitting Enter
            ('command',        None,              None),
            ('extraArgs',      [],                None),
            # Command to be called when enter is hit but we fail to submit
            ('failedCommand',  None,              None),
            ('failedExtraArgs',[],                None),
            # commands to be called when focus is gained or lost
            ('focusInCommand', None,              None),
            ('focusInExtraArgs', [],              None),
            ('focusOutCommand', None,             None),
            ('focusOutExtraArgs', [],             None),
            # Sounds to be used for button events
            ('rolloverSound',   DGG.getDefaultRolloverSound(), self.setRolloverSound),
            ('clickSound',      DGG.getDefaultClickSound(),    self.setClickSound),
            ('autoCapitalize',  0,                self.autoCapitalizeFunc),
            ('autoCapitalizeAllowPrefixes', DirectEntry.AllowCapNamePrefixes, None),
            ('autoCapitalizeForcePrefixes', DirectEntry.ForceCapNamePrefixes, None),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        if self['entryFont'] == None:
            font = DGG.getDefaultFont()
        else:
            font = self['entryFont']

        # Create Text Node Component
        self.onscreenText = self.createcomponent(
            'text', (), None,
            OnscreenText,
            (), parent = ShowBaseGlobal.hidden,
            # Pass in empty text to avoid extra work, since its really
            # The PGEntry which will use the TextNode to generate geometry
            text = '',
            align = TextNode.ALeft,
            font = font,
            scale = 1,
            # Don't get rid of the text node
            mayChange = 1)

        # We can get rid of the node path since we're just using the
        # onscreenText as an easy way to access a text node as a
        # component
        self.onscreenText.removeNode()

        # Bind command function
        self.bind(DGG.ACCEPT, self.commandFunc)
        self.bind(DGG.ACCEPTFAILED, self.failedCommandFunc)

        self.accept(self.guiItem.getFocusInEvent(), self.focusInCommandFunc)
        self.accept(self.guiItem.getFocusOutEvent(), self.focusOutCommandFunc)

        # listen for auto-capitalize events on a separate object to prevent
        # clashing with other parts of the system
        self._autoCapListener = DirectObject()

        # Call option initialization functions
        self.initialiseoptions(DirectEntry)

        if not hasattr(self, 'autoCapitalizeAllowPrefixes'):
            self.autoCapitalizeAllowPrefixes = DirectEntry.AllowCapNamePrefixes
        if not hasattr(self, 'autoCapitalizeForcePrefixes'):
            self.autoCapitalizeForcePrefixes = DirectEntry.ForceCapNamePrefixes

        # Update TextNodes for each state
        for i in range(self['numStates']):
            self.guiItem.setTextDef(i, self.onscreenText.textNode)

        # Now we should call setup() again to make sure it has the
        # right font def.
        self.setup()

        # Update initial text
        self.unicodeText = 0
        if self['initialText']:
            self.enterText(self['initialText'])

    def destroy(self):
        self.ignoreAll()
        self._autoCapListener.ignoreAll()
        DirectFrame.destroy(self)

    def setup(self):
        self.guiItem.setupMinimal(self['width'], self['numLines'])

    def updateWidth(self):
        self.guiItem.setMaxWidth(self['width'])

    def updateNumLines(self):
        self.guiItem.setNumLines(self['numLines'])

    def setFocus(self):
        PGEntry.setFocus(self.guiItem, self['focus'])

    def setCursorKeysActive(self):
        PGEntry.setCursorKeysActive(self.guiItem, self['cursorKeys'])

    def setOverflowMode(self):
        PGEntry.set_overflow_mode(self.guiItem, self['overflow'])

    def setObscureMode(self):
        PGEntry.setObscureMode(self.guiItem, self['obscured'])

    def setBackgroundFocus(self):
        PGEntry.setBackgroundFocus(self.guiItem, self['backgroundFocus'])

    def setRolloverSound(self):
        rolloverSound = self['rolloverSound']
        if rolloverSound:
            self.guiItem.setSound(DGG.ENTER + self.guiId, rolloverSound)
        else:
            self.guiItem.clearSound(DGG.ENTER + self.guiId)

    def setClickSound(self):
        clickSound = self['clickSound']
        if clickSound:
            self.guiItem.setSound(DGG.ACCEPT + self.guiId, clickSound)
        else:
            self.guiItem.clearSound(DGG.ACCEPT + self.guiId)

    def commandFunc(self, event):
        if self['command']:
            # Pass any extra args to command
            self['command'](*[self.get()] + self['extraArgs'])

    def failedCommandFunc(self, event):
        if self['failedCommand']:
            # Pass any extra args
            self['failedCommand'](*[self.get()] + self['failedExtraArgs'])

    def autoCapitalizeFunc(self):
        if self['autoCapitalize']:
            self._autoCapListener.accept(self.guiItem.getTypeEvent(), self._handleTyping)
            self._autoCapListener.accept(self.guiItem.getEraseEvent(), self._handleErasing)
        else:
            self._autoCapListener.ignore(self.guiItem.getTypeEvent())
            self._autoCapListener.ignore(self.guiItem.getEraseEvent())

    def focusInCommandFunc(self):
        if self['focusInCommand']:
            self['focusInCommand'](*self['focusInExtraArgs'])
        if self['autoCapitalize']:
            self.accept(self.guiItem.getTypeEvent(), self._handleTyping)
            self.accept(self.guiItem.getEraseEvent(), self._handleErasing)

    def _handleTyping(self, guiEvent):
        self._autoCapitalize()
    def _handleErasing(self, guiEvent):
        self._autoCapitalize()

    def _autoCapitalize(self):
        name = self.guiItem.getWtext()
        # capitalize each word, allowing for things like McMutton
        capName = u''
        # track each individual word to detect prefixes like Mc
        wordSoFar = u''
        # track whether the previous character was part of a word or not
        wasNonWordChar = True
        for i, character in enumerate(name):
            # test to see if we are between words
            # - Count characters that can't be capitalized as a break between words
            #   This assumes that string.lower and string.upper will return different
            #   values for all unicode letters.
            # - Don't count apostrophes as a break between words
            if character.lower() == character.upper() and character != u"'":
                # we are between words
                wordSoFar = u''
                wasNonWordChar = True
            else:
                capitalize = False
                if wasNonWordChar:
                    # first letter of a word, capitalize it unconditionally;
                    capitalize = True
                elif (character == character.upper() and
                      len(self.autoCapitalizeAllowPrefixes) and
                      wordSoFar in self.autoCapitalizeAllowPrefixes):
                    # first letter after one of the prefixes, allow it to be capitalized
                    capitalize = True
                elif (len(self.autoCapitalizeForcePrefixes) and
                      wordSoFar in self.autoCapitalizeForcePrefixes):
                    # first letter after one of the force prefixes, force it to be capitalized
                    capitalize = True
                if capitalize:
                    # allow this letter to remain capitalized
                    character = character.upper()
                else:
                    character = character.lower()
                wordSoFar += character
                wasNonWordChar = False
            capName += character
        self.guiItem.setWtext(capName)
        self.guiItem.setCursorPosition(self.guiItem.getNumCharacters())

    def focusOutCommandFunc(self):
        if self['focusOutCommand']:
            self['focusOutCommand'](*self['focusOutExtraArgs'])
        if self['autoCapitalize']:
            self.ignore(self.guiItem.getTypeEvent())
            self.ignore(self.guiItem.getEraseEvent())

    def set(self, text):
        """ Changes the text currently showing in the typable region;
        does not change the current cursor position.  Also see
        enterText(). """

        if sys.version_info >= (3, 0):
            assert not isinstance(text, bytes)
            self.unicodeText = True
            self.guiItem.setWtext(text)
        else:
            self.unicodeText = isinstance(text, unicode)
            if self.unicodeText:
                self.guiItem.setWtext(text)
            else:
                self.guiItem.setText(text)

    def get(self, plain = False):
        """ Returns the text currently showing in the typable region.
        If plain is True, the returned text will not include any
        formatting characters like nested color-change codes. """

        wantWide = self.unicodeText or self.guiItem.isWtext()
        if not self.directWtext.getValue():
            # If the user has configured wide-text off, then always
            # return an 8-bit string.  This will be encoded if
            # necessary, according to Panda's default encoding.
            wantWide = False

        if plain:
            if wantWide:
                return self.guiItem.getPlainWtext()
            else:
                return self.guiItem.getPlainText()
        else:
            if wantWide:
                return self.guiItem.getWtext()
            else:
                return self.guiItem.getText()

    def getCursorPosition(self):
        return self.guiItem.getCursorPosition()

    def setCursorPosition(self, pos):
        if (pos < 0):
            self.guiItem.setCursorPosition(self.guiItem.getNumCharacters() + pos)
        else:
            self.guiItem.setCursorPosition(pos)

    def getNumCharacters(self):
        return self.guiItem.getNumCharacters()

    def enterText(self, text):
        """ sets the entry's text, and moves the cursor to the end """
        self.set(text)
        self.setCursorPosition(self.guiItem.getNumCharacters())

    def getFont(self):
        return self.onscreenText.getFont()

    def getBounds(self, state = 0):
        # Compute the width and height for the entry itself, ignoring
        # geometry etc.
        tn = self.onscreenText.textNode
        mat = tn.getTransform()
        align = tn.getAlign()
        lineHeight = tn.getLineHeight()
        numLines = self['numLines']
        width = self['width']

        if align == TextNode.ALeft:
            left = 0.0
            right = width
        elif align == TextNode.ACenter:
            left = -width / 2.0
            right = width / 2.0
        elif align == TextNode.ARight:
            left = -width
            right = 0.0

        bottom = -0.3 * lineHeight - (lineHeight * (numLines - 1))
        top = lineHeight

        self.ll.set(left, 0.0, bottom)
        self.ur.set(right, 0.0, top)
        self.ll = mat.xformPoint(Point3.rfu(left, 0.0, bottom))
        self.ur = mat.xformPoint(Point3.rfu(right, 0.0, top))

        vec_right = Vec3.right()
        vec_up = Vec3.up()
        left = (vec_right[0] * self.ll[0]
              + vec_right[1] * self.ll[1]
              + vec_right[2] * self.ll[2])
        right = (vec_right[0] * self.ur[0]
               + vec_right[1] * self.ur[1]
               + vec_right[2] * self.ur[2])
        bottom = (vec_up[0] * self.ll[0]
                + vec_up[1] * self.ll[1]
                + vec_up[2] * self.ll[2])
        top = (vec_up[0] * self.ur[0]
             + vec_up[1] * self.ur[1]
             + vec_up[2] * self.ur[2])
        self.ll = Point3(left, 0.0, bottom)
        self.ur = Point3(right, 0.0, top)

        # Scale bounds to give a pad around graphics.  We also want to
        # scale around the border width.
        pad = self['pad']
        borderWidth = self['borderWidth']
        self.bounds = [self.ll[0] - pad[0] - borderWidth[0],
                       self.ur[0] + pad[0] + borderWidth[0],
                       self.ll[2] - pad[1] - borderWidth[1],
                       self.ur[2] + pad[1] + borderWidth[1]]
        return self.bounds
