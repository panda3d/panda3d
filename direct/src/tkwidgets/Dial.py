from Tkinter import *
from tkSimpleDialog import askfloat
from PandaModules import ClockObject
import Task
import Pmw
import math
import string
import operator

TWO_PI = 2.0 * math.pi
ONEPOINTFIVE_PI = 1.5 * math.pi
POINTFIVE_PI = 0.5 * math.pi
INNER_SF = 0.2
MAX_EXP = 5

globalClock = ClockObject.getGlobalClock()

class Dial(Pmw.MegaWidget):
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
            ('text',              'Dial Widget',  self.setLabel),
            ('numDigits',         2,              self.setEntryFormat),
            ('command',           None,           None),
            ('commandData',       [],             None),
            ('callbackData',      [],             self.setCallbackData),
            ('min',               0.0,            self.setMin),
            ('max',               1.0,            self.setMax),
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
                                          MiniDial, (interior,),
                                          value = self['value'])
        # Attach dial to entry
        self._dial['command'] = self.setEntry
        self._dial.grid(rowspan = 2, columnspan = 2)

        # The Label
        self._label = self.createcomponent('label', (), None,
                                           Label, (interior,),
                                           text = self['text'],
                                           font = ('MS Sans Serif',12,'bold'),
                                           anchor = CENTER)
        self._label.grid(row = 0, col = 2, sticky = EW)

        # The entry
        self._entryVal = StringVar()
        self._entry = self.createcomponent('entry', (), None,
                                           Entry, (interior,),
                                           justify = RIGHT,
                                           textvariable = self._entryVal)
        self._entry.grid(row = 1, col = 2, sticky = EW)
        self._entry.bind('<Return>', self.validateEntryInput)
        self._entryBackground = self._entry.cget('background')
        interior.columnconfigure(2, weight = 1)

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
            ('max',               360.0,          None),
            ('dial_fRollover',    0,              None),
            ('dial_numSegments',  12,             None),
            )
        self.defineoptions(kw, optiondefs)
        # Initialize the superclass
        Dial.__init__(self, parent)
        # Needed because this method checks if self.__class__ is myClass
        # where myClass is the argument passed into inialiseoptions
        self.initialiseoptions(AngleDial)


class MiniDial(Pmw.MegaWidget):
    sfBase = 3.0
    sfDist = 15
    def __init__(self, parent = None, **kw):
        #define the megawidget options
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ## Appearance
            # Edge size of the dial
            ('size',            40,             INITOPT),
            # Widget relief
            ('relief',          GROOVE,         self.setRelief),
            # Widget borderwidth
            ('borderwidth',     2,              self.setBorderwidth),
            # Number of segments the dial is divided into
            ('numSegments',     10,             self.setNumSegments),
            ## Values
            # Initial value of dial, use self.set to change value
            ('value',           0.0,            INITOPT),
            ('min',             0.0,            self.setDelta),
            ('max',             1.0,            self.setDelta),
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
        # Current angle of the dial
        self.dialAngle = None
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
            self._popupMenu.add_command(label = 'Min...',
                                        command = self.getMin)
            self._popupMenu.add_command(label = 'Max...',
                                        command = self.getMax)
            self._popupMenu.add_command(label = 'Reset Value...',
                                        command = self.getResetValue)
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
        self.initialiseoptions(MiniDial)

    def set(self, value, fCommand = 1):
        """
        self.set(value, fCommand = 1)
        Set dial to new value, execute command if fCommand == 1
        """
        # Adjust for rollover
        if not self['fRollover']:
            if value > self['max']:
                self.rollCount = 0
            value = self['min'] + ((value - self['min']) % self.delta)
        if self.dialAngle:
            self.updateIndicatorRadians(self.dialAngle)
            self.dialAngle = None
        else:
            self.updateIndicator(value)
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
        delta = self.delta
        dialAngle = dialAngle % TWO_PI
        # Check for rollover, if necessary
        if (self.lastAngle > ONEPOINTFIVE_PI) and (dialAngle < POINTFIVE_PI):
            self.rollCount += 1
        elif (self.lastAngle < POINTFIVE_PI) and (dialAngle > ONEPOINTFIVE_PI):
            self.rollCount -= 1
        self.lastAngle = dialAngle
        # Update value
        newValue = self['min'] + (self.rollCount + (dialAngle/TWO_PI)) * delta
        self.dialAngle = dialAngle
        self.set(newValue)

    def updateIndicator(self, value):
        # compute new indicator angle
        delta = self.delta
        factors = divmod(value - self['min'], delta)
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
        self.set(self.value + self.delta * self.knobSF * dt)
        state.lastTime = currT
        return Task.cont

    def knobMouseMotion(self, event):
        # What is the current knob angle
        self.knobSF = self.computeKnobSF(event)

    def computeKnobSF(self, event):
        x = self._canvas.canvasx(event.x)
        y = self._canvas.canvasy(event.y)
        sf = math.pow(MiniDial.sfBase, -10 + (abs(x) / MiniDial.sfDist))
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

    def setDelta(self):
        self.delta = self['max'] - self['min']

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
    def getMin(self):
        newMin = askfloat('Dial Min', 'Min:',
                          initialvalue = `self['min']`,
                          parent = self.interior())
        if newMin:
            self['min'] = newMin
            self.updateIndicator(self.value)

    # This handles the popup dial max dialog
    def getMax(self):
        newMax = askfloat('Dial Max', 'Max:',
                          initialvalue = `self['max']`,
                          parent = self.interior())
        if newMax:
            self['max'] = newMax
            self.updateIndicator(self.value)

    # This handles the popup dial resetValue dialog
    def getResetValue(self):
        newResetValue = askfloat('Dial ResetValue', 'ResetValue:',
                                 initialvalue = `self['resetValue']`,
                                 parent = self.interior())
        if newResetValue:
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
    d2 = Dial(tl, numTicks = 12, max = 360, fRollover = 0, initialValue = 180)
    d3 = Dial(tl, numTicks = 12, max = 90, min = -90, fRollover = 0)
    d4 = Dial(tl, numTicks = 16, max = 256, fRollover = 0)
    d.pack(expand = 1, fill = X)
    d2.pack(expand = 1, fill = X)
    d3.pack(expand = 1, fill = X)
    d4.pack(expand = 1, fill = X)
