from direct.gui.DirectGui import *
from pandac.PandaModules import *


class DirectCheckBox(DirectButton):
    """
    DirectCheckBox(parent) - Create a DirectGuiWidget which responds
    to mouse clicks by setting a state of True or False and executes
    a callback function if defined.

    Uses an image swap rather than a text change to indicate state.
    """
    def __init__(self, parent = None, **kw):

        optiondefs = (
            # Define type of DirectGuiWidget
            ('pgFunc',         PGButton,   None),
            ('numStates',      4,          None),
            ('state',          DGG.NORMAL, None),
            ('relief',         DGG.RAISED, None),
            ('invertedFrames', (1,),       None),
            # Command to be called on button click
            ('command',        None,       None),
            ('extraArgs',      [],         None),
            # Which mouse buttons can be used to click the button
            ('commandButtons', (DGG.LMB,),     self.setCommandButtons),
            # Sounds to be used for button events
            ('rolloverSound', DGG.getDefaultRolloverSound(), self.setRolloverSound),
            ('clickSound',    DGG.getDefaultClickSound(),    self.setClickSound),
            # Can only be specified at time of widget contruction
            # Do the text/graphics appear to move when the button is clicked
            ('pressEffect',     1,         DGG.INITOPT),
            ('uncheckedImage',  None,      None),
            ('checkedImage',    None,      None),
            ('isChecked',       False,     None),
            )
        
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        DirectButton.__init__(self,parent)
        
        self.initialiseoptions(DirectCheckBox)


    def commandFunc(self, event):
        self['isChecked'] = not self['isChecked']

        if self['isChecked']:
            self['image'] = self['checkedImage']
        else:
            self['image'] = self['uncheckedImage']

        self.setImage()
        
        if self['command']:
            # Pass any extra args to command
            apply(self['command'], [self['isChecked']] + self['extraArgs'])

