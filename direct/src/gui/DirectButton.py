from DirectFrame import *

# DirectButton States:
BUTTON_READY_STATE     = PGButton.SReady       # 0
BUTTON_DEPRESSED_STATE = PGButton.SDepressed   # 1
BUTTON_ROLLOVER_STATE  = PGButton.SRollover    # 2
BUTTON_INACTIVE_STATE  = PGButton.SInactive    # 3

class DirectButton(DirectFrame):
    """
    DirectButton(parent) - Create a DirectGuiWidget which responds
    to mouse clicks and execute a callback function if defined
    """
    def __init__(self, parent = None, **kw):
        # Inherits from DirectFrame
        # A Direct Frame can have:
        # - A background texture (pass in path to image, or Texture Card)
        # - A midground geometry item (pass in geometry)
        # - A foreground text Node (pass in text string or Onscreen Text)
        # For a direct button:
        # Each button has 4 states (ready, press, rollover, disabled)
        # The same image/geom/text can be used for all four states or each
        # state can have a different text/geom/image
        # State transitions happen automatically based upon mouse interaction
        # Responds to click event and calls command if None
        optiondefs = (
            # Define type of DirectGuiWidget
            ('pgFunc',         PGButton,   None),
            ('numStates',      4,          None),
            ('state',          NORMAL,     None),
            ('invertedFrames', (1,),       None),
            # Command to be called on button click
            ('command',        None,       None),
            ('extraArgs',      [],         None),
            # Which mouse buttons can be used to click the button
            ('commandButtons', (LMB,),     self.setCommandButtons),
            # Sounds to be used for button events
            ('rolloverSound', getDefaultRolloverSound(), self.setRolloverSound),
            ('clickSound',    getDefaultClickSound(),    self.setClickSound),
            # Can only be specified at time of widget contruction
            # Do the text/graphics appear to move when the button is clicked
            ('pressEffect',     1,          INITOPT),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        # Initialize superclasses
        DirectFrame.__init__(self, parent)
        
        # If specifed, add scaling to the pressed state to make it look
        # like the button is moving when you press it
        if self['pressEffect']:
            np = self.stateNodePath[1].attachNewNode('pressEffect', 1)
            np.setScale(0.98)
            self.stateNodePath[1] = np
            
        # Call option initialization functions
        self.initialiseoptions(DirectButton)

    def setCommandButtons(self):
        # Attach command function to specified buttons
        # Left mouse button
        if LMB in self['commandButtons']:
            self.guiItem.addClickButton(MouseButton.one())
            self.bind(B1CLICK, self.commandFunc)
        else:
            self.unbind(B1CLICK)
            self.guiItem.removeClickButton(MouseButton.one())
        # Middle mouse button
        if MMB in self['commandButtons']:
            self.guiItem.addClickButton(MouseButton.two())
            self.bind(B2CLICK, self.commandFunc)
        else:
            self.unbind(B2CLICK)
            self.guiItem.removeClickButton(MouseButton.two())
        # Right mouse button
        if RMB in self['commandButtons']:
            self.guiItem.addClickButton(MouseButton.three())
            self.bind(B3CLICK, self.commandFunc)
        else:
            self.unbind(B3CLICK)
            self.guiItem.removeClickButton(MouseButton.three())

    def commandFunc(self, event):
        if self['command']:
            # Pass any extra args to command
            apply(self['command'], self['extraArgs'])
            
    def setClickSound(self):
        clickSound = self['clickSound']
        # Clear out sounds
        self.guiItem.clearSound(B1PRESS + self.guiId)
        self.guiItem.clearSound(B2PRESS + self.guiId)
        self.guiItem.clearSound(B3PRESS + self.guiId)
        if clickSound:
            if LMB in self['commandButtons']:
                self.guiItem.setSound(B1PRESS + self.guiId, clickSound)
            if MMB in self['commandButtons']:
                self.guiItem.setSound(B2PRESS + self.guiId, clickSound)
            if RMB in self['commandButtons']:
                self.guiItem.setSound(B3PRESS + self.guiId, clickSound)

    def setRolloverSound(self):
        rolloverSound = self['rolloverSound']
        if rolloverSound:
            self.guiItem.setSound(ENTER + self.guiId, rolloverSound)
        else:
            self.guiItem.clearSound(ENTER + self.guiId)


