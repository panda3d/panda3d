# Based on iwidgets2.2.0/scrolledtext.itk code.   

import Tkinter
import Pmw

class ScrolledText(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('borderframe',    0,            INITOPT),
	    ('columnheader',   0,            INITOPT),
	    ('hscrollmode',    'dynamic',    self._hscrollMode),
	    ('labelmargin',    0,            INITOPT),
	    ('labelpos',       None,         INITOPT),
	    ('rowcolumnheader',0,            INITOPT),
	    ('rowheader',      0,            INITOPT),
	    ('scrollmargin',   2,            INITOPT),
	    ('usehullsize',    0,            INITOPT),
	    ('vscrollmode',    'dynamic',    self._vscrollMode),
	)
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

	# Create the components.
	interior = self.interior()

	if self['usehullsize']:
	    interior.grid_propagate(0)

	if self['borderframe']:
	    # Create a frame widget to act as the border of the text 
	    # widget.  Later, pack the text widget so that it fills
	    # the frame.  This avoids a problem in Tk, where window
	    # items in a text widget may overlap the border of the
	    # text widget.
	    self._borderframe = self.createcomponent('borderframe',
		    (), None,
		    Tkinter.Frame, (interior,),
		    relief = 'sunken',
		    borderwidth = 2,
	    )
	    self._borderframe.grid(row = 4, column = 4, sticky = 'news')

	    # Create the text widget.
	    self._textbox = self.createcomponent('text',
		    (), None,
		    Tkinter.Text, (self._borderframe,),
		    highlightthickness = 0,
		    borderwidth = 0,
	    )
	    self._textbox.pack(fill = 'both', expand = 1)

            bw = self._borderframe.cget('borderwidth'),
            ht = self._borderframe.cget('highlightthickness'),
	else:
	    # Create the text widget.
	    self._textbox = self.createcomponent('text',
		    (), None,
		    Tkinter.Text, (interior,),
	    )
	    self._textbox.grid(row = 4, column = 4, sticky = 'news')

            bw = self._textbox.cget('borderwidth'),
            ht = self._textbox.cget('highlightthickness'),

        # Create the header text widgets
        if self['columnheader']:
            self._columnheader = self.createcomponent('columnheader',
                    (), 'Header',
                    Tkinter.Text, (interior,),
                    height=1,
                    wrap='none',
                    borderwidth = bw,
                    highlightthickness = ht,
            )
            self._columnheader.grid(row = 2, column = 4, sticky = 'ew')
            self._columnheader.configure(
                    xscrollcommand = self._columnheaderscrolled)

        if self['rowheader']:
            self._rowheader = self.createcomponent('rowheader',
                    (), 'Header',
                    Tkinter.Text, (interior,),
                    wrap='none',
                    borderwidth = bw,
                    highlightthickness = ht,
            )
            self._rowheader.grid(row = 4, column = 2, sticky = 'ns')
            self._rowheader.configure(
                    yscrollcommand = self._rowheaderscrolled)

        if self['rowcolumnheader']:
            self._rowcolumnheader = self.createcomponent('rowcolumnheader',
                    (), 'Header',
                    Tkinter.Text, (interior,),
                    height=1,
                    wrap='none',
                    borderwidth = bw,
                    highlightthickness = ht,
            )
            self._rowcolumnheader.grid(row = 2, column = 2, sticky = 'nsew')

	interior.grid_rowconfigure(4, weight = 1, minsize = 0)
	interior.grid_columnconfigure(4, weight = 1, minsize = 0)

	# Create the horizontal scrollbar
	self._horizScrollbar = self.createcomponent('horizscrollbar',
		(), 'Scrollbar',
		Tkinter.Scrollbar, (interior,),
	        orient='horizontal',
		command=self._textbox.xview
	)

	# Create the vertical scrollbar
	self._vertScrollbar = self.createcomponent('vertscrollbar',
		(), 'Scrollbar',
		Tkinter.Scrollbar, (interior,),
		orient='vertical',
		command=self._textbox.yview
	)

	self.createlabel(interior, childCols = 5, childRows = 5)

	# Initialise instance variables.
	self._horizScrollbarOn = 0
	self._vertScrollbarOn = 0
	self.scrollTimer = None
        self._scrollRecurse = 0
	self._horizScrollbarNeeded = 0
	self._vertScrollbarNeeded = 0
	self._textWidth = None

        # These four variables avoid an infinite loop caused by the
        # row or column header's scrollcommand causing the main text
        # widget's scrollcommand to be called and vice versa.
	self._textboxLastX = None
	self._textboxLastY = None
	self._columnheaderLastX = None
	self._rowheaderLastY = None

	# Check keywords and initialise options.
	self.initialiseoptions()

    def destroy(self):
	if self.scrollTimer is not None:
	    self.after_cancel(self.scrollTimer)
	    self.scrollTimer = None
	Pmw.MegaWidget.destroy(self)

    # ======================================================================

    # Public methods.

    def clear(self):
	self.settext('')

    def importfile(self, fileName, where = 'end'):
	file = open(fileName, 'r')
	self._textbox.insert(where, file.read())
	file.close()

    def exportfile(self, fileName):
	file = open(fileName, 'w')
	file.write(self._textbox.get('1.0', 'end'))
	file.close()

    def settext(self, text):
	disabled = (str(self._textbox.cget('state')) == 'disabled')
	if disabled:
	    self._textbox.configure(state='normal')
	self._textbox.delete('0.0', 'end')
	self._textbox.insert('end', text)
	if disabled:
	    self._textbox.configure(state='disabled')

    # Override Tkinter.Text get method, so that if it is called with
    # no arguments, return all text (consistent with other widgets).
    def get(self, first=None, last=None):
	if first is None:
	    return self._textbox.get('1.0', 'end')
	else:
	    return self._textbox.get(first, last)

    def getvalue(self):
        return self.get()

    def setvalue(self, text):
        return self.settext(text)

    def appendtext(self, text):
        oldTop, oldBottom = self._textbox.yview()
     
        disabled = (str(self._textbox.cget('state')) == 'disabled')
        if disabled:
            self._textbox.configure(state='normal')
        self._textbox.insert('end', text)
        if disabled:
            self._textbox.configure(state='disabled')
     
        if oldBottom == 1.0:
            self._textbox.yview('moveto', 1.0)

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
        tclCommandName = str(self._textbox.cget('xscrollcommand'))
        if tclCommandName != '':   
            self._textbox.deletecommand(tclCommandName)
        tclCommandName = str(self._textbox.cget('yscrollcommand'))
        if tclCommandName != '':   
            self._textbox.deletecommand(tclCommandName)

	if self['hscrollmode'] == self['vscrollmode'] == 'dynamic':
            self._textbox.configure(
                    xscrollcommand=self._scrollBothLater,
                    yscrollcommand=self._scrollBothLater
            )
        else:
            self._textbox.configure(
                    xscrollcommand=self._scrollXNow,
                    yscrollcommand=self._scrollYNow
            )

    def _scrollXNow(self, first, last):
        self._horizScrollbar.set(first, last)
        self._horizScrollbarNeeded = ((first, last) != ('0', '1'))

        # This code is the same as in _scrollBothNow.  Keep it that way.
        if self['hscrollmode'] == 'dynamic':
            currentWidth = self._textbox.winfo_width()
            if self._horizScrollbarNeeded != self._horizScrollbarOn:
                if self._horizScrollbarNeeded or \
                        self._textWidth != currentWidth:
                    self._toggleHorizScrollbar()
            self._textWidth = currentWidth

        if self['columnheader']:
	    if self._columnheaderLastX != first:
		self._columnheaderLastX = first
		self._columnheader.xview('moveto', first)

    def _scrollYNow(self, first, last):
        if first == '0' and last == '0':
            return
        self._vertScrollbar.set(first, last)
        self._vertScrollbarNeeded = ((first, last) != ('0', '1'))

        if self['vscrollmode'] == 'dynamic':
            if self._vertScrollbarNeeded != self._vertScrollbarOn:
                self._toggleVertScrollbar()

        if self['rowheader']:
	    if self._rowheaderLastY != first:
		self._rowheaderLastY = first
		self._rowheader.yview('moveto', first)

    def _scrollBothLater(self, first, last):
	# Called by the text widget to set the horizontal or vertical
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

	xview = self._textbox.xview()
	yview = self._textbox.yview()

	# The text widget returns a yview of (0.0, 0.0) just after it
	# has been created. Ignore this.
	if yview == (0.0, 0.0):
	    return

        if self['columnheader']:
	    if self._columnheaderLastX != xview[0]:
		self._columnheaderLastX = xview[0]
		self._columnheader.xview('moveto', xview[0])
        if self['rowheader']:
	    if self._rowheaderLastY != yview[0]:
		self._rowheaderLastY = yview[0]
		self._rowheader.yview('moveto', yview[0])

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

	    # The following test is done to prevent continuous
	    # mapping and unmapping of the horizontal scrollbar. 
	    # This may occur when some event (scrolling, resizing
	    # or text changes) modifies the displayed text such
	    # that the bottom line in the window is the longest
	    # line displayed.  If this causes the horizontal
	    # scrollbar to be mapped, the scrollbar may "cover up"
	    # the bottom line, which would mean that the scrollbar
	    # is no longer required.  If the scrollbar is then
	    # unmapped, the bottom line will then become visible
	    # again, which would cause the scrollbar to be mapped
	    # again, and so on...
	    #
	    # The idea is that, if the width of the text widget
	    # has not changed and the scrollbar is currently
	    # mapped, then do not unmap the scrollbar even if it
	    # is no longer required.  This means that, during
	    # normal scrolling of the text, once the horizontal
	    # scrollbar has been mapped it will not be unmapped
	    # (until the width of the text widget changes).

	    currentWidth = self._textbox.winfo_width()
	    if self._horizScrollbarNeeded != self._horizScrollbarOn:
		if self._horizScrollbarNeeded or \
			self._textWidth != currentWidth:
		    self._toggleHorizScrollbar()
	    self._textWidth = currentWidth

	if self['vscrollmode'] == 'dynamic':
	    if self._vertScrollbarNeeded != self._vertScrollbarOn:
		self._toggleVertScrollbar()

    def _columnheaderscrolled(self, first, last):
	if self._textboxLastX != first:
	    self._textboxLastX = first
	    self._textbox.xview('moveto', first)

    def _rowheaderscrolled(self, first, last):
	if self._textboxLastY != first:
	    self._textboxLastY = first
	    self._textbox.yview('moveto', first)

    def _toggleHorizScrollbar(self):

	self._horizScrollbarOn = not self._horizScrollbarOn

	interior = self.interior()
	if self._horizScrollbarOn:
	    self._horizScrollbar.grid(row = 6, column = 4, sticky = 'news')
	    interior.grid_rowconfigure(5, minsize = self['scrollmargin'])
	else:
	    self._horizScrollbar.grid_forget()
	    interior.grid_rowconfigure(5, minsize = 0)

    def _toggleVertScrollbar(self):

	self._vertScrollbarOn = not self._vertScrollbarOn

	interior = self.interior()
	if self._vertScrollbarOn:
	    self._vertScrollbar.grid(row = 4, column = 6, sticky = 'news')
	    interior.grid_columnconfigure(5, minsize = self['scrollmargin'])
	else:
	    self._vertScrollbar.grid_forget()
	    interior.grid_columnconfigure(5, minsize = 0)

    # Need to explicitly forward this to override the stupid
    # (grid_)bbox method inherited from Tkinter.Frame.Grid.
    def bbox(self, index):
	return self._textbox.bbox(index)

Pmw.forwardmethods(ScrolledText, Tkinter.Text, '_textbox')
