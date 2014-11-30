#----------------------------------------------------------------------
# Name:        wx.lib.activex
# Purpose:     The 3rd (and hopefully final) implementation of an
#              ActiveX container for wxPython.
#
# Author:      Robin Dunn
#
# Created:     5-June-2008
# RCS-ID:      $Id: $
# Copyright:   (c) 2008 by Total Control Software
# Licence:     wxWindows license
#----------------------------------------------------------------------


"""
This module provides a wx.Window that hosts ActiveX Controls using
just the ctypes and comtypes packages.  This provides a light-weight
COM implementation with full dynamic dispatch support.

The only requirements are ctypes (included with Python 2.5 and
available separately for earlier versions of Python) and the comtypes
package, which is available from
http://starship.python.net/crew/theller/comtypes/.  Be sure to get at
least version 0.5, which at the time of this writing is only available
from SVN.  You can fetch it with easy_install with a command like
this:

    easy_install http://svn.python.org/projects/ctypes/trunk/comtypes

"""

import wx

import ctypes as ct
import ctypes.wintypes as wt
import comtypes
import comtypes.client as cc
import comtypes.hresult as hr

import sys, os
if not hasattr(sys, 'frozen'):
    f = os.path.join(os.path.dirname(__file__), 'myole4ax.tlb')
    cc.GetModule(f)
from comtypes.gen import myole4ax


kernel32 = ct.windll.kernel32
user32 = ct.windll.user32
atl = ct.windll.atl

WS_CHILD        = 0x40000000
WS_VISIBLE      = 0x10000000
WS_CLIPCHILDREN = 0x2000000
WS_CLIPSIBLINGS = 0x4000000
CW_USEDEFAULT   = 0x80000000
WM_KEYDOWN      = 256
WM_DESTROY      = 2

#------------------------------------------------------------------------------

class ActiveXCtrl(wx.PyAxBaseWindow):
    """
    A wx.Window for hosting ActiveX controls.  The COM interface of
    the ActiveX control is accessible through the ctrl property of
    this class, and this class is also set as the event sink for COM
    events originating from the ActiveX control.  In other words, to
    catch the COM events you mearly have to derive from this class and
    provide a method with the correct name.  See the comtypes package
    documentation for more details.
    """
    
    def __init__(self, parent, axID, wxid=-1, pos=wx.DefaultPosition, 
                 size=wx.DefaultSize, style=0, name="activeXCtrl"):
        """
        All parameters are like those used in normal wx.Windows with
        the addition of axID which is a string that is either a ProgID
        or a CLSID used to identify the ActiveX control.
        """
        pos = wx.Point(*pos)    # in case the arg is a tuple
        size = wx.Size(*size)   # ditto
        
        x = pos.x
        y = pos.y
        if x == -1: x = CW_USEDEFAULT
        if y == -1: y = 20
        w = size.width
        h = size.height
        if w == -1: w = 20
        if h == -1: h = 20
        
        # create the control
        atl.AtlAxWinInit()
        hInstance = kernel32.GetModuleHandleA(None)
        hwnd = user32.CreateWindowExA(0, "AtlAxWin", axID,
                                      WS_CHILD | WS_VISIBLE 
                                      | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                      x,y, w,h, parent.GetHandle(), None, 
                                      hInstance, 0)
        assert hwnd != 0
        
        # get the Interface for the Ax control
        unknown = ct.POINTER(comtypes.IUnknown)()
        res = atl.AtlAxGetControl(hwnd, ct.byref(unknown))
        assert res == hr.S_OK
        self._ax = cc.GetBestInterface(unknown)
        
        # Fetch the interface for IOleInPlaceActiveObject. We'll use this
        # later to call its TranslateAccelerator method so the AX Control can
        # deal with things like tab traversal and such within itself.
        self.ipao = self._ax.QueryInterface(myole4ax.IOleInPlaceActiveObject)
        
        # Use this object as the event sink for the ActiveX events
        self._evt_connections = []
        self.AddEventSink(self)
        
        # Turn the window handle into a wx.Window and set this object to be that window
        win = wx.PyAxBaseWindow_FromHWND(parent, hwnd)
        self.PostCreate(win)
        
        # Set some wx.Window properties
        if wxid == wx.ID_ANY: 
            wxid = wx.Window.NewControlId()
        self.SetId(wxid)
        self.SetName(name)
        self.SetMinSize(size)
        
        self.Bind(wx.EVT_SET_FOCUS, self.OnSetFocus)
        self.Bind(wx.EVT_KILL_FOCUS, self.OnKillFocus)
        self.Bind(wx.EVT_WINDOW_DESTROY, self.OnDestroyWindow)
        
    def AddEventSink(self, sink, interface=None):
        """
        Add a new target to search for method names that match the COM
        Event names.
        """
        self._evt_connections.append(cc.GetEvents(self._ax, sink, interface))
        
    def GetCtrl(self):
        """Easy access to the COM interface for the ActiveX Control"""
        return self._ax
    # And an even easier property
    ctrl = property(GetCtrl)


    def MSWTranslateMessage(self, msg):
        # Pass native messages to the IOleInPlaceActiveObject
        # interface before wx processes them, so navigation keys and
        # accelerators can be dealt with the way that the AXControl
        # wants them to be done. MSWTranslateMessage is called before
        # wxWidgets handles and eats the navigation keys itself.
        res = self.ipao.TranslateAccelerator(msg)   
        if res == hr.S_OK:
            return True
        else:
            return wx.PyAxBaseWindow.MSWTranslateMessage(self, msg)

    
    # TBD: Are the focus handlers needed?
    def OnSetFocus(self, evt):
        self.ipao.OnFrameWindowActivate(True)
    
    def OnKillFocus(self, evt):
        self.ipao.OnFrameWindowActivate(False)

    def OnDestroyWindow(self, evt):
        # release our event sinks while the window still exists
        self._evt_connections = None
        
#------------------------------------------------------------------------------


                                 
                                 
