"""
Dial Class: Velocity style controller for floating point values with
             a label, entry (validated), and scale
"""

__all__ = ['Dial', 'AngleDial', 'DialWidget']

from direct.showbase.TkGlobal import *
from Tkinter import *
from Valuator import Valuator, VALUATOR_MINI, VALUATOR_FULL
from direct.task import Task
import math, string, operator, Pmw
from pandac.PandaModules import ClockObject

TWO_PI = 2.0 * math.pi
ONEPOINTFIVE_PI = 1.5 * math.pi
POINTFIVE_PI = 0.5 * math.pi
INNER_SF = 0.2

DIAL_FULL_SIZE = 45
DIAL_MINI_SIZE = 30

class Dial(Valuator):
    """
    Valuator widget which includes an angle dial and an entry for setting
    floating point values
    """
    def __init__(self, parent = None, **kw):
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('style',             VALUATOR_FULL,  INITOPT),
            ('base',              0.0,            self.setBase),
            ('delta',             1.0,            self.setDelta),
            ('fSnap',             0,              self.setSnap),
            ('fRollover',         1,              self.setRollover),
            )
        self.defineoptions(kw, optiondefs)
        Valuator.__init__(self, parent)
        self.initialiseoptions(Dial)

    def createValuator(self):
        self._valuator = self.createcomponent(
            'valuator',
            (('dial', 'valuator'),),
            None,
            DialWidget,
            (self.interior(),),
            style = self['style'],
            command = self.setEntry,
            value = self['value'])
        self._valuator._widget.bind('<Double-ButtonPress-1>', self.mouseReset)

    def packValuator(self):
        if self['style'] == VALUATOR_FULL:
            self._valuator.grid(rowspan = 2, columnspan = 2,
                                padx = 2, pady = 2)
            if self._label:
                self._label.grid(row = 0, column = 2, sticky = EW)
            self._entry.grid(row = 1, column = 2, sticky = EW)
            self.interior().columnconfigure(2, weight = 1)
        else:
            if self._label:
                self._label.grid(row=0, column=0, sticky = EW)
            self._entry.grid(row=0, column=1, sticky = EW)
            self._valuator.grid(row=0, column=2, padx = 2, pady = 2)
            self.interior().columnconfigure(0, weight = 1)

    def addValuatorPropertiesToDialog(self):
        self.addPropertyToDialog(
            'base',
            { 'widget': self._valuator,
              'type': 'real',
              'help': 'Dial value = base + delta * numRevs'})
        self.addPropertyToDialog(
            'delta',
            { 'widget': self._valuator,
              'type': 'real',
              'help': 'Dial value = base + delta * numRevs'})
        self.addPropertyToDialog(
            'numSegments',
            { 'widget': self._valuator,
              'type': 'integer',
              'help': 'Number of segments to divide dial into.'})

    def addValuatorMenuEntries(self):
        # The popup menu
        self._fSnap = IntVar()
        self._fSnap.set(self['fSnap'])
        self._popupMenu.add_checkbutton(label = 'Snap',
                                        variable = self._fSnap,
                                        command = self._setSnap)
        self._fRollover = IntVar()
        self._fRollover.set(self['fRollover'])
        if self['fAdjustable']:
            self._popupMenu.add_checkbutton(label = 'Rollover',
                                            variable = self._fRollover,
                                            command = self._setRollover)

    def setBase(self):
        """ Set Dial base value: value = base + delta * numRevs """
        self._valuator['base'] = self['base']

    def setDelta(self):
        """ Set Dial delta value: value = base + delta * numRevs """
        self._valuator['delta'] = self['delta']

    def _setSnap(self):
        """ Menu command to turn Dial angle snap on/off """
        self._valuator['fSnap'] = self._fSnap.get()

    def setSnap(self):
        """ Turn Dial angle snap on/off """
        self._fSnap.set(self['fSnap'])
        # Call menu command to send down to valuator
        self._setSnap()

    def _setRollover(self):
        """
        Menu command to turn Dial rollover on/off (i.e. does value accumulate
        every time you complete a revolution of the dial?)
        """
        self._valuator['fRollover'] = self._fRollover.get()

    def setRollover(self):
        """ Turn Dial rollover (accumulation of a sum) on/off """
        self._fRollover.set(self['fRollover'])
        # Call menu command to send down to valuator
        self._setRollover()


class AngleDial(Dial):
    def __init__(self, parent = None, **kw):
        # Set the typical defaults for a 360 degree angle dial
        optiondefs = (
            ('delta',             360.0,          None),
            ('fRollover',         0,              None),
            ('dial_numSegments',  12,             None),
            )
        self.defineoptions(kw, optiondefs)
        # Initialize the superclass
        Dial.__init__(self, parent)
        # Needed because this method checks if self.__class__ is myClass
        # where myClass is the argument passed into inialiseoptions
        self.initialiseoptions(AngleDial)


class DialWidget(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):
        #define the megawidget options
        INITOPT = Pmw.INITOPT
        optiondefs = (
            # Appearance
            ('style',           VALUATOR_FULL,      INITOPT),
            ('size',            None,           INITOPT),
            ('relief',          SUNKEN,         self.setRelief),
            ('borderwidth',     2,              self.setBorderwidth),
            ('background',      'white',        self.setBackground),
            # Number of segments the dial is divided into
            ('numSegments',     10,             self.setNumSegments),
            # Behavior
            # Initial value of dial, use self.set to change value
            ('value',           0.0,            INITOPT),
            ('numDigits',       2,              self.setNumDigits),
            # Dial specific options
            ('base',            0.0,            None),
            ('delta',           1.0,            None),
            # Snap to angle on/off
            ('fSnap',           0,              None),
            # Do values rollover (i.e. accumulate) with multiple revolutions
            ('fRollover',       1,              None),
            # Command to execute on dial updates
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

        # Running total which increments/decrements every time around dial
        self.rollCount = 0

        # Base dial size on style, if size not specified,
        if not self['size']:
            if self['style'] == VALUATOR_FULL:
                size = DIAL_FULL_SIZE
            else:
                size = DIAL_MINI_SIZE
        else:
            size = self['size']

        # Radius of the dial
        radius = self.radius = int(size/2.0)
        # Radius of the inner knob
        inner_radius = max(3, radius * INNER_SF)

        # The canvas
        self._widget = self.createcomponent('canvas', (), None,
                                            Canvas, (interior,),
                                            width = size, height = size,
                                            background = self['background'],
                                            highlightthickness = 0,
                                            scrollregion = (-radius, -radius,
                                                            radius, radius))
        self._widget.pack(expand = 1, fill = BOTH)

        # The dial face (no outline/fill, primarily for binding mouse events)
        self._widget.create_oval(-radius, -radius, radius, radius,
                                 outline = '',
                                 tags = ('dial',))

        # The indicator
        self._widget.create_line(0, 0, 0, -radius, width = 2,
                                 tags = ('indicator', 'dial'))

        # The central knob
        self._widget.create_oval(-inner_radius, -inner_radius,
                                 inner_radius, inner_radius,
                                 fill = 'grey50',
                                 tags = ('knob',))

        # Add event bindings
        self._widget.tag_bind('dial', '<ButtonPress-1>', self.mouseDown)
        self._widget.tag_bind('dial', '<B1-Motion>', self.mouseMotion)
        self._widget.tag_bind('dial', '<Shift-B1-Motion>',
                              self.shiftMouseMotion)
        self._widget.tag_bind('dial', '<ButtonRelease-1>', self.mouseUp)
        self._widget.tag_bind('knob', '<ButtonPress-1>', self.knobMouseDown)
        self._widget.tag_bind('knob', '<B1-Motion>', self.updateDialSF)
        self._widget.tag_bind('knob', '<ButtonRelease-1>', self.knobMouseUp)
        self._widget.tag_bind('knob', '<Enter>', self.highlightKnob)
        self._widget.tag_bind('knob', '<Leave>', self.restoreKnob)

        # Make sure input variables processed
        self.initialiseoptions(DialWidget)

    def set(self, value, fCommand = 1):
        """
        self.set(value, fCommand = 1)
        Set dial to new value, execute command if fCommand == 1
        """
        # Adjust for rollover
        if not self['fRollover']:
            if value > self['delta']:
                self.rollCount = 0
            value = self['base'] + ((value - self['base']) % self['delta'])
        # Send command if any
        if fCommand and (self['command'] != None):
            apply(self['command'], [value] + self['commandData'])
        # Record value
        self.value = value

    def get(self):
        """
        self.get()
        Get current dial value
        """
        return self.value

    ## Canvas callback functions
    # Dial
    def mouseDown(self, event):
        self._onButtonPress()
        self.lastAngle = dialAngle = self.computeDialAngle(event)
        self.computeValueFromAngle(dialAngle)

    def mouseUp(self, event):
        self._onButtonRelease()

    def shiftMouseMotion(self, event):
        self.mouseMotion(event, 1)

    def mouseMotion(self, event, fShift = 0):
        dialAngle = self.computeDialAngle(event, fShift)
        self.computeValueFromAngle(dialAngle)

    def computeDialAngle(self, event, fShift = 0):
        x = self._widget.canvasx(event.x)
        y = self._widget.canvasy(event.y)
        rawAngle = math.atan2(y, x)
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
        self.updateIndicatorRadians((factors[1]/delta) * TWO_PI)

    def updateIndicatorDegrees(self, degAngle):
        self.updateIndicatorRadians(degAngle * (math.pi/180.0))

    def updateIndicatorRadians(self, dialAngle):
        rawAngle = dialAngle - POINTFIVE_PI
        # Compute end points
        endx = math.cos(rawAngle) * self.radius
        endy = math.sin(rawAngle) * self.radius
        # Draw new indicator
        self._widget.coords('indicator', endx * INNER_SF, endy * INNER_SF,
                            endx, endy)

    # Knob velocity controller
    def knobMouseDown(self, event):
        self._onButtonPress()
        self.knobSF = 0.0
        self.updateTask = taskMgr.add(self.updateDialTask, 'updateDial')
        self.updateTask.lastTime = globalClock.getFrameTime()

    def updateDialTask(self, state):
        # Update value
        currT = globalClock.getFrameTime()
        dt = currT - state.lastTime
        self.set(self.value + self.knobSF * dt)
        state.lastTime = currT
        return Task.cont

    def updateDialSF(self, event):
        x = self._widget.canvasx(event.x)
        y = self._widget.canvasy(event.y)
        offset = max(0, abs(x) - Valuator.deadband)
        if offset == 0:
            return 0
        sf = math.pow(Valuator.sfBase,
                      self.minExp + offset/Valuator.sfDist)
        if x > 0:
            self.knobSF = sf
        else:
            self.knobSF = -sf

    def knobMouseUp(self, event):
        taskMgr.remove(self.updateTask)
        self.knobSF = 0.0
        self._onButtonRelease()

    def setNumDigits(self):
        # Set minimum exponent to use in velocity task
        self.minExp = math.floor(-self['numDigits']/
                                 math.log10(Valuator.sfBase))

    # Methods to modify dial characteristics
    def setRelief(self):
        self.interior()['relief'] = self['relief']

    def setBorderwidth(self):
        self.interior()['borderwidth'] = self['borderwidth']

    def setBackground(self):
        self._widget['background'] = self['background']

    def setNumSegments(self):
        self._widget.delete('ticks')
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
            self._widget.create_line(startx, starty, endx, endy,
                                     tags = ('ticks','dial'))

    def highlightKnob(self, event):
        self._widget.itemconfigure('knob', fill = 'black')

    def restoreKnob(self, event):
        self._widget.itemconfigure('knob', fill = 'grey50')

    # To call user callbacks
    def _onButtonPress(self, *args):
        """ User redefinable callback executed on button press """
        if self['preCallback']:
            apply(self['preCallback'], self['callbackData'])

    def _onButtonRelease(self, *args):
        """ User redefinable callback executed on button release """
        if self['postCallback']:
            apply(self['postCallback'], self['callbackData'])


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
