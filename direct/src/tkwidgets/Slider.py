"""
Slider Class: Velocity style controller for floating point values with
               a label, entry (validated), and min/max slider
"""

__all__ = ['Slider', 'SliderWidget', 'rgbPanel']

from direct.showbase.TkGlobal import *
from .Valuator import Valuator, rgbPanel, VALUATOR_MINI, VALUATOR_FULL
import Pmw

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
        # Can not enter None for min or max, update propertyDict to reflect
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
        #self._valuator._widget.bind('<Double-ButtonPress-1>', self.mouseReset)

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
                self._label.grid(row = 0, column = 0, sticky = EW)
            self._entry.grid(row = 0, column = 1, sticky = EW)
            self._valuator.grid(row = 1, columnspan = 2,
                                padx = 2, pady = 2, sticky = 'ew')
            self.interior().columnconfigure(0, weight = 1)
        else:
            if self._label:
                self._label.grid(row=0, column=0, sticky = EW)
            self._entry.grid(row=0, column=1, sticky = EW)
            self._valuator.grid(row=0, column=2, padx = 2, pady = 2)
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
            ('style',           VALUATOR_MINI,      INITOPT),
            ('relief',          RAISED,             self.setRelief),
            ('borderwidth',     2,                  self.setBorderwidth),
            ('background',      'grey75',           self.setBackground),
            ('fliparrow',       0,                  INITOPT),
            # Behavior
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
        self._isPosted = 0
        self._fUnpost = 0
        self._fUpdate = 0
        self._firstPress = 1
        self._fPressInsde = 0

        # Slider dimensions
        width = 100
        self.xPad = xPad = 10
        sliderWidth = width + 2 * xPad
        height = 20
        self.left = left = -(width/2.0)
        self.right = right = (width/2.0)
        top = -5
        bottom = top + height

        def createSlider(parent):
            # Create the slider inside the dropdown window.
            # Min label
            self._minLabel = Label(parent, text = self['min'], width = 8,
                                   anchor = W)
            self._minLabel.pack(side = LEFT)
            # Slider widget
            if self['style'] == VALUATOR_FULL:
                # Use a scale slider
                self._widgetVar = DoubleVar()
                self._widgetVar.set(self['value'])
                self._widget = self.createcomponent(
                    'slider', (), None,
                    Scale, (interior,),
                    variable = self._widgetVar,
                    from_ = self['min'], to = self['max'],
                    resolution = 0.0,
                    width = 10,
                    orient = 'horizontal',
                    showvalue = 0,
                    length = sliderWidth,
                    relief = FLAT, bd = 2,
                    highlightthickness = 0)
            else:
                # Use a canvas slider
                self._widget = self.createcomponent(
                    'slider', (), None,
                    Canvas, (parent,),
                    width = sliderWidth,
                    height = height,
                    bd = 2,
                    highlightthickness = 0,
                    scrollregion = (left - xPad, top, right + xPad, bottom))
                # Interaction marker
                xShift = 1
                # Shadow arrow
                self._marker = self._widget.create_polygon(-7 + xShift, 12,
                                                           7 + xShift, 12,
                                                           xShift, 0,
                                                           fill = 'black',
                                                           tags = ('marker',))
                # Arrow
                self._widget.create_polygon(-6.0, 10,
                                            6.0, 10,
                                            0, 0,
                                            fill = 'grey85',
                                            outline = 'black',
                                            tags = ('marker',))
                # The indicator
                self._widget.create_line(left, 0,
                                         right, 0,
                                         width = 2,
                                         tags = ('line',))

            self._widget.pack(side = LEFT, expand=1, fill=X)

            # Max label
            self._maxLabel = Label(parent, text = self['max'], width = 8,
                                   anchor = W)
            self._maxLabel.pack(side = LEFT)

        # Create slider
        if self['style'] == VALUATOR_MINI:

            # Create the arrow button to invoke slider
            self._arrowBtn = self.createcomponent(
                'arrowbutton',
                (), None,
                Canvas, (interior,), borderwidth = 0,
                relief = FLAT, width = 14, height = 14,
                scrollregion = (-7, -7, 7, 7))
            self._arrowBtn.pack(expand = 1, fill = BOTH)
            self._arrowBtn.create_polygon(-5, -5, 5, -5, 0, 5,
                                          fill = 'grey50',
                                          tags = 'arrow')
            self._arrowBtn.create_line(-5, 5, 5, 5,
                                       fill = 'grey50',
                                       tags = 'arrow')
            # Create the dropdown window.
            self._popup = self.createcomponent(
                'popup',
                (), None,
                Toplevel, (interior,),
                relief = RAISED, borderwidth = 2)
            self._popup.withdraw()
            self._popup.overrideredirect(1)

            # Create popup slider
            createSlider(self._popup)

            # Bind events to the arrow button.
            self._arrowBtn.bind('<1>', self._postSlider)
            self._arrowBtn.bind('<Enter>', self.highlightWidget)
            self._arrowBtn.bind('<Leave>', self.restoreWidget)
            # Need to unpost the popup if the arrow Button is unmapped (eg:
            # its toplevel window is withdrawn) while the popup slider is
            # displayed.
            self._arrowBtn.bind('<Unmap>', self._unpostSlider)

            # Bind events to the dropdown window.
            self._popup.bind('<Escape>', self._unpostSlider)
            self._popup.bind('<ButtonRelease-1>', self._widgetBtnRelease)
            self._popup.bind('<ButtonPress-1>', self._widgetBtnPress)
            self._popup.bind('<Motion>', self._widgetMove)

            self._widget.bind('<Left>', self._decrementValue)
            self._widget.bind('<Right>', self._incrementValue)
            self._widget.bind('<Shift-Left>', self._bigDecrementValue)
            self._widget.bind('<Shift-Right>', self._bigIncrementValue)
            self._widget.bind('<Home>', self._goToMin)
            self._widget.bind('<End>', self._goToMax)
        else:
            createSlider(interior)
            self._widget['command'] = self._firstScaleCommand
            self._widget.bind('<ButtonRelease-1>', self._scaleBtnRelease)
            self._widget.bind('<ButtonPress-1>', self._scaleBtnPress)

        # Check keywords and initialise options.
        self.initialiseoptions(SliderWidget)

        # Adjust relief
        if 'relief' not in kw:
            if self['style'] == VALUATOR_FULL:
                self['relief'] = FLAT

        self.updateIndicator(self['value'])

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
            self['command'](*[value] + self['commandData'])
        # Record value
        self.value = value

    def get(self):
        """
        self.get()
        Get current slider value
        """
        return self.value

    def updateIndicator(self, value):
        if self['style'] == VALUATOR_MINI:
            # Get current marker position
            percentX = (value - self['min'])/float(self['max'] - self['min'])
            newX = percentX * (self.right - self.left) + self.left
            markerX = self._getMarkerX()
            dx = newX - markerX
            self._widget.move('marker', dx, 0)
        else:
            # Update scale's variable, which update scale without
            # Calling scale's command
            self._widgetVar.set(value)

    #======================================================================

    # Private methods for slider.

    def _postSlider(self, event = None):
        self._isPosted = 1
        self._fUpdate = 0

        # Make sure that the arrow is displayed sunken.
        self.interior()['relief'] = SUNKEN
        self.update_idletasks()
        # Position popup so that marker is immediately below center of
        # Arrow button
        # Find screen space position of bottom/center of arrow button
        x = (self._arrowBtn.winfo_rootx() + self._arrowBtn.winfo_width()/2.0 -
             self.interior()['bd'])
#             int(self.interior()['bd']))
        y = self._arrowBtn.winfo_rooty() + self._arrowBtn.winfo_height()
        # Popup border width
        bd = self._popup['bd']
#        bd = int(self._popup['bd'])
        # Get width of label
        minW = self._minLabel.winfo_width()
        # Width of canvas to adjust for
        cw = (self._getMarkerX() - self.left) + self.xPad
        popupOffset = bd + minW + cw
        ch =  self._widget.winfo_height()
        sh = self.winfo_screenheight()

        # Compensate if too close to edge of screen
        if y + ch > sh and y > sh / 2:
            y = self._arrowBtn.winfo_rooty() - ch
        # Popup window
        Pmw.setgeometryanddeiconify(self._popup, '+%d+%d' % (x-popupOffset, y))

        # Grab the popup, so that all events are delivered to it, and
        # set focus to the slider, to make keyboard navigation
        # easier.
        Pmw.pushgrab(self._popup, 1, self._unpostSlider)
        self._widget.focus_set()

        # Ignore the first release of the mouse button after posting the
        # dropdown slider, unless the mouse enters the dropdown slider.
        self._fUpdate = 0
        self._fUnpost = 0
        self._firstPress = 1
        self._fPressInsde = 0

    def _updateValue(self, event):
        mouseX = self._widget.canvasx(
            event.x_root - self._widget.winfo_rootx())
        if mouseX < self.left:
            mouseX = self.left
        if mouseX > self.right:
            mouseX = self.right
        # Update value
        sf = (mouseX - self.left)/(self.right - self.left)
        newVal = sf * (self['max'] - self['min']) + self['min']
        self.set(newVal)

    def _widgetBtnPress(self, event):
        # Check behavior for this button press
        widget = self._popup
        xPos = event.x_root - widget.winfo_rootx()
        yPos = event.y_root - widget.winfo_rooty()
        fInside = ((xPos > 0) and (xPos < widget.winfo_width()) and
                   (yPos > 0) and (yPos < widget.winfo_height()))
        # Set flags based upon result
        if fInside:
            self._fPressInside = 1
            self._fUpdate = 1
            if self['preCallback']:
                self['preCallback'](*self['callbackData'])
            self._updateValue(event)
        else:
            self._fPressInside = 0
            self._fUpdate = 0

    def _widgetMove(self, event):
        if self._firstPress and not self._fUpdate:
            canvasY = self._widget.canvasy(
                event.y_root - self._widget.winfo_rooty())
            if canvasY > 0:
                self._fUpdate = 1
                if self['preCallback']:
                    self['preCallback'](*self['callbackData'])
                self._unpostOnNextRelease()
        elif self._fUpdate:
            self._updateValue(event)

    def _scaleBtnPress(self, event):
        if self['preCallback']:
            self['preCallback'](*self['callbackData'])

    def _scaleBtnRelease(self, event):
        # Do post callback if any
        if self['postCallback']:
            self['postCallback'](*self['callbackData'])

    def _widgetBtnRelease(self, event):
        # Do post callback if any
        if self._fUpdate and self['postCallback']:
            self['postCallback'](*self['callbackData'])
        if (self._fUnpost or
            (not (self._firstPress or self._fPressInside))):
            self._unpostSlider()
        # Otherwise, continue
        self._fUpdate = 0
        self._firstPress = 0
        self._fPressInside = 0

    def _unpostOnNextRelease(self, event = None):
        self._fUnpost = 1

    def _unpostSlider(self, event=None):
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

        # Raise up arrow button
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

    def _firstScaleCommand(self, val):
        """ Hack to avoid calling command on instantiation of Scale """
        self._widget['command'] = self._scaleCommand

    def _scaleCommand(self, val):
        self.set(float(val))

    # Methods to modify floater characteristics
    def setMin(self):
        self._minLabel['text'] = self.formatString % self['min']
        if self['style'] == VALUATOR_FULL:
            self._widget['from_'] = self['min']
        self.updateIndicator(self.value)

    def setMax(self):
        self._maxLabel['text'] = self.formatString % self['max']
        if self['style'] == VALUATOR_FULL:
            self._widget['to'] = self['max']
        self.updateIndicator(self.value)

    def setNumDigits(self):
        self.formatString = '%0.' + ('%d' % self['numDigits']) + 'f'
        self._minLabel['text'] = self.formatString % self['min']
        self._maxLabel['text'] = self.formatString % self['max']
        self.updateIndicator(self.value)
        self.increment = pow(10, -self['numDigits'])

    def _getMarkerX(self):
        # Get marker triangle coordinates
        c = self._widget.coords(self._marker)
        # Marker postion defined as X position of third vertex
        return c[4]

    def setRelief(self):
        self.interior()['relief'] = self['relief']

    def setBorderwidth(self):
        self.interior()['borderwidth'] = self['borderwidth']

    def setBackground(self):
        self._widget['background'] = self['background']

    def highlightWidget(self, event):
        self._arrowBtn.itemconfigure('arrow', fill = 'black')

    def restoreWidget(self, event):
        self._arrowBtn.itemconfigure('arrow', fill = 'grey50')

