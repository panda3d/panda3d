"""
Floater Class: Velocity style controller for floating point values with
                a label, entry (validated), and scale
"""

from Tkinter import *
import Pmw
import string

class Floater(Pmw.MegaWidget):
    "Velocity style floating point controller"
 
    def __init__(self, parent = None, **kw):

        # Define the megawidget options.
        optiondefs = (
            ('initialValue',        0.0,        Pmw.INITOPT),
            ('resolution',          None,       None),
            ('command',             None,       None),
            ('maxVelocity',         100.0,      None),
            ('min',                 None,       self._updateValidate),
            ('max',                 None,       self._updateValidate),
            ('text',                'Floater',  self._updateLabelText),
            ('significantDigits',   2,          self._setSigDigits),
            )
        self.defineoptions(kw, optiondefs)
 
        # Initialise superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Initialize some class variables
        self.value = self['initialValue']
        self.velocity = 0.0
        self.entryFormat = '%.2f'

        # Create the components.

        # Setup up container
        interior = self.interior()
        interior.configure(relief = GROOVE, borderwidth = 2)

        # Create a label and an entry
        self.labelFrame = self.createcomponent('frame', (), None,
                                               Frame, interior)
        # Create an entry field to display and validate the floater's value
        self.entryValue = StringVar()
        self.entryValue.set(self['initialValue'])
        self.entry = self.createcomponent('entryField',
                                          # Access floaters entry using "entry"
                                          (('entry', 'entryField_entry'),),
                                          None,
                                          Pmw.EntryField, self.labelFrame,
                                          entry_width = 10,
                                          validate = { 'validator' : 'real',
                                                       'min' : self['min'],
                                                       'max' : self['max'],
                                                       'minstrict' : 0,
                                                       'maxstrict' : 0},
                                          entry_justify = 'right',
                                          entry_textvar = self.entryValue,
                                          command = self._entryCommand)
        self.entry.pack(side='left',padx = 4)
                                          
        # Create the Floater's label
        self.label = self.createcomponent('label', (), None,
                                          Label, self.labelFrame,
                                          text = self['text'],
                                          width = 12,
                                          anchor = 'center',
                                          font = "Arial 12 bold")
        self.label.pack(side='left', expand = 1, fill = 'x')

        # Now pack the frame
        self.labelFrame.pack(expand = 1, fill = 'both')

        # Create the scale component.
        self.scale = self.createcomponent('scale', (), None,
                                          Scale, interior,
                                          command = self._scaleToVelocity,
                                          orient = 'horizontal',
                                          length = 150,
                                          from_ = -1.0,
                                          to = 1.0,
                                          resolution = 0.001,
                                          showvalue = 0)
        self.scale.pack(expand = 1, fill = 'x')
        # Set scale to the middle of its range
        self.scale.set(0.0)
        
        # Add scale bindings: When interacting with mouse:
        self.scale.bind('<Button-1>', self._startFloaterTask)
        self.scale.bind('<ButtonRelease-1>', self._floaterReset)
        # In case you wish to interact using keys
        self.scale.bind('<KeyPress-Right>', self._floaterKeyCommand)
        self.scale.bind('<KeyRelease-Right>', self._floaterReset)
        self.scale.bind('<KeyPress-Left>', self._floaterKeyCommand)
        self.scale.bind('<KeyRelease-Left>', self._floaterReset)
 
        # Check keywords and initialise options based on input values.
        self.initialiseoptions(Floater)

    def label(self):
        return self.label
    def scale(self):
        return self.scale
    def entry(self):
        return self.entry
    
    def _updateLabelText(self):
        self.label['text'] = self['text']

    def _updateValidate(self):
        self.configure(entryField_validate = {
            'validator' : 'real',
            'min' : self['min'],
            'max' : self['max'],
            'minstrict' : 0,
            'maxstrict' : 0})

    def _scaleToVelocity(self, strVal):
        # convert scale val to float
        val = string.atof(strVal)
        # Square val, but retain sign of velocity by only calling abs once
        self.velocity = self['maxVelocity'] * val * abs(val)

    def _startFloaterTask(self,event):
        self._fFloaterTask = 1
        self._floaterTask()

    def _floaterTask(self):
        if self.velocity != 0.0:
            self.set( self.value + self.velocity )
        if self._fFloaterTask:
            self.after(50, self._floaterTask)

    def _floaterReset(self, event):
        self._fFloaterTask = 0
        self.velocity = 0.0
        self.scale.set(0.0)

    def _floaterKeyCommand(self, event):
        if self.velocity != 0.0:
            self.set( self.value + self.velocity )

    def _entryCommand(self, event = None):
        try:
            val = string.atof( self.entryValue.get() )
            self.set( val )
        except ValueError:
            pass

    def _setSigDigits(self):
        sd = self['significantDigits']
        self.entryFormat = '%.' + '%d' % sd + 'f'
        # And reset value to reflect change
        self.entryValue.set( self.entryFormat % self.value )

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
        
        # Update floater's value
        self.value = newVal
        # Update entry to reflect formatted value
        self.entryValue.set( self.entryFormat % self.value )
        self.entry.checkentry()
        
        # execute command
        if fCommand & (self['command'] is not None):
            self['command']( newVal )

    def reset(self):
        self.set(self['initialValue'])

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
            ('initialValue',    DEFAULT_VALUE,          INITOPT),
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
        self._value = list(self['initialValue'])

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
                (interior,), initialValue = self._value[index],
                text = self['labels'][index])
            # Do this separately so command doesn't get executed during construction
            f['command'] = lambda val, s=self, i=index: s._floaterSetAt(i, val)
            f.pack(side = self['side'], expand = 1, fill = X)
            self.floaterList.append(f)

        # Make sure floaters are initialized
        self.set(self['initialValue'])
        
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
        if fCommand & (self['command'] is not None):
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
        self.set(self['initialValue'])



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
    mega1['initialValue'] = 123.456
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
    # mega1['significantDigits'] = 5
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
