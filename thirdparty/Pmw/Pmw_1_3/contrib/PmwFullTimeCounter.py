# Authors: Joe VanAndel, Greg McFarlane and Daniel Michelson

import string
import sys
import time
import Tkinter
import Pmw

class FullTimeCounter(Pmw.MegaWidget):
    """Up-down counter

    A TimeCounter is a single-line entry widget with Up and Down arrows
    which increment and decrement the Time value in the entry.  
    """

    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('autorepeat',    1,    INITOPT),
	    ('buttonaspect',  1.0,  INITOPT),
	    ('initwait',      300,  INITOPT),
	    ('labelmargin',   0,    INITOPT),
	    ('labelpos',      None, INITOPT),
	    ('max',           '',   self._max),
	    ('min',           '',   self._min),
	    ('padx',          0,    INITOPT),
	    ('pady',          0,    INITOPT),
	    ('repeatrate',    50,   INITOPT),
	    ('value',         '',   INITOPT),
	)
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

    	self.arrowDirection = {}
	self._flag = 'stopped'
	self._timerId = None

	self._createComponents()

	value = self['value']
	if value is None or value == '':
	    now = time.time()
	    value = time.strftime('%Y:%m:%d:%H:%M',time.gmtime(now))
    	self._setTimeFromStr(value)

	# Check keywords and initialise options.
	self.initialiseoptions()

    def _createComponents(self):

	# Create the components.
	interior = self.interior()

	# If there is no label, put the arrows and the entry directly
	# into the interior, otherwise create a frame for them.  In
	# either case the border around the arrows and the entry will
	# be raised (but not around the label).
	if self['labelpos'] is None:
	    frame = interior
	else:
	    frame = self.createcomponent('frame',
		    (), None,
		    Tkinter.Frame, (interior,))
	    frame.grid(column=2, row=2, sticky='nsew')
	    interior.grid_columnconfigure(2, weight=1)
	    interior.grid_rowconfigure(2, weight=1)

	frame.configure(relief = 'raised', borderwidth = 1)

	# Create the down arrow buttons.

	# Create the year down arrow.
	self._downYearArrowBtn = self.createcomponent('downyeararrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._downYearArrowBtn] = 0
	self._downYearArrowBtn.grid(column = 0, row = 2)

	# Create the month down arrow.
	self._downMonthArrowBtn = self.createcomponent('downmontharrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._downMonthArrowBtn] = 0
	self._downMonthArrowBtn.grid(column = 1, row = 2)

	# Create the day down arrow.
	self._downDayArrowBtn = self.createcomponent('downdayarrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._downDayArrowBtn] = 0
	self._downDayArrowBtn.grid(column = 2, row = 2)

	# Create the hour down arrow.
	self._downHourArrowBtn = self.createcomponent('downhourarrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._downHourArrowBtn] = 0
	self._downHourArrowBtn.grid(column = 3, row = 2)

	# Create the minute down arrow.
	self._downMinuteArrowBtn = self.createcomponent('downminutearrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._downMinuteArrowBtn] = 0
	self._downMinuteArrowBtn.grid(column = 4, row = 2)

	# Create the entry fields.

	# Create the year entry field.
	self._yearCounterEntry = self.createcomponent('yearentryfield',
		(('yearentry', 'yearentryfield_entry'),), None,
		Pmw.EntryField, (frame,), validate='integer', entry_width = 4)
	self._yearCounterEntry.grid(column = 0, row = 1, sticky = 'news')

	# Create the month entry field.
	self._monthCounterEntry = self.createcomponent('monthentryfield',
		(('monthentry', 'monthentryfield_entry'),), None,
		Pmw.EntryField, (frame,), validate='integer', entry_width = 2)
	self._monthCounterEntry.grid(column = 1, row = 1, sticky = 'news')

	# Create the day entry field.
	self._dayCounterEntry = self.createcomponent('dayentryfield',
		(('dayentry', 'dayentryfield_entry'),), None,
		Pmw.EntryField, (frame,), validate='integer', entry_width = 2)
	self._dayCounterEntry.grid(column = 2, row = 1, sticky = 'news')

	# Create the hour entry field.
	self._hourCounterEntry = self.createcomponent('hourentryfield',
		(('hourentry', 'hourentryfield_entry'),), None,
		Pmw.EntryField, (frame,), validate='integer', entry_width = 2)
	self._hourCounterEntry.grid(column = 3, row = 1, sticky = 'news')

	# Create the minute entry field.
	self._minuteCounterEntry = self.createcomponent('minuteentryfield',
		(('minuteentry', 'minuteentryfield_entry'),), None,
		Pmw.EntryField, (frame,), validate='integer', entry_width = 2)
	self._minuteCounterEntry.grid(column = 4, row = 1, sticky = 'news')

	# Create the up arrow buttons.

	# Create the year up arrow.
	self._upYearArrowBtn = self.createcomponent('upyeararrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._upYearArrowBtn] = 1
	self._upYearArrowBtn.grid(column = 0, row = 0)

	# Create the month up arrow.
	self._upMonthArrowBtn = self.createcomponent('upmontharrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._upMonthArrowBtn] = 1
	self._upMonthArrowBtn.grid(column = 1, row = 0)

	# Create the day up arrow.
	self._upDayArrowBtn = self.createcomponent('updayarrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._upDayArrowBtn] = 1
	self._upDayArrowBtn.grid(column = 2, row = 0)

	# Create the hour up arrow.
	self._upHourArrowBtn = self.createcomponent('uphourarrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._upHourArrowBtn] = 1
	self._upHourArrowBtn.grid(column = 3, row = 0)

	# Create the minute up arrow.
	self._upMinuteArrowBtn = self.createcomponent('upminutearrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)
    	self.arrowDirection[self._upMinuteArrowBtn] = 1
	self._upMinuteArrowBtn.grid(column = 4, row = 0)

	# Make it resize nicely.
	padx = self['padx']
	pady = self['pady']
	for col in range(5): # YY, MM, DD, HH, mm
	    frame.grid_columnconfigure(col, weight = 1, pad = padx)
	frame.grid_rowconfigure(0, pad = pady)
	frame.grid_rowconfigure(2, pad = pady)

	frame.grid_rowconfigure(1, weight = 1)

	# Create the label.
	self.createlabel(interior)

	# Set bindings.

	# Up year
	self._upYearArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._upYearArrowBtn: 
		s._drawArrow(button, 1))
	self._upYearArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._upYearArrowBtn: 
		s._countUp(button))
	self._upYearArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._upYearArrowBtn:
		s._stopUpDown(button))

	# Up month
	self._upMonthArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._upMonthArrowBtn: 
		s._drawArrow(button, 1))
	self._upMonthArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._upMonthArrowBtn: 
		s._countUp(button))
	self._upMonthArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._upMonthArrowBtn:
		s._stopUpDown(button))

	# Up day
	self._upDayArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._upDayArrowBtn: 
		s._drawArrow(button, 1))
	self._upDayArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._upDayArrowBtn: 
		s._countUp(button))
	self._upDayArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._upDayArrowBtn:
		s._stopUpDown(button))

	# Up hour
	self._upHourArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._upHourArrowBtn: 
		s._drawArrow(button, 1))
	self._upHourArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._upHourArrowBtn: 
		s._countUp(button))
	self._upHourArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._upHourArrowBtn:
		s._stopUpDown(button))

	# Up minute
	self._upMinuteArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._upMinuteArrowBtn: 
		s._drawArrow(button, 1))
	self._upMinuteArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._upMinuteArrowBtn: 
		s._countUp(button))
	self._upMinuteArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._upMinuteArrowBtn:
		s._stopUpDown(button))


	# Down year
	self._downYearArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._downYearArrowBtn: 
		s._drawArrow(button, 0))
	self._downYearArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._downYearArrowBtn: 
		s._countDown(button))
	self._downYearArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._downYearArrowBtn:
		s._stopUpDown(button))

	# Down month
	self._downMonthArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._downMonthArrowBtn: 
		s._drawArrow(button, 0))
	self._downMonthArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._downMonthArrowBtn: 
		s._countDown(button))
	self._downMonthArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._downMonthArrowBtn:
		s._stopUpDown(button))

	# Down day
	self._downDayArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._downDayArrowBtn: 
		s._drawArrow(button, 0))
	self._downDayArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._downDayArrowBtn: 
		s._countDown(button))
	self._downDayArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._downDayArrowBtn:
		s._stopUpDown(button))

	# Down hour
	self._downHourArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._downHourArrowBtn: 
		s._drawArrow(button, 0))
	self._downHourArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._downHourArrowBtn: 
		s._countDown(button))
	self._downHourArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._downHourArrowBtn:
		s._stopUpDown(button))

	# Down minute
	self._downMinuteArrowBtn.bind('<Configure>', 
		lambda  event, s=self,button=self._downMinuteArrowBtn: 
		s._drawArrow(button, 0))
	self._downMinuteArrowBtn.bind('<1>', 
    	    	lambda event, s=self,button=self._downMinuteArrowBtn: s._countDown(button))
	self._downMinuteArrowBtn.bind('<Any-ButtonRelease-1>', 
		lambda event, s=self, button=self._downMinuteArrowBtn:
		s._stopUpDown(button))


	self._yearCounterEntry.bind('<Return>', self.invoke)
	self._monthCounterEntry.bind('<Return>', self.invoke)
	self._dayCounterEntry.bind('<Return>', self.invoke)
	self._hourCounterEntry.bind('<Return>', self.invoke)
	self._minuteCounterEntry.bind('<Return>', self.invoke)

	self._yearCounterEntry.bind('<Configure>', self._resizeArrow)
	self._monthCounterEntry.bind('<Configure>', self._resizeArrow)
	self._dayCounterEntry.bind('<Configure>', self._resizeArrow)
	self._hourCounterEntry.bind('<Configure>', self._resizeArrow)
	self._minuteCounterEntry.bind('<Configure>', self._resizeArrow)

    def _drawArrow(self, arrow, direction):
	arrow.delete('arrow')

	fg = self._yearCounterEntry.cget('entry_foreground')

	bw = (string.atoi(arrow['borderwidth']) +
		string.atoi(arrow['highlightthickness'])) / 2
	h = string.atoi(arrow['height']) + 2 * bw
	w =  string.atoi(arrow['width']) + 2 * bw

	if direction == 0:
    	     # down arrow
	     arrow.create_polygon(
		 0.25 * w + bw, 0.25 * h + bw,
	         0.50 * w + bw, 0.75 * h + bw,
	         0.75 * w + bw, 0.25 * h + bw,
		 fill=fg, tag='arrow')
	else:
	     arrow.create_polygon(
	         0.25 * w + bw, 0.75 * h + bw,
		 0.50 * w + bw, 0.25 * h + bw,
	         0.75 * w + bw, 0.75 * h + bw,
		 fill=fg, tag='arrow')

    def _resizeArrow(self, event = None):
	for btn in (self._upYearArrowBtn, self._upMonthArrowBtn, 
		    self._upDayArrowBtn, self._upHourArrowBtn, 
		    self._upMinuteArrowBtn, self._downYearArrowBtn,
		    self._downMonthArrowBtn, self._downDayArrowBtn,
		    self._downHourArrowBtn, self._downMinuteArrowBtn):
	    bw = (string.atoi(btn['borderwidth']) + \
		    string.atoi(btn['highlightthickness']))
	    newHeight = self._yearCounterEntry.winfo_reqheight() - 2 * bw
	    newWidth = newHeight * self['buttonaspect']
	    btn.configure(width=newWidth, height=newHeight)
	    self._drawArrow(btn, self.arrowDirection[btn])

    def _min(self):
	self._minVal = None

    def _max(self):
	self._maxVal = None

    def _setTimeFromStr(self, str):
        list = string.split(str, ':')
	if len(list) != 5:
	    raise ValueError, 'invalid value: ' + str

	self._year = string.atoi(list[0])
	self._month = string.atoi(list[1])
	self._day = string.atoi(list[2])
	self._hour = string.atoi(list[3])
	self._minute = string.atoi(list[4])

    	self._setHMS()

    def getstring(self):
    	return '%04d:%02d:%02d:%02d:%02d' % (self._year, self._month, 
					     self._day, self._hour, 
					     self._minute)

    def getint(self):
	pass

    def _countUp(self, button):
	self._relief = self._upYearArrowBtn.cget('relief')
	button.configure(relief='sunken')
	if button == self._upYearArrowBtn: datetype = "year"
	elif button == self._upMonthArrowBtn: datetype = "month"
	elif button == self._upDayArrowBtn: datetype = "day"
	elif button == self._upHourArrowBtn: datetype = "hour"
	elif button == self._upMinuteArrowBtn: datetype = "minute"
	self._count(1, datetype, 'start')

    def _countDown(self, button):
	self._relief = self._downYearArrowBtn.cget('relief')
	button.configure(relief='sunken')
	if button == self._downYearArrowBtn: datetype = "year"
	elif button == self._downMonthArrowBtn: datetype = "month"
	elif button == self._downDayArrowBtn: datetype = "day"
	elif button == self._downHourArrowBtn: datetype = "hour"
	elif button == self._downMinuteArrowBtn: datetype = "minute"
	self._count(-1, datetype, 'start')

    def _count(self, factor, datetype, newFlag=None):
	if newFlag != 'force':
	  if newFlag is not None:
	    self._flag = newFlag

	  if self._flag == 'stopped':
	    return

	if datetype == "year": self._year = self._year + factor
	elif datetype == "month": self._month = self._month + factor
	elif datetype == "day": self._day = self._day + factor
	elif datetype == "hour": self._hour = self._hour + factor
	elif datetype == "minute": self._minute = self._minute + factor
	secs = time.mktime((self._year, self._month, self._day, self._hour, 
			   self._minute, 0, 0, 0, -1))
	tt = time.localtime(secs) # NOT gmtime!

	self._year = tt[0]
	self._month = tt[1]
	self._day = tt[2]
	self._hour = tt[3]
	self._minute = tt[4]
	self._setHMS()

	if newFlag != 'force':
	  if self['autorepeat']:
	    if self._flag == 'start':
	      delay = self['initwait']
	      self._flag = 'running'
	    else:
	      delay = self['repeatrate']
	    self._timerId = self.after(
		delay, lambda self=self, factor=factor, datetype=datetype: 
		  self._count(factor, datetype, 'running'))

    def _setHMS(self):
        self._yearCounterEntry.setentry('%04d' % self._year)
        self._monthCounterEntry.setentry('%02d' % self._month)
        self._dayCounterEntry.setentry('%02d' % self._day)
        self._hourCounterEntry.setentry('%02d' % self._hour)
        self._minuteCounterEntry.setentry('%02d' % self._minute)

    def _stopUpDown(self, button):
        if self._timerId is not None:
            self.after_cancel(self._timerId)
	    self._timerId = None
        button.configure(relief=self._relief)
        self._flag = 'stopped'

    def invoke(self, event = None):
        cmd = self['command']
        if callable(cmd):
	    cmd()

    def destroy(self):
        if self._timerId is not None:
            self.after_cancel(self._timerId)
	    self._timerId = None
        Pmw.MegaWidget.destroy(self)

if __name__=="__main__":

    def showString():
        stringVal = _time.getstring()
        print stringVal

    root = Tkinter.Tk()
    Pmw.initialise(root)
    root.title('FullTimeCounter')

    exitButton = Tkinter.Button(root, text = 'Exit', command = root.destroy)
    exitButton.pack(side = 'bottom')

    _time = FullTimeCounter(root,
            labelpos = 'n',
            label_text = 'YYYY:MM:DD:HH:mm')
    _time.pack(fill = 'both', expand = 1, padx=10, pady=5)

    button = Tkinter.Button(root, text = 'Show', command = showString)
    button.pack()
    root.mainloop()
