from DirectFrame import *

# DirectEntry States:
ENTRY_FOCUS_STATE    = PGEntry.SFocus      # 0
ENTRY_NO_FOCUS_STATE = PGEntry.SNoFocus    # 1
ENTRY_INACTIVE_STATE = PGEntry.SInactive   # 2

class DirectEntry(DirectFrame):
    """
    DirectEntry(parent) - Create a DirectGuiWidget which responds
    to keyboard buttons
    """
    def __init__(self, parent = guiTop, **kw):
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
            ('state',           NORMAL,           None),
            ('width',           10,               self.setup),
            ('numLines',        5,                self.setup),
            ('focus',           0,                self.setFocus),
            ('cursorKeys',      0,                self.setCursorKeysActive),
            ('backgroundFocus', 0,                self.setBackgroundFocus),
            # Text used for the PGEntry text node
            # NOTE: This overrides the DirectFrame text option
            ('initialText',     '',               INITOPT),
            # Command to be called on hitting Enter
            ('command',        None,              None),
            ('extraArgs',      [],                None),
            # Sounds to be used for button events
            ('rolloverSound',   getDefaultRolloverSound(), self.setRolloverSound),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        # Create Text Node Component
        self.onscreenText = self.createcomponent(
            'text', (), None,
            OnscreenText.OnscreenText,
            (), parent = hidden,
            # Pass in empty text to avoid extra work, since its really
            # The PGEntry which will use the TextNode to generate geometry
            text = '',
            # PGEntry assumes left alignment
            align = TMALIGNLEFT,
            font = getDefaultFont(),
            scale = 1,
            # Don't get rid of the text node
            mayChange = 1)
        # We want to keep this sucker frozen since its not
        # in the scene graph
        self.onscreenText.freeze()
        # We can also get rid of the node path since we're just using the
        # onscreenText as an easy way to access a text node as a component
        self.onscreenText.removeNode()

        # Bind command function
        self.bind(ACCEPT, self.commandFunc)

        # Call option initialization functions
        self.initialiseoptions(DirectEntry)

        # Update TextNodes for each state
        for i in range(self['numStates']):
            self.guiItem.setTextDef(i, self.onscreenText.textNode)

        # Update initial text
        if self['initialText']:
            self.set(self['initialText'])

    def setup(self):
        self.node().setup(self['width'], self['numLines'])

    def setFocus(self):
        PGEntry.setFocus(self.guiItem, self['focus'])

    def setCursorKeysActive(self):
        PGEntry.setCursorKeysActive(self.guiItem, self['cursorKeys'])

    def setBackgroundFocus(self):
        PGEntry.setBackgroundFocus(self.guiItem, self['backgroundFocus'])

    def setRolloverSound(self):
        if base.wantSfx:
            rolloverSound = self['rolloverSound']
            if rolloverSound:
                self.guiItem.setSound(ENTER + self.guiId, rolloverSound)
            else:
                self.guiItem.clearSound(ENTER + self.guiId)

    def commandFunc(self, event):
        if self['command']:
            # Pass any extra args to command
            apply(self['command'], [self.get()] + self['extraArgs'])
            
    def set(self, text):
        self.guiItem.setText(text)

    def get(self):
        return self.guiItem.getText()

    def setCursorPosition(self, pos):
        if (pos < 0):
            self.guiItem.setCursorPosition(len(self.get()) + pos)
        else:
            self.guiItem.setCursorPosition(pos)
            


