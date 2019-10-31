"""This module defines various dialog windows for the DirectGUI system."""

__all__ = [
    'findDialog', 'cleanupDialog', 'DirectDialog', 'OkDialog',
    'OkCancelDialog', 'YesNoDialog', 'YesNoCancelDialog', 'RetryCancelDialog',
]

from panda3d.core import *
from direct.showbase import ShowBaseGlobal
from . import DirectGuiGlobals as DGG
from .DirectFrame import *
from .DirectButton import *
import types


def findDialog(uniqueName):
    """
    Returns the panel whose uniqueName is given.  This is mainly
    useful for debugging, to get a pointer to the current onscreen
    panel of a particular type.
    """
    if uniqueName in DirectDialog.AllDialogs:
        return DirectDialog.AllDialogs[uniqueName]
    return None


def cleanupDialog(uniqueName):
    """cleanupPanel(string uniqueName)

    Cleans up (removes) the panel with the given uniqueName.  This
    may be useful when some panels know about each other and know
    that opening panel A should automatically close panel B, for
    instance.
    """
    if uniqueName in DirectDialog.AllDialogs:
        # calling cleanup() will remove it out of the AllDialogs dict
        # This way it will get removed from the dict even it we did
        # not clean it up using this interface (ie somebody called
        # self.cleanup() directly
        DirectDialog.AllDialogs[uniqueName].cleanup()


class DirectDialog(DirectFrame):

    AllDialogs = {}
    PanelIndex = 0

    def __init__(self, parent=None, **kw):
        """Creates a popup dialog to alert and/or interact with user.
        Some of the main keywords that can be used to customize the dialog:

        Parameters:
            text (str): Text message/query displayed to user
            geom: Geometry to be displayed in dialog
            buttonTextList: List of text to show on each button
            buttonGeomList: List of geometry to show on each button
            buttonImageList: List of images to show on each button
            buttonValueList: List of values sent to dialog command for
                each button.  If value is [] then the ordinal rank of
                the button is used as its value.
            buttonHotKeyList: List of hotkeys to bind to each button.
                Typing the hotkey is equivalent to pressing the
                corresponding button.
            suppressKeys: Set to true if you wish to suppress keys
                (i.e. Dialog eats key event), false if you wish Dialog
                to pass along key event.
            buttonSize: 4-tuple used to specify custom size for each
                button (to make bigger then geom/text for example)
            pad: Space between border and interior graphics
            topPad: Extra space added above text/geom/image
            midPad: Extra space added between text/buttons
            sidePad: Extra space added to either side of text/buttons
            buttonPadSF: Scale factor used to expand/contract button
                horizontal spacing
            command: Callback command used when a button is pressed.
                Value supplied to command depends on values in
                buttonValueList.

        Note:
            The number of buttons on the dialog depends on the maximum
            length of any button[Text|Geom|Image|Value]List specified.
            Values of None are substituted for lists that are shorter
            than the max length
         """

        # Inherits from DirectFrame
        optiondefs = (
            # Define type of DirectGuiWidget
            ('dialogName',        'DirectDialog_' + repr(DirectDialog.PanelIndex),  DGG.INITOPT),
            # Default position is slightly forward in Y, so as not to
            # intersect the near plane, which is incorrectly set to 0
            # in DX for some reason.
            ('pos',               (0, 0.1, 0),   None),
            ('pad',               (0.1, 0.1),    None),
            ('text',              '',            None),
            ('text_align',        TextNode.ALeft,   None),
            ('text_scale',        0.06,          None),
            ('image',             DGG.getDefaultDialogGeom(), None),
            ('relief',            DGG.getDefaultDialogRelief(), None),
            ('borderWidth',       (0.01, 0.01),  None),
            ('buttonTextList',    [],            DGG.INITOPT),
            ('buttonGeomList',    [],            DGG.INITOPT),
            ('buttonImageList',   [],            DGG.INITOPT),
            ('buttonValueList',   [],            DGG.INITOPT),
            ('buttonHotKeyList',  [],            DGG.INITOPT),
            ('button_borderWidth', (.01, .01),   None),
            ('button_pad',        (.01, .01),    None),
            ('button_relief',     DGG.RAISED,    None),
            ('button_text_scale', 0.06,          None),
            ('buttonSize',        None,          DGG.INITOPT),
            ('topPad',            0.06,          DGG.INITOPT),
            ('midPad',            0.12,          DGG.INITOPT),
            ('sidePad',           0.,            DGG.INITOPT),
            ('buttonPadSF',       1.1,           DGG.INITOPT),
            # Alpha of fade screen behind dialog
            ('fadeScreen',        0,             None),
            ('command',           None,          None),
            ('extraArgs',         [],            None),
            ('sortOrder',    DGG.NO_FADE_SORT_INDEX, None),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs, dynamicGroups = ("button",))

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        #if not self['dialogName']:
        #    self['dialogName'] = 'DirectDialog_' + repr(DirectDialog.PanelIndex)

        # Clean up any previously existing panel with the same unique
        # name.  We don't allow any two panels with the same name to
        # coexist.
        cleanupDialog(self['dialogName'])
        # Store this panel in our map of all open panels.
        DirectDialog.AllDialogs[self['dialogName']] = self
        DirectDialog.PanelIndex += 1

        # Determine number of buttons
        self.numButtons = max(len(self['buttonTextList']),
                              len(self['buttonGeomList']),
                              len(self['buttonImageList']),
                              len(self['buttonValueList']))
        # Create buttons
        self.buttonList = []
        index = 0
        for i in range(self.numButtons):
            name = 'Button' + repr(i)
            try:
                text = self['buttonTextList'][i]
            except IndexError:
                text = None
            try:
                geom = self['buttonGeomList'][i]
            except IndexError:
                geom = None
            try:
                image = self['buttonImageList'][i]
            except IndexError:
                image = None
            try:
                value = self['buttonValueList'][i]
            except IndexError:
                value = i
                self['buttonValueList'].append(i)
            try:
                hotKey = self['buttonHotKeyList'][i]
            except IndexError:
                hotKey = None
            button = self.createcomponent(
                name, (), "button",
                DirectButton, (self,),
                text = text,
                geom = geom,
                image = image,
                suppressKeys = self['suppressKeys'],
                frameSize = self['buttonSize'],
                command = lambda s = self, v = value: s.buttonCommand(v)
                )
            self.buttonList.append(button)

        # Update dialog when everything has been initialised
        self.postInitialiseFuncList.append(self.configureDialog)
        self.initialiseoptions(DirectDialog)

    def configureDialog(self):
        # Set up hot key bindings
        bindList = zip(self.buttonList, self['buttonHotKeyList'],
                       self['buttonValueList'])
        for button, hotKey, value in bindList:
            if ((type(hotKey) == list) or
                (type(hotKey) == tuple)):
                for key in hotKey:
                    button.bind('press-' + key + '-', self.buttonCommand,
                                extraArgs = [value])
                    self.bind('press-' + key + '-', self.buttonCommand,
                              extraArgs = [value])

            else:
                button.bind('press-' + hotKey + '-', self.buttonCommand,
                            extraArgs = [value])
                self.bind('press-' + hotKey + '-', self.buttonCommand,
                          extraArgs = [value])
        # Position buttons and text
        pad = self['pad']
        if self.hascomponent('image0'):
            image = self.component('image0')
        else:
            image = None
        # Get size of text/geom without image (for state 0)
        if image:
            image.reparentTo(ShowBaseGlobal.hidden)
        bounds = self.stateNodePath[0].getTightBounds()
        if image:
            image.reparentTo(self.stateNodePath[0])
        if bounds is None:
            l = 0
            r = 0
            b = 0
            t = 0
        else:
            l = bounds[0][0]
            r = bounds[1][0]
            b = bounds[0][2]
            t = bounds[1][2]
        # Center text and geom around origin
        # How far is center of text from origin?
        xOffset = -(l+r)*0.5
        zOffset = -(b+t)*0.5
        # Update bounds to reflect text movement
        l += xOffset
        r += xOffset
        b += zOffset
        t += zOffset
        # Offset text and geom to center
        if self['text']:
            self['text_pos'] = (self['text_pos'][0] + xOffset,
                                self['text_pos'][1] + zOffset)
        if self['geom']:
            self['geom_pos'] = Point3(self['geom_pos'][0] + xOffset,
                                      self['geom_pos'][1],
                                      self['geom_pos'][2] + zOffset)
        if self.numButtons != 0:
            bpad = self['button_pad']
            # Get button size
            if self['buttonSize']:
                # Either use given size
                buttonSize = self['buttonSize']
                bl = buttonSize[0]
                br = buttonSize[1]
                bb = buttonSize[2]
                bt = buttonSize[3]
            else:
                # Or get bounds of union of buttons
                bl = br = bb = bt = 0
                for button in self.buttonList:
                    bounds = button.stateNodePath[0].getTightBounds()
                    if bounds is None:
                        bl = 0
                        br = 0
                        bb = 0
                        bt = 0
                    else:
                        bl = min(bl, bounds[0][0])
                        br = max(br, bounds[1][0])
                        bb = min(bb, bounds[0][2])
                        bt = max(bt, bounds[1][2])
                bl -= bpad[0]
                br += bpad[0]
                bb -= bpad[1]
                bt += bpad[1]
                # Now resize buttons to match largest
                for button in self.buttonList:
                    button['frameSize'] = (bl, br, bb, bt)
            # Must compensate for scale
            scale = self['button_scale']
            # Can either be a Vec3 or a tuple of 3 values
            if (isinstance(scale, Vec3) or
                (type(scale) == list) or
                (type(scale) == tuple)):
                sx = scale[0]
                sz = scale[2]
            elif ((type(scale) == int) or
                  (type(scale) == float)):
                sx = sz = scale
            else:
                sx = sz = 1
            bl *= sx
            br *= sx
            bb *= sz
            bt *= sz
            # Position buttons
            # Calc button width and height
            bHeight = bt - bb
            bWidth = br - bl
            # Add pad between buttons
            bSpacing = self['buttonPadSF'] * bWidth
            bPos = -bSpacing * (self.numButtons - 1)*0.5
            index = 0
            for button in self.buttonList:
                button.setPos(bPos + index * bSpacing, 0,
                              b - self['midPad'] - bpad[1] - bt)
                index += 1
            bMax = bPos + bSpacing * (self.numButtons - 1)
        else:
            bpad = 0
            bl = br = bb = bt = 0
            bPos = 0
            bMax = 0
            bpad = (0, 0)
            bHeight = bWidth = 0
        # Resize frame to fit text and buttons
        l = min(bPos + bl, l) - pad[0]
        r = max(bMax + br, r) + pad[0]
        sidePad = self['sidePad']
        l -= sidePad
        r += sidePad
        # reduce bottom by pad, button height and 2*button pad
        b = min(b - self['midPad'] - bpad[1] - bHeight - bpad[1], b) - pad[1]
        t = t + self['topPad'] + pad[1]
        if self['frameSize'] is None:
            self['frameSize'] = (l, r, b, t)
        self['image_scale'] = (r - l, 1, t - b)
        # Center frame about text and buttons
        self['image_pos'] = ((l+r)*0.5, 0.0, (b+t)*0.5)
        self.resetFrameSize()

    def show(self):
        if self['fadeScreen']:
            base.transitions.fadeScreen(self['fadeScreen'])
            self.setBin('gui-popup', 0)
        NodePath.show(self)

    def hide(self):
        if self['fadeScreen']:
            base.transitions.noTransitions()
        NodePath.hide(self)

    def buttonCommand(self, value, event = None):
        if self['command']:
            self['command'](value, *self['extraArgs'])

    def setMessage(self, message):
        self['text'] = message
        self.configureDialog()

    def cleanup(self):
        # Remove this panel out of the AllDialogs list
        uniqueName = self['dialogName']
        if uniqueName in DirectDialog.AllDialogs:
            del DirectDialog.AllDialogs[uniqueName]
        self.destroy()

    def destroy(self):
        if self['fadeScreen']:
            base.transitions.noTransitions()
        for button in self.buttonList:
            button.destroy()
        DirectFrame.destroy(self)

class OkDialog(DirectDialog):
    def __init__(self, parent = None, **kw):
        # Inherits from DirectFrame
        optiondefs = (
            # Define type of DirectGuiWidget
            ('buttonTextList',  ['OK'],       DGG.INITOPT),
            ('buttonValueList', [DGG.DIALOG_OK],          DGG.INITOPT),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)
        DirectDialog.__init__(self, parent)
        self.initialiseoptions(OkDialog)

class OkCancelDialog(DirectDialog):
    def __init__(self, parent = None, **kw):
        # Inherits from DirectFrame
        optiondefs = (
            # Define type of DirectGuiWidget
            ('buttonTextList',  ['OK','Cancel'],       DGG.INITOPT),
            ('buttonValueList', [DGG.DIALOG_OK, DGG.DIALOG_CANCEL], DGG.INITOPT),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)
        DirectDialog.__init__(self, parent)
        self.initialiseoptions(OkCancelDialog)

class YesNoDialog(DirectDialog):
    def __init__(self, parent = None, **kw):
        # Inherits from DirectFrame
        optiondefs = (
            # Define type of DirectGuiWidget
            ('buttonTextList',  ['Yes', 'No'],       DGG.INITOPT),
            ('buttonValueList', [DGG.DIALOG_YES, DGG.DIALOG_NO], DGG.INITOPT),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)
        DirectDialog.__init__(self, parent)
        self.initialiseoptions(YesNoDialog)

class YesNoCancelDialog(DirectDialog):
    def __init__(self, parent = None, **kw):
        # Inherits from DirectFrame
        optiondefs = (
            # Define type of DirectGuiWidget
            ('buttonTextList',  ['Yes', 'No', 'Cancel'],  DGG.INITOPT),
            ('buttonValueList', [DGG.DIALOG_YES, DGG.DIALOG_NO, DGG.DIALOG_CANCEL],
             DGG.INITOPT),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)
        DirectDialog.__init__(self, parent)
        self.initialiseoptions(YesNoCancelDialog)

class RetryCancelDialog(DirectDialog):
    def __init__(self, parent = None, **kw):
        # Inherits from DirectFrame
        optiondefs = (
            # Define type of DirectGuiWidget
            ('buttonTextList',  ['Retry','Cancel'],   DGG.INITOPT),
            ('buttonValueList', [DGG.DIALOG_RETRY, DGG.DIALOG_CANCEL], DGG.INITOPT),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)
        DirectDialog.__init__(self, parent)
        self.initialiseoptions(RetryCancelDialog)

