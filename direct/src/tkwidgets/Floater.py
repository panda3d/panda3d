"""
Floater Class: Velocity style controller for floating point values with
                a label, entry (validated), and scale
"""
from PandaObject import *
from Tkinter import *
import Pmw
import WidgetPropertiesDialog
import string

globalClock = ClockObject.getGlobalClock()


FLOATER_FULL = 'full'
FLOATER_MINI = 'mini'

FLOATER_WIDTH = 25
FLOATER_HEIGHT = 20

class FloaterWidget(Pmw.MegaWidget):
    sfBase = 3.0
    sfDist = 15
    deadband = 10
    def __init__(self, parent = None, **kw):
        #define the megawidget options
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ## Appearance
            # Edge size of the floater
            ('width',           FLOATER_WIDTH,  INITOPT),
            ('height',          FLOATER_HEIGHT, INITOPT),
            # Color
            ('background',      'white',        INITOPT),
            # Widget relief
            ('relief',          SUNKEN,         self.setRelief),
            # Widget borderwidth
            ('borderwidth',     2,              self.setBorderwidth),
            ## Values
            # Initial value of floater, use self.set to change value
            ('value',           0.0,            INITOPT),
            ('min',             None,           None),
            ('max',             None,           None),
            ('resolution',      None,           None),
            ('numDigits',       2,              self.setNumDigits),
            # Value floater jumps to on reset
            ('resetValue',      0.0,            None),
            ## Behavior
            # Able to adjust max/min
            ('fAdjustable',     1,              None),
            # Command to execute on floater updates
            ('command',         None,           None),
            # Extra data to be passed to command function
            ('commandData',     [],             None),
            # Callback's to execute during mouse interaction
            ('preCallback',   None,           None),
            ('postCallback', None,           None),
            # Extra data to be passed to callback function, needs to be a list
            ('callbackData',    [],             None),
            )
        self.defineoptions(kw, optiondefs)

        #print 'FLOATER WIDGET', self['resetValue']
        
        # Initialize the superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Set up some local and instance variables        
        # Current value
        self.value = self['value']

        # Create the components
        interior = self.interior()

        # The canvas
        width = self['width']
        height = self['height']
        self._canvas = self.createcomponent('canvas', (), None,
                                            Canvas, (interior,),
                                            width = self['width'],
                                            height = self['height'],
                                            background = self['background'],
                                            highlightthickness = 0,
                                            scrollregion = (-width/2.0,
                                                            -height/2.0,
                                                            width/2.0,
                                                            height/2.0))
        self._canvas.pack(expand = 1, fill = BOTH)

        # The floater icon
        self._canvas.create_polygon(-width/2.0, 0, -2.0, -height/2.0,
                                    -2.0, height/2.0,
                                    fill = '#A0A0A0',
                                    tags = ('floater',))
        self._canvas.create_polygon(width/2.0, 0, 2.0, height/2.0,
                                    2.0, -height/2.0,
                                    fill = '#A0A0A0',
                                    tags = ('floater',))

        # A Dictionary of dictionaries for the popup property dialog
        self.propertyDict = {
            'min' : { 'widget' : self,
                      'type' : 'real',
                      'fNone' : 1,
                      'help' : 'Minimum allowable floater value, Enter None for no minimum'},
            'max' : { 'widget' : self,
                      'type' : 'real',
                      'fNone' : 1,
                      'help' : 'Maximum allowable floater value, Enter None for no maximum'},
            'resetValue' : { 'widget' : self,
                             'type' : 'real',
                             'help' : 'Enter value to set floater to on reset.'}
            }
        self.propertyList = ['min', 'max', 'resetValue']

        # The popup menu
        self._popupMenu = Menu(interior, tearoff = 0)

        if self['fAdjustable']:
            self._popupMenu.add_command(
                label = 'Properties...',
                command = self.popupPropertiesDialog)
        self._popupMenu.add_command(label = 'Zero Floater',
                                    command = self.zero)
        self._popupMenu.add_command(label = 'Reset Floater',
                                    command = self.reset)

        # Add event bindings
        self._canvas.bind('<ButtonPress-1>', self.mouseDown)
        self._canvas.bind('<B1-Motion>', self.mouseMotion)
        self._canvas.bind('<ButtonRelease-1>', self.mouseUp)
        self._canvas.bind('<ButtonPress-3>', self.popupFloaterMenu)
        self._canvas.bind('<Double-ButtonPress-1>', self.mouseReset)
        self._canvas.bind('<ButtonPress-3>', self.popupFloaterMenu)
        self._canvas.bind('<Enter>', self.highlightIcon)
        self._canvas.bind('<Leave>', self.restoreIcon)
        self._canvas.tag_bind('floater', '<ButtonPress-1>', self.mouseDown)
        self._canvas.tag_bind('floater', '<B1-Motion>', self.mouseMotion)
        self._canvas.tag_bind('floater', '<ButtonRelease-1>', self.mouseUp)

        # Make sure input variables processed 
        self.initialiseoptions(FloaterWidget)

    def set(self, value, fCommand = 1):
        """
        self.set(value, fCommand = 1)
        Set floater to new value, execute command if fCommand == 1
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

        # Send command if any
        if fCommand and (self['command'] != None):
            apply(self['command'], [value] + self['commandData'])
        # Record value
        self.value = value
    
    # Set floater to zero
    def zero(self):
        """
        self.reset()
        Set floater to zero
        """
        self.set(0.0)

    # Reset floater to reset value
    def reset(self):
        """
        self.reset()
        Reset floater to reset value
        """
        self.set(self['resetValue'])

    def mouseReset(self,event):
        # If not over any canvas item
        #if not self._canvas.find_withtag(CURRENT):
        self.reset()
        
    def get(self):
        """
        self.get()
        Get current floater value
        """
        return self.value

    ## Canvas callback functions
    # Floater velocity controller
    def mouseDown(self,event):
        self._onButtonPress()
        self.velocitySF = 0.0
        t = taskMgr.add(self.computeVelocity, 'cv')
        t.lastTime = globalClock.getFrameTime()

    def computeVelocity(self, state):
        # Update value
        currT = globalClock.getFrameTime()
        dt = currT - state.lastTime
        self.set(self.value + self.velocitySF * dt)
        state.lastTime = currT
        return Task.cont

    def mouseMotion(self, event):
        # What is the current knob angle
        self.velocitySF = self.computeVelocitySF(event)

    def computeVelocitySF(self, event):
        x = self._canvas.canvasx(event.x)
        y = self._canvas.canvasy(event.y)
        offset = max(0, abs(x) - FloaterWidget.deadband)
        if offset == 0:
            return 0
        sf = math.pow(FloaterWidget.sfBase,
                      self.minExp + offset/FloaterWidget.sfDist)
        if x > 0:
            return sf
        else:
            return -sf

    def mouseUp(self, event):
        taskMgr.remove('cv')
        self.velocitySF = 0.0
        self._onButtonRelease()

    def highlightIcon(self, event):
        self._canvas.itemconfigure('floater', fill = 'black')

    def restoreIcon(self, event):
        self._canvas.itemconfigure('floater', fill = '#A0A0A0')

    # Methods to modify floater characteristics    
    def setRelief(self):
        self.interior()['relief'] = self['relief']

    def setBorderwidth(self):
        self.interior()['borderwidth'] = self['borderwidth']

    def setNumDigits(self):
        # Set minimum exponent to use in velocity task
        self.minExp = math.floor(-self['numDigits']/
                                 math.log10(FloaterWidget.sfBase))        

    # The following methods are used to handle the popup menu
    def popupFloaterMenu(self,event):
        self._popupMenu.post(event.widget.winfo_pointerx(),
                             event.widget.winfo_pointery())

    # Popup dialog to adjust widget properties
    def popupPropertiesDialog(self):
        WidgetPropertiesDialog.WidgetPropertiesDialog(
            self.propertyDict,
            propertyList = self.propertyList,
            title = 'Floater Widget Properties',
            parent = self._canvas)

    def addPropertyToDialog(self, property, pDict):
        self.propertyDict[property] = pDict
        self.propertyList.append(property)
            
    # User callbacks
    def _onButtonPress(self, *args):
        """ User redefinable callback executed on button press """
        if self['preCallback']:
            apply(self['preCallback'], self['callbackData'])

    def _onButtonRelease(self, *args):
        """ User redefinable callback executed on button release """
        if self['postCallback']:
            apply(self['postCallback'], self['callbackData'])


class Floater(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):
        #define the megawidget options
        INITOPT = Pmw.INITOPT
        optiondefs = (
            # Widget relief
            ('relief',            GROOVE,         None),
            # Widget borderwidth
            ('borderwidth',       2,              None),
            ('value',             0.0,            INITOPT),
            ('resetValue',        0.0,            self.setResetValue),
            ('text',              'Floater',      self.setLabel),
            ('numDigits',         2,              self.setEntryFormat),
            ('command',           None,           None),
            ('commandData',       [],             None),
            ('min',               None,           self.setMin),
            ('max',               None,           self.setMax),
            # Callbacks to execute when updating widget's value
            ('preCallback',     None,           self.setButtonPressCmd),
            ('postCallback',   None,           self.setButtonReleaseCmd),
            # Extra data to be passed to callback function, needs to be a list
            ('callbackData',      [],             self.setCallbackData),
            )
        self.defineoptions(kw, optiondefs)
        
        # Initialize the superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Create the components
        interior = self.interior()
        interior.configure(relief = self['relief'], bd = self['borderwidth'])
        
        # The Floater
        #print self['text'], self['value'], self['resetValue']
        self._floater = self.createcomponent('floater', (), None,
                                             FloaterWidget, (interior,),
                                             command = self.setEntry,
                                             resetValue = self['value'],
                                             value = self['value'])

        if not kw.has_key('resetValue'):
            self['resetValue'] = self['value']
        self._floater.addPropertyToDialog(
            'text',
            {'widget' : self,
             'type' : 'string',
             'help' : 'Enter label text for Floater.'
             }
            )
        self._floater.addPropertyToDialog(
            'numDigits',
            {'widget' : self,
             'type' : 'integer',
             'help' : 'Enter number of digits after decimal point.'
             }
            )

        # The Label
        self._label = self.createcomponent('label', (), None,
                                           Label, (interior,),
                                           text = self['text'],
                                           font = ('MS Sans Serif',12,'bold'),
                                           anchor = CENTER)
        self._label.bind('<ButtonPress-3>', self._floater.popupFloaterMenu)

        # The entry
        self._entryVal = StringVar()
        self._entry = self.createcomponent('entry', (), None,
                                           Entry, (interior,),
                                           justify = RIGHT,
                                           width = 12,
                                           textvariable = self._entryVal)
        self._entry.bind('<Return>', self.validateEntryInput)
        self._entry.bind('<ButtonPress-3>', self._floater.popupFloaterMenu)
        self._entryBackground = self._entry.cget('background')

        # Position components
        self._label.grid(row=0,col=0, sticky = EW)
        self._entry.grid(row=0,col=1, sticky = EW)
        self._floater.grid(row=0,col=2, padx = 2, pady = 2)
        interior.columnconfigure(0, weight = 1)

        # Make sure input variables processed
        self.fInit = 0
        self.initialiseoptions(Floater)
        self.fInit = 1

    def set(self, value, fCommand = 1):
        # Pass fCommand to user specified data (to control if command
        # is executed or not) to floater which will return it to self.setEntry
        self._floater['commandData'] = [fCommand]
        self._floater.set(value)
        # Restore commandData to 1 so that interaction via floater widget
        # will result in command being executed, otherwise a set with
        # commandData == 0 will stick and commands will not be executed
        self._floater['commandData'] = [1]
        
    def get(self):
        return self._floater.get()

    def setEntry(self, value, fCommand = 1):
        self._entryVal.set(self.entryFormat % value)
        # Execute command
        if self.fInit and fCommand and (self['command'] != None):
            apply(self['command'], [value] + self['commandData'])

    def setEntryFormat(self):
        self.entryFormat = "%." + "%df" % self['numDigits']
        self.setEntry(self.get())
        self._floater['numDigits'] = self['numDigits']

    def validateEntryInput(self, event):
        input = self._entryVal.get()
        try:
            self._onReturnPress()
            self._entry.configure(background = self._entryBackground)
            newValue = string.atof(input)
            self.set(newValue)
            self._onReturnRelease()
        except ValueError:
            self._entry.configure(background = 'Pink')

    def _onReturnPress(self, *args):
        """ User redefinable callback executed on <Return> in entry """
        if self['preCallback']:
            apply(self['preCallback'], self['callbackData'])

    def _onReturnRelease(self, *args):
        """ User redefinable callback executed on <Return> release in entry """
        if self['postCallback']:
            apply(self['postCallback'], self['callbackData'])

    # Pass settings down to floater
    def setCallbackData(self):
        # Pass callback data down to floater
        self._floater['callbackData'] = self['callbackData']

    def setResetValue(self):
        self._floater['resetValue'] = self['resetValue']

    def setMin(self):
        self._floater['min'] = self['min']

    def setMax(self):
        self._floater['max'] = self['max']

    def setLabel(self):
        self._label['text'] = self['text']

    def setButtonPressCmd(self):
        self._floater['preCallback'] = self['preCallback']

    def setButtonReleaseCmd(self):
        self._floater['postCallback'] = self['postCallback']


class FloaterGroup(Pmw.MegaToplevel):
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
            ('dim',             DEFAULT_DIM,            INITOPT),
            ('side',            TOP,                    INITOPT),
            ('title',           'Floater Group',        None),
            # A tuple of initial values, one for each floater
            ('value',    DEFAULT_VALUE,          INITOPT),
            # The command to be executed any time one of the floaters is updated
            ('command',         None,                   None),
            # A tuple of labels, one for each floater
            ('labels',          DEFAULT_LABELS,         self._updateLabels),
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
        menubar = self.createcomponent('menubar',(), None,
                                       Pmw.MenuBar, (interior,),
                                       balloon = self.balloon)
        menubar.pack(fill=X)
        
        # FloaterGroup Menu
        menubar.addmenu('Floater Group', 'Floater Group Operations')
        menubar.addmenuitem(
            'Floater Group', 'command', 'Reset the Floater Group panel',
            label = 'Reset',
            command = lambda s = self: s.reset())
        menubar.addmenuitem(
            'Floater Group', 'command', 'Dismiss Floater Group panel',
            label = 'Dismiss', command = self.withdraw)
        
        menubar.addmenu('Help', 'Floater Group Help Operations')
        self.toggleBalloonVar = IntVar()
        self.toggleBalloonVar.set(0)
        menubar.addmenuitem('Help', 'checkbutton',
                            'Toggle balloon help',
                            label = 'Balloon Help',
                            variable = self.toggleBalloonVar,
                            command = self.toggleBalloon)

        self.floaterList = []
        for index in range(self['dim']):
            # Add a group alias so you can configure the floaters via:
            #   fg.configure(Valuator_XXX = YYY)
            f = self.createcomponent(
                'floater%d' % index, (), 'Valuator', Floater,
                (interior,), value = self._value[index],
                text = self['labels'][index])
            # Do this separately so command doesn't get executed during construction
            f['command'] = lambda val, s=self, i=index: s._floaterSetAt(i, val)
            f.pack(side = self['side'], expand = 1, fill = X)
            self.floaterList.append(f)

        # Make sure floaters are initialized
        self.set(self['value'])
        
        # Make sure input variables processed 
        self.initialiseoptions(FloaterGroup)

    def _updateLabels(self):
        if self['labels']:
            for index in range(self['dim']):
                self.floaterList[index]['text'] = self['labels'][index]

    def toggleBalloon(self):
        if self.toggleBalloonVar.get():
            self.balloon.configure(state = 'balloon')
        else:
            self.balloon.configure(state = 'none')

    def get(self):
        return self._value

    def getAt(self,index):
        return self._value[index]

    # This is the command is used to set the groups value
    def set(self, value, fCommand = 1):
        for i in range(self['dim']):
            self._value[i] = value[i]
            # Update floater, but don't execute its command
            self.floaterList[i].set(value[i], 0)
        if fCommand and (self['command'] is not None):
            self['command'](self._value)

    def setAt(self, index, value):
        # Update floater and execute its command
        self.floaterList[index].set(value)

    # This is the command used by the floater
    def _floaterSetAt(self, index, value):
        self._value[index] = value
        if self['command']:
            self['command'](self._value)

    def reset(self):
        self.set(self['value'])

  
## SAMPLE CODE
if __name__ == '__main__':
    # Initialise Tkinter and Pmw.
    root = Toplevel()
    root.title('Pmw Floater demonstration')

    # Dummy command
    def printVal(val):
        print val
    
    # Create and pack a Floater megawidget.
    mega1 = Floater(root, command = printVal)
    mega1.pack(side = 'left', expand = 1, fill = 'x')

    """
    # These are things you can set/configure
    # Starting value for floater    
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

    # To create a floater group to set an RGBA value:
    group1 = FloaterGroup(root, dim = 4,
                          title = 'Simple RGBA Panel',
                          labels = ('R', 'G', 'B', 'A'),
                          Valuator_min = 0.0,
                          Valuator_max = 255.0,
                          Valuator_resolution = 1.0,
                          command = printVal)
    
    # Uncomment this if you aren't running in IDLE
    #root.mainloop()
