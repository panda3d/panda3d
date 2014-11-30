import string
import sys
import types
import Tkinter
import Pmw

class Counter(Pmw.MegaWidget):

    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('autorepeat',     1,             None),
	    ('buttonaspect',   1.0,           INITOPT),
	    ('datatype',       'numeric',     self._datatype),
	    ('increment',      1,             None),
	    ('initwait',       300,           None),
	    ('labelmargin',    0,             INITOPT),
	    ('labelpos',       None,          INITOPT),
	    ('orient',         'horizontal',  INITOPT),
	    ('padx',           0,             INITOPT),
	    ('pady',           0,             INITOPT),
	    ('repeatrate',     50,            None),
	    ('sticky',         'ew',          INITOPT),
	)
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

	# Initialise instance variables.
	self._timerId = None
	self._normalRelief = None

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
	    frame.grid(column=2, row=2, sticky=self['sticky'])
	    interior.grid_columnconfigure(2, weight=1)
	    interior.grid_rowconfigure(2, weight=1)

	# Create the down arrow.
	self._downArrowBtn = self.createcomponent('downarrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)

	# Create the entry field.
	self._counterEntry = self.createcomponent('entryfield',
		(('entry', 'entryfield_entry'),), None,
		Pmw.EntryField, (frame,))

	# Create the up arrow.
	self._upArrowBtn = self.createcomponent('uparrow',
		(), 'Arrow',
		Tkinter.Canvas, (frame,),
		width = 16, height = 16, relief = 'raised', borderwidth = 2)

	padx = self['padx']
	pady = self['pady']
	orient = self['orient']
	if orient == 'horizontal':
	    self._downArrowBtn.grid(column = 0, row = 0)
	    self._counterEntry.grid(column = 1, row = 0,
                    sticky = self['sticky'])
	    self._upArrowBtn.grid(column = 2, row = 0)
	    frame.grid_columnconfigure(1, weight = 1)
	    frame.grid_rowconfigure(0, weight = 1)
	    if Tkinter.TkVersion >= 4.2:
		frame.grid_columnconfigure(0, pad = padx)
		frame.grid_columnconfigure(2, pad = padx)
		frame.grid_rowconfigure(0, pad = pady)
	elif orient == 'vertical':
	    self._upArrowBtn.grid(column = 0, row = 0, sticky = 's')
	    self._counterEntry.grid(column = 0, row = 1,
                    sticky = self['sticky'])
	    self._downArrowBtn.grid(column = 0, row = 2, sticky = 'n')
	    frame.grid_columnconfigure(0, weight = 1)
	    frame.grid_rowconfigure(0, weight = 1)
	    frame.grid_rowconfigure(2, weight = 1)
	    if Tkinter.TkVersion >= 4.2:
		frame.grid_rowconfigure(0, pad = pady)
		frame.grid_rowconfigure(2, pad = pady)
		frame.grid_columnconfigure(0, pad = padx)
	else:
	    raise ValueError, 'bad orient option ' + repr(orient) + \
		': must be either \'horizontal\' or \'vertical\''

	self.createlabel(interior)

	self._upArrowBtn.bind('<Configure>', self._drawUpArrow)
	self._upArrowBtn.bind('<1>', self._countUp)
	self._upArrowBtn.bind('<Any-ButtonRelease-1>', self._stopCounting)
	self._downArrowBtn.bind('<Configure>', self._drawDownArrow)
	self._downArrowBtn.bind('<1>', self._countDown)
	self._downArrowBtn.bind('<Any-ButtonRelease-1>', self._stopCounting)
	self._counterEntry.bind('<Configure>', self._resizeArrow)
	entry = self._counterEntry.component('entry')
	entry.bind('<Down>', lambda event, s = self: s._key_decrement(event))
	entry.bind('<Up>', lambda event, s = self: s._key_increment(event))

	# Need to cancel the timer if an arrow button is unmapped (eg: 
	# its toplevel window is withdrawn) while the mouse button is
	# held down.  The canvas will not get the ButtonRelease event
	# if it is not mapped, since the implicit grab is cancelled.
	self._upArrowBtn.bind('<Unmap>', self._stopCounting)
	self._downArrowBtn.bind('<Unmap>', self._stopCounting)

	# Check keywords and initialise options.
	self.initialiseoptions()

    def _resizeArrow(self, event):
	for btn in (self._upArrowBtn, self._downArrowBtn):
	    bw = (string.atoi(btn['borderwidth']) +
		    string.atoi(btn['highlightthickness']))
	    newHeight = self._counterEntry.winfo_reqheight() - 2 * bw
	    newWidth = int(newHeight * self['buttonaspect'])
	    btn.configure(width=newWidth, height=newHeight)
	    self._drawArrow(btn)

    def _drawUpArrow(self, event):
	self._drawArrow(self._upArrowBtn)

    def _drawDownArrow(self, event):
	self._drawArrow(self._downArrowBtn)

    def _drawArrow(self, arrow):
        if self['orient'] == 'vertical':
            if arrow == self._upArrowBtn:
                direction = 'up'
            else:
                direction = 'down'
        else:
            if arrow == self._upArrowBtn:
                direction = 'right'
            else:
                direction = 'left'
        Pmw.drawarrow(arrow, self['entry_foreground'], direction, 'arrow')

    def _stopCounting(self, event = None):
        if self._timerId is not None:
            self.after_cancel(self._timerId)
	    self._timerId = None
	if self._normalRelief is not None:
	    button, relief = self._normalRelief
	    button.configure(relief=relief)
	    self._normalRelief = None

    def _countUp(self, event):
	self._normalRelief = (self._upArrowBtn, self._upArrowBtn.cget('relief'))
	self._upArrowBtn.configure(relief='sunken')
	# Force arrow down (it may come up immediately, if increment fails).
	self._upArrowBtn.update_idletasks()
	self._count(1, 1)

    def _countDown(self, event):
	self._normalRelief = (self._downArrowBtn, self._downArrowBtn.cget('relief'))
	self._downArrowBtn.configure(relief='sunken')
	# Force arrow down (it may come up immediately, if increment fails).
	self._downArrowBtn.update_idletasks()
	self._count(-1, 1)

    def increment(self):
	self._forceCount(1)

    def decrement(self):
	self._forceCount(-1)

    def _key_increment(self, event):
	self._forceCount(1)
	self.update_idletasks()

    def _key_decrement(self, event):
	self._forceCount(-1)
	self.update_idletasks()

    def _datatype(self):
	datatype = self['datatype']

	if type(datatype) is types.DictionaryType:
	    self._counterArgs = datatype.copy()
	    if self._counterArgs.has_key('counter'):
		datatype = self._counterArgs['counter']
		del self._counterArgs['counter']
	    else:
		datatype = 'numeric'
	else:
	    self._counterArgs = {}

	if _counterCommands.has_key(datatype):
	    self._counterCommand = _counterCommands[datatype]
	elif callable(datatype):
	    self._counterCommand = datatype
	else:
	    validValues = _counterCommands.keys()
	    validValues.sort()
	    raise ValueError, ('bad datatype value "%s":  must be a' +
		    ' function or one of %s') % (datatype, validValues)

    def _forceCount(self, factor):
	if not self.valid():
	    self.bell()
	    return

	text = self._counterEntry.get()
	try:
	    value = apply(self._counterCommand,
		    (text, factor, self['increment']), self._counterArgs)
	except ValueError:
	    self.bell()
	    return

        previousICursor = self._counterEntry.index('insert')
	if self._counterEntry.setentry(value) == Pmw.OK:
	    self._counterEntry.xview('end')
	    self._counterEntry.icursor(previousICursor)

    def _count(self, factor, first):
	if not self.valid():
	    self.bell()
	    return

	self._timerId = None
	origtext = self._counterEntry.get()
	try:
	    value = apply(self._counterCommand,
		    (origtext, factor, self['increment']), self._counterArgs)
	except ValueError:
	    # If text is invalid, stop counting.
	    self._stopCounting()
	    self.bell()
	    return

	# If incrementing produces an invalid value, restore previous
	# text and stop counting.
        previousICursor = self._counterEntry.index('insert')
	valid = self._counterEntry.setentry(value)
	if valid != Pmw.OK:
	    self._stopCounting()
	    self._counterEntry.setentry(origtext)
	    if valid == Pmw.PARTIAL:
		self.bell()
	    return
	self._counterEntry.xview('end')
	self._counterEntry.icursor(previousICursor)

	if self['autorepeat']:
	    if first:
		delay = self['initwait']
	    else:
		delay = self['repeatrate']
	    self._timerId = self.after(delay,
		    lambda self=self, factor=factor: self._count(factor, 0))

    def destroy(self):
	self._stopCounting()
        Pmw.MegaWidget.destroy(self)

Pmw.forwardmethods(Counter, Pmw.EntryField, '_counterEntry')

def _changeNumber(text, factor, increment):
  value = string.atol(text)
  if factor > 0:
    value = (value / increment) * increment + increment
  else:
    value = ((value - 1) / increment) * increment

  # Get rid of the 'L' at the end of longs (in python up to 1.5.2).
  rtn = str(value)
  if rtn[-1] == 'L':
      return rtn[:-1]
  else:
      return rtn

def _changeReal(text, factor, increment, separator = '.'):
  value = Pmw.stringtoreal(text, separator)
  div = value / increment

  # Compare reals using str() to avoid problems caused by binary
  # numbers being only approximations to decimal numbers.
  # For example, if value is -0.3 and increment is 0.1, then
  # int(value/increment) = -2, not -3 as one would expect.
  if str(div)[-2:] == '.0':
    # value is an even multiple of increment.
    div = round(div) + factor
  else:
    # value is not an even multiple of increment.
    div = int(div) * 1.0
    if value < 0:
      div = div - 1
    if factor > 0:
      div = (div + 1)

  value = div * increment

  text = str(value)
  if separator != '.':
      index = string.find(text, '.')
      if index >= 0:
	text = text[:index] + separator + text[index + 1:]
  return text

def _changeDate(value, factor, increment, format = 'ymd',
	separator = '/', yyyy = 0):

  jdn = Pmw.datestringtojdn(value, format, separator) + factor * increment

  y, m, d = Pmw.jdntoymd(jdn)
  result = ''
  for index in range(3):
    if index > 0:
      result = result + separator
    f = format[index]
    if f == 'y':
      if yyyy:
        result = result + '%02d' % y
      else:
        result = result + '%02d' % (y % 100)
    elif f == 'm':
      result = result + '%02d' % m
    elif f == 'd':
      result = result + '%02d' % d

  return result

_SECSPERDAY = 24 * 60 * 60
def _changeTime(value, factor, increment, separator = ':', time24 = 0):
  unixTime = Pmw.timestringtoseconds(value, separator)
  if factor > 0:
    chunks = unixTime / increment + 1
  else:
    chunks = (unixTime - 1) / increment
  unixTime = chunks * increment
  if time24:
      while unixTime < 0:
	  unixTime = unixTime + _SECSPERDAY
      while unixTime >= _SECSPERDAY:
	  unixTime = unixTime - _SECSPERDAY
  if unixTime < 0:
    unixTime = -unixTime
    sign = '-'
  else:
    sign = ''
  secs = unixTime % 60
  unixTime = unixTime / 60
  mins = unixTime % 60
  hours = unixTime / 60
  return '%s%02d%s%02d%s%02d' % (sign, hours, separator, mins, separator, secs)

# hexadecimal, alphabetic, alphanumeric not implemented
_counterCommands = {
    'numeric'   : _changeNumber,      # } integer
    'integer'   : _changeNumber,      # } these two use the same function
    'real'      : _changeReal,        # real number
    'time'      : _changeTime,
    'date'      : _changeDate,
}
