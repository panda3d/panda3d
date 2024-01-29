import string
import types
import tkinter
import Pmw

class ScrolledFrame(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):

        # Define the megawidget options.
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('borderframe',    1,            INITOPT),
            ('horizflex',      'fixed',      self._horizflex),
            ('horizfraction',  0.05,         INITOPT),
            ('hscrollmode',    'dynamic',    self._hscrollMode),
            ('labelmargin',    0,            INITOPT),
            ('labelpos',       None,         INITOPT),
            ('scrollmargin',   2,            INITOPT),
            ('usehullsize',    0,            INITOPT),
            ('vertflex',       'fixed',      self._vertflex),
            ('vertfraction',   0.05,         INITOPT),
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
            # Create a frame widget to act as the border of the clipper.
            self._borderframe = self.createcomponent('borderframe',
                    (), None,
                    tkinter.Frame, (self.origInterior,),
                    relief = 'sunken',
                    borderwidth = 2,
            )
            self._borderframe.grid(row = 2, column = 2, sticky = 'news')

            # Create the clipping window.
            self._clipper = self.createcomponent('clipper',
                    (), None,
                    tkinter.Frame, (self._borderframe,),
                    width = 400,
                    height = 300,
                    highlightthickness = 0,
                    borderwidth = 0,
            )
            self._clipper.pack(fill = 'both', expand = 1)
        else:
            # Create the clipping window.
            self._clipper = self.createcomponent('clipper',
                    (), None,
                    tkinter.Frame, (self.origInterior,),
                    width = 400,
                    height = 300,
                    relief = 'sunken',
                    borderwidth = 2,
            )
            self._clipper.grid(row = 2, column = 2, sticky = 'news')

        self.origInterior.grid_rowconfigure(2, weight = 1, minsize = 0)
        self.origInterior.grid_columnconfigure(2, weight = 1, minsize = 0)

        # Create the horizontal scrollbar
        self._horizScrollbar = self.createcomponent('horizscrollbar',
                (), 'Scrollbar',
                tkinter.Scrollbar, (self.origInterior,),
                orient='horizontal',
                command=self.xview
        )

        # Create the vertical scrollbar
        self._vertScrollbar = self.createcomponent('vertscrollbar',
                (), 'Scrollbar',
                tkinter.Scrollbar, (self.origInterior,),
                orient='vertical',
                command=self.yview
        )

        self.createlabel(self.origInterior, childCols = 3, childRows = 3)

        # Initialise instance variables.
        self._horizScrollbarOn = 0
        self._vertScrollbarOn = 0
        self.scrollTimer = None
        self._scrollRecurse = 0
        self._horizScrollbarNeeded = 0
        self._vertScrollbarNeeded = 0
        self.startX = 0
        self.startY = 0
        self._flexoptions = ('fixed', 'expand', 'shrink', 'elastic')

        # Create a frame in the clipper to contain the widgets to be
        # scrolled.
        self._frame = self.createcomponent('frame',
                (), None,
                tkinter.Frame, (self._clipper,)
        )

        # Whenever the clipping window or scrolled frame change size,
        # update the scrollbars.
        self._frame.bind('<Configure>', self._reposition)
        self._clipper.bind('<Configure>', self._reposition)

        # Work around a bug in Tk where the value returned by the
        # scrollbar get() method is (0.0, 0.0, 0.0, 0.0) rather than
        # the expected 2-tuple.  This occurs if xview() is called soon
        # after the Pmw.ScrolledFrame has been created.
        self._horizScrollbar.set(0.0, 1.0)
        self._vertScrollbar.set(0.0, 1.0)

        # Check keywords and initialise options.
        self.initialiseoptions()

    def destroy(self):
        if self.scrollTimer is not None:
            self.after_cancel(self.scrollTimer)
            self.scrollTimer = None
        Pmw.MegaWidget.destroy(self)

    # ======================================================================

    # Public methods.

    def interior(self):
        return self._frame

    # Set timer to call real reposition method, so that it is not
    # called multiple times when many things are reconfigured at the
    # same time.
    def reposition(self):
        if self.scrollTimer is None:
            self.scrollTimer = self.after_idle(self._scrollBothNow)

    # Called when the user clicks in the horizontal scrollbar.
    # Calculates new position of frame then calls reposition() to
    # update the frame and the scrollbar.
    def xview(self, mode = None, value = None, units = None):

        if type(value) is str:
            value = float(value)
        if mode is None:
            return self._horizScrollbar.get()
        elif mode == 'moveto':
            frameWidth = self._frame.winfo_reqwidth()
            self.startX = value * float(frameWidth)
        else: # mode == 'scroll'
            clipperWidth = self._clipper.winfo_width()
            if units == 'units':
                jump = int(clipperWidth * self['horizfraction'])
            else:
                jump = clipperWidth
            self.startX = self.startX + value * jump

        self.reposition()

    # Called when the user clicks in the vertical scrollbar.
    # Calculates new position of frame then calls reposition() to
    # update the frame and the scrollbar.
    def yview(self, mode = None, value = None, units = None):

        if type(value) is str:
            value = float(value)
        if mode is None:
            return self._vertScrollbar.get()
        elif mode == 'moveto':
            frameHeight = self._frame.winfo_reqheight()
            self.startY = value * float(frameHeight)
        else: # mode == 'scroll'
            clipperHeight = self._clipper.winfo_height()
            if units == 'units':
                jump = int(clipperHeight * self['vertfraction'])
            else:
                jump = clipperHeight
            self.startY = self.startY + value * jump

        self.reposition()

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
            raise ValueError(message)

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
            raise ValueError(message)

    def _horizflex(self):
        # The horizontal flex mode has been configured.

        flex = self['horizflex']

        if flex not in self._flexoptions:
            message = 'bad horizflex option "%s": should be one of %s' % \
                    (flex, str(self._flexoptions))
            raise ValueError(message)

        self.reposition()

    def _vertflex(self):
        # The vertical flex mode has been configured.

        flex = self['vertflex']

        if flex not in self._flexoptions:
            message = 'bad vertflex option "%s": should be one of %s' % \
                    (flex, str(self._flexoptions))
            raise ValueError(message)

        self.reposition()

    # ======================================================================

    # Private methods.

    def _reposition(self, event):
        self.reposition()

    def _getxview(self):

        # Horizontal dimension.
        clipperWidth = self._clipper.winfo_width()
        frameWidth = self._frame.winfo_reqwidth()
        if frameWidth <= clipperWidth:
            # The scrolled frame is smaller than the clipping window.

            self.startX = 0
            endScrollX = 1.0

            if self['horizflex'] in ('expand', 'elastic'):
                relwidth = 1
            else:
                relwidth = ''
        else:
            # The scrolled frame is larger than the clipping window.

            if self['horizflex'] in ('shrink', 'elastic'):
                self.startX = 0
                endScrollX = 1.0
                relwidth = 1
            else:
                if self.startX + clipperWidth > frameWidth:
                    self.startX = frameWidth - clipperWidth
                    endScrollX = 1.0
                else:
                    if self.startX < 0:
                        self.startX = 0
                    endScrollX = (self.startX + clipperWidth) / float(frameWidth)
                relwidth = ''

        # Position frame relative to clipper.
        self._frame.place(x = -self.startX, relwidth = relwidth)
        return (self.startX / float(frameWidth), endScrollX)

    def _getyview(self):

        # Vertical dimension.
        clipperHeight = self._clipper.winfo_height()
        frameHeight = self._frame.winfo_reqheight()
        if frameHeight <= clipperHeight:
            # The scrolled frame is smaller than the clipping window.

            self.startY = 0
            endScrollY = 1.0

            if self['vertflex'] in ('expand', 'elastic'):
                relheight = 1
            else:
                relheight = ''
        else:
            # The scrolled frame is larger than the clipping window.

            if self['vertflex'] in ('shrink', 'elastic'):
                self.startY = 0
                endScrollY = 1.0
                relheight = 1
            else:
                if self.startY + clipperHeight > frameHeight:
                    self.startY = frameHeight - clipperHeight
                    endScrollY = 1.0
                else:
                    if self.startY < 0:
                        self.startY = 0
                    endScrollY = (self.startY + clipperHeight) / float(frameHeight)
                relheight = ''

        # Position frame relative to clipper.
        self._frame.place(y = -self.startY, relheight = relheight)
        return (self.startY / float(frameHeight), endScrollY)

    # According to the relative geometries of the frame and the
    # clipper, reposition the frame within the clipper and reset the
    # scrollbars.
    def _scrollBothNow(self):
        self.scrollTimer = None

        # Call update_idletasks to make sure that the containing frame
        # has been resized before we attempt to set the scrollbars.
        # Otherwise the scrollbars may be mapped/unmapped continuously.
        self._scrollRecurse = self._scrollRecurse + 1
        self.update_idletasks()
        self._scrollRecurse = self._scrollRecurse - 1
        if self._scrollRecurse != 0:
            return

        xview = self._getxview()
        yview = self._getyview()
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
