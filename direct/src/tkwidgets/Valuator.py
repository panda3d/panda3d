"""Undocumented Module"""

__all__ = ['Valuator', 'ValuatorGroup', 'ValuatorGroupPanel']

from direct.showbase.DirectObject import *
from direct.showbase.TkGlobal import *
from Tkinter import *
import tkColorChooser
import WidgetPropertiesDialog
import string, Pmw
from direct.directtools.DirectUtil import getTkColorString

VALUATOR_MINI = 'mini'
VALUATOR_FULL = 'full'

class Valuator(Pmw.MegaWidget):
    sfBase = 3.0
    sfDist = 7
    deadband = 5
    """ Base class for widgets used to interactively adjust numeric values """
    def __init__(self, parent = None, **kw):
        #define the megawidget options
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('state',             NORMAL,         self.setState),
            # Widget appearance
            ('relief',            GROOVE,         None),
            ('borderwidth',       2,              None),
            ('text',              'Valuator',     self.setLabel),
            # Initial and reset values
            ('value',             0.0,            INITOPT),
            ('resetValue',        0.0,            None),
            # Behavior
            ('min',               None,           None),
            ('max',               None,           None),
            ('resolution',        None,           None),
            ('numDigits',         2,              self.setEntryFormat),
            # Enable/disable popup menu
            ('fAdjustable',       1,              None),
            # Actions
            ('command',           None,           None),
            ('commandData',       [],             None),
            ('fCommandOnInit',    0,              INITOPT),
            # Callbacks to execute when updating widget's value
            ('preCallback',       None,           None),
            ('postCallback',      None,           None),
            # Extra data to be passed to callback function, needs to be a list
            ('callbackData',      [],             None),
            )
        self.defineoptions(kw, optiondefs)

        # Initialize the superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Current adjusted (for min/max/resolution) value
        self.adjustedValue = self['value']

        # Create the components
        interior = self.interior()
        interior.configure(relief = self['relief'], bd = self['borderwidth'])

        # The Valuator
        self.createValuator()
        # Set valuator callbacks for mouse start/stop
        self._valuator['preCallback'] = self._mouseDown
        self._valuator['postCallback'] = self._mouseUp

        # The Label
        if self['text'] is not None:
            self._label = self.createcomponent('label', (), None,
                                               Label, (interior,),
                                               text = self['text'],
                                               font = ('MS Sans Serif', 12),
                                               anchor = CENTER)
        else:
            self._label = None

        # The entry
        self._entryVal = StringVar()
        self._entry = self.createcomponent('entry', (), None,
                                           Entry, (interior,),
                                           justify = RIGHT,
                                           width = 12,
                                           textvariable = self._entryVal)
        self._entry.bind('<Return>', self.validateEntryInput)
        self._entryBackground = self._entry.cget('background')

        # Pack Valuator Widget
        self.packValuator()

        # Set reset value if none specified
        if not kw.has_key('resetValue'):
            self['resetValue'] = self['value']

        if self['fAdjustable']:
            # The popup menu
            self._popupMenu = Menu(interior, tearoff = 0)
            self.addValuatorMenuEntries()
            self._popupMenu.add_command(label = 'Reset',
                                        command = self.reset)
            self._popupMenu.add_command(label = 'Set to Zero',
                                        command = self.zero)
            self._popupMenu.add_command(
                label = 'Properties...',
                command = self._popupPropertiesDialog)
            # Add key bindings
            if self._label:
                self._label.bind(
                    '<ButtonPress-3>', self._popupValuatorMenu)
            self._entry.bind(
                '<ButtonPress-3>', self._popupValuatorMenu)
            self._valuator._widget.bind(
                '<ButtonPress-3>', self._popupValuatorMenu)

            # A Dictionary of dictionaries for the popup property dialog
            self.propertyDict = {
                'state':
                {'widget': self,
                 'type': 'string',
                 'help': 'Enter state: normal or disabled.'
                 },

                'text':
                {'widget': self,
                 'type': 'string',
                 'help': 'Enter label text.'
                 },

                'min':
                { 'widget': self,
                  'type': 'real',
                  'fNone': 1,
                  'help': 'Minimum allowable value. Enter None for no minimum.'},
                'max':
                { 'widget': self,
                  'type': 'real',
                  'fNone': 1,
                  'help': 'Maximum allowable value. Enter None for no maximum.'},
                'numDigits':
                {'widget': self,
                 'type': 'integer',
                 'help': 'Number of digits after decimal point.'
                 },

                'resolution':
                {'widget': self,
                 'type': 'real',
                 'fNone': 1,
                 'help':'Widget resolution. Enter None for no resolution .'
                 },

                'resetValue':
                { 'widget': self,
                  'type': 'real',
                  'help': 'Enter value to set widget to on reset.'}
                }
            # Property list defines the display order of the properties
            self.propertyList = [
                'state', 'text', 'min', 'max', 'numDigits',
                'resolution', 'resetValue']
            # Add any valuator specific properties
            self.addValuatorPropertiesToDialog()

        # Make sure input variables processed
        self.fInit = self['fCommandOnInit']
        self.initialiseoptions(Valuator)

    def set(self, value, fCommand = 1):
        """
        Update widget's value by setting valuator, which will in
        turn update the entry.  fCommand flag (which is passed to the
        valuator as commandData, which is then passed in turn to
        self.setEntry) controls command execution.
        """
        self._valuator['commandData'] = [fCommand]
        self._valuator.set(value)
        # Restore commandData to 1 so that interaction via valuator widget
        # will result in command being executed, otherwise a set with
        # commandData == 0 will stick and commands will not be executed
        self._valuator['commandData'] = [1]

    def get(self):
        """ Return current widget value """
        return self.adjustedValue

    def setEntry(self, value, fCommand = 1):
        """
        Update value displayed in entry, fCommand flag controls
        command execution
        """
        # Clamp value
        if self['min'] is not None:
            if value < self['min']:
                value = self['min']
        if self['max'] is not None:
            if value > self['max']:
                value = self['max']
        # Round by resolution
        if self['resolution'] is not None:
            value = round(value / self['resolution']) * self['resolution']
        # Format value and use it to set entry
        self._entryVal.set(self.entryFormat % value)
        # Update indicator (if any) to reflect new adjusted value
        self._valuator.updateIndicator(value)
        # Execute command if required
        if fCommand and self.fInit and (self['command'] is not None):
            apply(self['command'], [value] + self['commandData'])
        # Record adjusted value
        self.adjustedValue = value
        # Once initialization is finished, allow commands to execute
        self.fInit = 1

    def setEntryFormat(self):
        """
        Change the number of significant digits in entry
        """
        # Create new format string
        self.entryFormat = "%." + "%df" % self['numDigits']
        # Update entry to reflect new format
        self.setEntry(self.get())
        # Pass info down to valuator to adjust valuator sensitivity
        self._valuator['numDigits'] = self['numDigits']

    def validateEntryInput(self, event):
        """ Check validity of entry and if valid pass along to valuator """
        input = self._entryVal.get()
        try:
            # Reset background
            self._entry.configure(background = self._entryBackground)
            # Get new value and check validity
            newValue = string.atof(input)
            # If OK, execute preCallback if one defined
            self._preCallback()
            # Call set to update valuator
            self.set(newValue)
            # Execute callback
            self._postCallback()
            # Update valuator to reflect adjusted value
            # Don't execute command
            self._valuator.set(self.adjustedValue, 0)
        except ValueError:
            # Invalid entry, flash background
            self._entry.configure(background = 'Pink')

    # Callbacks executed on mouse down/up
    def _mouseDown(self):
        """ Function to execute at start of mouse interaction """
        # Execute pre interaction callback
        self._preCallback()

    def _mouseUp(self):
        """ Function to execute at end of mouse interaction """
        # Execute post interaction callback
        self._postCallback()
        # Update valuator to reflect adjusted value
        # Don't execute command
        self._valuator.set(self.adjustedValue, 0)

    # Callback functions
    def _preCallback(self):
        if self['preCallback']:
            apply(self['preCallback'], self['callbackData'])

    def _postCallback(self):
        # Exectute post callback if one defined
        if self['postCallback']:
            apply(self['postCallback'], self['callbackData'])

    def setState(self):
        """ Enable/disable widget """
        if self['state'] == NORMAL:
            self._entry['state'] = NORMAL
            self._entry['background'] = self._entryBackground
            self._valuator._widget['state'] = NORMAL
        elif self['state'] == DISABLED:
            self._entry['background'] = 'grey75'
            self._entry['state'] = DISABLED
            self._valuator._widget['state'] = DISABLED

    def setLabel(self):
        """ Update label's text """
        if self._label:
            self._label['text'] = self['text']

    def zero(self):
        """
        self.zero()
        Set valuator to zero
        """
        self.set(0.0)

    def reset(self):
        """
        self.reset()
        Reset valuator to reset value
        """
        self.set(self['resetValue'])

    def mouseReset(self, event):
        """
        Reset valuator to resetValue
        """
        # If not over any canvas item
        #if not self._widget.find_withtag(CURRENT):
        self.reset()

    # Popup dialog to adjust widget properties
    def _popupValuatorMenu(self, event):
        self._popupMenu.post(event.widget.winfo_pointerx(),
                             event.widget.winfo_pointery())


    def _popupPropertiesDialog(self):
        WidgetPropertiesDialog.WidgetPropertiesDialog(
            self.propertyDict,
            propertyList = self.propertyList,
            title = 'Widget Properties',
            parent = self.interior())

    def addPropertyToDialog(self, property, pDict):
        self.propertyDict[property] = pDict
        self.propertyList.append(property)

    # Virtual functions to be redefined by subclass
    def createValuator(self):
        """ Function used by subclass to create valuator geometry """
        pass

    def packValuator(self):
        """ Function used by subclass to pack widget """
        pass

    def addValuatorMenuEntries(self):
        """ Function used by subclass to add menu entries to popup menu """
        pass

    def addValuatorPropertiesToDialog(self):
        """ Function used by subclass to add properties to property dialog """
        pass


FLOATER = 'floater'
DIAL = 'dial'
ANGLEDIAL = 'angledial'
SLIDER = 'slider'

class ValuatorGroup(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):

        # Default group size
        DEFAULT_DIM = 1
        # Default value depends on *actual* group size, test for user input
        DEFAULT_VALUE = [0.0] * kw.get('dim', DEFAULT_DIM)
        DEFAULT_LABELS = map(lambda x: 'v[%d]' % x,
                             range(kw.get('dim', DEFAULT_DIM)))

        #define the megawidget options
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('type',            FLOATER,                INITOPT),
            ('dim',             DEFAULT_DIM,            INITOPT),
            ('side',            TOP,                    INITOPT),
            # A list of initial values, one for each valuator
            ('value',           DEFAULT_VALUE,          INITOPT),
            ('min',             None,                   INITOPT),
            ('max',             None,                   INITOPT),
            ('resolution',      None,                   INITOPT),
            ('numDigits',       2,                      self._setNumDigits),
            # A tuple of labels, one for each valuator
            ('labels',          DEFAULT_LABELS,         self._updateLabels),
            # The command to be executed when one of the valuators is updated
            ('command',         None,                   None),
            # Callbacks to execute when updating widget's value
            ('preCallback',       None,                 None),
            ('postCallback',      None,                 None),
            # Extra data to be passed to callback function, needs to be a list
            ('callbackData',      [],                   None),
            )
        self.defineoptions(kw, optiondefs)

        # Initialize the toplevel widget
        Pmw.MegaWidget.__init__(self, parent)

        # Create the components
        interior = self.interior()
        # Get a copy of the initial value (making sure its a list)
        self._value = list(self['value'])

        # Create the valuators
        self._valuatorList = []
        for index in range(self['dim']):
            # Add a group alias so you can configure the valuators via:
            #   fg.configure(Valuator_XXX = YYY)
            if self['type'] == DIAL:
                import Dial
                valuatorType = Dial.Dial
            elif self['type'] == ANGLEDIAL:
                import Dial
                valuatorType = Dial.AngleDial
            elif self['type'] == SLIDER:
                import Slider
                valuatorType = Slider.Slider
            else:
                import Floater
                valuatorType = Floater.Floater
            f = self.createcomponent(
                'valuator%d' % index, (), 'valuator', valuatorType,
                (interior,), value = self._value[index],
                min = self['min'], max = self['max'],
                resolution = self['resolution'],
                text = self['labels'][index],
                command = lambda val, i = index: self._valuatorSetAt(i, val),
                preCallback = self._preCallback,
                postCallback = self._postCallback,
                callbackData = [self],
                )
            f.pack(side = self['side'], expand = 1, fill = X)
            self._valuatorList.append(f)

        # Make sure valuators are initialized
        self.set(self['value'], fCommand = 0)

        # Make sure input variables processed
        self.initialiseoptions(ValuatorGroup)

    # This is the command is used to set the groups value
    def set(self, value, fCommand = 1):
        for i in range(self['dim']):
            self._value[i] = value[i]
            # Update valuator, but don't execute its command
            self._valuatorList[i].set(value[i], 0)
        if fCommand and (self['command'] is not None):
            self['command'](self._value)

    def setAt(self, index, value):
        # Update valuator and execute its command
        self._valuatorList[index].set(value)

    # This is the command used by the valuator
    def _valuatorSetAt(self, index, value):
        self._value[index] = value
        if self['command']:
            self['command'](self._value)

    def get(self):
        return self._value

    def getAt(self, index):
        return self._value[index]

    def _setNumDigits(self):
        self['valuator_numDigits'] = self['numDigits']
        self.formatString = '%0.' + '%df' % self['numDigits']

    def _updateLabels(self):
        if self['labels']:
            for index in range(self['dim']):
                self._valuatorList[index]['text'] = self['labels'][index]

    def _preCallback(self, valGroup):
        # Execute pre callback
        if self['preCallback']:
            apply(self['preCallback'], valGroup.get())

    def _postCallback(self, valGroup):
        # Execute post callback
        if self['postCallback']:
            apply(self['postCallback'], valGroup.get())

    def __len__(self):
        return self['dim']

    def __repr__(self):
        str = '[' + self.formatString % self._value[0]
        for val in self._value[1:]:
            str += ', ' + self.formatString % val
        str += ']'
        return str



class ValuatorGroupPanel(Pmw.MegaToplevel):
    def __init__(self, parent = None, **kw):

        # Default group size
        DEFAULT_DIM = 1
        # Default value depends on *actual* group size, test for user input
        DEFAULT_VALUE = [0.0] * kw.get('dim', DEFAULT_DIM)
        DEFAULT_LABELS = map(lambda x: 'v[%d]' % x,
                             range(kw.get('dim', DEFAULT_DIM)))

        #define the megawidget options
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('type',            FLOATER,                INITOPT),
            ('dim',             DEFAULT_DIM,            INITOPT),
            ('side',            TOP,                    INITOPT),
            ('title',           'Valuator Group',       None),
            # A list of initial values, one for each floater
            ('value',           DEFAULT_VALUE,          INITOPT),
            ('min',             None,                   INITOPT),
            ('max',             None,                   INITOPT),
            ('resolution',      None,                   INITOPT),
            # A tuple of labels, one for each floater
            ('labels',          DEFAULT_LABELS,         self._updateLabels),
            ('numDigits',       2,                      self._setNumDigits),
            # The command to be executed when one of the floaters is updated
            ('command',         None,                   self._setCommand),
            # Callbacks to execute when updating widget's value
            ('preCallback',       None,                 self._setPreCallback),
            ('postCallback',      None,                 self._setPostCallback),
            # Extra data to be passed to callback function, needs to be a list
            ('callbackData',      [],                   self._setCallbackData),
            # Destroy or withdraw
            ('fDestroy',        0,                      INITOPT)
            )
        self.defineoptions(kw, optiondefs)

        # Initialize the toplevel widget
        Pmw.MegaToplevel.__init__(self, parent)

        # Create the components
        interior = self.interior()

        # The Menu Bar
        self.balloon = Pmw.Balloon()
        menubar = self.createcomponent('menubar', (), None,
                                       Pmw.MenuBar, (interior,),
                                       balloon = self.balloon)
        menubar.pack(fill=X)

        # ValuatorGroup Menu
        menubar.addmenu('Valuator Group', 'Valuator Group Operations')
        menubar.addmenuitem(
            'Valuator Group', 'command', 'Reset the Valuator Group panel',
            label = 'Reset',
            command = lambda s = self: s.reset())

        if self['fDestroy']:
            dismissCommand = self.destroy
        else:
            dismissCommand = self.withdraw

        menubar.addmenuitem(
            'Valuator Group', 'command', 'Dismiss Valuator Group panel',
            label = 'Dismiss', command = dismissCommand)

        menubar.addmenu('Help', 'Valuator Group Help Operations')
        self.toggleBalloonVar = IntVar()
        self.toggleBalloonVar.set(0)
        menubar.addmenuitem('Help', 'checkbutton',
                            'Toggle balloon help',
                            label = 'Balloon Help',
                            variable = self.toggleBalloonVar,
                            command = self.toggleBalloon)

        # Create the valuator group
        self.valuatorGroup = self.createcomponent(
            'valuatorGroup',
            (('valuator', 'valuatorGroup_valuator'),),
            None, ValuatorGroup,
            (interior,),
            type = self['type'],
            dim = self['dim'],
            value = self['value'],
            min = self['min'],
            max = self['max'],
            resolution = self['resolution'],
            labels = self['labels'],
            command = self['command'])
        self.valuatorGroup.pack(expand = 1, fill = X)

        # Make sure input variables processed
        self.initialiseoptions(ValuatorGroupPanel)

    def toggleBalloon(self):
        if self.toggleBalloonVar.get():
            self.balloon.configure(state = 'balloon')
        else:
            self.balloon.configure(state = 'none')

    def _updateLabels(self):
        self.valuatorGroup['labels'] = self['labels']

    def _setNumDigits(self):
        self.valuatorGroup['numDigits'] = self['numDigits']

    def _setCommand(self):
        self.valuatorGroup['command'] = self['command']

    def _setPreCallback(self):
        self.valuatorGroup['preCallback'] = self['preCallback']

    def _setPostCallback(self):
        self.valuatorGroup['postCallback'] = self['postCallback']

    def _setCallbackData(self):
        self.valuatorGroup['callbackData'] = self['callbackData']

    def reset(self):
        self.set(self['value'])

Pmw.forwardmethods(ValuatorGroupPanel, ValuatorGroup, 'valuatorGroup')


def rgbPanel(nodePath, callback = None, style = 'mini'):
    def onRelease(r, g, b, a, nodePath = nodePath):
        messenger.send('RGBPanel_setColor', [nodePath, r, g, b, a])

    def popupColorPicker():
        # Can pass in current color with: color = (255, 0, 0)
        color = tkColorChooser.askcolor(
            parent = vgp.interior(),
            # Initialize it to current color
            initialcolor = tuple(vgp.get()[:3]))[0]
        if color:
            vgp.set((color[0], color[1], color[2], vgp.getAt(3)))

    def printToLog():
        c=nodePath.getColor()
        print "Vec4(%.3f, %.3f, %.3f, %.3f)"%(c[0], c[1], c[2], c[3])

    # Check init color
    if nodePath.hasColor():
        initColor = nodePath.getColor() * 255.0
    else:
        initColor = Vec4(255)
    # Create entry scale group
    vgp = ValuatorGroupPanel(title = 'RGBA Panel: ' + nodePath.getName(),
                             dim = 4,
                             labels = ['R','G','B','A'],
                             value = [int(initColor[0]),
                                      int(initColor[1]),
                                      int(initColor[2]),
                                      int(initColor[3])],
                             type = 'slider',
                             valuator_style = style,
                             valuator_min = 0,
                             valuator_max = 255,
                             valuator_resolution = 1,
                             # Destroy not withdraw panel on dismiss
                             fDestroy = 1)
    # Update menu button
    vgp.component('menubar').component('Valuator Group-button')['text'] = (
        'RGBA Panel')

    # Set callback
    vgp['postCallback'] = onRelease

    # Add a print button which will also serve as a color tile
    pButton = Button(vgp.interior(), text = 'Print to Log',
                     bg = getTkColorString(initColor),
                     command = printToLog)
    pButton.pack(expand = 1, fill = BOTH)

    # Update menu
    menu = vgp.component('menubar').component('Valuator Group-menu')
    # Some helper functions
    # Clear color
    menu.insert_command(index = 1, label = 'Clear Color',
                        command = lambda: nodePath.clearColor())
    # Set Clear Transparency
    menu.insert_command(index = 2, label = 'Set Transparency',
                        command = lambda: nodePath.setTransparency(1))
    menu.insert_command(
        index = 3, label = 'Clear Transparency',
        command = lambda: nodePath.clearTransparency())


    # System color picker
    menu.insert_command(index = 4, label = 'Popup Color Picker',
                        command = popupColorPicker)

    menu.insert_command(index = 5, label = 'Print to log',
                        command = printToLog)

    def setNodePathColor(color):
        nodePath.setColor(color[0]/255.0, color[1]/255.0,
                          color[2]/255.0, color[3]/255.0)
        # Update color chip button
        pButton['bg'] = getTkColorString(color)
        # Execute callback to pass along color info
        if callback:
            callback(color)
    vgp['command'] = setNodePathColor

    return vgp


def lightRGBPanel(light, style = 'mini'):
    # Color picker for lights
    def popupColorPicker():
        # Can pass in current color with: color = (255, 0, 0)
        color = tkColorChooser.askcolor(
            parent = vgp.interior(),
            # Initialize it to current color
            initialcolor = tuple(vgp.get()[:3]))[0]
        if color:
            vgp.set((color[0], color[1], color[2], vgp.getAt(3)))
    def printToLog():
        n = light.getName()
        c=light.getColor()
        print n + (".setColor(Vec4(%.3f, %.3f, %.3f, %.3f))" %
                   (c[0], c[1], c[2], c[3]))
    # Check init color
    initColor = light.getColor() * 255.0
    # Create entry scale group
    vgp = ValuatorGroupPanel(title = 'RGBA Panel: ' + light.getName(),
                             dim = 4,
                             labels = ['R','G','B','A'],
                             value = [int(initColor[0]),
                                      int(initColor[1]),
                                      int(initColor[2]),
                                      int(initColor[3])],
                             type = 'slider',
                             valuator_style = style,
                             valuator_min = 0,
                             valuator_max = 255,
                             valuator_resolution = 1,
                             # Destroy not withdraw panel on dismiss
                             fDestroy = 1)
    # Update menu button
    vgp.component('menubar').component('Valuator Group-button')['text'] = (
        'Light Control Panel')
    # Add a print button which will also serve as a color tile
    pButton = Button(vgp.interior(), text = 'Print to Log',
                     bg = getTkColorString(initColor),
                     command = printToLog)
    pButton.pack(expand = 1, fill = BOTH)
    # Update menu
    menu = vgp.component('menubar').component('Valuator Group-menu')
    # System color picker
    menu.insert_command(index = 4, label = 'Popup Color Picker',
                        command = popupColorPicker)
    menu.insert_command(index = 5, label = 'Print to log',
                        command = printToLog)
    def setLightColor(color):
        light.setColor(Vec4(color[0]/255.0, color[1]/255.0,
                            color[2]/255.0, color[3]/255.0))
        # Update color chip button
        pButton['bg'] = getTkColorString(color)
    vgp['command'] = setLightColor
    return vgp

