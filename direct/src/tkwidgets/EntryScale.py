"""
EntryScale Class: Scale with a label, and a linked and validated entry
"""

__all__ = ['EntryScale', 'EntryScaleGroup']

from direct.showbase.TkGlobal import *
from panda3d.core import Vec4
import Pmw, sys

if sys.version_info >= (3, 0):
    from tkinter.simpledialog import *
    from tkinter.colorchooser import askcolor
else:
    from tkSimpleDialog import *
    from tkColorChooser import askcolor

"""
Change Min/Max buttons to labels, add highlight binding
"""

class EntryScale(Pmw.MegaWidget):
    "Scale with linked and validated entry"

    def __init__(self, parent = None, **kw):

        # Define the megawidget options.
        optiondefs = (
            ('state',        None,          None),
            ('value',        0.0,           Pmw.INITOPT),
            ('resolution',          0.001,         None),
            ('command',             None,          None),
            ('preCallback',         None,          None),
            ('postCallback',        None,          None),
            ('callbackData',        [],            None),
            ('min',                 0.0,           self._updateValidate),
            ('max',                 100.0,         self._updateValidate),
            ('text',                'EntryScale',  self._updateLabelText),
            ('numDigits',   2,             self._setSigDigits),
            )
        self.defineoptions(kw, optiondefs)

        # Initialise superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Initialize some class variables
        self.value = self['value']
        self.entryFormat = '%.2f'
        self.fScaleCommand = 0

        # Create the components.

        # Setup up container
        interior = self.interior()
        interior.configure(relief = GROOVE, borderwidth = 2)

        # Create a label and an entry
        self.labelFrame = self.createcomponent('frame', (), None,
                                               Frame, interior)
        # Create an entry field to display and validate the entryScale's value
        self.entryValue = StringVar()
        self.entryValue.set(self['value'])
        self.entry = self.createcomponent('entryField',
                                          # Access widget's entry using "entry"
                                          (('entry', 'entryField_entry'),),
                                          None,
                                          Pmw.EntryField, self.labelFrame,
                                          entry_width = 10,
                                          validate = { 'validator': 'real',
                                                       'min': self['min'],
                                                       'max': self['max'],
                                                       'minstrict': 0,
                                                       'maxstrict': 0},
                                          entry_justify = 'right',
                                          entry_textvar = self.entryValue,
                                          command = self._entryCommand)
        self.entry.pack(side='left', padx = 4)

        # Create the EntryScale's label
        self.label = self.createcomponent('label', (), None,
                                          Label, self.labelFrame,
                                          text = self['text'],
                                          width = 12,
                                          anchor = 'center',
                                          font = "Arial 12 bold")
        self.label.pack(side='left', expand = 1, fill = 'x')
        self.label.bind('<Button-3>', self.askForLabel)

        # Now pack the frame
        self.labelFrame.pack(expand = 1, fill = 'both')

        # Create a label and an entry
        self.minMaxFrame = self.createcomponent('mmFrame', (), None,
                                                Frame, interior)
        # Create the EntryScale's min max labels
        self.minLabel = self.createcomponent('minLabel', (), None,
                                             Label, self.minMaxFrame,
                                             text = repr(self['min']),
                                             relief = FLAT,
                                             width = 5,
                                             anchor = W,
                                             font = "Arial 8")
        self.minLabel.pack(side='left', fill = 'x')
        self.minLabel.bind('<Button-3>', self.askForMin)

        # Create the scale component.
        self.scale = self.createcomponent('scale', (), None,
                                          Scale, self.minMaxFrame,
                                          command = self._scaleCommand,
                                          orient = 'horizontal',
                                          length = 150,
                                          from_ = self['min'],
                                          to = self['max'],
                                          resolution = self['resolution'],
                                          showvalue = 0)
        self.scale.pack(side = 'left', expand = 1, fill = 'x')
        # Set scale to the middle of its range
        self.scale.set(self['value'])
        self.scale.bind('<Button-1>', self.__onPress)
        self.scale.bind('<ButtonRelease-1>', self.__onRelease)
        self.scale.bind('<Button-3>', self.askForResolution)

        self.maxLabel = self.createcomponent('maxLabel', (), None,
                                             Label, self.minMaxFrame,
                                             text = repr(self['max']),
                                             relief = FLAT,
                                             width = 5,
                                             anchor = E,
                                             font = "Arial 8")
        self.maxLabel.bind('<Button-3>', self.askForMax)
        self.maxLabel.pack(side='left', fill = 'x')
        self.minMaxFrame.pack(expand = 1, fill = 'both')

        # Check keywords and initialise options based on input values.
        self.initialiseoptions(EntryScale)

    def label(self):
        return self.label
    def scale(self):
        return self.scale
    def entry(self):
        return self.entry

    def askForLabel(self, event = None):
        newLabel = askstring(title = self['text'],
                             prompt = 'New label:',
                             initialvalue = repr(self['text']),
                             parent = self.interior())
        if newLabel:
            self['text'] = newLabel

    def askForMin(self, event = None):
        newMin = askfloat(title = self['text'],
                          prompt = 'New min val:',
                          initialvalue = repr(self['min']),
                          parent = self.interior())
        if newMin:
            self.setMin(newMin)

    def setMin(self, newMin):
        self['min'] = newMin
        self.scale['from_'] = newMin
        self.minLabel['text'] = newMin
        self.entry.checkentry()

    def askForMax(self, event = None):
        newMax = askfloat(title = self['text'],
                          parent = self.interior(),
                          initialvalue = self['max'],
                          prompt = 'New max val:')
        if newMax:
            self.setMax(newMax)

    def setMax(self, newMax):
        self['max'] = newMax
        self.scale['to'] = newMax
        self.maxLabel['text'] = newMax
        self.entry.checkentry()

    def askForResolution(self, event = None):
        newResolution = askfloat(title = self['text'],
                                 parent = self.interior(),
                                 initialvalue = self['resolution'],
                                 prompt = 'New resolution:')
        if newResolution:
            self.setResolution(newResolution)

    def setResolution(self, newResolution):
        self['resolution'] = newResolution
        self.scale['resolution'] = newResolution
        self.entry.checkentry()

    def _updateLabelText(self):
        self.label['text'] = self['text']

    def _updateValidate(self):
        self.configure(entryField_validate = {
            'validator': 'real',
            'min': self['min'],
            'max': self['max'],
            'minstrict': 0,
            'maxstrict': 0})
        self.minLabel['text'] = self['min']
        self.scale['from_'] = self['min']
        self.scale['to'] = self['max']
        self.maxLabel['text'] = self['max']

    def _scaleCommand(self, strVal):
        if not self.fScaleCommand:
            return
        # convert scale val to float
        self.set(float(strVal))
        """
        # Update entry to reflect formatted value
        self.entryValue.set(self.entryFormat % self.value)
        self.entry.checkentry()
        if self['command']:
            self['command'](self.value)
        """

    def _entryCommand(self, event = None):
        try:
            val = float(self.entryValue.get())
            self.onReturn(*self['callbackData'])
            self.set(val)
            self.onReturnRelease(*self['callbackData'])
        except ValueError:
            pass

    def _setSigDigits(self):
        sd = self['numDigits']
        self.entryFormat = '%.' + '%d' % sd + 'f'
        # And reset value to reflect change
        self.entryValue.set(self.entryFormat % self.value)

    def get(self):
        return self.value

    def set(self, newVal, fCommand = 1):
        # Clamp value
        if self['min'] is not None:
            if newVal < self['min']:
                newVal = self['min']
        if self['max'] is not None:
            if newVal > self['max']:
                newVal = self['max']
        # Round by resolution
        if self['resolution'] is not None:
            newVal = round(newVal / self['resolution']) * self['resolution']

        # Record updated value
        self.value = newVal
        # Update scale's position
        self.scale.set(newVal)
        # Update entry to reflect formatted value
        self.entryValue.set(self.entryFormat % self.value)
        self.entry.checkentry()

        # execute command
        if fCommand and (self['command'] is not None):
            self['command'](newVal)

    def onReturn(self, *args):
        """ User redefinable callback executed on <Return> in entry """
        pass

    def onReturnRelease(self, *args):
        """ User redefinable callback executed on <Return> release in entry """
        pass

    def __onPress(self, event):
        # First execute onpress callback
        if self['preCallback']:
            self['preCallback'](*self['callbackData'])
        # Now enable slider command
        self.fScaleCommand = 1

    def onPress(self, *args):
        """ User redefinable callback executed on button press """
        pass

    def __onRelease(self, event):
        # Now disable slider command
        self.fScaleCommand = 0
        # First execute onpress callback
        if self['postCallback']:
            self['postCallback'](*self['callbackData'])

    def onRelease(self, *args):
        """ User redefinable callback executed on button release """
        pass

class EntryScaleGroup(Pmw.MegaToplevel):
    def __init__(self, parent = None, **kw):

        # Default group size
        DEFAULT_DIM = 1
        # Default value depends on *actual* group size, test for user input
        DEFAULT_VALUE = [0.0] * kw.get('dim', DEFAULT_DIM)
        DEFAULT_LABELS = ['v[%d]' % x for x in range(kw.get('dim', DEFAULT_DIM))]

        #define the megawidget options
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('dim',             DEFAULT_DIM,            INITOPT),
            ('side',            TOP,                    INITOPT),
            ('title',           'Group',                None),
            # A tuple of initial values, one for each entryScale
            ('value',    DEFAULT_VALUE,          INITOPT),
            # The command to be executed any time one of the entryScales is updated
            ('command',         None,                   None),
            ('preCallback',     None,                   None),
            ('postCallback',    None,                   None),
            # A tuple of labels, one for each entryScale
            ('labels',          DEFAULT_LABELS,         self._updateLabels),
            # Destroy or withdraw
            ('fDestroy',        0,                      INITOPT)
            )
        self.defineoptions(kw, optiondefs)

        # Initialize the toplevel widget
        Pmw.MegaToplevel.__init__(self, parent)

        # Create the components
        interior = self.interior()
        # Get a copy of the initial value (making sure its a list)
        self._value = list(self['value'])

        # The Menu Bar
        self.balloon = Pmw.Balloon()
        menubar = self.createcomponent('menubar', (), None,
                                       Pmw.MenuBar, (interior,),
                                       balloon = self.balloon)
        menubar.pack(fill=X)

        # EntryScaleGroup Menu
        menubar.addmenu('EntryScale Group', 'EntryScale Group Operations')
        menubar.addmenuitem(
            'EntryScale Group', 'command', 'Reset the EntryScale Group panel',
            label = 'Reset',
            command = lambda s = self: s.reset())
        if self['fDestroy']:
            dismissCommand = self.destroy
        else:
            dismissCommand = self.withdraw
        menubar.addmenuitem(
            'EntryScale Group', 'command', 'Dismiss EntryScale Group panel',
            label = 'Dismiss', command = dismissCommand)

        menubar.addmenu('Help', 'EntryScale Group Help Operations')
        self.toggleBalloonVar = IntVar()
        self.toggleBalloonVar.set(0)
        menubar.addmenuitem('Help', 'checkbutton',
                            'Toggle balloon help',
                            label = 'Balloon Help',
                            variable = self.toggleBalloonVar,
                            command = self.toggleBalloon)

        self.entryScaleList = []
        for index in range(self['dim']):
            # Add a group alias so you can configure the entryScales via:
            #   fg.configure(Valuator_XXX = YYY)
            f = self.createcomponent(
                'entryScale%d' % index, (), 'Valuator', EntryScale,
                (interior,), value = self._value[index],
                text = self['labels'][index])
            # Do this separately so command doesn't get executed during construction
            f['command'] = lambda val, s=self, i=index: s._entryScaleSetAt(i, val)
            f['callbackData'] = [self]
            # Callbacks
            f.onReturn = self.__onReturn
            f.onReturnRelease = self.__onReturnRelease
            f['preCallback'] = self.__onPress
            f['postCallback'] = self.__onRelease
            f.pack(side = self['side'], expand = 1, fill = X)
            self.entryScaleList.append(f)

        # Make sure entryScales are initialized
        self.set(self['value'])

        # Make sure input variables processed
        self.initialiseoptions(EntryScaleGroup)

    def _updateLabels(self):
        if self['labels']:
            for index in range(self['dim']):
                self.entryScaleList[index]['text'] = self['labels'][index]

    def toggleBalloon(self):
        if self.toggleBalloonVar.get():
            self.balloon.configure(state = 'balloon')
        else:
            self.balloon.configure(state = 'none')

    def get(self):
        return self._value

    def getAt(self, index):
        return self._value[index]

    # This is the command is used to set the groups value
    def set(self, value, fCommand = 1):
        for i in range(self['dim']):
            self._value[i] = value[i]
            # Update entryScale, but don't execute its command
            self.entryScaleList[i].set(value[i], 0)
        if fCommand and (self['command'] is not None):
            self['command'](self._value)

    def setAt(self, index, value):
        # Update entryScale and execute its command
        self.entryScaleList[index].set(value)

    # This is the command used by the entryScale
    def _entryScaleSetAt(self, index, value):
        self._value[index] = value
        if self['command']:
            self['command'](self._value)

    def reset(self):
        self.set(self['value'])

    def __onReturn(self, esg):
        # Execute onReturn callback
        self.onReturn(*esg.get())

    def onReturn(self, *args):
        """ User redefinable callback executed on button press """
        pass

    def __onReturnRelease(self, esg):
        # Execute onReturnRelease callback
        self.onReturnRelease(*esg.get())

    def onReturnRelease(self, *args):
        """ User redefinable callback executed on button press """
        pass

    def __onPress(self, esg):
        # Execute onPress callback
        if self['preCallback']:
            self['preCallback'](*esg.get())

    def onPress(self, *args):
        """ User redefinable callback executed on button press """
        pass

    def __onRelease(self, esg):
        # Execute onRelease callback
        if self['postCallback']:
            self['postCallback'](*esg.get())

    def onRelease(self, *args):
        """ User redefinable callback executed on button release """
        pass

def rgbPanel(nodePath, callback = None):
    def setNodePathColor(color, np = nodePath, cb = callback):
        np.setColor(color[0]/255.0, color[1]/255.0,
                    color[2]/255.0, color[3]/255.0)
        # Execute callback to pass along color info
        if cb:
            cb(color)
    # Check init color
    if nodePath.hasColor():
        initColor = nodePath.getColor() * 255.0
    else:
        initColor = Vec4(255)
    # Create entry scale group
    esg = EntryScaleGroup(title = 'RGBA Panel: ' + nodePath.getName(),
                          dim = 4,
                          labels = ['R','G','B','A'],
                          value = [int(initColor[0]),
                                          int(initColor[1]),
                                          int(initColor[2]),
                                          int(initColor[3])],
                          Valuator_max = 255,
                          Valuator_resolution = 1,
                          # Destroy not withdraw panel on dismiss
                          fDestroy = 1,
                          command = setNodePathColor)
    # Update menu button
    esg.component('menubar').component('EntryScale Group-button')['text'] = (
        'RGBA Panel')
    # Update menu
    menubar = esg.component('menubar')
    menubar.deletemenuitems('EntryScale Group', 1, 1)

    # Some helper functions
    # Clear color
    menubar.addmenuitem(
        'EntryScale Group', 'command',
        label='Clear Color', command=lambda np=nodePath: np.clearColor())

    # Set/Clear Transparency
    menubar.addmenuitem(
        'EntryScale Group', 'command',
        label='Set Transparency', command=lambda np=nodePath: np.setTransparency(1))
    menubar.addmenuitem(
        'EntryScale Group', 'command',
        label='Clear Transparency',
        command=lambda np=nodePath: np.clearTransparency())

    # System color picker
    def popupColorPicker(esg = esg):
        # Can pass in current color with: color = (255, 0, 0)
        color = askcolor(
            parent = esg.interior(),
            # Initialize it to current color
            initialcolor = tuple(esg.get()[:3]))[0]
        if color:
            esg.set((color[0], color[1], color[2], esg.getAt(3)))

    menubar.addmenuitem(
        'EntryScale Group', 'command',
        label='Popup Color Picker', command=popupColorPicker)

    def printToLog(nodePath=nodePath):
        c = nodePath.getColor()
        print("Vec4(%.3f, %.3f, %.3f, %.3f)" % (c[0], c[1], c[2], c[3]))

    menubar.addmenuitem(
        'EntryScale Group', 'command',
        label='Print to log', command=printToLog)

    # Add back the Dismiss item we removed.
    if esg['fDestroy']:
        dismissCommand = esg.destroy
    else:
        dismissCommand = esg.withdraw
    menubar.addmenuitem(
        'EntryScale Group', 'command', 'Dismiss EntryScale Group panel',
        label='Dismiss', command=dismissCommand)

    # Set callback
    def onRelease(r, g, b, a, nodePath = nodePath):
        messenger.send('RGBPanel_setColor', [nodePath, r, g, b, a])
    esg['postCallback'] = onRelease
    return esg

## SAMPLE CODE
if __name__ == '__main__':
    # Initialise Tkinter and Pmw.
    root = Toplevel()
    root.title('Pmw EntryScale demonstration')

    # Dummy command
    def printVal(val):
        print(val)

    # Create and pack a EntryScale megawidget.
    mega1 = EntryScale(root, command = printVal)
    mega1.pack(side = 'left', expand = 1, fill = 'x')

    """
    # These are things you can set/configure
    # Starting value for entryScale
    mega1['value'] = 123.456
    mega1['text'] = 'Drive delta X'
    mega1['min'] = 0.0
    mega1['max'] = 1000.0
    mega1['resolution'] = 1.0
    # To change the color of the label:
    mega1.label['foreground'] = 'Red'
    # Max change/update, default is 100
    # To have really fine control, for example
    # mega1['maxVelocity'] = 0.1
    # Number of digits to the right of the decimal point, default = 2
    # mega1['numDigits'] = 5
    """

    # To create a entryScale group to set an RGBA value:
    group1 = EntryScaleGroup(root, dim = 4,
                          title = 'Simple RGBA Panel',
                          labels = ('R', 'G', 'B', 'A'),
                          Valuator_min = 0.0,
                          Valuator_max = 255.0,
                          Valuator_resolution = 1.0,
                          command = printVal)

    # Uncomment this if you aren't running in IDLE
    #root.mainloop()
