from DirectGuiGlobals import *
from DirectFrame import *
from DirectButton import *

class DirectDialog(DirectFrame):
    def __init__(self, parent = guiTop, **kw):
        # Inherits from DirectFrame
        optiondefs = (
            # Define type of DirectGuiWidget
            ('pad',             (0.4, 0.4),    None),
            ('text',            '',            None),
            ('text_align',      TMALIGNCENTER, None),
            ('geom',  getDefaultDialogGeom(),  None),
            ('relief',          None,          None),
            ('buttons',         [],            INITOPT),
            ('buttonOffset',    0.2,           None),
            ('button_pad',      (.1,.1),       None),
            ('button_relief',   RAISED,        None),
            ('button_frameSize', (-1,1,-.2,.9),    None),
            ('command',         None,          None),
            ('extraArgs',       [],            None),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs, dynamicGroups = ("button",))

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        self.numButtons = len(self['buttons'])
        self.buttonList = []
        for button in self['buttons']:
            name = button + 'Button'
            button = self.createcomponent(
                name, (), "button",
                DirectButton, (self,),
                text = button,
                command = lambda s = self, b = button: s.__buttonCommand(b)
                )
            self.buttonList.append(button)
            self.numButtons += 1
        self.initialiseoptions(DirectDialog)

        # Position buttons and text
        # Get size of text
        text = self.component('text0')
        bounds = text.getTightBounds()
        pad = self['pad']
        l = bounds[0][0] - pad[0]
        r = bounds[1][0] + pad[0]
        b = bounds[0][2] - pad[1]
        t = bounds[1][2] + pad[1]
        text.setPos((l+r)/2.0, (b+t)/2.0)
        # Move buttons
        if self['button_frameSize']:
            frameSize = self['button_frameSize']
            bl = frameSize[0]
            br = frameSize[1]
            bb = frameSize[2]
            bt = frameSize[3]
        else:
            # Get bounds of union of buttons
            bl = br = bb = bt = 0
            for button in self.buttonList:
                bounds = button.stateNodePath[0].getTightBounds()
                bl = min(bl, bounds[0][0])
                br = max(br, bounds[1][0])
                bb = min(bb, bounds[0][2])
                bt = max(bt, bounds[1][2])
        bWidth = br - bl
        bSpacing = 1.1 * bWidth
        bHeight = bt - bb
        index = 0
        if self.numButtons == 0:
            return
        bPos = bSpacing * (self.numButtons - 1)/2.0
        for button in self.buttonList:
            button.setPos(bPos + index * bSpacing, 0,
                          b - self['buttonOffset'] - bt)
            index += 1
        # Resize frame
        b = b - self['buttonOffset'] - bHeight - pad[1]
        frame = self.component('geom0')
        frame.setScale(r - l, 1, t - b)
        frame.setPos((l+r)/2.0, 0.0, (b+t)/2.0)
        self.resetFrameSize()

    def __buttonCommand(self, button):
        if self['command']:
            self['command'](button)
        
    def destroy(self):
        DirectFrame.destroy(self)


class YesNoDialog(DirectDialog):
    def __init__(self, parent = guiTop, **kw):
        # Inherits from DirectFrame
        optiondefs = (
            # Define type of DirectGuiWidget
            ('buttons',       ['Yes', 'No'],       INITOPT),
            ('text',          'Yes or No?',        None),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)
        apply(DirectDialog.__init__, (self, parent), kw)
        self.initialiseoptions(YesNoDialog)
