from Tkinter import *
from tkSimpleDialog import askfloat
from PandaModules import ClockObject
import Task
import Pmw
import math
import string
import operator

# TODO:
# More standardized use of 'max' and 'min'
# Better floater style action
# New option? 'delta'? 'repeatVal'? 'modulus'

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
            ('relief',          GROOVE,         INITOPT),
            # Widget borderwidth
            ('borderwidth',     2,              INITOPT),
            # Relief of dial inset
            ('canvas_relief',   GROOVE,         INITOPT),
            # Borderwidth of dial inset
            ('canvas_bd',       2,              INITOPT),
            # Size of edge of dial inset
            ('edgeLength',      50,             INITOPT),            
            ('initialValue',    0.0,            INITOPT),
            # Snap to angle on/off
            ('fSnap',           0,              None),
            # Do values rollover (i.e. accumulate) with multiple revolutions
            ('fRollover',       1,              None),
            ('command',         None,           None),
            ('commandData',     [],             None),
            ('callbackData',    [],           None),
            ('text',           'Dial Widget',   self.updateLabel),
            ('numTicks',        10,             self.createTicks),
            ('numDigits',       2,              self.updateEntryFormat),
            ('min',             0.0,            self.setScaleFactor),
            ('max',             1.0,            self.setScaleFactor),
            )
        self.defineoptions(kw, optiondefs)
        
        # Initialize the superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Set up some local and instance variables        
        dim = self['edgeLength']
        self.sfGridDelta = dim / 10
        half = self.half = int(dim/2.0)
        radius = self.radius = half - 4

        # Running total which increments/decrements every time around dial
        self.baseVal = 0.0
        # Determines value of one dial revolution
        self.scaleFactor = 1.0
        self.dialAngle = None
        # Current value
        self.value = self['initialValue']

        # Create the components
        interior = self.interior()
        interior.configure(relief = self['relief'], bd = self['borderwidth'])
        
        # The canvas 
        self._canvas = self.createcomponent('canvas', (), None,
                                            Canvas, (interior,),
                                            width = dim + 12, height = dim,
                                            scrollregion = ((- half),(- half),
                                                            half, half))
        self._canvas.grid(rowspan = 2, columnspan = 2)

        # The shuttle ring
        self._canvas.create_oval(-half, -half, half, half,
                                 fill = 'white', tags = ('ring',))

        # The dial face
        self._canvas.create_oval(-radius, -radius, radius, radius,
                                 fill = 'white', tags = ('dial',))
        self.createTicks()

        # The velocity knob
        self._canvas.create_oval(-radius * INNER_SF, -radius * INNER_SF,
                                 radius * INNER_SF, radius * INNER_SF,
                                 fill = '#909090', tags = ('velocityKnob',))

        # The indicator
        self._canvas.create_line(0, 0, 0, (- radius), width = 2,
                                 tags = ('indicator', 'dial'))

        # The Scale Factor marker
        self._canvas.create_polygon( half + 4, - 4, half + 12, 0,
                                     half + 4, + 4, fill = '#A0A0A0',
                                     tags = ('sfMarker',))
        self.sfy = 0

        # The Dial's label
        self._label = self.createcomponent('label', (), None,
                                           Label, (interior,),
                                           text = self['text'],
                                           font = ('MS Sans Serif', 12, 'bold'),
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

        # The popup menu
        self._popupMenu = Menu(interior, tearoff = 0)
        self._fAngleSnap = IntVar()
        self._fAngleSnap.set(self['fSnap'])
        self._popupMenu.add_checkbutton(label = 'Angle snap',
                                        variable = self._fAngleSnap,
                                        command = self.setAngleSnap)
        self._fRollover = IntVar()
        self._fRollover.set(self['fRollover'])
        self._popupMenu.add_checkbutton(label = 'Rollover',
                                        variable = self._fRollover,
                                        command = self.setRollover)

        sfMenu = Menu(interior, tearoff = 1)
        self.expVar = DoubleVar()
        self.expVar.set(0)
        for exp in range (MAX_EXP, -(MAX_EXP + 1), -1):
            sf = "%g" % math.pow(10, exp)
            sfMenu.add_radiobutton(label = sf, value = exp,
                                   variable = self.expVar,
                                   command = self.setScaleFactor)
        sfMenu.add_command(label = 'Scale Factor...',
                           command = self.getScaleFactor)
        self._popupMenu.add_cascade(label = 'Scale Factor',
                                    menu = sfMenu)
        self._popupMenu.add_command(label = 'Reset Dial',
                                    command = self.reset)

        # Add event bindings
        self._canvas.tag_bind('ring', '<Enter>', self.highlightRing)
        self._canvas.tag_bind('ring', '<Leave>', self.restoreRing)
        self._canvas.tag_bind('ring', '<ButtonPress-1>', self.ringMouseDown)
        self._canvas.tag_bind('ring', '<B1-Motion>', self.ringMouseMotion)
        self._canvas.tag_bind('ring', '<ButtonRelease-1>', self.ringMouseUp)
        self._canvas.tag_bind('dial', '<ButtonPress-1>', self.mouseDown)
        self._canvas.tag_bind('dial', '<B1-Motion>', self.mouseMotion)
        self._canvas.tag_bind('dial', '<Shift-B1-Motion>', self.shiftMouseMotion)
        self._canvas.tag_bind('dial', '<ButtonRelease-1>', self.mouseUp)
        self._canvas.tag_bind('sfMarker', '<Enter>', self.highlightSFMarker)
        self._canvas.tag_bind('sfMarker', '<Leave>', self.restoreSFMarker)
        self._canvas.tag_bind('velocityKnob', '<Enter>', self.highlightKnob)
        self._canvas.tag_bind('velocityKnob', '<Leave>', self.restoreKnob)
        self._canvas.tag_bind('sfMarker', '<ButtonPress-1>', self.sfMouseDown)
        self._canvas.tag_bind('sfMarker', '<B1-Motion>', self.sfMouseMotion)
        self._canvas.tag_bind('sfMarker', '<ButtonRelease-1>', self.sfMouseUp)
        self._canvas.tag_bind('velocityKnob', '<ButtonPress-1>', self.knobMouseDown)
        self._canvas.tag_bind('velocityKnob', '<B1-Motion>', self.knobMouseMotion)
        self._canvas.tag_bind('velocityKnob', '<ButtonRelease-1>', self.knobMouseUp)
        self._canvas.bind('<ButtonPress-3>', self.popupDialMenu)
        self._canvas.bind('<Double-ButtonPress-1>', self.mouseReset)
        self._canvas.bind('<Up>', self.expUp)
        self._canvas.bind('<Down>', self.expDown)

        # Make sure input variables processed 
        self.initialiseoptions(Dial)
        
    def updateLabel(self):
        self._label['text'] = self['text']

    def createTicks(self):
        self._canvas.delete('ticks')
        # Based upon input snap angle, how many ticks
        numTicks = self['numTicks']
        # Compute snapAngle (radians)
        self.snapAngle = snapAngle = TWO_PI / numTicks
        # Create the ticks at the snap angles
        for ticknum in range(numTicks):
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

    def mouseDown(self,event):
        apply(self.onPress, self['callbackData'])
        self.lastAngle = dialAngle = self.computeDialAngle(event)
        self.computeValueFromAngle(dialAngle)

    def mouseUp(self,event):
        apply(self.onRelease, self['callbackData'])

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
            self.baseVal = self.baseVal + delta
        elif (self.lastAngle < POINTFIVE_PI) and (dialAngle > ONEPOINTFIVE_PI):
            self.baseVal = self.baseVal - delta
        self.lastAngle = dialAngle
        # Update value and entry
        newValue = self['min'] + self.baseVal + delta * (dialAngle / TWO_PI)
        self.dialAngle = dialAngle
        self.set(newValue)

    def get(self):
        return self.value
    
    def set(self, value, fCommand = 1):
        if not self['fRollover']:
            if value > self['max']:
                self.baseVal = 0.0
            value = ((value - self['min']) %
                     (self['max'] - self['min'])) + self['min']
        self.updateEntry(value)
        if self.dialAngle:
            self.updateIndicatorRadians(self.dialAngle)
            self.dialAngle = None
        else:
            self.updateIndicator(value)
        if fCommand and (self['command'] != None):
            apply(self['command'], [value] + self['commandData'])

    def updateIndicator(self, value):
        # compute new indicator angle
        delta = self.delta
        factors = divmod(value - self['min'], delta)
        self.baseVal = factors[0] * delta
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
        
    def updateEntry(self, value):
        self._entryVal.set(self.entryFormat % value)

    def updateEntryFormat(self):
        self.entryFormat = "%." + "%df" % self['numDigits']
        self.updateEntry(self.value)

    def validateEntryInput(self, event):
        input = self._entryVal.get()
        try:
            self._entry.configure(background = self._entryBackground)
            newValue = string.atof(input)
            apply(self.onReturn, self['callbackData'])
            self.set(newValue)
            apply(self.onReturnRelease, self['callbackData'])
        except ValueError:
            self._entry.configure(background = 'Pink')

    def sfMouseDown(self, event):
        # Record marker starting position
        self.starty = self.sfy
        # Record mouse starting position (convert to canvas coords)
        self.lasty = self._canvas.canvasy(event.y)

    def sfMouseMotion(self, event):
        # How far did the mouse move?
        dy = self._canvas.canvasy(event.y) - self.lasty
        # Apply this delta to the marker
        newy = self.starty + dy
        # Compute new exponent based upon current position
        exp = self.sfComputeExp(newy)
        # Set resulting scale factor
        self.setScaleFactorExp(exp)

    def sfMouseUp(self, event):
        self._canvas.delete('sfText')

    # Compute exponent based on current marker position
    def sfComputeExp(self, y, fSnap = 1):
        # Force marker to stay visible
        newy = max( -self.half, min( self.half, y ) )
        # Snap it
        gridDelta = self.sfGridDelta
        if fSnap:
            newy = round( newy / gridDelta ) * gridDelta
        # Compute resulting exponent
        return (-(newy / gridDelta))
                                       
    def setScaleFactorExp(self, exp, showText = 1, fUpdateIndicator = 1):
        self.exp = exp
        # Update popup scale factor menu to nearest exponent
        self.expVar.set(int(round(exp)))
        # Compute new scale factor
        self.scaleFactor = math.pow(10, exp)
        # Compute resulting delta
        self.delta = self.scaleFactor * (self['max'] - self['min'])
        # Update indicator to reflect new scale factor
        if fUpdateIndicator:
            self.updateIndicator(self.value)
        # Move marker to correct position
        self.updateScaleFactorMarker(-exp*self.sfGridDelta, showText)

    def expUp(self,event):
        self.setScaleFactorExp(min(MAX_EXP, self.exp + 1), 0)

    def expDown(self,event):
        self.setScaleFactorExp(max(-MAX_EXP, self.exp - 1), 0)

    def ringMouseDown(self,event):
        apply(self.onPress, self['callbackData'])
        self.startRingAngle = self.computeRingAngle(event)
        self.deltaRingAngle = 0.0
        self.velocityTask = self.after(100, self.ringComputeVelocity)

    def ringMouseMotion(self, event):
        # What is the current ring angle
        ringAngle = self.computeRingAngle(event)
        self.deltaRingAngle = ringAngle - self.startRingAngle

    def ringComputeVelocity(self):
        # Compute new exponent based upon current ring position
        exp = self.deltaRingAngle/POINTFIVE_PI
        # Set resulting scale factor
        self.setScaleFactorExp(exp, fUpdateIndicator = 1)
        # Update value and entry
        delta = self.delta
        self.value = newValue = self.value + delta * self.deltaRingAngle
        self.set(newValue)
        self.updateIndicator(self.value)
        self.velocityTask = self.after(100, self.ringComputeVelocity)

    def computeRingAngle(self, event):
        x = self._canvas.canvasx(event.x)
        y = self._canvas.canvasy(event.y)
        rawAngle = math.atan2(y,x)
        # Convert to dial coords
        dialAngle = rawAngle + TWO_PI
        return dialAngle

    def ringMouseUp(self, event):
        self.after_cancel(self.velocityTask)
        # reset indicator
        self.updateIndicator(self.value)
        apply(self.onRelease, self['callbackData'])

    def knobMouseDown(self,event):
        apply(self.onPress, self['callbackData'])
        self.lasty = self._canvas.canvasy(event.y)
        self.updateIndicatorRadians(0.0)
        self.velocityTask = self.after(100, self.computeVelocity)
        
    def knobMouseMotion(self, event):
        # How far is the mouse from the origin?
        dx = self._canvas.canvasx(event.x)
        self.lasty = self._canvas.canvasy(event.y)
        exp = -5 + dx/20.0
        exp = max( -5, min( 5, exp ) )
        # Set resulting scale factor
        self.setScaleFactorExp(exp, 0, fUpdateIndicator = 0)

    def knobMouseUp(self, event):
        self.after_cancel(self.velocityTask)
        # reset indicator
        self.updateIndicator(self.value)
        apply(self.onRelease, self['callbackData'])

    def computeVelocity(self):
        if self.lasty < 0:
            sign = -1.0
        else:
            sign = 1.0
        lasty = abs(self.lasty)
        if lasty > 5:
            lasty = lasty - 5
            sf = min(100, lasty)/100.0
            sf = pow(sf, 3.0)
            newVal = self.value - sign * sf * self.delta
            self.dialAngle = - sign * sf * POINTFIVE_PI
            self.set(newVal)
        self.velocityTask = self.after(100, self.computeVelocity)

    def updateScaleFactorMarker(self, newy, showText = 1):
        # Move marker
        self._canvas.move('sfMarker', 0, newy - self.sfy)
        self.sfy = newy

        # Show current scaling factor
        if showText:
            sfText = '%g' % (self.delta / 10.0,)
            self._canvas.delete('sfText')
            self._canvas.create_rectangle( self.half - 40, newy - 6,
                                           self.half, newy + 7,
                                           fill = 'white',
                                           tags = ('sfText',))
            self._canvas.create_text( self.half, newy,
                                      justify = RIGHT,
                                      anchor = E,
                                      text = sfText,
                                      fill = 'Red',
                                      tags = ('sfText',))

    # The following routines are used to handle the popup menu
    def popupDialMenu(self,event):
        self._popupMenu.post(event.widget.winfo_pointerx(),
                             event.widget.winfo_pointery())

    # This is called by the scale factor popup menu and when the user
    # changes the dial 'delta' value
    def setScaleFactor(self):
        exp = self.expVar.get()
        self.setScaleFactorExp(exp, showText = 0)

    # This handles the popup scale factor dialog
    def getScaleFactor(self):
        sf = askfloat('Dial Scale Factor', 'Scale Factor:',
                      parent = self.interior())
        if sf:
            self.setScaleFactorExp(math.log10(sf), showText = 0)

    # Turn angle snap on/off
    def setAngleSnap(self):
        self['fSnap'] = self._fAngleSnap.get()

    # Turn rollover (accumulation of a sum) on/off
    def setRollover(self):
        self['fRollover'] = self._fRollover.get()

    def highlightSFMarker(self, event):
        self._canvas.itemconfigure('sfMarker', fill = '#252525')

    def restoreSFMarker(self, event):
        self._canvas.itemconfigure('sfMarker', fill = '#A0A0A0')

    def highlightRing(self, event):
        self._canvas.itemconfigure('ring', fill = '#A0A0A0')

    def restoreRing(self, event):
        self._canvas.itemconfigure('ring', fill = 'white')

    def highlightKnob(self, event):
        self._canvas.itemconfigure('velocityKnob', fill = '#252525')

    def restoreKnob(self, event):
        self._canvas.itemconfigure('velocityKnob', fill = '#A0A0A0')

    # Reset dial to zero
    def mouseReset(self,event):
        if not self._canvas.find_withtag(CURRENT):
            self.reset()
        
    def reset(self):
        self.set(self['initialValue'])
        # Should we do this?
        self.setScaleFactorExp(0, showText = 0)

    def onReturn(self, *args):
        """ User redefinable callback executed on <Return> in entry """
        pass

    def onReturnRelease(self, *args):
        """ User redefinable callback executed on <Return> release in entry """
        pass

    def onPress(self, *args):
        """ User redefinable callback executed on button press """
        pass

    def onRelease(self, *args):
        """ User redefinable callback executed on button release """
        pass

class AngleDial(Dial):
    def __init__(self, parent = None, **kw):
        # Set the typical defaults for a 360 degree angle dial
        optiondefs = (
            ('fRollover',       0,              None),
            ('numTicks',        12,             None),
            ('max',             360.0,          None),
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
            ('size',            50,             INITOPT),
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
        apply(self.onPress, self['callbackData'])
        self.lastAngle = dialAngle = self.computeDialAngle(event)
        self.computeValueFromAngle(dialAngle)

    def mouseUp(self,event):
        apply(self.onRelease, self['callbackData'])

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
        apply(self.onPress, self['callbackData'])
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
        apply(self.onRelease, self['callbackData'])

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
    def onPress(self, *args):
        """ User redefinable callback executed on button press """
        pass

    def onRelease(self, *args):
        """ User redefinable callback executed on button release """
        pass
        
  
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
