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

SLIDER_FULL = 'full'
SLIDER_MINI = 'mini'

SLIDER_FULL_WIDTH = 50
SLIDER_FULL_HEIGHT = 25

SLIDER_MINI_WIDTH = 16
SLIDER_MINI_HEIGHT = 16

globalClock = ClockObject.getGlobalClock()


class Slider(Valuator):
    """
    Valuator widget which includes an min/max slider and an entry for setting
    floating point values in a range
    """
    def __init__(self, parent = None, **kw):
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('style',             SLIDER_FULL,    INITOPT),
            )
        self.defineoptions(kw, optiondefs)
        Valuator.__init__(self, parent)
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

    def packValuator(self):
        if self['style'] == SLIDER_FULL:
            if self._label:
                self._label.grid(row = 0, col = 0, sticky = EW)
            self._entry.grid(row = 0, col = 1, sticky = EW)
            self._valuator.grid(row = 1, columnspan = 2,
                                padx = 2, pady = 2)
            self.interior().columnconfigure(0, weight = 1)
        else:
            if self._label:
                self._label.grid(row=0,col=0, sticky = EW)
            self._entry.grid(row=0,col=1, sticky = EW)
            self._valuator.grid(row=0,col=2, padx = 2, pady = 2)
            self.interior().columnconfigure(0, weight = 1)


class SliderWidget(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):
        #define the megawidget options
        INITOPT = Pmw.INITOPT
        optiondefs = (
            # Appearance
            ('style',           SLIDER_MINI,    INITOPT),
            ('width',           SLIDER_MINI_WIDTH,   INITOPT),
            ('height',          SLIDER_MINI_HEIGHT,  INITOPT),
            ('relief',          SUNKEN,         self.setRelief),
            ('borderwidth',     2,              self.setBorderwidth),
            ('background',      'white',        self.setBackground),
            # Behavior
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
        
        # Initialize the superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Set up some local and instance variables        
        # Create the components
        interior = self.interior()

        # Current value
        self.value = self['value']

        # Base slider size on style, if size not specified, 
        if not self['width']:
            if self['style'] == SLIDER_FULL_SIZE:
                width = SLIDER_FULL_WIDTH
            else:
                width = SLIDER_MINI_WIDTH
        else:
            width = self['width']

        if not self['height']:
            if self['style'] == SLIDER_FULL_SIZE:
                height = SLIDER_FULL_HEIGHT
            else:
                height = SLIDER_MINI_HEIGHT
        else:
            height = self['height']

        halfWidth = width/2.0
        halfHeight = height/2.0
        left = -(halfWidth - 2)
        right = halfWidth - 2
        top = -(halfHeight - 2)
        bottom = halfHeight - 2

        # The canvas 
        self._canvas = self.createcomponent('canvas', (), None,
                                            Canvas, (interior,),
                                            width = width,
                                            height = height,
                                            background = self['background'],
                                            highlightthickness = 0,
                                            scrollregion = (-halfWidth,
                                                            -halfHeight,
                                                            halfWidth,
                                                            halfHeight))
        self._canvas.pack(expand = 1, fill = BOTH)

        self._canvas.create_polygon(left,top,
                                    0, bottom,
                                    right, top,
                                    fill = '#A0A0A0',
                                    tags = ('slider',))

        # The indicator
        self._canvas.create_line(left, bottom,
                                 right, bottom,
                                 width = 2)

        # Add event bindings
        self._canvas.bind('<ButtonPress-1>', self.mouseDown)
        self._canvas.bind('<B1-Motion>', self.updateSliderSF)
        self._canvas.bind('<ButtonRelease-1>', self.mouseUp)
        self._canvas.bind('<Enter>', self.highlightWidget)
        self._canvas.bind('<Leave>', self.restoreWidget)

        # Make sure input variables processed 
        self.initialiseoptions(SliderWidget)

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

    def updateIndicator(self, value):
        # Nothing visible to update on this type of widget
        pass
    
    def get(self):
        """
        self.get()
        Get current slider value
        """
        return self.value

    ## Canvas callback functions
    # Slider velocity controller
    def mouseDown(self,event):
        """ Begin mouse interaction """
        # Exectute user redefinable callback function (if any)
        if self['preCallback']:
            apply(self['preCallback'], self['callbackData'])
        self.velocitySF = 0.0
        self.updateTask = taskMgr.add(self.updateSliderTask,
                                        'updateSlider')
        self.updateTask.lastTime = globalClock.getFrameTime()

    def updateSliderTask(self, state):
        """
        Update sliderWidget value based on current scaleFactor
        Adjust for time to compensate for fluctuating frame rates
        """
        currT = globalClock.getFrameTime()
        dt = currT - state.lastTime
        self.set(self.value + self.velocitySF * dt)
        state.lastTime = currT
        return Task.cont

    def updateSliderSF(self, event):
        """
        Update velocity scale factor based of mouse distance from origin
        """
        x = self._canvas.canvasx(event.x)
        y = self._canvas.canvasy(event.y)
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

    def setNumDigits(self):
        """
        Adjust minimum exponent to use in velocity task based
        upon the number of digits to be displayed in the result
        """
        self.minExp = math.floor(-self['numDigits']/
                                 math.log10(Valuator.sfBase))        

    # Methods to modify slider characteristics    
    def setRelief(self):
        self.interior()['relief'] = self['relief']

    def setBorderwidth(self):
        self.interior()['borderwidth'] = self['borderwidth']

    def setBackground(self):
        self._canvas['background'] = self['background']

    def highlightWidget(self, event):
        self._canvas.itemconfigure('slider', fill = 'black')

    def restoreWidget(self, event):
        self._canvas.itemconfigure('slider', fill = '#A0A0A0')
  
if __name__ == '__main__':
    tl = Toplevel()
    d = Slider(tl)
    d2 = Slider(tl, slider_numSegments = 12, max = 360,
              slider_fRollover = 0, value = 180)
    d3 = Slider(tl, slider_numSegments = 12, max = 90, min = -90,
              slider_fRollover = 0)
    d4 = Slider(tl, slider_numSegments = 16, max = 256,
              slider_fRollover = 0)
    d.pack(expand = 1, fill = X)
    d2.pack(expand = 1, fill = X)
    d3.pack(expand = 1, fill = X)
    d4.pack(expand = 1, fill = X)



class PopupSliderWidget(Pmw.MegaToplevel):
    def __init__(self, parent = None, **kw):
        optiondefs = (
            ('width',                    150,            None),
            ('height',                   25,             None),
            ('xoffset',                  0,              None), # pixels
            ('yoffset',                  1,              None), # pixels
            )
        self.defineoptions(kw, optiondefs)
        Pmw.MegaToplevel.__init__(self, parent)
        interior = self.interior()
        self.withdraw()
        self.overrideredirect(1)
        interior['relief'] = RAISED
        interior['borderwidth'] = 2
        left = -self['width']/2.0
        right = self['width']/2.0
        top = -10
        bottom = top + self['height']
        self._canvas = self.createcomponent('canvas', (), None,
                                            Canvas, (interior,),
                                            relief = FLAT,
                                            width = self['width'],
                                            height = self['height'],
                                            highlightthickness = 0,
                                            scrollregion = (left, top,
                                                            right, bottom)
                                            )
        self._canvas.pack(expand = 1, fill = BOTH)
        self.marker = self._canvas.create_polygon(-6.9 + 1,12,
                                    6.9+1,12,
                                    1, 0,
                                    fill = 'black',
                                    tags = ('slider',))
        self._canvas.create_polygon(-5.75,10,
                                    5.75,10,
                                    0, 0,
                                    fill = 'grey85',
                                    outline = 'black',
                                    tags = ('slider',))
        # The indicator
        self.lineLeft = lineLeft = left + 10
        self.lineRight = lineRight = right -10
        self._canvas.create_line(lineLeft, 1,
                                 lineRight, 1,
                                 width = 2)

        self.b = Button(interior, text = 'hello')
        self.b['command'] = self.withdraw
        self.b.pack()
        self.initialiseoptions(PopupSliderWidget)
        
    def showPopup(self, widget):
        x = widget.winfo_rootx() + widget.winfo_width() - self['width']
        y = widget.winfo_rooty() + widget.winfo_height()
        Pmw.setgeometryanddeiconify(self, '%dx%d+%d+%d' %
                                    (self['width'], self['height'],x,y))

"""

pw = PopupSliderWidget()
tl = Toplevel()
b = Button(tl, text = 'V')
b.pack()

fCrossedLine = 0
def move(event):
    global fCrossedLine
    if fCrossedLine:
        newX = pw._canvas.canvasx(event.x_root) - pw.winfo_rootx()
        if newX < pw.lineLeft:
            newX = pw.lineLeft
        elif newX > pw.lineRight:
            newX = pw.lineRight
        print (newX - pw.lineLeft)/(pw.lineRight - pw.lineLeft)
        startX = getX()
        dx = newX - startX
        pw._canvas.move('slider', dx, 0)
    else:
        if event.y_root >= pw.winfo_rooty() + 10:
            fCrossedLine = 1

def getX():
    c = pw._canvas.coords(pw.marker)
    return c[4]

def press(event):
    global fCrossedLine
    print 'press'
    fCrossedLine = 0
    pw.showPopup(b)
    b.bind('<Motion>', move)

def unpostCanvas(event):
    print 'unpostCanvas', event.x_root, pw._canvas.winfo_rootx()
    if event.x_root < pw._canvas.winfo_rootx():
        print 'blah'
    Pmw.popgrab(pw._canvas)
    pw._canvas.withdraw()

def popupPress(event):
    global fCrossedLine
    print 'popupPress'
    fCrossedLine = 1
    pw._canvas.bind('<Motion>', move)

def release(event):
    if fCrossedLine:
        pw.withdraw()
        b.unbind('<Motion>')
        pw._canvas.unbind('<Motion>')
    else:
        Pmw.pushgrab(pw._canvas, 1, unpostCanvas)
        pw._canvas.focus_set()

b.bind('<ButtonPress-1>', press)
b.bind('<ButtonRelease-1>', release)
pw._canvas.bind('<ButtonPress-1>', popupPress)
pw._canvas.bind('<ButtonRelease-1>', release)


"""


# Based on Pmw ComboBox code.
class PopupSlider(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('dropdown',           1,          INITOPT),
	    ('buttonaspect',       1.0,        INITOPT),
	    ('fliparrow',          0,          INITOPT),
	    ('labelmargin',        0,          INITOPT),
	    ('labelpos',           None,       INITOPT),
            # Behavior
            # Initial value of slider, use self.set to change value
            ('value',           0.0,            INITOPT),
            ('numDigits',       2,              self._setNumDigits),
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

        # Interaction flags
        self._fUpdate = 0
        self._fUnpost = 0
        self._firstPress = 1
        self._fPressInsde = 0

	self._entryfield = self.createcomponent('entryfield',
		(('entry', 'entryfield_entry'),), None,
		Pmw.EntryField, (interior,))
	self._entryfield.grid(column=2, row=2, sticky='nsew')
	interior.grid_columnconfigure(2, weight = 1)
	self._entryWidget = self._entryfield.component('entry')

        # Slider dimensions
        width = 100
        xPad = 10
        canvasWidth = width + 2 * xPad
        height = 20
        self.left = left = -(width/2.0)
        self.right = right = (width/2.0)
        top = -5
        bottom = top + height

        # Create slider
	if self['dropdown']:
	    self._isPosted = 0
            interior.grid_rowconfigure(2, weight = 1)

	    # Create the arrow button.
	    self._arrowBtn = self.createcomponent('arrowbutton',
		    (), None,
		    Canvas, (interior,), borderwidth = 2,
		    relief = 'raised',
		    width = 16, height = 16)
	    self._arrowBtn.grid(column=3, row=2)
	    self._arrowRelief = self._arrowBtn.cget('relief')

	    # Create the label.
	    self.createlabel(interior, childCols=2)

	    # Create the dropdown window.
	    self._popup = self.createcomponent(
                'popup',
                (), None,
                Toplevel, (interior,),
                relief = RAISED, borderwidth = 2)
	    self._popup.withdraw()
	    self._popup.overrideredirect(1)

	    # Create the canvas inside the dropdown window.
            # Min label
            self._minLabel = Label(self._popup, text = 'MINAAAAAA')
            self._minLabel.pack(side = LEFT)
            # Slider
            self._canvas = self.createcomponent(
                'canvas', (), None,
                Canvas, (self._popup,),
                width = canvasWidth,
                height = height,
                bd = 3,
                highlightthickness = 0,
                scrollregion = (left - xPad, top, right + xPad, bottom))
	    self._canvas.pack(side = LEFT, expand=1, fill='both')
            # Max label
            self._maxLabel = Label(self._popup, text = 'MAX')
            self._maxLabel.pack(side = LEFT)
            
	    # Bind events to the arrow button.
	    self._arrowBtn.bind('<1>', self._postCanvas)
	    self._arrowBtn.bind('<Configure>', self._drawArrow)

	    # Bind events to the dropdown window.
	    self._popup.bind('<Escape>', self._unpostCanvas)
	    self._popup.bind('<ButtonRelease-1>', self._dropdownBtnRelease)
	    self._popup.bind('<ButtonPress-1>', self._dropdownBtnPress)
            self._popup.bind('<Motion>', self._dropdownMove)

	    # Bind events to the Tk listbox.
	    #self._canvas.bind('<Enter>', self._unpostOnNextRelease)

	    # Bind events to the Tk entry widget.
	    self._entryWidget.bind('<Configure>', self._resizeArrow)

            # Need to unpost the popup if the entryfield is unmapped (eg: 
            # its toplevel window is withdrawn) while the popup canvas is
            # displayed.
            self._entryWidget.bind('<Unmap>', self._unpostCanvas)
	else:
	    # Create the slider below the entry field.
            self._canvas = self.createcomponent(
                'canvas', (), None,
                Canvas, (interior,),
                width = canvasWidth,
                height = height,
                highlightthickness = 0,
                scrollregion = (left - xPad, top, right + xPad, bottom))
	    self._canvas.grid(column=2, row=3, sticky='nsew')

	    # The scrolled canvas should expand vertically.
	    interior.grid_rowconfigure(3, weight = 1)

	    # Create the label.
	    self.createlabel(interior, childRows=2)

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
                                 width = 2)

        
	# Check keywords and initialise options.
	self.initialiseoptions(PopupSlider)

    def destroy(self):
	if self['dropdown'] and self._isPosted:
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

    def updateIndicator(self, value):
        # Nothing visible to update on this type of widget
        pass
    
    def get(self):
        """
        self.get()
        Get current slider value
        """
        return self.value

    #======================================================================

    # Private methods for dropdown canvas.

    def _setNumDigits(self):
        pass

    def _drawArrow(self, event=None, sunken=0):
        arrow = self._arrowBtn
	if sunken:
	    self._arrowRelief = arrow.cget('relief')
	    arrow.configure(relief = 'sunken')
	else:
	    arrow.configure(relief = self._arrowRelief)

	if self._isPosted and self['fliparrow']:
            direction = 'up'
        else:
            direction = 'down'
        Pmw.drawarrow(arrow, self['entry_foreground'], direction, 'arrow')

    def _postCanvas(self, event = None):
        self._isPosted = 1
        self._fUpdate = 0
        self._drawArrow(sunken=1)

        # Make sure that the arrow is displayed sunken.
        self.update_idletasks()

        x = self._entryfield.winfo_rootx()
        y = self._entryfield.winfo_rooty() + self._entryfield.winfo_height()
        w = self._entryfield.winfo_width() + self._arrowBtn.winfo_width()
        minW = self._minLabel.winfo_width()
        cw =  self._canvas.winfo_width()
        maxW = self._maxLabel.winfo_width()
        pw = minW + cw + maxW
        ch =  self._canvas.winfo_height()
        sh = self.winfo_screenheight()

        # Compensate if too close to edge of screen
        if y + ch > sh and y > sh / 2:
            y = self._entryfield.winfo_rooty() - ch

        Pmw.setgeometryanddeiconify(self._popup, '+%d+%d' % (x + w - pw, y))

        # Grab the popup, so that all events are delivered to it, and
        # set focus to the canvas, to make keyboard navigation
        # easier.
        Pmw.pushgrab(self._popup, 1, self._unpostCanvas)
        self._canvas.focus_set()

        self._drawArrow()

        # Ignore the first release of the mouse button after posting the
        # dropdown canvas, unless the mouse enters the dropdown canvas.
        self._fUpdate = 0
        self._fUnpost = 0
        self._firstPress = 1
        self._fPressInsde = 0

    def _updateValue(self,event):
        canvasX = self._canvas.canvasx(
            event.x_root - self._canvas.winfo_rootx())
        if canvasX < self.left:
            canvasX = self.left
        if canvasX > self.right:
            canvasX = self.right
        # Get current marker position
        currX = self._getMarkerX()
        dx = canvasX - currX
        self._canvas.move('slider', dx, 0)

    def _dropdownBtnPress(self, event):
        self._fUpdate = 1
        self._fPressInside = 1
        self._updateValue(event)
            
    def _dropdownMove(self, event):
        if self._firstPress and not self._fUpdate:
            canvasY = self._canvas.canvasy(
                event.y_root - self._canvas.winfo_rooty())
            if canvasY > 0:
                self._fUpdate = 1
                self._unpostOnNextRelease()
        elif self._fUpdate:
            self._updateValue(event)

    def _dropdownBtnRelease(self, event):
        if (self._fUnpost or
            (not (self._firstPress or self._fPressInside))):
            self._unpostCanvas()
        # Otherwise, continue
        self._fUpdate = 0
        self._firstPress = 0
        self._fPressInside = 0

    def _unpostOnNextRelease(self, event = None):
	self._fUnpost = 1

    def _resizeArrow(self, event):
	bw = (string.atoi(self._arrowBtn['borderwidth']) + 
		string.atoi(self._arrowBtn['highlightthickness']))
	newHeight = self._entryfield.winfo_reqheight() - 2 * bw
	newWidth = int(newHeight * self['buttonaspect'])
	self._arrowBtn.configure(width=newWidth, height=newHeight)
	self._drawArrow()

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
	self._drawArrow()

    def _getMarkerX(self):
        # Get marker triangle coordinates
        c = self._canvas.coords(self._marker)
        # Marker postion defined as X position of third vertex
        return c[4]
        
Pmw.forwardmethods(PopupSlider, Pmw.EntryField, '_entryfield')
