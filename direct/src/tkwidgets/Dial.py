from Tkinter import *
from tkSimpleDialog import askfloat
from PandaModules import ClockObject
import Task
import Pmw
import tkMessageBox
import math
import string
import operator
import types

TWO_PI = 2.0 * math.pi
ONEPOINTFIVE_PI = 1.5 * math.pi
POINTFIVE_PI = 0.5 * math.pi
INNER_SF = 0.2
MAX_EXP = 5

DIAL_FULL = 'full'
DIAL_MINI = 'mini'

DIAL_FULL_SIZE = 45
DIAL_MINI_SIZE = 20

globalClock = ClockObject.getGlobalClock()

from tkSimpleDialog import Dialog

class WidgetPropertiesDialog(Toplevel):
    """Class to open dialogs to adjust widget properties."""
    def __init__(self, widget, propertyList, title = None, parent = None):
        """Initialize a dialog.
        Arguments:
            propertyList -- a list of properties to be edited
            parent -- a parent window (the application window)
            title -- the dialog title
        """
        # Record widget and property list
        self.widget = widget
        self.propertyList = propertyList
        # Use default parent if none specified
        if not parent:
            import Tkinter
            parent = Tkinter._default_root
        # Create toplevel window
        Toplevel.__init__(self, parent)
        self.transient(parent)
        # Set title
        if title:
            self.title(title)
        # Record parent
        self.parent = parent
        # Initialize result
        self.result = None
        # Create body
        body = Frame(self)
        self.initial_focus = self.body(body)
        body.pack(padx=5, pady=5)
        # Create OK Cancel button
        self.buttonbox()
        # Initialize window state
        self.grab_set()
        self.protocol("WM_DELETE_WINDOW", self.cancel)
        self.geometry("+%d+%d" % (parent.winfo_rootx()+50,
                                  parent.winfo_rooty()+50))
        self.initial_focus.focus_set()
        self.wait_window(self)
        
    def destroy(self):
        """Destroy the window"""
        self.propertyList = []
        self.entryList = []
        self.initial_focus = None
        Toplevel.destroy(self)
        
    #
    # construction hooks
    def body(self, master):
        """create dialog body.
        return widget that should have initial focus. 
        This method should be overridden, and is called
        by the __init__ method.
        """
        self.labelList = []
        self.entryList = []
        count = 0
        for propertySet in self.propertyList:
            # Make singletons into lists
            if type(propertySet) is not types.ListType:
                propertySet = [propertySet]
            # Name of widget property
            property = propertySet[0]
            initialvalue = self.widget[property]
            try:
                entryType = propertySet[1]
            except IndexError:
                entryType = 'float'
            try:
                fAllowNone = propertySet[2]
            except IndexError:
                fAllowNone = 0
            # Create label
            label = Label(master, text=property, justify=LEFT)
            label.grid(row=count, col = 0, padx=5, sticky=W)
            self.labelList.append(label)
            # Create entry
            entry = Entry(master)
            entry.grid(row=count, col = 1, padx=5, sticky=W+E)
            if initialvalue is None:
                entry.insert(0, 'None')
            else:
                entry.insert(0, initialvalue)
            if entryType == 'float':
                validateFunc = self.validateFloat
            elif entryType == 'int':
                validateFunc = self.validateInt
            else:
                validateFunc = self.validateString
            callback = (lambda event, vf = validateFunc,
                        e=entry,p=property,fn = fAllowNone,: vf(e, p, fn))
            entry.bind('<Return>', callback)
            self.entryList.append(entry)
            count += 1
        # Set initial focus
        if len(self.entryList) > 0:
            entry = self.entryList[0]
            entry.select_range(0, END)
            # Set initial focus to first entry in the list
            return self.entryList[0]
        else:
            # Just set initial focus to self
            return self
        
    def buttonbox(self):
        """add standard button box buttons. 
        """
        box = Frame(self)
        # Create buttons
        w = Button(box, text="OK", width=10, command=self.ok, default=ACTIVE)
        w.pack(side=LEFT, padx=5, pady=5)
        w = Button(box, text="Cancel", width=10, command=self.cancel)
        w.pack(side=LEFT, padx=5, pady=5)
        # Bind commands
        self.bind("<Escape>", self.cancel)
        # Pack
        box.pack()
        
    #
    # standard button semantics
    def ok(self, event=None):
        self.withdraw()
        self.update_idletasks()
        self.apply()
        self.cancel()
        
    def cancel(self, event=None):
        # put focus back to the parent window
        self.parent.focus_set()
        self.destroy()

    def validateFloat(self, entry, property, fAllowNone):
        value = entry.get()
        errormsg =  "Please enter a floating point value"
        if fAllowNone:
            errormsg += "\nor the string 'None'"
        try:
            value = string.atof(value)
        except ValueError:
            if fAllowNone and (value == 'None'):
                value = None
            else:
                tkMessageBox.showwarning(
                    "Illegal value", errormsg, parent = self)
                return 0
        self.widget[property] = value
        return 1

    def validateInt(self, entry, property, fAllowNone):
        value = entry.get()
        errormsg =  "Please enter an integer value"
        if fAllowNone:
            errormsg += "\nor the string 'None'"
        try:
            value = string.atoi(value)
        except ValueError:
            if fAllowNone and (value == 'None'):
                value = None
            else:
                tkMessageBox.showwarning(
                    "Illegal value", errormsg, parent = self)
                return 0
        self.widget[property] = value
        return 1

    def validateString(self, entry, property, fAllowNone):
        value = entry.get()
        if fAllowNone and (value == 'None'):
            value = None
        self.widget[property] = value

    def apply(self):
        """process the data

        This method is called automatically to process the data, *after*
        the dialog is destroyed. By default, it does nothing.
        """
        pass # override




class Dial(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):
        #define the megawidget options
        INITOPT = Pmw.INITOPT
        if 'full' == kw.get('style', DIAL_FULL):
            DIAL_SIZE = DIAL_FULL_SIZE
        else:
            DIAL_SIZE = DIAL_MINI_SIZE
        optiondefs = (
            ('style',             DIAL_FULL,      INITOPT),
            ('dial_size',         DIAL_SIZE,      None),
            # Widget relief
            ('relief',            GROOVE,         None),
            # Widget borderwidth
            ('borderwidth',       2,              None),
            ('value',             0.0,            INITOPT),
            ('resetValue',        0.0,            self.setResetValue),
            ('text',              'Dial Widget',  self.setLabel),
            ('numDigits',         2,              self.setEntryFormat),
            ('command',           None,           None),
            ('commandData',       [],             None),
            ('callbackData',      [],             self.setCallbackData),
            ('min',               None,           self.setMin),
            ('max',               None,           self.setMax),
            ('base',              0.0,            self.setBase),
            ('delta',             1.0,            self.setDelta),
            ('onReturnPress',     None,           None),
            ('onReturnRelease',   None,           None),
            ('onButtonPress',     None,           self.setButtonPressCmd),
            ('onButtonRelease',   None,           self.setButtonReleaseCmd),
            )
        self.defineoptions(kw, optiondefs)
        
        # Initialize the superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Create the components
        interior = self.interior()
        interior.configure(relief = self['relief'], bd = self['borderwidth'])
        
        # The Dial 
        self._dial = self.createcomponent('dial', (), None,
                                          DialWidget, (interior,),
                                          command = self.setEntry,
                                          value = self['value'])

        # The Label
        self._label = self.createcomponent('label', (), None,
                                           Label, (interior,),
                                           text = self['text'],
                                           font = ('MS Sans Serif',12,'bold'),
                                           anchor = CENTER)

        # The entry
        self._entryVal = StringVar()
        self._entry = self.createcomponent('entry', (), None,
                                           Entry, (interior,),
                                           justify = RIGHT,
                                           width = 12,
                                           textvariable = self._entryVal)
        self._entry.bind('<Return>', self.validateEntryInput)
        self._entryBackground = self._entry.cget('background')

        if self['style'] == DIAL_FULL:
            # Attach dial to entry
            self._dial.grid(rowspan = 2, columnspan = 2)
            self._label.grid(row = 0, col = 2, sticky = EW)
            self._entry.grid(row = 1, col = 2, sticky = EW)
            interior.columnconfigure(2, weight = 1)
        else:
            self._label.grid(row=0,col=0, sticky = EW)
            self._entry.grid(row=0,col=1, sticky = EW)
            self._dial.grid(row=0,col=2)
            interior.columnconfigure(0, weight = 1)

        # Make sure input variables processed 
        self.initialiseoptions(Dial)

    def set(self, value, fCommand = 1):
        # Pass fCommand to dial which will return it to self.setEntry
        self._dial['commandData'] = [fCommand]
        self._dial.set(value)
        
    def get(self):
        return self._dial.get()

    def setEntry(self, value, fCommand = 1):
        self._entryVal.set(self.entryFormat % value)
        # Execute command
        if fCommand and (self['command'] != None):
            apply(self['command'], [value] + self['commandData'])

    def setEntryFormat(self):
        self.entryFormat = "%." + "%df" % self['numDigits']
        self.setEntry(self.get())
        self._dial['numDigits'] = self['numDigits']

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
        if self['onReturnPress']:
            apply(self['onReturnPress'], self['callbackData'])

    def _onReturnRelease(self, *args):
        """ User redefinable callback executed on <Return> release in entry """
        if self['onReturnRelease']:
            apply(self['onReturnRelease'], self['callbackData'])

    # Pass settings down to dial
    def setCallbackData(self):
        # Pass callback data down to dial
        self._dial['callbackData'] = self['callbackData']

    def setResetValue(self):
        self._dial['resetValue'] = self['resetValue']

    def setMin(self):
        self._dial['min'] = self['min']

    def setMax(self):
        self._dial['max'] = self['max']

    def setBase(self):
        self._dial['base'] = self['base']
        
    def setDelta(self):
        self._dial['delta'] = self['delta']
        
    def setLabel(self):
        self._label['text'] = self['text']

    def setButtonPressCmd(self):
        self._dial['onButtonPress'] = self['onButtonPress']

    def setButtonReleaseCmd(self):
        self._dial['onButtonRelease'] = self['onButtonRelease']

class AngleDial(Dial):
    def __init__(self, parent = None, **kw):
        # Set the typical defaults for a 360 degree angle dial
        optiondefs = (
            ('delta',             360.0,          None),
            ('dial_fRollover',    0,              None),
            ('dial_numSegments',  12,             None),
            )
        self.defineoptions(kw, optiondefs)
        # Initialize the superclass
        Dial.__init__(self, parent)
        # Needed because this method checks if self.__class__ is myClass
        # where myClass is the argument passed into inialiseoptions
        self.initialiseoptions(AngleDial)


class DialWidget(Pmw.MegaWidget):
    sfBase = 3.0
    sfDist = 15
    def __init__(self, parent = None, **kw):
        #define the megawidget options
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ## Appearance
            # Edge size of the dial
            ('size',            DIAL_FULL_SIZE, INITOPT),
            # Widget relief
            ('relief',          SUNKEN,         self.setRelief),
            # Widget borderwidth
            ('borderwidth',     2,              self.setBorderwidth),
            ('background',      'white',        INITOPT),
            # Number of segments the dial is divided into
            ('numSegments',     10,             self.setNumSegments),
            ## Values
            # Initial value of dial, use self.set to change value
            ('value',           0.0,            INITOPT),
            ('base',            0.0,            None),
            ('delta',           1.0,            None),
            ('min',             None,           None),
            ('max',             None,           None),
            ('resolution',      None,           None),
            ('numDigits',       2,              None),
            # Value dial jumps to on reset
            ('resetValue',      0.0,            None),
            ## Behavior
            # Able to adjust max/min
            ('fAdjustable',     1,              None),
            # Snap to angle on/off
            ('fSnap',           0,              None),
            # Do values rollover (i.e. accumulate) with multiple revolutions
            ('fRollover',       1,              None),
            # Command to execute on dial updates
            ('command',         None,           None),
            # Extra data to be passed to command function
            ('commandData',     [],             None),
            # Extra data to be passed to callback function
            ('callbackData',    [],             None),
            ('onButtonPress',   None,           None),
            ('onButtonRelease', None,           None),
            )
        self.defineoptions(kw, optiondefs)
        
        # Initialize the superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Set up some local and instance variables        

        # Running total which increments/decrements every time around dial
        self.rollCount = 0
        # Current value
        self.value = self['value']

        # Create the components
        interior = self.interior()
        dim = self['size']
        # Radius of the dial
        radius = self.radius = int(dim/2.0)
        # Radius of the inner knob
        inner_radius = max(3,radius * INNER_SF)

        # The canvas 
        self._canvas = self.createcomponent('canvas', (), None,
                                            Canvas, (interior,),
                                            width = dim, height = dim,
                                            background = self['background'],
                                            highlightthickness = 0,
                                            scrollregion = (-radius,-radius,
                                                            radius, radius))
        self._canvas.pack(expand = 1, fill = BOTH)

        # The dial face (no outline/fill, primarily for binding mouse events)
        self._canvas.create_oval(-radius, -radius, radius, radius,
                                 outline = '',
                                 tags = ('dial',))

        # The indicator
        self._canvas.create_line(0, 0, 0, -radius, width = 2,
                                 tags = ('indicator', 'dial'))

        # The central knob
        self._canvas.create_oval(-inner_radius, -inner_radius,
                                 inner_radius, inner_radius,
                                 fill = '#A0A0A0',
                                 tags = ('knob',))

        # The popup menu
        self._popupMenu = Menu(interior, tearoff = 0)
        self._fSnap = IntVar()
        self._fSnap.set(self['fSnap'])
        self._popupMenu.add_checkbutton(label = 'Snap',
                                        variable = self._fSnap,
                                        command = self.setSnap)
        self._fRollover = IntVar()
        self._fRollover.set(self['fRollover'])
        if self['fAdjustable']:
            self._popupMenu.add_checkbutton(label = 'Rollover',
                                            variable = self._fRollover,
                                            command = self.setRollover)
            self._popupMenu.add_command(label = 'Properties...',
                                        command = self.getProperties)
        self._popupMenu.add_command(label = 'Reset Dial',
                                    command = self.reset)

        # Add event bindings
        self._canvas.tag_bind('dial', '<ButtonPress-1>', self.mouseDown)
        self._canvas.tag_bind('dial', '<B1-Motion>', self.mouseMotion)
        self._canvas.tag_bind('dial', '<Shift-B1-Motion>',
                              self.shiftMouseMotion)
        self._canvas.tag_bind('dial', '<ButtonRelease-1>', self.mouseUp)
        self._canvas.tag_bind('knob', '<ButtonPress-1>', self.knobMouseDown)
        self._canvas.tag_bind('knob', '<B1-Motion>', self.knobMouseMotion)
        self._canvas.tag_bind('knob', '<ButtonRelease-1>', self.knobMouseUp)
        self._canvas.tag_bind('knob', '<Enter>', self.highlightKnob)
        self._canvas.tag_bind('knob', '<Leave>', self.restoreKnob)
        self._canvas.bind('<Double-ButtonPress-1>', self.mouseReset)
        self._canvas.bind('<ButtonPress-3>', self.popupDialMenu)

        # Make sure input variables processed 
        self.initialiseoptions(DialWidget)

    def set(self, value, fCommand = 1):
        """
        self.set(value, fCommand = 1)
        Set dial to new value, execute command if fCommand == 1
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
        # Adjust for rollover
        if not self['fRollover']:
            if value > self['delta']:
                self.rollCount = 0
            value = self['base'] + ((value - self['base']) % self['delta'])
        # Update indicator to reflect adjusted value
        self.updateIndicator(value)
        # Send command if any
        if fCommand and (self['command'] != None):
            apply(self['command'], [value] + self['commandData'])
        # Record value
        self.value = value
    
    # Reset dial to reset value
    def reset(self):
        """
        self.reset()
        Reset dial to reset value
        """
        self.set(self['resetValue'])

    def mouseReset(self,event):
        if not self._canvas.find_withtag(CURRENT):
            self.reset()
        
    def get(self):
        """
        self.get()
        Get current dial value
        """
        return self.value

    ## Canvas callback functions
    # Dial
    def mouseDown(self,event):
        self._onButtonPress()
        self.lastAngle = dialAngle = self.computeDialAngle(event)
        self.computeValueFromAngle(dialAngle)

    def mouseUp(self,event):
        self._onButtonRelease()

    def shiftMouseMotion(self,event):
        self.mouseMotion(event, 1)

    def mouseMotion(self, event, fShift = 0):
        dialAngle = self.computeDialAngle(event, fShift)
        self.computeValueFromAngle(dialAngle)
        
    def computeDialAngle(self,event, fShift = 0):
        x = self._canvas.canvasx(event.x)
        y = self._canvas.canvasy(event.y)
        rawAngle = math.atan2(y,x)
        # Snap to grid
        # Convert to dial coords to do snapping
        dialAngle = rawAngle + POINTFIVE_PI
        if operator.xor(self['fSnap'], fShift):
            dialAngle = round(dialAngle / self.snapAngle) * self.snapAngle
        return dialAngle

    def computeValueFromAngle(self, dialAngle):
        delta = self['delta']
        dialAngle = dialAngle % TWO_PI
        # Check for rollover, if necessary
        if (self.lastAngle > ONEPOINTFIVE_PI) and (dialAngle < POINTFIVE_PI):
            self.rollCount += 1
        elif (self.lastAngle < POINTFIVE_PI) and (dialAngle > ONEPOINTFIVE_PI):
            self.rollCount -= 1
        self.lastAngle = dialAngle
        # Update value
        newValue = self['base'] + (self.rollCount + (dialAngle/TWO_PI)) * delta
        self.set(newValue)

    def updateIndicator(self, value):
        # compute new indicator angle
        delta = self['delta']
        factors = divmod(value - self['base'], delta)
        self.rollCount = factors[0]
        self.updateIndicatorRadians( (factors[1]/delta) * TWO_PI )

    def updateIndicatorDegrees(self, degAngle):
        self.updateIndicatorRadians(degAngle * (math.pi/180.0))
        
    def updateIndicatorRadians(self,dialAngle):
        rawAngle = dialAngle - POINTFIVE_PI
        # Compute end points
        endx = math.cos(rawAngle) * self.radius
        endy = math.sin(rawAngle) * self.radius
        # Draw new indicator
        self._canvas.coords('indicator', endx * INNER_SF, endy * INNER_SF,
                            endx, endy)

    # Knob velocity controller
    def knobMouseDown(self,event):
        self._onButtonPress()
        self.knobSF = 0.0
        t = taskMgr.spawnMethodNamed(self.knobComputeVelocity, 'cv')
        t.lastTime = globalClock.getFrameTime()

    def knobComputeVelocity(self, state):
        # Update value
        currT = globalClock.getFrameTime()
        dt = currT - state.lastTime
        #self.set(self.value + self['delta'] * self.knobSF * dt)
        self.set(self.value + self.knobSF * dt)
        state.lastTime = currT
        return Task.cont

    def knobMouseMotion(self, event):
        # What is the current knob angle
        self.knobSF = self.computeKnobSF(event)

    def computeKnobSF(self, event):
        x = self._canvas.canvasx(event.x)
        y = self._canvas.canvasy(event.y)
        minExp = math.floor(-self['numDigits']/math.log10(DialWidget.sfBase))
        sf = math.pow(DialWidget.sfBase, minExp + (abs(x) / DialWidget.sfDist))
        if x > 0:
            return sf
        else:
            return -sf

    def knobMouseUp(self, event):
        taskMgr.removeTasksNamed('cv')
        self.knobSF = 0.0
        self._onButtonRelease()

    def highlightKnob(self, event):
        self._canvas.itemconfigure('knob', fill = 'black')

    def restoreKnob(self, event):
        self._canvas.itemconfigure('knob', fill = '#A0A0A0')

    # Methods to modify dial characteristics    
    def setNumSegments(self):
        self._canvas.delete('ticks')
        # Based upon input snap angle, how many ticks
        numSegments = self['numSegments']
        # Compute snapAngle (radians)
        self.snapAngle = snapAngle = TWO_PI / numSegments
        # Create the ticks at the snap angles
        for ticknum in range(numSegments):
            angle = snapAngle * ticknum
            # convert to canvas coords
            angle = angle - POINTFIVE_PI
            # Compute tick endpoints
            startx = math.cos(angle) * self.radius
            starty = math.sin(angle) * self.radius
            # Elongate ticks at 90 degree points
            if (angle % POINTFIVE_PI) == 0.0:
                sf = 0.6
            else:
                sf = 0.8
            endx = startx * sf
            endy = starty * sf
            self._canvas.create_line(startx, starty, endx, endy,
                                     tags = ('ticks','dial'))

    def setRelief(self):
        self.interior()['relief'] = self['relief']

    def setBorderwidth(self):
        self.interior()['borderwidth'] = self['borderwidth']

    # The following methods are used to handle the popup menu
    def popupDialMenu(self,event):
        self._popupMenu.post(event.widget.winfo_pointerx(),
                             event.widget.winfo_pointery())

    # Turn angle snap on/off
    def setSnap(self):
        self['fSnap'] = self._fSnap.get()

    # Turn rollover (accumulation of a sum) on/off
    def setRollover(self):
        self['fRollover'] = self._fRollover.get()

    # This handles the popup dial min dialog
    def getProperties(self):
        # Popup dialog to adjust widget properties
        WidgetPropertiesDialog(self, [
            ['min', 'float', 1],
            ['min', 'float', 1],
            ['base', 'float', 1],
            ['delta', 'float', 0],
            ['resetValue', 'float', 0]])
            
    def getMin(self):
        newMin = askfloat('Dial Min', 'Min:',
                          initialvalue = `self['min']`,
                          parent = self.interior())
        if newMin is not None:
            self['min'] = newMin
            self.updateIndicator(self.value)

    # This handles the popup dial base value dialog
    def getBase(self):
        newBase = askfloat('Dial Base Value', 'Base:',
                           initialvalue = `self['base']`,
                           parent = self.interior())
        if newBase is not None:
            self['base'] = newBase
            self.updateIndicator(self.value)

    # This handles the popup dial delta dialog
    def getDelta(self):
        newDelta = askfloat('Delta Per Revolution', 'Delta:',
                          initialvalue = `self['delta']`,
                          parent = self.interior())
        if newDelta is not None:
            self['delta'] = newDelta
            self.updateIndicator(self.value)

    # This handles the popup dial max dialog
    def getMax(self):
        newMax = askfloat('Dial Max', 'Max:',
                          initialvalue = `self['max']`,
                          parent = self.interior())
        if newMax is not None:
            self['max'] = newMax
            self.updateIndicator(self.value)

    # This handles the popup dial resetValue dialog
    def getResetValue(self):
        newResetValue = askfloat('Dial ResetValue', 'ResetValue:',
                                 initialvalue = `self['resetValue']`,
                                 parent = self.interior())
        if newResetValue is not None:
            self['resetValue'] = newResetValue

    # User callbacks
    def _onButtonPress(self, *args):
        """ User redefinable callback executed on button press """
        if self['onButtonPress']:
            apply(self['onButtonPress'], self['callbackData'])

    def _onButtonRelease(self, *args):
        """ User redefinable callback executed on button release """
        if self['onButtonRelease']:
            apply(self['onButtonRelease'], self['callbackData'])

  
if __name__ == '__main__':
    tl = Toplevel()
    d = Dial(tl)
    d2 = Dial(tl, dial_numSegments = 12, max = 360,
              dial_fRollover = 0, value = 180)
    d3 = Dial(tl, dial_numSegments = 12, max = 90, min = -90,
              dial_fRollover = 0)
    d4 = Dial(tl, dial_numSegments = 16, max = 256,
              dial_fRollover = 0)
    d.pack(expand = 1, fill = X)
    d2.pack(expand = 1, fill = X)
    d3.pack(expand = 1, fill = X)
    d4.pack(expand = 1, fill = X)
