import types
import Tkinter
import Pmw

class RadioSelect(Pmw.MegaWidget):
    # A collection of several buttons.  In single mode, only one
    # button may be selected.  In multiple mode, any number of buttons
    # may be selected.

    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('buttontype',    'button',      INITOPT),
	    ('command',       None,          None),
	    ('labelmargin',   0,             INITOPT),
	    ('labelpos',      None,          INITOPT),
	    ('orient',       'horizontal',   INITOPT),
	    ('padx',          5,             INITOPT),
	    ('pady',          5,             INITOPT),
	    ('selectmode',    'single',      INITOPT),
	)
	self.defineoptions(kw, optiondefs, dynamicGroups = ('Button',))

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

	# Create the components.
	interior = self.interior()
	if self['labelpos'] is None:
	    self._radioSelectFrame = self._hull
	else:
	    self._radioSelectFrame = self.createcomponent('frame',
		    (), None,
		    Tkinter.Frame, (interior,))
	    self._radioSelectFrame.grid(column=2, row=2, sticky='nsew')
	    interior.grid_columnconfigure(2, weight=1)
	    interior.grid_rowconfigure(2, weight=1)

	    self.createlabel(interior)

	# Initialise instance variables.
	self._buttonList = []
	if self['selectmode'] == 'single':
	    self._singleSelect = 1
	elif self['selectmode'] == 'multiple':
	    self._singleSelect = 0
	else: 
	    raise ValueError, 'bad selectmode option "' + \
		    self['selectmode'] + '": should be single or multiple'

	if self['buttontype'] == 'button':
	    self.buttonClass = Tkinter.Button
	elif self['buttontype'] == 'radiobutton':
	    self._singleSelect = 1
	    self.var = Tkinter.StringVar()
	    self.buttonClass = Tkinter.Radiobutton
	elif self['buttontype'] == 'checkbutton':
	    self._singleSelect = 0
	    self.buttonClass = Tkinter.Checkbutton
	else:
	    raise ValueError, 'bad buttontype option "' + \
		    self['buttontype'] + \
		    '": should be button, radiobutton or checkbutton'

	if self._singleSelect:
	    self.selection = None
	else:
	    self.selection = []

	if self['orient'] not in ('horizontal', 'vertical'):
	    raise ValueError, 'bad orient option ' + repr(self['orient']) + \
		': must be either \'horizontal\' or \'vertical\''

	# Check keywords and initialise options.
	self.initialiseoptions()

    def getcurselection(self):
	if self._singleSelect:
            return self.selection
        else:
            return tuple(self.selection)

    def getvalue(self):
        return self.getcurselection()

    def setvalue(self, textOrList):
	if self._singleSelect:
            self.__setSingleValue(textOrList)
        else:
	    # Multiple selections
            oldselection = self.selection
            self.selection = textOrList
            for button in self._buttonList:
                if button in oldselection:
                    if button not in self.selection:
                        # button is currently selected but should not be
                        widget = self.component(button)
                        if self['buttontype'] == 'checkbutton':
                            widget.deselect()
                        else:  # Button
                            widget.configure(relief='raised')
                else:
                    if button in self.selection:
                        # button is not currently selected but should be
                        widget = self.component(button)
                        if self['buttontype'] == 'checkbutton':
                            widget.select()
                        else:  # Button
                            widget.configure(relief='sunken')

    def numbuttons(self):
        return len(self._buttonList)

    def index(self, index):
	# Return the integer index of the button with the given index.

	listLength = len(self._buttonList)
	if type(index) == types.IntType:
	    if index < listLength:
		return index
	    else:
		raise ValueError, 'index "%s" is out of range' % index
	elif index is Pmw.END:
	    if listLength > 0:
		return listLength - 1
	    else:
		raise ValueError, 'RadioSelect has no buttons'
	else:
	    for count in range(listLength):
		name = self._buttonList[count]
		if index == name:
		    return count
	    validValues = 'a name, a number or Pmw.END'
	    raise ValueError, \
		    'bad index "%s": must be %s' % (index, validValues)

    def button(self, buttonIndex):
	name = self._buttonList[self.index(buttonIndex)]
        return self.component(name)

    def add(self, componentName, **kw):
	if componentName in self._buttonList:
	    raise ValueError, 'button "%s" already exists' % componentName

	kw['command'] = \
                lambda self=self, name=componentName: self.invoke(name)
	if not kw.has_key('text'):
	    kw['text'] = componentName

	if self['buttontype'] == 'radiobutton':
	    if not kw.has_key('anchor'):
		kw['anchor'] = 'w'
	    if not kw.has_key('variable'):
		kw['variable'] = self.var
	    if not kw.has_key('value'):
		kw['value'] = kw['text']
	elif self['buttontype'] == 'checkbutton':
	    if not kw.has_key('anchor'):
		kw['anchor'] = 'w'

	button = apply(self.createcomponent, (componentName,
		(), 'Button',
		self.buttonClass, (self._radioSelectFrame,)), kw)

	if self['orient'] == 'horizontal':
	    self._radioSelectFrame.grid_rowconfigure(0, weight=1)
	    col = len(self._buttonList)
	    button.grid(column=col, row=0, padx = self['padx'],
		    pady = self['pady'], sticky='nsew')
	    self._radioSelectFrame.grid_columnconfigure(col, weight=1)
	else:
	    self._radioSelectFrame.grid_columnconfigure(0, weight=1)
	    row = len(self._buttonList)
	    button.grid(column=0, row=row, padx = self['padx'],
		    pady = self['pady'], sticky='ew')
	    self._radioSelectFrame.grid_rowconfigure(row, weight=1)

	self._buttonList.append(componentName)
	return button

    def deleteall(self):
	for name in self._buttonList:
	    self.destroycomponent(name)
	self._buttonList = []
	if self._singleSelect:
	    self.selection = None
	else: 
	    self.selection = []

    def __setSingleValue(self, value):
            self.selection = value
            if self['buttontype'] == 'radiobutton':
                widget = self.component(value)
                widget.select()
            else:  # Button
                for button in self._buttonList:
                    widget = self.component(button)
                    if button == value:
                        widget.configure(relief='sunken')
                    else:
                        widget.configure(relief='raised')

    def invoke(self, index):
	index = self.index(index)
	name = self._buttonList[index]

	if self._singleSelect:
            self.__setSingleValue(name)
	    command = self['command']
	    if callable(command):
		return command(name)
        else:
	    # Multiple selections
	    widget = self.component(name)
	    if name in self.selection:
		if self['buttontype'] == 'checkbutton':
		    widget.deselect()
		else:
		    widget.configure(relief='raised')
		self.selection.remove(name)
		state = 0
	    else:
		if self['buttontype'] == 'checkbutton':
		    widget.select()
		else:
		    widget.configure(relief='sunken')
		self.selection.append(name)
		state = 1

	    command = self['command']
	    if callable(command):
	      return command(name, state)
