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
            ('font',            getDefaultFont(), self.setFont),
            ('width',           10,               self.setup),
            ('numLines',        5,                self.setup),
            ('focus',           0,                self.setFocus),
            ('initialText',     '',               INITOPT),
            # Command to be called on hitting Enter
            ('command',        None,       None),
            ('extraArgs',      [],         None),
            # Sounds to be used for button events
            ('rolloverSound',   getDefaultRolloverSound(), self.setRolloverSound),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        # Bind command function
        self.bind(ACCEPT, self.commandFunc)

        # Call option initialization functions
        self.initialiseoptions(DirectEntry)

        # Update entry if init text specified
        if self['initialText']:
            self.guiItem.setText(self['initialText'])

    def setup(self):
        self.node().setup(self['width'], self['numLines'])

    def setFont(self):
        self.guiItem.getTextNode().setFont(self['font'])

    def setFocus(self):
        PGEntry.setFocus(self.guiItem, self['focus'])

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


