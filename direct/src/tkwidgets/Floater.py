title = 'DIRECT Floater megawidget'

import string
import Tkinter 
import Pmw

OK = 1
ERROR = 0

class Floater(Pmw.MegaWidget):
    """ Megawidget containing a label, an entry, and a scale.
        Used as a velocity style controller for floating point values
    """
 
    def __init__(self, parent = None, **kw):

        # Define the megawidget options.
        optiondefs = (
            ('command',           None,        None),
            ('value',             0.0,         self._updateValue),
            ('text',              '',          self._updateLabelText),
            ('min',               None,        None),
            ('max',               None,        None),
            ('resolution',        None,        None),
            ('maxVelocity',       100.0,       None),
	    ('errorbackground',   'pink',      None),
            ('significantDigits', 2,           self._setSigDigits),
            )
        self.defineoptions(kw, optiondefs)
 
        # Initialise base class (after defining options).
        Pmw.MegaWidget.__init__(self, parent)

        # Initialize some variables
        self.value = 0.0
        self.velocity = 0.0
        self.entryValue = Tkinter.StringVar()
        self.entryFormat = '%.2f'
        self.normalBackground = None

        # Create the components.
        interior = self.interior()
        interior['relief'] = 'groove'
        interior['borderwidth'] = 2

        self.infoFrame = self.createcomponent('frame',
                                              (), None,
                                              Tkinter.Frame, interior)
        self.infoFrame.pack(expand = 1, fill = 'both')
 
        # Create the Floater's label
        self.label = self.createcomponent('label',
                                          (), None,
                                          Tkinter.Label, self.infoFrame,
                                          text = self['text'],
                                          anchor = 'center',
                                          width = 12,
                                          font = "Arial 12 bold")
        self.label.pack(side='left', expand = 1, fill = 'x')

        # Create an entry field to display and validate the floater's value
        self.entry = self.createcomponent('entry',
                                          (), None,
                                          Tkinter.Entry, self.infoFrame,
                                          width = 10,
                                          justify = 'right',
                                          textvar = self.entryValue)
        self.entry.pack(side='left',padx = 4)
        self.entry.bind('<Return>', self._entryCommand)
                                          
        # Create the scale component.
        self.scale = self.createcomponent('scale',
                                          (), None,
                                          Tkinter.Scale, interior,
                                          command = self._scaleToVelocity,
                                          orient = 'horizontal',
                                          length = 150,
                                          from_ = -1.0,
                                          to = 1.0,
                                          resolution = 0.01,
                                          showvalue = 0)
        self.scale.pack(expand = 1, fill = 'x')
        self.scale.set(0.0)
        # When interacting with mouse:
        self.scale.bind('<Button-1>', self._startFloaterTask)
        self.scale.bind('<ButtonRelease-1>', self._floaterReset)
        # In case you wish to interact using keys
        self.scale.bind('<KeyPress-Right>', self._floaterKeyCommand)
        self.scale.bind('<KeyRelease-Right>', self._floaterReset)
        self.scale.bind('<KeyPress-Left>', self._floaterKeyCommand)
        self.scale.bind('<KeyRelease-Left>', self._floaterReset)
 
        # Check keywords and initialise options.
        self.initialiseoptions(Floater)

        # Now that the widgets have been created, update significant digits
        self._setSigDigits()
        self._updateValue()

    def _scaleToVelocity(self, strVal):
        # convert scale val to float
        val = string.atof(strVal)
        # retain sign of velocity by only calling abs once
        self.velocity = self['maxVelocity'] * val * abs(val)

    def _startFloaterTask(self,event):
        self._fFloaterTask = 1
        self._floaterTask()

    def _floaterTask(self):
        if self.velocity != 0.0:
            self.setValue( self.value + self.velocity )
        if self._fFloaterTask:
            self.after(50, self._floaterTask)

    def _floaterReset(self, event):
        self._fFloaterTask = 0
        self.velocity = 0.0
        self.scale.set(0.0)

    def _floaterKeyCommand(self, event):
        if self.velocity != 0.0:
            self.setValue( self.value + self.velocity )

    def _entryCommand(self, event = None):
        try:
            val = string.atof( self.entryValue.get() )
            self.setValue( val )
        except ValueError:
            # invalid entry, ring bell set background to warning color
            self.entry.bell()
            if self.normalBackground is None:
		self.normalBackground = self.entry.cget('background')
		self.entry.configure( background = self['errorbackground'] )

    def _updateValue(self):
        self.setValue(self['value'])
        
    def setValue(self, newVal):
        if self['min'] is not None:
            if newVal < self['min']:
                newVal = self['min']
        if self['max'] is not None:
            if newVal > self['max']:
                newVal = self['max']
        if self['resolution'] is not None:
            newVal = round(newVal / self['resolution']) * self['resolution']
        # Update floater's value
        self.value = newVal
        # Update entry to reflect formatted value
        self.entryValue.set( self.entryFormat % self.value )
        # Reset background
        if self.normalBackground is not None:
            self.entry.configure(background = self.normalBackground)
            self.normalBackground = None
        # execute command
        command = self['command']
        if command is not None:
            command( newVal )

    def _setSigDigits(self):
        sd = self['significantDigits']
        self.entryFormat = '%.' + '%d' % sd + 'f'
        self.scale['resolution'] = 10 ** (-1.0 * sd)
        # And reset value to reflect change
        self.entryValue.set( self.entryFormat % self.value )

    def _updateLabelText(self):
        self.label['text'] = self['text']
        
Pmw.forwardmethods(Floater, Tkinter.Scale, 'scale')

## SAMPLE CODE
if __name__ == '__main__':
    # Initialise Tkinter and Pmw.
    root = Pmw.initialise()
    root.title('Pmw Floater demonstration')

    # Dummy command
    def printVal(val):
        print `val`
    
    # Create and pack a Floater megawidget.
    mega1 = Floater(root)
    mega1.pack(side = 'left', expand = 1, fill = 'x')

    # These are things you can set/configure
    mega1['command'] = printVal
    mega1['text'] = 'Drive delta X'
    mega1['min'] = 0.0
    #mega1['max'] = 1000.0
    mega1['resolution'] = 1.0

    # UNCOMMENT THESE TO CUSTOMIZE THE FLOATER
    # To change the color of the label:
    # mega1.label['foreground'] = 'Red'
    
    # Max change/update, default is 100
    # To have really fine control, for example
    # mega1['maxVelocity'] = 0.1
    
    # Number of digits to the right of the decimal point, default = 2
    # mega1['significantDigits'] = 5

    # Starting value for floater
    # mega1['value'] = 123.4557

    # Let's go.
    #root.mainloop()
