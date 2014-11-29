import Tkinter
import Pmw

class ScrolledCanvas(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
	    ('borderframe',    0,            INITOPT),
	    ('canvasmargin',   0,            INITOPT),
	    ('hscrollmode',    'dynamic',    self._hscrollMode),
	    ('labelmargin',    0,            INITOPT),
	    ('labelpos',       None,         INITOPT),
	    ('scrollmargin',   2,            INITOPT),
	    ('usehullsize',    0,            INITOPT),
	    ('vscrollmode',    'dynamic',    self._vscrollMode),
	)
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

	# Create the components.
	self.origInterior = Pmw.MegaWidget.interior(self)

	if self['usehullsize']:
	    self.origInterior.grid_propagate(0)

	if self['borderframe']:
	    # Create a frame widget to act as the border of the canvas. 
	    self._borderframe = self.createcomponent('borderframe',
		    (), None,
		    Tkinter.Frame, (self.origInterior,),
		    relief = 'sunken',
		    borderwidth = 2,
	    )
	    self._borderframe.grid(row = 2, column = 2, sticky = 'news')

	    # Create the canvas widget.
	    self._canvas = self.createcomponent('canvas',
		    (), None,
		    Tkinter.Canvas, (self._borderframe,),
		    highlightthickness = 0,
		    borderwidth = 0,
	    )
	    self._canvas.pack(fill = 'both', expand = 1)
	else:
	    # Create the canvas widget.
	    self._canvas = self.createcomponent('canvas',
		    (), None,
		    Tkinter.Canvas, (self.origInterior,),
		    relief = 'sunken',
		    borderwidth = 2,
	    )
	    self._canvas.grid(row = 2, column = 2, sticky = 'news')

	self.origInterior.grid_rowconfigure(2, weight = 1, minsize = 0)
	self.origInterior.grid_columnconfigure(2, weight = 1, minsize = 0)
	
	# Create the horizontal scrollbar
	self._horizScrollbar = self.createcomponent('horizscrollbar',
		(), 'Scrollbar',
		Tkinter.Scrollbar, (self.origInterior,),
	        orient='horizontal',
		command=self._canvas.xview
	)

	# Create the vertical scrollbar
	self._vertScrollbar = self.createcomponent('vertscrollbar',
		(), 'Scrollbar',
		Tkinter.Scrollbar, (self.origInterior,),
		orient='vertical',
		command=self._canvas.yview
	)

	self.createlabel(self.origInterior, childCols = 3, childRows = 3)

	# Initialise instance variables.
	self._horizScrollbarOn = 0
	self._vertScrollbarOn = 0
	self.scrollTimer = None
        self._scrollRecurse = 0
	self._horizScrollbarNeeded = 0
	self._vertScrollbarNeeded = 0
	self.setregionTimer = None

	# Check keywords and initialise options.
	self.initialiseoptions()

    def destroy(self):
	if self.scrollTimer is not None:
	    self.after_cancel(self.scrollTimer)
	    self.scrollTimer = None
	if self.setregionTimer is not None:
	    self.after_cancel(self.setregionTimer)
	    self.setregionTimer = None
	Pmw.MegaWidget.destroy(self)

    # ======================================================================

    # Public methods.

    def interior(self):
	return self._canvas

    def resizescrollregion(self):
	if self.setregionTimer is None:
	    self.setregionTimer = self.after_idle(self._setRegion)

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
        tclCommandName = str(self._canvas.cget('xscrollcommand'))
        if tclCommandName != '':   
            self._canvas.deletecommand(tclCommandName)
        tclCommandName = str(self._canvas.cget('yscrollcommand'))
        if tclCommandName != '':   
            self._canvas.deletecommand(tclCommandName)

	if self['hscrollmode'] == self['vscrollmode'] == 'dynamic':
            self._canvas.configure(
                    xscrollcommand=self._scrollBothLater,
                    yscrollcommand=self._scrollBothLater
            )
        else:
            self._canvas.configure(
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
	# Called by the canvas to set the horizontal or vertical
	# scrollbar when it has scrolled or changed scrollregion.

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

	xview = self._canvas.xview()
	yview = self._canvas.yview()
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

	interior = self.origInterior
	if self._horizScrollbarOn:
	    self._horizScrollbar.grid(row = 4, column = 2, sticky = 'news')
	    interior.grid_rowconfigure(3, minsize = self['scrollmargin'])
	else:
	    self._horizScrollbar.grid_forget()
	    interior.grid_rowconfigure(3, minsize = 0)

    def _toggleVertScrollbar(self):

	self._vertScrollbarOn = not self._vertScrollbarOn

	interior = self.origInterior
	if self._vertScrollbarOn:
	    self._vertScrollbar.grid(row = 2, column = 4, sticky = 'news')
	    interior.grid_columnconfigure(3, minsize = self['scrollmargin'])
	else:
	    self._vertScrollbar.grid_forget()
	    interior.grid_columnconfigure(3, minsize = 0)

    def _setRegion(self):
	self.setregionTimer = None

	region = self._canvas.bbox('all')
        if region is not None:
	    canvasmargin = self['canvasmargin']
	    region = (region[0] - canvasmargin, region[1] - canvasmargin,
		region[2] + canvasmargin, region[3] + canvasmargin)
	    self._canvas.configure(scrollregion = region)

    # Need to explicitly forward this to override the stupid
    # (grid_)bbox method inherited from Tkinter.Frame.Grid.
    def bbox(self, *args):
	return apply(self._canvas.bbox, args)

Pmw.forwardmethods(ScrolledCanvas, Tkinter.Canvas, '_canvas')
