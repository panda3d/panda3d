"""
Floater Class: Velocity style controller for floating point values with
                a label, entry (validated), and scale
"""

__all__ = ['Floater', 'FloaterWidget', 'FloaterGroup']

from direct.showbase.TkGlobal import *
from Tkinter import *
from Valuator import Valuator, VALUATOR_MINI, VALUATOR_FULL
from direct.task import Task
import math, sys, string, Pmw

FLOATER_WIDTH = 22
FLOATER_HEIGHT = 18

class Floater(Valuator):
    def __init__(self, parent = None, **kw):
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('style',  VALUATOR_MINI,   INITOPT),
            )
        self.defineoptions(kw, optiondefs)
        # Initialize the superclass
        Valuator.__init__(self, parent)
        self.initialiseoptions(Floater)

    def createValuator(self):
        self._valuator = self.createcomponent('valuator',
                                              (('floater', 'valuator'),),
                                              None,
                                              FloaterWidget,
                                              (self.interior(),),
                                              command = self.setEntry,
                                              value = self['value'])
        self._valuator._widget.bind('<Double-ButtonPress-1>', self.mouseReset)

    def packValuator(self):
        # Position components
        if self._label:
            self._label.grid(row=0, column=0, sticky = EW)
        self._entry.grid(row=0, column=1, sticky = EW)
        self._valuator.grid(row=0, column=2, padx = 2, pady = 2)
        self.interior().columnconfigure(0, weight = 1)


class FloaterWidget(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):
        #define the megawidget options
        INITOPT = Pmw.INITOPT
        optiondefs = (
            # Appearance
            ('width',           FLOATER_WIDTH,      INITOPT),
            ('height',          FLOATER_HEIGHT,     INITOPT),
            ('relief',          RAISED,             self.setRelief),
            ('borderwidth',     2,                  self.setBorderwidth),
            ('background',      'grey75',           self.setBackground),
            # Behavior
            # Initial value of floater, use self.set to change value
            ('value',           0.0,            INITOPT),
            ('numDigits',       2,              self.setNumDigits),
            # Command to execute on floater updates
            ('command',         None,           None),
            # Extra data to be passed to command function
            ('commandData',     [],             None),
            # Callback's to execute during mouse interaction
            ('preCallback',     None,           None),
            ('postCallback',    None,           None),
            # Extra data to be passed to callback function, needs to be a list
            ('callbackData',    [],             None),
            )
        self.defineoptions(kw, optiondefs)

        # Initialize the superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Set up some local and instance variables
        # Create the components
        interior = self.interior()

        # Current value
        self.value = self['value']

        # The canvas
        width = self['width']
        height = self['height']
        self._widget = self.createcomponent('canvas', (), None,
                                            Canvas, (interior,),
                                            width = width,
                                            height = height,
                                            background = self['background'],
                                            highlightthickness = 0,
                                            scrollregion = (-width/2.0,
                                                            -height/2.0,
                                                            width/2.0,
                                                            height/2.0))
        self._widget.pack(expand = 1, fill = BOTH)

        # The floater icon
        self._widget.create_polygon(-width/2.0, 0, -2.0, -height/2.0,
                                    -2.0, height/2.0,
                                    fill = 'grey50',
                                    tags = ('floater',))
        self._widget.create_polygon(width/2.0, 0, 2.0, height/2.0,
                                    2.0, -height/2.0,
                                    fill = 'grey50',
                                    tags = ('floater',))

        # Add event bindings
        self._widget.bind('<ButtonPress-1>', self.mouseDown)
        self._widget.bind('<B1-Motion>', self.updateFloaterSF)
        self._widget.bind('<ButtonRelease-1>', self.mouseUp)
        self._widget.bind('<Enter>', self.highlightWidget)
        self._widget.bind('<Leave>', self.restoreWidget)

        # Make sure input variables processed
        self.initialiseoptions(FloaterWidget)

    def set(self, value, fCommand = 1):
        """
        self.set(value, fCommand = 1)
        Set floater to new value, execute command if fCommand == 1
        """
        # Send command if any
        if fCommand and (self['command'] != None):
            apply(self['command'], [value] + self['commandData'])
        # Record value
        self.value = value

    def updateIndicator(self, value):
        # Nothing visible to update on this type of widget
        pass

    def get(self):
        """
        self.get()
        Get current floater value
        """
        return self.value

    ## Canvas callback functions
    # Floater velocity controller
    def mouseDown(self, event):
        """ Begin mouse interaction """
        # Exectute user redefinable callback function (if any)
        self['relief'] = SUNKEN
        if self['preCallback']:
            apply(self['preCallback'], self['callbackData'])
        self.velocitySF = 0.0
        self.updateTask = taskMgr.add(self.updateFloaterTask,
                                        'updateFloater')
        self.updateTask.lastTime = globalClock.getFrameTime()

    def updateFloaterTask(self, state):
        """
        Update floaterWidget value based on current scaleFactor
        Adjust for time to compensate for fluctuating frame rates
        """
        currT = globalClock.getFrameTime()
        dt = currT - state.lastTime
        self.set(self.value + self.velocitySF * dt)
        state.lastTime = currT
        return Task.cont

    def updateFloaterSF(self, event):
        """
        Update velocity scale factor based of mouse distance from origin
        """
        x = self._widget.canvasx(event.x)
        y = self._widget.canvasy(event.y)
        offset = max(0, abs(x) - Valuator.deadband)
        if offset == 0:
            return 0
        sf = math.pow(Valuator.sfBase,
                      self.minExp + offset/Valuator.sfDist)
        if x > 0:
            self.velocitySF = sf
        else:
            self.velocitySF = -sf

    def mouseUp(self, event):
        taskMgr.remove(self.updateTask)
        self.velocitySF = 0.0
        # Execute user redefinable callback function (if any)
        if self['postCallback']:
            apply(self['postCallback'], self['callbackData'])
        self['relief'] = RAISED

    def setNumDigits(self):
        """
        Adjust minimum exponent to use in velocity task based
        upon the number of digits to be displayed in the result
        """
        self.minExp = math.floor(-self['numDigits']/
                                 math.log10(Valuator.sfBase))

    # Methods to modify floater characteristics
    def setRelief(self):
        self.interior()['relief'] = self['relief']

    def setBorderwidth(self):
        self.interior()['borderwidth'] = self['borderwidth']

    def setBackground(self):
        self._widget['background'] = self['background']

    def highlightWidget(self, event):
        self._widget.itemconfigure('floater', fill = 'black')

    def restoreWidget(self, event):
        self._widget.itemconfigure('floater', fill = 'grey50')


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
        menubar = self.createcomponent('menubar', (), None,
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

    def getAt(self, index):
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
