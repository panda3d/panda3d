from DirectFrame import *

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
            ('pgFunc',          PGEntry,   None),
            ('numStates',       3,         None),
            ('font',     getDefaultFont(), self.setFont),
            ('width',           10,        self.setup),
            ('numLines',        5,         self.setup),
            ('contents',        '',        self.setContents),
            # Sounds to be used for button events
            ('rolloverSound',   getDefaultRolloverSound(), self.setRolloverSound),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        # Call option initialization functions
        self.initialiseoptions(DirectEntry)

    def setup(self):
        self.node().setup(self['width'], self['numLines'])

    def setFont(self):
        self.guiItem.getTextNode().setFont(self['font'])

    def setContents(self):
        self.guiItem.setText(self['contents'])

    def setRolloverSound(self):
        if base.wantSfx:
            rolloverSound = self['rolloverSound']
            if rolloverSound:
                self.guiItem.setSound(ENTER + self.guiId, rolloverSound)
            else:
                self.guiItem.clearSound(ENTER + self.guiId)


