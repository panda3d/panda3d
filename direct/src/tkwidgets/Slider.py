"""
Slider Class: Velocity style controller for floating point values with
               a label, entry (validated), and min/max slider
"""
from Tkinter import *
from Valuator import *
import Pmw
import Task
import math
import string
import operator
from PandaModules import ClockObject

globalClock = ClockObject.getGlobalClock()

class Slider(Valuator):
    """
    Valuator widget which includes an min/max slider and an entry for setting
    floating point values in a range
    """
    def __init__(self, parent = None, **kw):
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('min',        0.0,           self.setMin),
            ('max',        100.0,         self.setMax),
            ('style',      VALUATOR_MINI,   INITOPT),
            )
        self.defineoptions(kw, optiondefs)
        Valuator.__init__(self, parent)
        # Can not enter none for min or max
        self.propertyDict['min']['fNone'] = 0
        self.propertyDict['min']['help'] = 'Minimum allowable value.'
        self.propertyDict['max']['fNone'] = 0
        self.propertyDict['max']['help'] = 'Maximum allowable value.'
        self.initialiseoptions(Slider)

    def createValuator(self):
        self._valuator = self.createcomponent(
            'valuator',
            (('slider', 'valuator'),),
            None,
            SliderWidget,
            (self.interior(),),
            style = self['style'],
            command = self.setEntry,
            value = self['value'])
        self._valuator._canvas.bind('<Double-ButtonPress-1>', self.mouseReset)

        # Add popup bindings to slider widget
        try:
            self._valuator._arrowBtn.bind(
                '<ButtonPress-3>', self._popupValuatorMenu)
        except AttributeError:
            pass
        self._valuator._minLabel.bind(
            '<ButtonPress-3>', self._popupValuatorMenu)
        self._valuator._maxLabel.bind(
            '<ButtonPress-3>', self._popupValuatorMenu)

    def packValuator(self):
        if self['style'] == VALUATOR_FULL:
            if self._label:
                self._label.grid(row = 0, col = 0, sticky = EW)
            self._entry.grid(row = 0, col = 1, sticky = EW)
            self._valuator.grid(row = 1, columnspan = 2,
                                padx = 2, pady = 2, sticky = 'ew')
            self.interior().columnconfigure(0, weight = 1)
        else:
            if self._label:
                self._label.grid(row=0,col=0, sticky = EW)
            self._entry.grid(row=0,col=1, sticky = EW)
            self._valuator.grid(row=0,col=2, padx = 2, pady = 2)
            self.interior().columnconfigure(0, weight = 1)

    def setMin(self):
        if self['min'] is not None:
            self._valuator['min'] = self['min']

    def setMax(self):
        if self['max'] is not None:
            self._valuator['max'] = self['max']


# Based on Pmw ComboBox code.
class SliderWidget(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
            # Appearance
            ('relief',          RAISED,             self.setRelief),
            ('borderwidth',     2,                  self.setBorderwidth),
            ('background',      'SystemButtonFace', self.setBackground),
	    ('fliparrow',       0,                  INITOPT),
            # Behavior
	    ('style',           VALUATOR_MINI,        INITOPT),
            # Bounds
            ('min',             0.0,            self.setMin),
            ('max',             100.0,          self.setMax),
            # Initial value of slider, use self.set to change value
            ('value',           0.0,            INITOPT),
            ('numDigits',       2,              self.setNumDigits),
            # Command to execute on slider updates
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

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

	# Create the components.
	interior = self.interior()

        # Current value
        self.value = self['value']
        self.formatString = '%2f'
        self.increment = 0.01

        # Interaction flags
        self._fUpdate = 0
        self._fUnpost = 0
        self._fPressInsde = 0
        self._isPosted = 0
        if self['style'] == VALUATOR_MINI:
            self._firstPress = 1
        else:
            self._firstPress = 0

        # Slider dimensions
        width = 100
        self.xPad = xPad = 10
        canvasWidth = width + 2 * xPad
        height = 20
        self.left = left = -(width/2.0)
        self.right = right = (width/2.0)
        self.top = top = -5
        self.bottom = bottom = top + height

        def _createSlider(parent):
            # Create the canvas inside the dropdown window.
            # Min label
            self._minLabel = Label(parent, text = self['min'], width = 8,
                                   anchor = E)
            self._minLabel.pack(side = LEFT)
            # Slider
            self._canvas = self.createcomponent(
                'canvas', (), None,
                Canvas, (parent,),
                width = canvasWidth,
                height = height,
                bd = 2,
                highlightthickness = 0,
                scrollregion = (left - xPad, top, right + xPad, bottom))
            self._canvas.pack(side = LEFT, expand=1, fill=X)
            if self['style'] == VALUATOR_FULL:
                self._canvas.configure(relief = SUNKEN, bd = 2)
            # Max label
            self._maxLabel = Label(parent, text = self['max'], width = 8,
                                   anchor = W)
            self._maxLabel.pack(side = LEFT)

            # Interaction marker
            xShift = 1
            # Shadow arrow
            self._marker = self._canvas.create_polygon(-7 + xShift, 12,
                                                       7 + xShift, 12,
                                                       xShift, 0,
                                                       fill = 'black',
                                                       tags = ('slider',))
            # Arrow
            self._canvas.create_polygon(-6.0, 10,
                                        6.0, 10,
                                        0, 0,
                                        fill = 'grey85',
                                        outline = 'black',
                                        tags = ('slider',))
            # The indicator
            self._canvas.create_line(left, 0,
                                     right, 0,
                                     width = 2,
                                     tags = ('line',))
            
            self._canvas.bind('<Left>', self._decrementValue)
            self._canvas.bind('<Right>', self._incrementValue)
            self._canvas.bind('<Shift-Left>', self._bigDecrementValue)
            self._canvas.bind('<Shift-Right>', self._bigIncrementValue)
            self._canvas.bind('<Home>', self._goToMin)
            self._canvas.bind('<End>', self._goToMax)
            

        # Create slider
	if self['style'] == VALUATOR_MINI:
	    self._isPosted = 0

	    # Create the arrow button.
	    self._arrowBtn = self.createcomponent('arrowbutton',
		    (), None,
		    Canvas, (interior,), borderwidth = 0,
		    relief = FLAT, width = 14, height = 14)
	    self._arrowBtn.pack(expand = 1, fill = BOTH)
            self._arrowBtn.create_polygon(2.5, 4.5, 12.5, 4.5, 7.5, 12.5,
                                          tags = 'arrow')
	    self._arrowRelief = self._arrowBtn.cget('relief')

	    # Create the dropdown window.
	    self._popup = self.createcomponent(
                'popup',
                (), None,
                Toplevel, (interior,),
                relief = RAISED, borderwidth = 2)
	    self._popup.withdraw()
	    self._popup.overrideredirect(1)

            _createSlider(self._popup)

	    # Bind events to the arrow button.
	    self._arrowBtn.bind('<1>', self._postCanvas)
            self._arrowBtn.bind('<Enter>', self.highlightWidget)
            self._arrowBtn.bind('<Leave>', self.restoreWidget)
            # Need to unpost the popup if the arrow Button is unmapped (eg: 
            # its toplevel window is withdrawn) while the popup canvas is
            # displayed.
            self._arrowBtn.bind('<Unmap>', self._unpostCanvas)
            
	    # Bind events to the dropdown window.
	    self._popup.bind('<Escape>', self._unpostCanvas)
	    self._popup.bind('<ButtonRelease-1>', self._sliderBtnRelease)
	    self._popup.bind('<ButtonPress-1>', self._sliderBtnPress)
            self._popup.bind('<Motion>', self._sliderMove)
	else:
	    # Create the slider directly in the interior
            _createSlider(interior)
	    self._canvas.bind('<ButtonRelease-1>', self._sliderBtnRelease)
	    self._canvas.bind('<ButtonPress-1>', self._sliderBtnPress)
            self._canvas.bind('<Motion>', self._sliderMove)
            self._canvas.bind('<Configure>', self._changeConfiguration)

        
	# Check keywords and initialise options.
	self.initialiseoptions(SliderWidget)

        # Adjust relief
        if not kw.has_key('relief'):
            if self['style'] == VALUATOR_FULL:
                self['relief'] = FLAT

    def destroy(self):
	if (self['style'] == VALUATOR_MINI) and self._isPosted:
            Pmw.popgrab(self._popup)
        Pmw.MegaWidget.destroy(self)

    #======================================================================

    # Public methods

    def set(self, value, fCommand = 1):
        """
        self.set(value, fCommand = 1)
        Set slider to new value, execute command if fCommand == 1
        """
        # Send command if any
        if fCommand and (self['command'] != None):
            apply(self['command'], [value] + self['commandData'])
        # Record value
        self.value = value

    def get(self):
        """
        self.get()
        Get current slider value
        """
        return self.value

    def updateIndicator(self, value):
        # Get current marker position
        markerX = self._getMarkerX()
        percentX = (value - self['min'])/(self['max'] - self['min'])
        newX = percentX * (self.right - self.left) + self.left
        dx = newX - markerX
        self._canvas.move('slider', dx, 0)
    
    #======================================================================

    # Private methods for slider.

    def _postCanvas(self, event = None):
        self._isPosted = 1
        self._fUpdate = 0
        if self['style'] == VALUATOR_MINI:
            self.interior()['relief'] = SUNKEN

        # Make sure that the arrow is displayed sunken.
        self.update_idletasks()

        x = self._arrowBtn.winfo_rootx() + self._arrowBtn.winfo_width()/2.0
        y = self._arrowBtn.winfo_rooty() + self._arrowBtn.winfo_height()
        minW = self._minLabel.winfo_width()
        cw =  self._canvas.winfo_width()
        maxW = self._maxLabel.winfo_width()
        #pw = minW + cw + maxW
        pw = maxW + cw/2.0
        ch =  self._canvas.winfo_height()
        sh = self.winfo_screenheight()

        # Compensate if too close to edge of screen
        if y + ch > sh and y > sh / 2:
            y = self._arrowBtn.winfo_rooty() - ch

        Pmw.setgeometryanddeiconify(self._popup, '+%d+%d' % (x - pw, y))

        # Grab the popup, so that all events are delivered to it, and
        # set focus to the canvas, to make keyboard navigation
        # easier.
        Pmw.pushgrab(self._popup, 1, self._unpostCanvas)
        self._canvas.focus_set()

        # Ignore the first release of the mouse button after posting the
        # dropdown canvas, unless the mouse enters the dropdown canvas.
        self._fUpdate = 0
        self._fUnpost = 0
        self._firstPress = 1
        self._fPressInsde = 0

    def _updateValue(self,event):
        mouseX = self._canvas.canvasx(
            event.x_root - self._canvas.winfo_rootx())
        if mouseX < self.left:
            mouseX = self.left
        if mouseX > self.right:
            mouseX = self.right
        # Update value
        sf = (mouseX - self.left)/(self.right - self.left)
        newVal = sf * (self['max'] - self['min']) + self['min']
        self.set(newVal)

    def _sliderBtnPress(self, event):
        # Check behavior for this button press
        if self['style'] == VALUATOR_MINI:
            widget = self._popup
            xPos = event.x_root - widget.winfo_rootx()
            yPos = event.y_root - widget.winfo_rooty()
            fInside = ((xPos > 0) and (xPos < widget.winfo_width()) and
                       (yPos > 0) and (yPos < widget.winfo_height()))
        else:
            fInside = 1
        # Set flags based upon result
        if fInside:
            self._fPressInside = 1
            self._fUpdate = 1
            self._updateValue(event)
        else:
            self._fPressInside = 0
            self._fUpdate = 0
            
    def _sliderMove(self, event):
        if self._firstPress and not self._fUpdate:
            canvasY = self._canvas.canvasy(
                event.y_root - self._canvas.winfo_rooty())
            if canvasY > 0:
                self._fUpdate = 1
                self._unpostOnNextRelease()
        elif self._fUpdate:
            self._updateValue(event)

    def _sliderBtnRelease(self, event):
        if (self._fUnpost or
            (not (self._firstPress or self._fPressInside))):
            self._unpostCanvas()
        # Otherwise, continue
        self._fUpdate = 0
        self._firstPress = 0
        self._fPressInside = 0

    def _unpostOnNextRelease(self, event = None):
	self._fUnpost = 1

    def _unpostCanvas(self, event=None):
	if not self._isPosted:
            # It is possible to get events on an unposted popup.  For
            # example, by repeatedly pressing the space key to post
            # and unpost the popup.  The <space> event may be
            # delivered to the popup window even though
            # Pmw.popgrab() has set the focus away from the
            # popup window.  (Bug in Tk?)
            return

        # Restore the focus before withdrawing the window, since
        # otherwise the window manager may take the focus away so we
        # can't redirect it.  Also, return the grab to the next active
        # window in the stack, if any.
        Pmw.popgrab(self._popup)
	self._popup.withdraw()

	self._isPosted = 0

        if self['style'] == VALUATOR_MINI:
            self.interior()['relief'] = RAISED

    def _incrementValue(self, event):
        self.set(self.value + self.increment)
    def _bigIncrementValue(self, event):
        self.set(self.value + self.increment * 10.0)
    def _decrementValue(self, event):
        self.set(self.value - self.increment)
    def _bigDecrementValue(self, event):
        self.set(self.value - self.increment * 10.0)
    def _goToMin(self, event):
        self.set(self['min'])
    def _goToMax(self, event):
        self.set(self['max'])

    # Methods to modify floater characteristics    
    def setMin(self):
        self._minLabel['text'] = self.formatString % self['min']
        self.updateIndicator(self.value)

    def setMax(self):
        self._maxLabel['text'] = self.formatString % self['max']
        self.updateIndicator(self.value)

    def setNumDigits(self):
        self.formatString = '%0.' + ('%d' % self['numDigits']) + 'f'
        self._minLabel['text'] = self.formatString % self['min']
        self._maxLabel['text'] = self.formatString % self['max']
        self.updateIndicator(self.value)
        self.increment = pow(10, -self['numDigits'])

    def _changeConfiguration(self, event):
        newWidth = self._canvas.winfo_width()
        self.left = -newWidth/2.0 + self.xPad
        self.right = newWidth/2.0 - self.xPad
        self._canvas.configure(scrollregion = (-newWidth/2.0, self.top,
                                               newWidth/2.0, self.bottom))
        self._canvas.coords('line', self.left, 0, self.right, 0)
        self.updateIndicator(self.value)

    def _getMarkerX(self):
        # Get marker triangle coordinates
        c = self._canvas.coords(self._marker)
        # Marker postion defined as X position of third vertex
        return c[4]

    def setRelief(self):
        self.interior()['relief'] = self['relief']

    def setBorderwidth(self):
        self.interior()['borderwidth'] = self['borderwidth']

    def setBackground(self):
        self._canvas['background'] = self['background']

    def highlightWidget(self, event):
        self._arrowBtn.itemconfigure('arrow', fill = 'black')

    def restoreWidget(self, event):
        self._arrowBtn.itemconfigure('arrow', fill = '#A0A0A0')

