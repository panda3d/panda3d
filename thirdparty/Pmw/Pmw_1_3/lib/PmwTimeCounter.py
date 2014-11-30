# Authors: Joe VanAndel and Greg McFarlane

import string
import sys
import time
import Tkinter
import Pmw

class TimeCounter(Pmw.MegaWidget):
    """Up-down counter

    A TimeCounter is a single-line entry widget with Up and Down arrows
    which increment and decrement the Time value in the entry.  
    """

    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('autorepeat',    1,    None),
	    ('buttonaspect',  1.0,  INITOPT),
	    ('command',       None, None),
	    ('initwait',      300,  None),
	    ('labelmargin',   0,    INITOPT),
	    ('labelpos',      None, INITOPT),
	    ('max',           None, self._max),
	    ('min',           None, self._min),
	    ('padx',          0,    INITOPT),
	    ('pady',          0,    INITOPT),
	    ('repeatrate',    50,   None),
	    ('value',         None, INITOPT),
	)
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

    	self.arrowDirection = {}
	self._flag = 'stopped'
	self._timerId = None

	self._createComponents(kw)

	value = self['value']
	if value is None:
	    now = time.time()
	    value = time.strftime('%H:%M:%S', time.localtime(now))
    	self.setvalue(value)

	# Check keywords and initialise options.
	self.initialiseoptions()

    def _createComponents(self, kw):

	# Create the components.
	interior = self.interior()

	# If there is no label, put the arrows and the entry directly
	# into the interior, otherwise create a frame for them.  In
	# either case the border around the arrows and the entry will
	# be raised (but not around the label).
	if self['labelpos'] is None:
	    frame = interior
            if not kw.has_key('hull_relief'):
                frame.configure(relief = 'raised')
            if not kw.has_key('hull_borderwidth'):
                frame.configure(borderwidth = 1)
	else:
	    frame = self.createcomponent('frame',
		    (), None,
		    Tkinter.Frame, (interior,),
                    relief = 'raised', borderwidth = 1)
	    frame.grid(column=2, row=2, sticky='nsew')
	    interior.grid_columnconfigure(2, weight=1)
	    interior.grid_rowconfigure(2, weight=1)

	# Create the down arrow buttons.

	# Create the hour down arrow.
	self._downHourArrowBtn = self.createcomponent('downhourarrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._downHourArrowBtn] = 'down'
	self._downHourArrowBtn.grid(column = 0, row = 2)

	# Create the minute down arrow.
	self._downMinuteArrowBtn = self.createcomponent('downminutearrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._downMinuteArrowBtn] = 'down'
	self._downMinuteArrowBtn.grid(column = 1, row = 2)

	# Create the second down arrow.
	self._downSecondArrowBtn = self.createcomponent('downsecondarrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._downSecondArrowBtn] = 'down'
	self._downSecondArrowBtn.grid(column = 2, row = 2)

	# Create the entry fields.

	# Create the hour entry field.
	self._hourCounterEntry = self.createcomponent('hourentryfield',
		(('hourentry', 'hourentryfield_entry'),), None,
		Pmw.EntryField, (frame,), validate='integer', entry_width = 2)
	self._hourCounterEntry.grid(column = 0, row = 1, sticky = 'news')

	# Create the minute entry field.
	self._minuteCounterEntry = self.createcomponent('minuteentryfield',
		(('minuteentry', 'minuteentryfield_entry'),), None,
		Pmw.EntryField, (frame,), validate='integer', entry_width = 2)
	self._minuteCounterEntry.grid(column = 1, row = 1, sticky = 'news')

	# Create the second entry field.
	self._secondCounterEntry = self.createcomponent('secondentryfield',
		(('secondentry', 'secondentryfield_entry'),), None,
		Pmw.EntryField, (frame,), validate='integer', entry_width = 2)
	self._secondCounterEntry.grid(column = 2, row = 1, sticky = 'news')

	# Create the up arrow buttons.

	# Create the hour up arrow.
	self._upHourArrowBtn = self.createcomponent('uphourarrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._upHourArrowBtn] = 'up'
	self._upHourArrowBtn.grid(column = 0, row = 0)

	# Create the minute up arrow.
	self._upMinuteArrowBtn = self.createcomponent('upminutearrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._upMinuteArrowBtn] = 'up'
	self._upMinuteArrowBtn.grid(column = 1, row = 0)

	# Create the second up arrow.
	self._upSecondArrowBtn = self.createcomponent('upsecondarrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._upSecondArrowBtn] = 'up'
	self._upSecondArrowBtn.grid(column = 2, row = 0)

	# Make it resize nicely.
	padx = self['padx']
	pady = self['pady']
	for col in range(3):
	    frame.grid_columnconfigure(col, weight = 1, pad = padx)
	frame.grid_rowconfigure(0, pad = pady)
	frame.grid_rowconfigure(2, pad = pady)

	frame.grid_rowconfigure(1, weight = 1)

	# Create the label.
	self.createlabel(interior)

	# Set bindings.

	# Up hour
	self._upHourArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._upHourArrowBtn: 
		s._drawArrow(button, 'up'))

	self._upHourArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._upHourArrowBtn: 
		s._countUp(button, 3600))

	self._upHourArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._upHourArrowBtn:
		s._stopUpDown(button))

	# Up minute
	self._upMinuteArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._upMinuteArrowBtn: 
		s._drawArrow(button, 'up'))
	    

	self._upMinuteArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._upMinuteArrowBtn: 
		s._countUp(button, 60))

	self._upMinuteArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._upMinuteArrowBtn:
		s._stopUpDown(button))

	# Up second
	self._upSecondArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._upSecondArrowBtn: 
		s._drawArrow(button, 'up'))
	    

	self._upSecondArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._upSecondArrowBtn: 
		s._countUp(button, 1))

	self._upSecondArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._upSecondArrowBtn:
		s._stopUpDown(button))

	# Down hour
	self._downHourArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._downHourArrowBtn: 
		s._drawArrow(button, 'down'))

	self._downHourArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._downHourArrowBtn: 
		s._countDown(button, 3600))
	self._downHourArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._downHourArrowBtn:
		s._stopUpDown(button))


	# Down minute
	self._downMinuteArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._downMinuteArrowBtn: 
		s._drawArrow(button, 'down'))

	self._downMinuteArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._downMinuteArrowBtn:
                s._countDown(button, 60))
	self._downMinuteArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._downMinuteArrowBtn:
		s._stopUpDown(button))

	# Down second
	self._downSecondArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._downSecondArrowBtn: 
		s._drawArrow(button, 'down'))

	self._downSecondArrowBtn.bind('<1>', 
    	    	lambda event, s=self, button=self._downSecondArrowBtn: 
		s._countDown(button,1))
	self._downSecondArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._downSecondArrowBtn:
		s._stopUpDown(button))

	self._hourCounterEntry.component('entry').bind(
                '<Return>', self._invoke)
	self._minuteCounterEntry.component('entry').bind(
        	'<Return>', self._invoke)
	self._secondCounterEntry.component('entry').bind(
        	'<Return>', self._invoke)

	self._hourCounterEntry.bind('<Configure>', self._resizeArrow)
	self._minuteCounterEntry.bind('<Configure>', self._resizeArrow)
	self._secondCounterEntry.bind('<Configure>', self._resizeArrow)

    def _drawArrow(self, arrow, direction):
        Pmw.drawarrow(arrow, self['hourentry_foreground'], direction, 'arrow')

    def _resizeArrow(self, event = None):
	for btn in (self._upHourArrowBtn, self._upMinuteArrowBtn,
		self._upSecondArrowBtn,
		self._downHourArrowBtn,
		self._downMinuteArrowBtn, self._downSecondArrowBtn):
	    bw = (string.atoi(btn['borderwidth']) +
		    string.atoi(btn['highlightthickness']))
	    newHeight = self._hourCounterEntry.winfo_reqheight() - 2 * bw
	    newWidth = int(newHeight * self['buttonaspect'])
	    btn.configure(width=newWidth, height=newHeight)
	    self._drawArrow(btn, self.arrowDirection[btn])

    def _min(self):
	min = self['min']
        if min is None:
	    self._minVal = 0
	else:
	    self._minVal = Pmw.timestringtoseconds(min)

    def _max(self):
	max = self['max']
	if max is None:
	    self._maxVal = None
	else:
	    self._maxVal = Pmw.timestringtoseconds(max)

    def getvalue(self):
        return self.getstring()

    def setvalue(self, text):
        list = string.split(text, ':')
	if len(list) != 3:
	    raise ValueError, 'invalid value: ' + text

	self._hour = string.atoi(list[0])
	self._minute = string.atoi(list[1])
	self._second = string.atoi(list[2]) 

    	self._setHMS()

    def getstring(self):
    	return '%02d:%02d:%02d' % (self._hour, self._minute, self._second)

    def getint(self):
    	return self._hour * 3600 + self._minute * 60 + self._second

    def _countUp(self, button, increment):
	self._relief = self._upHourArrowBtn.cget('relief')
	button.configure(relief='sunken')
	self._count(1, 'start', increment)

    def _countDown(self, button, increment):

	self._relief = self._downHourArrowBtn.cget('relief')
	button.configure(relief='sunken')
	self._count(-1, 'start', increment)

    def increment(self, seconds = 1):
	self._count(1, 'force', seconds)

    def decrement(self, seconds = 1):
	self._count(-1, 'force', seconds)

    def _count(self, factor, newFlag = None, increment = 1):
	if newFlag != 'force':
	  if newFlag is not None:
	    self._flag = newFlag

	  if self._flag == 'stopped':
	    return

	value = (string.atoi(self._hourCounterEntry.get()) *3600) + \
	      (string.atoi(self._minuteCounterEntry.get()) *60) + \
	      string.atoi(self._secondCounterEntry.get()) + \
	      factor * increment
	min = self._minVal
	max = self._maxVal
	if value < min:
	  value = min
	if max is not None and value > max:
	  value = max

	self._hour = value /3600
	self._minute = (value - (self._hour*3600)) / 60
	self._second = value - (self._hour*3600) - (self._minute*60)
	self._setHMS()

	if newFlag != 'force':
	  if self['autorepeat']:
	    if self._flag == 'start':
	      delay = self['initwait']
	      self._flag = 'running'
	    else:
	      delay = self['repeatrate']
	    self._timerId = self.after(
		delay, lambda self=self, factor=factor,increment=increment: 
		  self._count(factor,'running', increment))

    def _setHMS(self):
        self._hourCounterEntry.setentry('%02d' % self._hour)
        self._minuteCounterEntry.setentry('%02d' % self._minute)
        self._secondCounterEntry.setentry('%02d' % self._second)

    def _stopUpDown(self, button):
        if self._timerId is not None:
            self.after_cancel(self._timerId)
	    self._timerId = None
        button.configure(relief=self._relief)
        self._flag = 'stopped'

    def _invoke(self, event):
        cmd = self['command']
        if callable(cmd):
	    cmd()

    def invoke(self):
        cmd = self['command']
        if callable(cmd):
	    return cmd()

    def destroy(self):
        if self._timerId is not None:
            self.after_cancel(self._timerId)
	    self._timerId = None
        Pmw.MegaWidget.destroy(self)
