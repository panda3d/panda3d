# --------------------------------------------------------------------------------- #
# PYCOLLAPSIBLEPANE wxPython IMPLEMENTATION
# Generic Implementation Based On wx.CollapsiblePane.
#
# Andrea Gavana, @ 09 Aug 2007
# Latest Revision: 12 Apr 2010, 12.00 GMT
#
#
# For All Kind Of Problems, Requests Of Enhancements And Bug Reports, Please
# Write To Me At:
#
# gavana@kpo.kz
# andrea.gavana@gmail.com
#
# Or, Obviously, To The wxPython Mailing List!!!
#
#
# End Of Comments
# --------------------------------------------------------------------------------- #

"""
PyCollapsiblePane is a container with an embedded button-like control which
can be used by the user to collapse or expand the pane's contents.


Description
===========

A collapsible pane is a container with an embedded button-like control which
can be used by the user to collapse or expand the pane's contents.
Once constructed you should use the L{GetPane} function to access the pane and
add your controls inside it (i.e. use the window returned from L{GetPane} as the
parent for the controls which must go in the pane, **not** the PyCollapsiblePane
itself!).

:note: Note that because of its nature of control which can dynamically (and drastically)
 change its size at run-time under user-input, when putting PyCollapsiblePane
 inside a `wx.Sizer` you should be careful to add it with a proportion value of zero;
 this is because otherwise all other windows with non-null proportion values would
 automatically get resized each time the user expands or collapse the pane window
 resulting usually in a weird, flickering effect.


Usage
=====

A simple example::

    collpane = PyCollapsiblePane(self, -1, "Details:")

    # add the pane with a zero proportion value to the 'sz' sizer which contains it
    sz.Add(collpane, 0, wx.GROW|wx.ALL, 5)

    # now add a test label in the collapsible pane using a sizer to layout it:
    win = collpane.GetPane()
    paneSz = wx.BoxSizer(wx.VERTICAL)
    paneSz.Add(wx.StaticText(win, -1, "test!"), 1, wx.GROW|wx.ALL, 2)
    win.SetSizer(paneSz)
    paneSz.SetSizeHints(win)



Window Styles
=============

This class supports the following window styles:

==================== =========== ==================================================
Window Styles        Hex Value   Description
==================== =========== ==================================================
``CP_NO_TLW_RESIZE``         0x2 By default `PyCollapsiblePane` resizes the top level window containing it when its own size changes. This allows to easily implement dialogs containing an optionally shown part, for example, and so is the default behaviour but can be inconvenient in some specific cases -- use this flag to disable this automatic parent resizing then.
``CP_GTK_EXPANDER``          0x4 Uses a GTK expander instead of a button.
``CP_USE_STATICBOX``         0x8 Uses a `wx.StaticBox` around `PyCollapsiblePane`.
``CP_LINE_ABOVE``           0x10 Draws a line above `PyCollapsiblePane`.
==================== =========== ==================================================


Events Processing
=================

This class processes the following events:

=============================== ==================================================
Event Name                      Description
=============================== ==================================================
``EVT_COLLAPSIBLEPANE_CHANGED`` The user showed or hid the collapsible pane.
=============================== ==================================================


License And Version
===================

PyCollapsiblePane is distributed under the wxPython license. 

Latest Revision: Andrea Gavana @ 12 Apr 2010, 12.00 GMT

Version 0.4

"""

import wx

CP_GTK_EXPANDER = 4
""" Uses a GTK expander instead of a button. """
CP_USE_STATICBOX = 8
""" Uses a `wx.StaticBox` around `PyCollapsiblePane`. """
CP_LINE_ABOVE = 16
""" Draws a line above `PyCollapsiblePane`. """
CP_DEFAULT_STYLE = wx.CP_DEFAULT_STYLE
""" The default style. It includes ``wx.TAB_TRAVERSAL`` and ``wx.BORDER_NONE``. """
CP_NO_TLW_RESIZE = wx.CP_NO_TLW_RESIZE
""" By default `PyCollapsiblePane` resizes the top level window containing it when its""" \
""" own size changes. This allows to easily implement dialogs containing an optionally""" \
""" shown part, for example, and so is the default behaviour but can be inconvenient""" \
""" in some specific cases -- use this flag to disable this automatic parent resizing then. """

# inject into the wx namespace with the other CP_* constants
wx.CP_GTK_EXPANDER = CP_GTK_EXPANDER
wx.CP_USE_STATICBOX = CP_USE_STATICBOX
wx.CP_LINE_ABOVE = CP_LINE_ABOVE

# PyCollapsiblePane events
EVT_COLLAPSIBLEPANE_CHANGED = wx.EVT_COLLAPSIBLEPANE_CHANGED
""" The user showed or hid the collapsible pane. """

#-----------------------------------------------------------------------------
# 2.9 deprecates SetSpacer, so we'll use AssignSpacer and monkey-patch 2.8 so
# it works there too

if wx.VERSION < (2, 9):
    wx.SizerItem.AssignSpacer = wx.SizerItem.SetSpacer

#-----------------------------------------------------------------------------
# GTKExpander widget
#-----------------------------------------------------------------------------

class GTKExpander(wx.PyControl):
    """
    A GtkExpander allows the user to hide or show its child by clicking on an expander
    triangle.
    """
    
    def __init__(self, parent, id=wx.ID_ANY, label="", pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.NO_BORDER):
        """
        Default class constructor.

        :param `parent`: the L{GTKExpander} parent. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `label`: the expander text label;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the expander style.
        """

        wx.PyControl.__init__(self, parent, id, pos, size, style)
        self.SetLabel(label)
        
        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        self._parent = parent
        
        self.Bind(wx.EVT_SIZE, self.OnSize)
        

    def OnDrawGTKExpander(self, dc):
        """
        Draws the L{GTKExpander} triangle.

        :param `dc`: an instance of `wx.DC`.
        """

        size = self.GetSize()
        label = self._parent.GetBtnLabel()
        triangleWidth, triangleHeight = self._parent.GetExpanderDimensions()
        textWidth, textHeight, descent, externalLeading = dc.GetFullTextExtent(label, self.GetFont())
        
        dc.SetBrush(wx.BLACK_BRUSH)
        dc.SetPen(wx.BLACK_PEN)
        
        if self._parent.IsCollapsed():
            startX, startY = triangleWidth/2, size.y - triangleHeight - 1 - descent
            pt1 = wx.Point(startX, startY)
            pt2 = wx.Point(startX, size.y - 1 - descent)
            pt3 = wx.Point(startX+triangleWidth, size.y-triangleHeight/2 - 1 - descent)
        else:

            startX, startY = 0, size.y - triangleWidth - descent - 1
            pt1 = wx.Point(startX, startY)
            pt2 = wx.Point(startX+triangleHeight, startY)
            pt3 = wx.Point(startX+triangleHeight/2, size.y - descent - 1)

        dc.DrawPolygon([pt1, pt2, pt3])            
                

    def OnDrawGTKText(self, dc):
        """
        Draws the L{GTKExpander} text label.

        :param `dc`: an instance of `wx.DC`.
        """

        size = self.GetSize()
        label = self._parent.GetBtnLabel()
        triangleWidth, triangleHeight = self._parent.GetExpanderDimensions()
        textWidth, textHeight, descent, externalLeading = dc.GetFullTextExtent(label, self.GetFont())
        dc.SetFont(self.GetFont())

        startX, startY = 2*triangleHeight+1, size.y - textHeight
        dc.DrawText(label, startX, startY)


    def DoGetBestSize(self):
        """
        Gets the size which best suits the window: for a control, it would be the
        minimal size which doesn't truncate the control, for a panel - the same size
        as it would have after a call to `Fit()`.
        """

        triangleWidth, triangleHeight = self._parent.GetExpanderDimensions()
        label = self._parent.GetBtnLabel()
        dc = wx.ClientDC(self)
        textWidth, textHeight, descent, externalLeading = dc.GetFullTextExtent(label, self.GetFont())

        maxHeight = max(textHeight+descent, triangleHeight)
        maxWidth = 2*triangleHeight+1 + textWidth

        return wx.Size(maxWidth, maxHeight)        


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{GTKExpander}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        self.Refresh()



#-----------------------------------------------------------------------------
# PyCollapsiblePane
#-----------------------------------------------------------------------------

class PyCollapsiblePane(wx.PyPanel):
    """
    PyCollapsiblePane is a container with an embedded button-like control which
    can be used by the user to collapse or expand the pane's contents.
    """

    def __init__(self, parent, id=wx.ID_ANY, label="", pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.CP_DEFAULT_STYLE, agwStyle=0,
                 validator=wx.DefaultValidator, name="CollapsiblePane"):
        """
        Default class constructor.

        :param `parent`: the L{PyCollapsiblePane} parent. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `label`: The initial label shown in the button which allows the
         user to expand or collapse the pane window.
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.PyControl` window style;
        :param `agwStyle`: the AGW-specifi window style. This can be a combination of the
         following bits:

         ==================== =========== ==================================================
         Window Styles        Hex Value   Description
         ==================== =========== ==================================================
         ``CP_NO_TLW_RESIZE``         0x2 By default L{PyCollapsiblePane} resizes the top level window containing it when its own size changes. This allows to easily implement dialogs containing an optionally shown part, for example, and so is the default behaviour but can be inconvenient in some specific cases -- use this flag to disable this automatic parent resizing then.
         ``CP_GTK_EXPANDER``          0x4 Uses a GTK expander instead of a button.
         ``CP_USE_STATICBOX``         0x8 Uses a `wx.StaticBox` around L{PyCollapsiblePane}.
         ``CP_LINE_ABOVE``           0x10 Draws a line above L{PyCollapsiblePane}.
         ==================== =========== ==================================================

        :param `validator`: the validator associated to the L{PyCollapsiblePane};
        :param `name`: the widget name.
        
        """
                
        wx.PyPanel.__init__(self, parent, id, pos, size, style, name)
        
        self._pButton = self._pStaticLine = self._pPane = self._sz = None            
        self._strLabel = label
        self._bCollapsed = True
        self._agwStyle = agwStyle

        self._pPane = wx.Panel(self, style=wx.TAB_TRAVERSAL|wx.NO_BORDER)
        self._pPane.Hide()

        if self.HasAGWFlag(CP_USE_STATICBOX):
            # Use a StaticBox instead of a StaticLine, and the button's
            # position will be handled separately so don't put it in the sizer
            self._pStaticBox = wx.StaticBox(self)
            self.SetButton(wx.Button(self, wx.ID_ANY, self.GetLabel(), style=wx.BU_EXACTFIT))
            self._sz = wx.BoxSizer(wx.VERTICAL)
            self._sz.Add((1,1))  # spacer, size will be reset later
            self._contentSizer = wx.StaticBoxSizer(self._pStaticBox, wx.VERTICAL)
            self._contentSizer.Add((1,1))  # spacer, size will be reset later
            self._contentSizer.Add(self._pPane, 1, wx.EXPAND)
            self._sz.Add(self._contentSizer, 1, wx.EXPAND)                    

            if self.HasAGWFlag(CP_USE_STATICBOX) and 'wxMSW' in wx.PlatformInfo:
                # This hack is needed on Windows because wxMSW clears the
                # CLIP_SIBLINGS style from all sibling controls that overlap the
                # static box, so the box ends up overdrawing the button since we
                # have the button overlapping the box. This hack will ensure that
                # the button is refreshed after every time that the box is drawn.
                # This adds a little flicker but it is not too bad compared to
                # others.
                def paint(evt):
                    def updateBtn():
                        self._pButton.Refresh()
                        self._pButton.Update()
                    wx.CallAfter(updateBtn)
                    evt.Skip()
                self._pStaticBox.Bind(wx.EVT_PAINT, paint)

        elif self.HasAGWFlag(CP_GTK_EXPANDER):
            self._sz = wx.BoxSizer(wx.HORIZONTAL)
            self.SetExpanderDimensions(3, 6)
            self.SetButton(GTKExpander(self, wx.ID_ANY, self.GetLabel()))
            self._sz.Add(self._pButton, 0, wx.LEFT|wx.TOP|wx.BOTTOM, self.GetBorder())

            self._pButton.Bind(wx.EVT_PAINT, self.OnDrawGTKStyle)
            self._pButton.Bind(wx.EVT_LEFT_DOWN, self.OnButton)
            if wx.Platform == "__WXMSW__":
                self._pButton.Bind(wx.EVT_LEFT_DCLICK, self.OnButton)

        else:
            # create children and lay them out using a wx.BoxSizer
            # (so that we automatically get RTL features)
            self.SetButton(wx.Button(self, wx.ID_ANY, self.GetLabel(), style=wx.BU_EXACTFIT))
            self._pStaticLine = wx.StaticLine(self, wx.ID_ANY)

            if self.HasAGWFlag(CP_LINE_ABOVE): 
                # put the static line above the button
                self._sz = wx.BoxSizer(wx.VERTICAL)
                self._sz.Add(self._pStaticLine, 0, wx.ALL|wx.GROW, self.GetBorder())
                self._sz.Add(self._pButton, 0, wx.LEFT|wx.RIGHT|wx.BOTTOM, self.GetBorder())
            else:
                # arrange the static line and the button horizontally
                self._sz = wx.BoxSizer(wx.HORIZONTAL)
                self._sz.Add(self._pButton, 0, wx.LEFT|wx.TOP|wx.BOTTOM, self.GetBorder())
                self._sz.Add(self._pStaticLine, 1, wx.ALIGN_CENTER|wx.LEFT|wx.RIGHT, self.GetBorder())
            
        self.Bind(wx.EVT_SIZE, self.OnSize)


    def SetAGWWindowStyleFlag(self, agwStyle):
        """
        Sets the L{PyCollapsiblePane} window style flags.

        :param `agwStyle`: the AGW-specific window style. This can be a combination of the
         following bits:

         ==================== =========== ==================================================
         Window Styles        Hex Value   Description
         ==================== =========== ==================================================
         ``CP_NO_TLW_RESIZE``         0x2 By default L{PyCollapsiblePane} resizes the top level window containing it when its own size changes. This allows to easily implement dialogs containing an optionally shown part, for example, and so is the default behaviour but can be inconvenient in some specific cases -- use this flag to disable this automatic parent resizing then.
         ``CP_GTK_EXPANDER``          0x4 Uses a GTK expander instead of a button.
         ``CP_USE_STATICBOX``         0x8 Uses a `wx.StaticBox` around L{PyCollapsiblePane}.
         ``CP_LINE_ABOVE``           0x10 Draws a line above L{PyCollapsiblePane}.
         ==================== =========== ==================================================

        """

        self._agwStyle = agwStyle
        self.Layout()
        self.Refresh()


    def GetAGWWindowStyleFlag(self):
        """
        Returns the L{PyCollapsiblePane} window style.

        :see: L{SetAGWWindowStyleFlag} for a list of possible window style flags.        
        """

        return self._agwStyle


    def HasAGWFlag(self, flag):
        """
        Returns whether a flag is present in the L{PyCollapsiblePane} style.

        :param `flag`: one of the possible L{PyCollapsiblePane} window styles.

        :see: L{SetAGWWindowStyleFlag} for a list of possible window style flags.
        """

        agwStyle = self.GetAGWWindowStyleFlag()
        res = (agwStyle & flag and [True] or [False])[0]
        return res
    

    def GetBtnLabel(self):
        """ Returns the button label. """

        if self.GetAGWWindowStyleFlag() & CP_GTK_EXPANDER:
            return self.GetLabel()
        
        return self.GetLabel() + (self.IsCollapsed() and [" >>"] or [" <<"])[0]


    def OnStateChange(self, sz):
        """
        Handles the status changes (collapsing/expanding).

        :param `sz`: an instance of `wx.Size`.
        """

        # minimal size has priority over the best size so set here our min size
        self.SetMinSize(sz)
        self.SetSize(sz)

        if self.HasAGWFlag(wx.CP_NO_TLW_RESIZE):
            # the user asked to explicitely handle the resizing itself...
            return
        
        # NB: the following block of code has been accurately designed to
        #     as much flicker-free as possible be careful when modifying it!

        top = wx.GetTopLevelParent(self)
        if top:
            # NB: don't Layout() the 'top' window as its size has not been correctly
            #     updated yet and we don't want to do an initial Layout() with the old
            #     size immediately followed by a SetClientSize/Fit call for the new
            #     size that would provoke flickering!

            if top.GetSizer():
                if (wx.Platform == "__WXGTK__" and self.IsCollapsed()) or wx.Platform != "__WXGTK__":
                # FIXME: the SetSizeHints() call would be required also for GTK+ for
                #        the expanded.collapsed transition. Unfortunately if we
                #        enable this line, then the GTK+ top window won't always be
                #        resized by the SetClientSize() call below! As a side effect
                #        of this dirty fix, the minimal size for the pane window is
                #        not set in GTK+ and the user can hide it shrinking the "top"
                #        window...

                    top.GetSizer().SetSizeHints(top)


            # we shouldn't attempt to resize a maximized window, whatever happens
            if not top.IsMaximized():
                
                if self.IsCollapsed():
                    # expanded . collapsed transition
                    if top.GetSizer():
                        # we have just set the size hints...
                        sz = top.GetSizer().CalcMin()

                        # use SetClientSize() and not SetSize() otherwise the size for
                        # e.g. a wxFrame with a menubar wouldn't be correctly set
                        top.SetClientSize(sz)
                    
                    else:
                        
                        top.Layout()
                
                else:
                
                    # collapsed . expanded transition

                    # force our parent to "fit", i.e. expand so that it can honour
                    # our minimal size
                    top.Fit()
                

    def Collapse(self, collapse=True):
        """
        Collapses or expands the pane window.

        :param `collapse`: ``True`` to collapse the pane window, ``False`` to expand it.
        """

        # optimization
        if self.IsCollapsed() == collapse:
            return

        self.Freeze()
        
        # update our state
        self._pPane.Show(not collapse)
        self._bCollapsed = collapse
        self.Thaw()

        # update button label
        # NB: this must be done after updating our "state"
        self._pButton.SetLabel(self.GetBtnLabel())

        self.OnStateChange(self.GetBestSize())
        

    def Expand(self):
        """ Same as L{Collapse} (False). """

        self.Collapse(False)


    def IsCollapsed(self):
        """ Returns ``True`` if the pane window is currently hidden. """

        return self._bCollapsed        


    def IsExpanded(self):
        """ Returns ``True`` if the pane window is currently shown. """

        return not self.IsCollapsed()        


    def GetPane(self):
        """
        Returns a reference to the pane window. Use the returned `wx.Window` as
        the parent of widgets to make them part of the collapsible area.
        """

        return self._pPane
    
        
    def SetLabel(self, label):
        """
        Sets the button label.

        :param `label`: the new button label.

        :note: Overridden from `wx.PyControl`.
        """

        self._strLabel =  label
        self._pButton.SetLabel(self.GetBtnLabel())
        self._pButton.SetInitialSize()
        self._pButton.Refresh()

        self.Layout()


    def GetLabel(self):
        """
        Returns the button label.

        :note: Overridden from `wx.PyControl`.
        """
        
        return self._strLabel


    def SetButtonFont(self, font):
        """
        Sets the button font.

        :param `font`: a valid `wx.Font` object.
        """
        
        self._pButton.SetFont(font)
        self.Layout()    


    def GetButtonFont(self):
        """ Returns the button font. """
        
        return self._pButton.GetFont()


    def GetBorder(self):
        """ Returns the L{PyCollapsiblePane} border in pixels (platform dependent). """

        if wx.Platform == "__WXMAC__":
            return 6
        elif wx.Platform == "__WXGTK__":
            return 3
        elif wx.Platform == "__WXMSW__":
            return self._pButton.ConvertDialogSizeToPixels(wx.Size(2, 0)).x
        else:
            return 5
        

    def SetExpanderDimensions(self, width, height):
        """
        Sets the expander width and height.

        :param `width`: an integer specifying the expander width in pixels;
        :param `height`: an integer specifying the expander height in pixels.
        """
        
        self._expanderDimensions = width, height
        if self._sz:
            self._sz.Layout()
        if self._pButton:
            self._pButton.Refresh()


    def GetExpanderDimensions(self):
        """
        Returns the expander dimensions, a tuple of integers representing the
        width and height of the expander, in pixels.
        """

        return self._expanderDimensions
    
        
    def DoGetBestSize(self):
        """
        Gets the size which best suits the window: for a control, it would be the
        minimal size which doesn't truncate the control, for a panel - the same size
        as it would have after a call to `Fit()`.
        """

        if self.HasAGWFlag(CP_USE_STATICBOX):
            # In this case the button is not in the sizer, and the static box
            # is not shown when not expanded, so use the size of the button as
            # our stating point. But first we'll make sure that it is it's
            # best size.
            sz = self._pButton.GetBestSize()
            self._pButton.SetSize(sz)
            bdr = self.GetBorder()
            sz += (2*bdr, 2*bdr)
                        
            if self.IsExpanded():
                # Reset the spacer sizes to be based on the button size
                adjustment = 0
                if 'wxMac' not in wx.PlatformInfo:
                    adjustment = -3
                self._sz.Children[0].AssignSpacer((1, sz.height/2 + adjustment))
                self._contentSizer.Children[0].AssignSpacer((1, sz.height/2))

                ssz = self._sz.GetMinSize()
                sz.width = max(sz.width, ssz.width)
                sz.height = max(sz.height, ssz.height)
                
        else:
            # do not use GetSize() but rather GetMinSize() since it calculates
            # the required space of the sizer
            sz = self._sz.GetMinSize()

            # when expanded, we need more space
            if self.IsExpanded():
                pbs = self._pPane.GetBestSize()
                sz.width = max(sz.GetWidth(), pbs.x)
                sz.height = sz.y + self.GetBorder() + pbs.y
        
        return sz

    
    def Layout(self):
        """ Layout the L{PyCollapsiblePane}. """

        if not self._pButton or not self._pPane or not self._sz:
            return False     # we need to complete the creation first!

        oursz = self.GetSize()

        if self.HasAGWFlag(CP_USE_STATICBOX):
            bdr = self.GetBorder()
            self._pButton.SetPosition((bdr, bdr))
            
            if self.IsExpanded():
                self._pPane.Show()
                self._pPane.Layout()
                self._pStaticBox.Show()
                self._pStaticBox.Lower()
                self._pButton.Raise()
                self._sz.SetDimension(0, 0, oursz.width, oursz.height)
                self._sz.Layout()
            else:
                if hasattr(self, '_pStaticBox'):
                    self._pStaticBox.Hide()
                
        else:    
            # move & resize the button and the static line
            self._sz.SetDimension(0, 0, oursz.GetWidth(), self._sz.GetMinSize().GetHeight())
            self._sz.Layout()
    
            if self.IsExpanded():
            
                # move & resize the container window
                yoffset = self._sz.GetSize().GetHeight() + self.GetBorder()
                self._pPane.SetDimensions(0, yoffset, oursz.x, oursz.y - yoffset)
    
                # this is very important to make the pane window layout show correctly
                self._pPane.Show()
                self._pPane.Layout()
        
        return True

    
    def SetButton(self, button):
        """
        Assign a new button to L{PyCollapsiblePane}.

        :param `button`: can be the standard `wx.Button` or any of the generic
         implementations which live in `wx.lib.buttons`.
        """
        
        if self._pButton:
            if not self.HasAGWFlag(CP_USE_STATICBOX):
                self._sz.Replace(self._pButton, button)
            self.Unbind(wx.EVT_BUTTON, self._pButton)
            self._pButton.Destroy()

        self._pButton = button
        self.SetLabel(button.GetLabel())
        self.Bind(wx.EVT_BUTTON, self.OnButton, self._pButton)

        if self._pPane:
            self._pButton.MoveBeforeInTabOrder(self._pPane)
        self.Layout()
        
            
    def GetButton(self):
        """ Returns the button associated with L{PyCollapsiblePane}. """
        
        return self._pButton
    
    
    
#-----------------------------------------------------------------------------
# PyCollapsiblePane - event handlers
#-----------------------------------------------------------------------------

    def OnButton(self, event):
        """
        Handles the ``wx.EVT_BUTTON`` event for L{PyCollapsiblePane}.

        :param `event`: a `wx.CommandEvent` event to be processed.
        """

        if event.GetEventObject() != self._pButton:
            event.Skip()
            return

        self.Collapse(not self.IsCollapsed())

        # this change was generated by the user - send the event
        ev = wx.CollapsiblePaneEvent(self, self.GetId(), self.IsCollapsed())
        self.GetEventHandler().ProcessEvent(ev)


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{PyCollapsiblePane}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        self.Layout()


    def OnDrawGTKStyle(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{PyCollapsiblePane}.

        :param `event`: a `wx.PaintEvent` event to be processed.

        :note: This is a drawing routine to paint the GTK-style expander.
        """

        dc = wx.AutoBufferedPaintDC(self._pButton)
        dc.SetBackground(wx.Brush(self.GetBackgroundColour()))
        dc.Clear()
        
        self.OnDrawGTKExpander(dc)
        self.OnDrawGTKText(dc)
        

    def OnDrawGTKExpander(self, dc):
        """
        Overridable method to draw the GTK-style expander.

        :param `dc`: an instance of `wx.DC`.
        """

        self._pButton.OnDrawGTKExpander(dc)


    def OnDrawGTKText(self, dc):
        """
        Overridable method to draw the L{PyCollapsiblePane} text in the expander.

        :param `dc`: an instance of `wx.DC`.
        """

        self._pButton.OnDrawGTKText(dc)

