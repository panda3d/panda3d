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

SLIDER_MINI_WIDTH = 22
SLIDER_MINI_HEIGHT = 18

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
        left = -(halfWidth - 2)
        right = halfWidth - 2
        halfHeight = height/2.0
        top = -(halfHeight - 2)
        bottom = halfHeight - 2

        print left, right,bottom,top
        
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
