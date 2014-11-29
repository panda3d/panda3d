# Based on iwidgets2.2.0/combobox.itk code.

import os
import string
import types
import Tkinter
import Pmw

class ComboBox(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('autoclear',          0,          INITOPT),
	    ('buttonaspect',       1.0,        INITOPT),
	    ('dropdown',           1,          INITOPT),
	    ('fliparrow',          0,          INITOPT),
	    ('history',            1,          INITOPT),
	    ('labelmargin',        0,          INITOPT),
	    ('labelpos',           None,       INITOPT),
	    ('listheight',         200,        INITOPT),
	    ('selectioncommand',   None,       None),
	    ('sticky',            'ew',        INITOPT),
	    ('unique',             1,          INITOPT),
	)
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

	# Create the components.
	interior = self.interior()

	self._entryfield = self.createcomponent('entryfield',
		(('entry', 'entryfield_entry'),), None,
		Pmw.EntryField, (interior,))
	self._entryfield.grid(column=2, row=2, sticky=self['sticky'])
	interior.grid_columnconfigure(2, weight = 1)
	self._entryWidget = self._entryfield.component('entry')

	if self['dropdown']:
	    self._isPosted = 0
            interior.grid_rowconfigure(2, weight = 1)

	    # Create the arrow button.
	    self._arrowBtn = self.createcomponent('arrowbutton',
		    (), None,
		    Tkinter.Canvas, (interior,), borderwidth = 2,
		    relief = 'raised',
		    width = 16, height = 16)
            if 'n' in self['sticky']:
                sticky = 'n'
            else:
                sticky = ''
            if 's' in self['sticky']:
                sticky = sticky + 's'
	    self._arrowBtn.grid(column=3, row=2, sticky = sticky)
	    self._arrowRelief = self._arrowBtn.cget('relief')

	    # Create the label.
	    self.createlabel(interior, childCols=2)

	    # Create the dropdown window.
	    self._popup = self.createcomponent('popup',
		    (), None,
		    Tkinter.Toplevel, (interior,))
	    self._popup.withdraw()
	    self._popup.overrideredirect(1)

	    # Create the scrolled listbox inside the dropdown window.
	    self._list = self.createcomponent('scrolledlist',
		    (('listbox', 'scrolledlist_listbox'),), None,
		    Pmw.ScrolledListBox, (self._popup,),
		    hull_borderwidth = 2,
		    hull_relief = 'raised',
		    hull_height = self['listheight'],
		    usehullsize = 1,
		    listbox_exportselection = 0)
	    self._list.pack(expand=1, fill='both')
	    self.__listbox = self._list.component('listbox')

	    # Bind events to the arrow button.
	    self._arrowBtn.bind('<1>', self._postList)
	    self._arrowBtn.bind('<Configure>', self._drawArrow)
	    self._arrowBtn.bind('<3>', self._next)
	    self._arrowBtn.bind('<Shift-3>', self._previous)
	    self._arrowBtn.bind('<Down>', self._next)
	    self._arrowBtn.bind('<Up>', self._previous)
	    self._arrowBtn.bind('<Control-n>', self._next)
	    self._arrowBtn.bind('<Control-p>', self._previous)
	    self._arrowBtn.bind('<Shift-Down>', self._postList)
	    self._arrowBtn.bind('<Shift-Up>', self._postList)
	    self._arrowBtn.bind('<F34>', self._postList)
	    self._arrowBtn.bind('<F28>', self._postList)
	    self._arrowBtn.bind('<space>', self._postList)

	    # Bind events to the dropdown window.
	    self._popup.bind('<Escape>', self._unpostList)
	    self._popup.bind('<space>', self._selectUnpost)
	    self._popup.bind('<Return>', self._selectUnpost)
	    self._popup.bind('<ButtonRelease-1>', self._dropdownBtnRelease)
	    self._popup.bind('<ButtonPress-1>', self._unpostOnNextRelease)

	    # Bind events to the Tk listbox.
	    self.__listbox.bind('<Enter>', self._unpostOnNextRelease)

	    # Bind events to the Tk entry widget.
	    self._entryWidget.bind('<Configure>', self._resizeArrow)
	    self._entryWidget.bind('<Shift-Down>', self._postList)
	    self._entryWidget.bind('<Shift-Up>', self._postList)
	    self._entryWidget.bind('<F34>', self._postList)
	    self._entryWidget.bind('<F28>', self._postList)

            # Need to unpost the popup if the entryfield is unmapped (eg: 
            # its toplevel window is withdrawn) while the popup list is
            # displayed.
            self._entryWidget.bind('<Unmap>', self._unpostList)

	else:
	    # Create the scrolled listbox below the entry field.
	    self._list = self.createcomponent('scrolledlist',
		    (('listbox', 'scrolledlist_listbox'),), None,
		    Pmw.ScrolledListBox, (interior,),
                    selectioncommand = self._selectCmd)
	    self._list.grid(column=2, row=3, sticky='nsew')
	    self.__listbox = self._list.component('listbox')

	    # The scrolled listbox should expand vertically.
	    interior.grid_rowconfigure(3, weight = 1)

	    # Create the label.
	    self.createlabel(interior, childRows=2)

	self._entryWidget.bind('<Down>', self._next)
	self._entryWidget.bind('<Up>', self._previous)
	self._entryWidget.bind('<Control-n>', self._next)
	self._entryWidget.bind('<Control-p>', self._previous)
	self.__listbox.bind('<Control-n>', self._next)
	self.__listbox.bind('<Control-p>', self._previous)

	if self['history']:
	    self._entryfield.configure(command=self._addHistory)

	# Check keywords and initialise options.
	self.initialiseoptions()

    def destroy(self):
	if self['dropdown'] and self._isPosted:
            Pmw.popgrab(self._popup)
        Pmw.MegaWidget.destroy(self)

    #======================================================================

    # Public methods

    def get(self, first = None, last=None):
	if first is None:
	    return self._entryWidget.get()
	else:
	    return self._list.get(first, last)

    def invoke(self):
	if self['dropdown']:
	    self._postList()
	else:
	    return self._selectCmd()

    def selectitem(self, index, setentry=1):
	if type(index) == types.StringType:
	    text = index
	    items = self._list.get(0, 'end')
	    if text in items:
		index = list(items).index(text)
	    else:
	    	raise IndexError, 'index "%s" not found' % text
	elif setentry:
	    text = self._list.get(0, 'end')[index]

	self._list.select_clear(0, 'end')
	self._list.select_set(index, index)
	self._list.activate(index)
	self.see(index)
	if setentry:
	    self._entryfield.setentry(text)

    # Need to explicitly forward this to override the stupid
    # (grid_)size method inherited from Tkinter.Frame.Grid.
    def size(self):
	return self._list.size()

    # Need to explicitly forward this to override the stupid
    # (grid_)bbox method inherited from Tkinter.Frame.Grid.
    def bbox(self, index):
	return self._list.bbox(index)

    def clear(self):
	self._entryfield.clear()
	self._list.clear()

    #======================================================================

    # Private methods for both dropdown and simple comboboxes.

    def _addHistory(self):
	input = self._entryWidget.get()

	if input != '':
	    index = None
	    if self['unique']:
		# If item is already in list, select it and return.
		items = self._list.get(0, 'end')
		if input in items:
		    index = list(items).index(input)

	    if index is None:
		index = self._list.index('end')
		self._list.insert('end', input)

	    self.selectitem(index)
	    if self['autoclear']:
		self._entryWidget.delete(0, 'end')

	    # Execute the selectioncommand on the new entry.
	    self._selectCmd()

    def _next(self, event):
	size = self.size()
	if size <= 1:
	    return

	cursels = self.curselection()

	if len(cursels) == 0:
	    index = 0
	else:
	    index = string.atoi(cursels[0])
	    if index == size - 1:
		index = 0
	    else:
		index = index + 1

	self.selectitem(index)

    def _previous(self, event):
	size = self.size()
	if size <= 1:
	    return

	cursels = self.curselection()

	if len(cursels) == 0:
	    index = size - 1
	else:
	    index = string.atoi(cursels[0])
	    if index == 0:
		index = size - 1
	    else:
		index = index - 1

	self.selectitem(index)

    def _selectCmd(self, event=None):

	sels = self.getcurselection()
	if len(sels) == 0:
	    item = None
	else:
	    item = sels[0]
	    self._entryfield.setentry(item)

	cmd = self['selectioncommand']
	if callable(cmd):
            if event is None:
                # Return result of selectioncommand for invoke() method.
                return cmd(item)
            else:
                cmd(item)

    #======================================================================

    # Private methods for dropdown combobox.

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

    def _postList(self, event = None):
        self._isPosted = 1
        self._drawArrow(sunken=1)

        # Make sure that the arrow is displayed sunken.
        self.update_idletasks()

        x = self._entryfield.winfo_rootx()
        y = self._entryfield.winfo_rooty() + \
            self._entryfield.winfo_height()
        w = self._entryfield.winfo_width() + self._arrowBtn.winfo_width()
        h =  self.__listbox.winfo_height()
        sh = self.winfo_screenheight()

        if y + h > sh and y > sh / 2:
            y = self._entryfield.winfo_rooty() - h

        self._list.configure(hull_width=w)

        Pmw.setgeometryanddeiconify(self._popup, '+%d+%d' % (x, y))

        # Grab the popup, so that all events are delivered to it, and
        # set focus to the listbox, to make keyboard navigation
        # easier.
        Pmw.pushgrab(self._popup, 1, self._unpostList)
        self.__listbox.focus_set()

        self._drawArrow()

        # Ignore the first release of the mouse button after posting the
        # dropdown list, unless the mouse enters the dropdown list.
        self._ignoreRelease = 1

    def _dropdownBtnRelease(self, event):
	if (event.widget == self._list.component('vertscrollbar') or
		event.widget == self._list.component('horizscrollbar')):
	    return

	if self._ignoreRelease:
	    self._unpostOnNextRelease()
	    return

        self._unpostList()

	if (event.x >= 0 and event.x < self.__listbox.winfo_width() and
		event.y >= 0 and event.y < self.__listbox.winfo_height()):
	    self._selectCmd()

    def _unpostOnNextRelease(self, event = None):
	self._ignoreRelease = 0

    def _resizeArrow(self, event):
	bw = (string.atoi(self._arrowBtn['borderwidth']) + 
		string.atoi(self._arrowBtn['highlightthickness']))
	newHeight = self._entryfield.winfo_reqheight() - 2 * bw
	newWidth = int(newHeight * self['buttonaspect'])
	self._arrowBtn.configure(width=newWidth, height=newHeight)
	self._drawArrow()

    def _unpostList(self, event=None):
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

    def _selectUnpost(self, event):
        self._unpostList()
	self._selectCmd()

Pmw.forwardmethods(ComboBox, Pmw.ScrolledListBox, '_list')
Pmw.forwardmethods(ComboBox, Pmw.EntryField, '_entryfield')
