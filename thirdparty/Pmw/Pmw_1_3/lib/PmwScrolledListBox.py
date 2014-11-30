# Based on iwidgets2.2.0/scrolledlistbox.itk code.

import types
import Tkinter
import Pmw

class ScrolledListBox(Pmw.MegaWidget):
    _classBindingsDefinedFor = 0

    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('dblclickcommand',    None,            None),
	    ('hscrollmode',        'dynamic',       self._hscrollMode),
	    ('items',              (),              INITOPT),
	    ('labelmargin',        0,               INITOPT),
	    ('labelpos',           None,            INITOPT),
	    ('scrollmargin',       2,               INITOPT),
	    ('selectioncommand',   None,            None),
	    ('usehullsize',        0,               INITOPT),
	    ('vscrollmode',        'dynamic',       self._vscrollMode),
	)
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

	# Create the components.
	interior = self.interior()

	if self['usehullsize']:
	    interior.grid_propagate(0)

	# Create the listbox widget.
	self._listbox = self.createcomponent('listbox',
		(), None,
		Tkinter.Listbox, (interior,))
	self._listbox.grid(row = 2, column = 2, sticky = 'news')
	interior.grid_rowconfigure(2, weight = 1, minsize = 0)
	interior.grid_columnconfigure(2, weight = 1, minsize = 0)

	# Create the horizontal scrollbar
	self._horizScrollbar = self.createcomponent('horizscrollbar',
		(), 'Scrollbar',
		Tkinter.Scrollbar, (interior,),
	        orient='horizontal',
		command=self._listbox.xview
	)

	# Create the vertical scrollbar
	self._vertScrollbar = self.createcomponent('vertscrollbar',
		(), 'Scrollbar',
		Tkinter.Scrollbar, (interior,),
		orient='vertical',
		command=self._listbox.yview
	)

	self.createlabel(interior, childCols = 3, childRows = 3)

	# Add the items specified by the initialisation option.
	items = self['items']
	if type(items) != types.TupleType:
	    items = tuple(items)
	if len(items) > 0:
	    apply(self._listbox.insert, ('end',) + items)

	_registerScrolledList(self._listbox, self)

        # Establish the special class bindings if not already done.
        # Also create bindings if the Tkinter default interpreter has
        # changed.  Use Tkinter._default_root to create class
        # bindings, so that a reference to root is created by
        # bind_class rather than a reference to self, which would
        # prevent object cleanup.
        theTag = 'ScrolledListBoxTag'
        if ScrolledListBox._classBindingsDefinedFor != Tkinter._default_root:
            root  = Tkinter._default_root
	    	    
            def doubleEvent(event):
                _handleEvent(event, 'double')
            def keyEvent(event):
                _handleEvent(event, 'key')
            def releaseEvent(event):
                _handleEvent(event, 'release')

            # Bind space and return keys and button 1 to the selectioncommand.
            root.bind_class(theTag, '<Key-space>', keyEvent)
            root.bind_class(theTag, '<Key-Return>', keyEvent)
            root.bind_class(theTag, '<ButtonRelease-1>', releaseEvent)

            # Bind double button 1 click to the dblclickcommand.
            root.bind_class(theTag, '<Double-ButtonRelease-1>', doubleEvent)

	    ScrolledListBox._classBindingsDefinedFor = root

	bindtags = self._listbox.bindtags()
	self._listbox.bindtags(bindtags + (theTag,))

	# Initialise instance variables.
	self._horizScrollbarOn = 0
	self._vertScrollbarOn = 0
	self.scrollTimer = None
        self._scrollRecurse = 0
	self._horizScrollbarNeeded = 0
	self._vertScrollbarNeeded = 0

	# Check keywords and initialise options.
	self.initialiseoptions()

    def destroy(self):
	if self.scrollTimer is not None:
	    self.after_cancel(self.scrollTimer)
	    self.scrollTimer = None
	_deregisterScrolledList(self._listbox)
	Pmw.MegaWidget.destroy(self)

    # ======================================================================

    # Public methods.

    def clear(self):
	self.setlist(())

    def getcurselection(self):
	rtn = []
	for sel in self.curselection():
	    rtn.append(self._listbox.get(sel))
	return tuple(rtn)

    def getvalue(self):
        return self.getcurselection()

    def setvalue(self, textOrList):
        self._listbox.selection_clear(0, 'end')
        listitems = list(self._listbox.get(0, 'end'))
        if type(textOrList) == types.StringType:
            if textOrList in listitems:
                self._listbox.selection_set(listitems.index(textOrList))
            else:
                raise ValueError, 'no such item "%s"' % textOrList
        else:
            for item in textOrList:
                if item in listitems:
                    self._listbox.selection_set(listitems.index(item))
                else:
                    raise ValueError, 'no such item "%s"' % item

    def setlist(self, items):
        self._listbox.delete(0, 'end')
	if len(items) > 0:
	    if type(items) != types.TupleType:
		items = tuple(items)
	    apply(self._listbox.insert, (0,) + items)

    # Override Tkinter.Listbox get method, so that if it is called with
    # no arguments, return all list elements (consistent with other widgets).
    def get(self, first=None, last=None):
	if first is None:
	    return self._listbox.get(0, 'end')
	else:
	    return self._listbox.get(first, last)

    # ======================================================================

    # Configuration methods.

    def _hscrollMode(self):
	# The horizontal scroll mode has been configured.

	mode = self['hscrollmode']

	if mode == 'static':
	    if not self._horizScrollbarOn:
		self._toggleHorizScrollbar()
	elif mode == 'dynamic':
	    if self._horizScrollbarNeeded != self._horizScrollbarOn:
		self._toggleHorizScrollbar()
	elif mode == 'none':
	    if self._horizScrollbarOn:
		self._toggleHorizScrollbar()
	else:
	    message = 'bad hscrollmode option "%s": should be static, dynamic, or none' % mode
	    raise ValueError, message

        self._configureScrollCommands()

    def _vscrollMode(self):
	# The vertical scroll mode has been configured.

	mode = self['vscrollmode']

	if mode == 'static':
	    if not self._vertScrollbarOn:
		self._toggleVertScrollbar()
	elif mode == 'dynamic':
	    if self._vertScrollbarNeeded != self._vertScrollbarOn:
		self._toggleVertScrollbar()
	elif mode == 'none':
	    if self._vertScrollbarOn:
		self._toggleVertScrollbar()
	else:
	    message = 'bad vscrollmode option "%s": should be static, dynamic, or none' % mode
	    raise ValueError, message

        self._configureScrollCommands()

    # ======================================================================

    # Private methods.

    def _configureScrollCommands(self):
        # If both scrollmodes are not dynamic we can save a lot of
        # time by not having to create an idle job to handle the
        # scroll commands.

        # Clean up previous scroll commands to prevent memory leak.
        tclCommandName = str(self._listbox.cget('xscrollcommand'))
        if tclCommandName != '':   
            self._listbox.deletecommand(tclCommandName)
        tclCommandName = str(self._listbox.cget('yscrollcommand'))
        if tclCommandName != '':   
            self._listbox.deletecommand(tclCommandName)

	if self['hscrollmode'] == self['vscrollmode'] == 'dynamic':
            self._listbox.configure(
                    xscrollcommand=self._scrollBothLater,
                    yscrollcommand=self._scrollBothLater
            )
        else:
            self._listbox.configure(
                    xscrollcommand=self._scrollXNow,
                    yscrollcommand=self._scrollYNow
            )

    def _scrollXNow(self, first, last):
        self._horizScrollbar.set(first, last)
        self._horizScrollbarNeeded = ((first, last) != ('0', '1'))

	if self['hscrollmode'] == 'dynamic':
	    if self._horizScrollbarNeeded != self._horizScrollbarOn:
		self._toggleHorizScrollbar()

    def _scrollYNow(self, first, last):
        self._vertScrollbar.set(first, last)
        self._vertScrollbarNeeded = ((first, last) != ('0', '1'))

        if self['vscrollmode'] == 'dynamic':
            if self._vertScrollbarNeeded != self._vertScrollbarOn:
                self._toggleVertScrollbar()

    def _scrollBothLater(self, first, last):
	# Called by the listbox to set the horizontal or vertical
	# scrollbar when it has scrolled or changed size or contents.

	if self.scrollTimer is None:
	    self.scrollTimer = self.after_idle(self._scrollBothNow)

    def _scrollBothNow(self):
        # This performs the function of _scrollXNow and _scrollYNow.
        # If one is changed, the other should be updated to match.
	self.scrollTimer = None

        # Call update_idletasks to make sure that the containing frame
        # has been resized before we attempt to set the scrollbars. 
        # Otherwise the scrollbars may be mapped/unmapped continuously.
        self._scrollRecurse = self._scrollRecurse + 1
        self.update_idletasks()
        self._scrollRecurse = self._scrollRecurse - 1
        if self._scrollRecurse != 0:
            return

	xview = self._listbox.xview()
	yview = self._listbox.yview()
	self._horizScrollbar.set(xview[0], xview[1])
	self._vertScrollbar.set(yview[0], yview[1])

	self._horizScrollbarNeeded = (xview != (0.0, 1.0))
	self._vertScrollbarNeeded = (yview != (0.0, 1.0))

	# If both horizontal and vertical scrollmodes are dynamic and
	# currently only one scrollbar is mapped and both should be
	# toggled, then unmap the mapped scrollbar.  This prevents a
	# continuous mapping and unmapping of the scrollbars. 
	if (self['hscrollmode'] == self['vscrollmode'] == 'dynamic' and
		self._horizScrollbarNeeded != self._horizScrollbarOn and
		self._vertScrollbarNeeded != self._vertScrollbarOn and
		self._vertScrollbarOn != self._horizScrollbarOn):
	    if self._horizScrollbarOn:
		self._toggleHorizScrollbar()
	    else:
		self._toggleVertScrollbar()
	    return

	if self['hscrollmode'] == 'dynamic':
	    if self._horizScrollbarNeeded != self._horizScrollbarOn:
		self._toggleHorizScrollbar()

	if self['vscrollmode'] == 'dynamic':
	    if self._vertScrollbarNeeded != self._vertScrollbarOn:
		self._toggleVertScrollbar()

    def _toggleHorizScrollbar(self):

	self._horizScrollbarOn = not self._horizScrollbarOn

	interior = self.interior()
	if self._horizScrollbarOn:
	    self._horizScrollbar.grid(row = 4, column = 2, sticky = 'news')
	    interior.grid_rowconfigure(3, minsize = self['scrollmargin'])
	else:
	    self._horizScrollbar.grid_forget()
	    interior.grid_rowconfigure(3, minsize = 0)

    def _toggleVertScrollbar(self):

	self._vertScrollbarOn = not self._vertScrollbarOn

	interior = self.interior()
	if self._vertScrollbarOn:
	    self._vertScrollbar.grid(row = 2, column = 4, sticky = 'news')
	    interior.grid_columnconfigure(3, minsize = self['scrollmargin'])
	else:
	    self._vertScrollbar.grid_forget()
	    interior.grid_columnconfigure(3, minsize = 0)

    def _handleEvent(self, event, eventType):
        if eventType == 'double':
            command = self['dblclickcommand']
        elif eventType == 'key':
            command = self['selectioncommand']
        else: #eventType == 'release'
            # Do not execute the command if the mouse was released
            # outside the listbox.
            if (event.x < 0 or self._listbox.winfo_width() <= event.x or
                    event.y < 0 or self._listbox.winfo_height() <= event.y):
                return

            command = self['selectioncommand']

        if callable(command):
            command()

    # Need to explicitly forward this to override the stupid
    # (grid_)size method inherited from Tkinter.Frame.Grid.
    def size(self):
	return self._listbox.size()

    # Need to explicitly forward this to override the stupid
    # (grid_)bbox method inherited from Tkinter.Frame.Grid.
    def bbox(self, index):
	return self._listbox.bbox(index)

Pmw.forwardmethods(ScrolledListBox, Tkinter.Listbox, '_listbox')

# ======================================================================

_listboxCache = {}

def _registerScrolledList(listbox, scrolledList):
    # Register an ScrolledList widget for a Listbox widget

    _listboxCache[listbox] = scrolledList

def _deregisterScrolledList(listbox):
    # Deregister a Listbox widget
    del _listboxCache[listbox]

def _handleEvent(event, eventType):
    # Forward events for a Listbox to it's ScrolledListBox

    # A binding earlier in the bindtags list may have destroyed the
    # megawidget, so need to check.
    if _listboxCache.has_key(event.widget):
        _listboxCache[event.widget]._handleEvent(event, eventType)
