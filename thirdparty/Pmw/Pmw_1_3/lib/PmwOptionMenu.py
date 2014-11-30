import types
import Tkinter
import Pmw

class OptionMenu(Pmw.MegaWidget):

    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('command',        None,       None),
            ('items',          (),         INITOPT),
            ('initialitem',    None,       INITOPT),
	    ('labelmargin',    0,          INITOPT),
	    ('labelpos',       None,       INITOPT),
	    ('sticky',         'ew',       INITOPT),
	)
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

	# Create the components.
	interior = self.interior()

	self._menubutton = self.createcomponent('menubutton',
		(), None,
		Tkinter.Menubutton, (interior,),
		borderwidth = 2,
		indicatoron = 1,
		relief = 'raised',
		anchor = 'c',
		highlightthickness = 2,
		direction = 'flush',
                takefocus = 1,
	)
	self._menubutton.grid(column = 2, row = 2, sticky = self['sticky'])

	self._menu = self.createcomponent('menu',
		(), None,
		Tkinter.Menu, (self._menubutton,),
		tearoff=0
	)
	self._menubutton.configure(menu = self._menu)

	interior.grid_columnconfigure(2, weight = 1)
	interior.grid_rowconfigure(2, weight = 1)

        # Create the label.
        self.createlabel(interior)

        # Add the items specified by the initialisation option.
	self._itemList = []
        self.setitems(self['items'], self['initialitem'])

	# Check keywords and initialise options.
	self.initialiseoptions()

    def setitems(self, items, index = None):

        # Clean up old items and callback commands.
        for oldIndex in range(len(self._itemList)):
            tclCommandName = str(self._menu.entrycget(oldIndex, 'command'))
            if tclCommandName != '':   
                self._menu.deletecommand(tclCommandName)
        self._menu.delete(0, 'end')
	self._itemList = list(items)

	# Set the items in the menu component.
        for item in items:
            self._menu.add_command(label = item,
		command = lambda self = self, item = item: self._invoke(item))

	# Set the currently selected value.
	if index is None:
            var = str(self._menubutton.cget('textvariable'))
	    if var != '':
		# None means do not change text variable.
		return
	    if len(items) == 0:
		text = ''
	    elif str(self._menubutton.cget('text')) in items:
                # Do not change selection if it is still valid
		return
	    else:
		text = items[0]
	else:
	    index = self.index(index)
	    text = self._itemList[index]

        self.setvalue(text)

    def getcurselection(self):
	var = str(self._menubutton.cget('textvariable'))
	if var == '':
	    return str(self._menubutton.cget('text'))
	else:
	    return self._menu.tk.globalgetvar(var)

    def getvalue(self):
        return self.getcurselection()

    def setvalue(self, text):
	var = str(self._menubutton.cget('textvariable'))
	if var == '':
	    self._menubutton.configure(text = text)
	else:
	    self._menu.tk.globalsetvar(var, text)

    def index(self, index):
	listLength = len(self._itemList)
	if type(index) == types.IntType:
	    if index < listLength:
		return index
	    else:
		raise ValueError, 'index "%s" is out of range' % index
	elif index is Pmw.END:
	    if listLength > 0:
		return listLength - 1
	    else:
		raise ValueError, 'OptionMenu has no items'
	else:
	    if index is Pmw.SELECT:
		if listLength > 0:
		    index = self.getcurselection()
		else:
		    raise ValueError, 'OptionMenu has no items'
            if index in self._itemList:
                return self._itemList.index(index)
	    raise ValueError, \
		    'bad index "%s": must be a ' \
                    'name, a number, Pmw.END or Pmw.SELECT' % (index,)

    def invoke(self, index = Pmw.SELECT):
	index = self.index(index)
	text = self._itemList[index]

        return self._invoke(text)

    def _invoke(self, text):
        self.setvalue(text)

	command = self['command']
	if callable(command):
	    return command(text)
