#----------------------------------------------------------------------------
# Name:         pydocview.py
# Purpose:      Python extensions to the wxWindows docview framework
#
# Author:       Peter Yared, Morgan Hua, Matt Fryer
#
# Created:      5/15/03
# CVS-ID:       $Id: pydocview.py 57781 2009-01-02 18:14:51Z RD $
# Copyright:    (c) 2003-2006 ActiveGrid, Inc.
# License:      wxWindows license
#----------------------------------------------------------------------------


import wx
import wx.lib.docview
import sys
import getopt
from wx.lib.rcsizer import RowColSizer
import os
import os.path
import time
import string
import pickle
import tempfile
import mmap
_ = wx.GetTranslation
if wx.Platform == '__WXMSW__':
    _WINDOWS = True
else:
    _WINDOWS = False

#----------------------------------------------------------------------------
# Constants
#----------------------------------------------------------------------------

VIEW_TOOLBAR_ID = wx.NewId()
VIEW_STATUSBAR_ID = wx.NewId()

EMBEDDED_WINDOW_TOP = 1
EMBEDDED_WINDOW_BOTTOM = 2
EMBEDDED_WINDOW_LEFT = 4
EMBEDDED_WINDOW_RIGHT = 8
EMBEDDED_WINDOW_TOPLEFT = 16
EMBEDDED_WINDOW_BOTTOMLEFT = 32
EMBEDDED_WINDOW_TOPRIGHT = 64
EMBEDDED_WINDOW_BOTTOMRIGHT = 128
EMBEDDED_WINDOW_ALL = EMBEDDED_WINDOW_TOP | EMBEDDED_WINDOW_BOTTOM | EMBEDDED_WINDOW_LEFT | EMBEDDED_WINDOW_RIGHT | \
                      EMBEDDED_WINDOW_TOPLEFT | EMBEDDED_WINDOW_BOTTOMLEFT | EMBEDDED_WINDOW_TOPRIGHT | EMBEDDED_WINDOW_BOTTOMRIGHT

SAVEALL_ID = wx.NewId()

WINDOW_MENU_NUM_ITEMS = 9


class DocFrameMixIn:
    """
    Class with common code used by DocMDIParentFrame, DocTabbedParentFrame, and
    DocSDIFrame.
    """


    def GetDocumentManager(self):
        """
        Returns the document manager associated with the DocMDIParentFrame.
        """
        return self._docManager


    def InitializePrintData(self):
        """
        Initializes the PrintData that is used when printing.
        """
        self._printData = wx.PrintData()
        self._printData.SetPaperId(wx.PAPER_LETTER)


    def CreateDefaultMenuBar(self, sdi=False):
        """
        Creates the default MenuBar.  Contains File, Edit, View, Tools, and Help menus.
        """
        menuBar = wx.MenuBar()

        fileMenu = wx.Menu()
        fileMenu.Append(wx.ID_NEW, _("&New...\tCtrl+N"), _("Creates a new document"))
        fileMenu.Append(wx.ID_OPEN, _("&Open...\tCtrl+O"), _("Opens an existing document"))
        fileMenu.Append(wx.ID_CLOSE, _("&Close"), _("Closes the active document"))
        if not sdi:
            fileMenu.Append(wx.ID_CLOSE_ALL, _("Close A&ll"), _("Closes all open documents"))
        fileMenu.AppendSeparator()
        fileMenu.Append(wx.ID_SAVE, _("&Save\tCtrl+S"), _("Saves the active document"))
        fileMenu.Append(wx.ID_SAVEAS, _("Save &As..."), _("Saves the active document with a new name"))
        fileMenu.Append(SAVEALL_ID, _("Save All\tCtrl+Shift+A"), _("Saves the all active documents"))
        wx.EVT_MENU(self, SAVEALL_ID, self.ProcessEvent)
        wx.EVT_UPDATE_UI(self, SAVEALL_ID, self.ProcessUpdateUIEvent)
        fileMenu.AppendSeparator()
        fileMenu.Append(wx.ID_PRINT, _("&Print\tCtrl+P"), _("Prints the active document"))
        fileMenu.Append(wx.ID_PREVIEW, _("Print Pre&view"), _("Displays full pages"))
        fileMenu.Append(wx.ID_PRINT_SETUP, _("Page Set&up"), _("Changes page layout settings"))
        fileMenu.AppendSeparator()
        if wx.Platform == '__WXMAC__':
            fileMenu.Append(wx.ID_EXIT, _("&Quit"), _("Closes this program"))
        else:
            fileMenu.Append(wx.ID_EXIT, _("E&xit"), _("Closes this program"))
        self._docManager.FileHistoryUseMenu(fileMenu)
        self._docManager.FileHistoryAddFilesToMenu()
        menuBar.Append(fileMenu, _("&File"));

        editMenu = wx.Menu()
        editMenu.Append(wx.ID_UNDO, _("&Undo\tCtrl+Z"), _("Reverses the last action"))
        editMenu.Append(wx.ID_REDO, _("&Redo\tCtrl+Y"), _("Reverses the last undo"))
        editMenu.AppendSeparator()
        #item = wxMenuItem(self.editMenu, wxID_CUT, _("Cu&t\tCtrl+X"), _("Cuts the selection and puts it on the Clipboard"))
        #item.SetBitmap(getCutBitmap())
        #editMenu.AppendItem(item)
        editMenu.Append(wx.ID_CUT, _("Cu&t\tCtrl+X"), _("Cuts the selection and puts it on the Clipboard"))
        wx.EVT_MENU(self, wx.ID_CUT, self.ProcessEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_CUT, self.ProcessUpdateUIEvent)
        editMenu.Append(wx.ID_COPY, _("&Copy\tCtrl+C"), _("Copies the selection and puts it on the Clipboard"))
        wx.EVT_MENU(self, wx.ID_COPY, self.ProcessEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_COPY, self.ProcessUpdateUIEvent)
        editMenu.Append(wx.ID_PASTE, _("&Paste\tCtrl+V"), _("Inserts Clipboard contents"))
        wx.EVT_MENU(self, wx.ID_PASTE, self.ProcessEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_PASTE, self.ProcessUpdateUIEvent)
        editMenu.Append(wx.ID_CLEAR, _("&Delete"), _("Erases the selection"))
        wx.EVT_MENU(self, wx.ID_CLEAR, self.ProcessEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_CLEAR, self.ProcessUpdateUIEvent)
        editMenu.AppendSeparator()
        editMenu.Append(wx.ID_SELECTALL, _("Select A&ll\tCtrl+A"), _("Selects all available data"))
        wx.EVT_MENU(self, wx.ID_SELECTALL, self.ProcessEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_SELECTALL, self.ProcessUpdateUIEvent)
        menuBar.Append(editMenu, _("&Edit"))
        if sdi:
            if self.GetDocument() and self.GetDocument().GetCommandProcessor():
                self.GetDocument().GetCommandProcessor().SetEditMenu(editMenu)

        viewMenu = wx.Menu()
        viewMenu.AppendCheckItem(VIEW_TOOLBAR_ID, _("&Toolbar"), _("Shows or hides the toolbar"))
        wx.EVT_MENU(self, VIEW_TOOLBAR_ID, self.OnViewToolBar)
        wx.EVT_UPDATE_UI(self, VIEW_TOOLBAR_ID, self.OnUpdateViewToolBar)
        viewMenu.AppendCheckItem(VIEW_STATUSBAR_ID, _("&Status Bar"), _("Shows or hides the status bar"))
        wx.EVT_MENU(self, VIEW_STATUSBAR_ID, self.OnViewStatusBar)
        wx.EVT_UPDATE_UI(self, VIEW_STATUSBAR_ID, self.OnUpdateViewStatusBar)
        menuBar.Append(viewMenu, _("&View"))

        helpMenu = wx.Menu()
        helpMenu.Append(wx.ID_ABOUT, _("&About" + " " + wx.GetApp().GetAppName()), _("Displays program information, version number, and copyright"))
        menuBar.Append(helpMenu, _("&Help"))

        wx.EVT_MENU(self, wx.ID_ABOUT, self.OnAbout)
        wx.EVT_UPDATE_UI(self, wx.ID_ABOUT, self.ProcessUpdateUIEvent)  # Using ID_ABOUT to update the window menu, the window menu items are not triggering

        if sdi:  # TODO: Is this really needed?
            wx.EVT_COMMAND_FIND_CLOSE(self, -1, self.ProcessEvent)

        return menuBar


    def CreateDefaultStatusBar(self):
        """
        Creates the default StatusBar.
        """
        wx.Frame.CreateStatusBar(self)
        self.GetStatusBar().Show(wx.ConfigBase_Get().ReadInt("ViewStatusBar", True))
        self.UpdateStatus()
        return self.GetStatusBar()


    def CreateDefaultToolBar(self):
        """
        Creates the default ToolBar.
        """
        self._toolBar = self.CreateToolBar(wx.TB_HORIZONTAL | wx.NO_BORDER | wx.TB_FLAT)
        self._toolBar.AddSimpleTool(wx.ID_NEW, New.GetBitmap(), _("New"), _("Creates a new document"))
        self._toolBar.AddSimpleTool(wx.ID_OPEN, Open.GetBitmap(), _("Open"), _("Opens an existing document"))
        self._toolBar.AddSimpleTool(wx.ID_SAVE, Save.GetBitmap(), _("Save"), _("Saves the active document"))
        self._toolBar.AddSimpleTool(SAVEALL_ID, SaveAll.GetBitmap(), _("Save All"), _("Saves all the active documents"))
        self._toolBar.AddSeparator()
        self._toolBar.AddSimpleTool(wx.ID_PRINT, Print.GetBitmap(), _("Print"), _("Displays full pages"))
        self._toolBar.AddSimpleTool(wx.ID_PREVIEW, PrintPreview.GetBitmap(), _("Print Preview"), _("Prints the active document"))
        self._toolBar.AddSeparator()
        self._toolBar.AddSimpleTool(wx.ID_CUT, Cut.GetBitmap(), _("Cut"), _("Cuts the selection and puts it on the Clipboard"))
        self._toolBar.AddSimpleTool(wx.ID_COPY, Copy.GetBitmap(), _("Copy"), _("Copies the selection and puts it on the Clipboard"))
        self._toolBar.AddSimpleTool(wx.ID_PASTE, Paste.GetBitmap(), _("Paste"), _("Inserts Clipboard contents"))
        self._toolBar.AddSimpleTool(wx.ID_UNDO, Undo.GetBitmap(), _("Undo"), _("Reverses the last action"))
        self._toolBar.AddSimpleTool(wx.ID_REDO, Redo.GetBitmap(), _("Redo"), _("Reverses the last undo"))
        self._toolBar.Realize()
        self._toolBar.Show(wx.ConfigBase_Get().ReadInt("ViewToolBar", True))

        return self._toolBar


    def OnFileSaveAll(self, event):
        """
        Saves all of the currently open documents.
        """
        docs = wx.GetApp().GetDocumentManager().GetDocuments()

        # save child documents first
        for doc in docs:
            if isinstance(doc, wx.lib.pydocview.ChildDocument):
                doc.Save()

        # save parent and other documents later
        for doc in docs:
            if not isinstance(doc, wx.lib.pydocview.ChildDocument):
                doc.Save()


    def OnAbout(self, event):
        """
        Invokes the about dialog.
        """
        aboutService = wx.GetApp().GetService(AboutService)
        if aboutService:
            aboutService.ShowAbout()


    def OnViewToolBar(self, event):
        """
        Toggles whether the ToolBar is visible.
        """
        self._toolBar.Show(not self._toolBar.IsShown())
        self._LayoutFrame()


    def OnUpdateViewToolBar(self, event):
        """
        Updates the View ToolBar menu item.
        """
        event.Check(self.GetToolBar().IsShown())


    def OnViewStatusBar(self, event):
        """
        Toggles whether the StatusBar is visible.
        """
        self.GetStatusBar().Show(not self.GetStatusBar().IsShown())
        self._LayoutFrame()


    def OnUpdateViewStatusBar(self, event):
        """
        Updates the View StatusBar menu item.
        """
        event.Check(self.GetStatusBar().IsShown())


    def UpdateStatus(self, message = _("Ready")):
        """
        Updates the StatusBar.
        """
        # wxBug: Menubar and toolbar help strings don't pop the status text back
        if self.GetStatusBar().GetStatusText() != message:
            self.GetStatusBar().PushStatusText(message)


class DocMDIParentFrameMixIn:
    """
    Class with common code used by DocMDIParentFrame and DocTabbedParentFrame.
    """


    def _GetPosSizeFromConfig(self, pos, size):
        """
        Adjusts the position and size of the frame using the saved config position and size.
        """
        config = wx.ConfigBase_Get()
        if pos == wx.DefaultPosition and size == wx.DefaultSize and config.ReadInt("MDIFrameMaximized", False):
            pos = [0, 0]
            size = wx.DisplaySize()
            # wxBug: Need to set to fill screen to get around bug where maximize is leaving shadow of statusbar, check out maximize call at end of this function
        else:
            if pos == wx.DefaultPosition:
                pos = config.ReadInt("MDIFrameXLoc", -1), config.ReadInt("MDIFrameYLoc", -1)

            if wx.Display_GetFromPoint(pos) == -1:  # Check if the frame position is offscreen
                pos = wx.DefaultPosition

            if size == wx.DefaultSize:
                size = wx.Size(config.ReadInt("MDIFrameXSize", 450), config.ReadInt("MDIFrameYSize", 300))
        return pos, size


    def _InitFrame(self, embeddedWindows, minSize):
        """
        Initializes the frame and creates the default menubar, toolbar, and status bar.
        """
        self._embeddedWindows = []
        self.SetDropTarget(_DocFrameFileDropTarget(self._docManager, self))

        if wx.GetApp().GetDefaultIcon():
            self.SetIcon(wx.GetApp().GetDefaultIcon())

        wx.EVT_MENU(self, wx.ID_ABOUT, self.OnAbout)
        wx.EVT_SIZE(self, self.OnSize)

        self.InitializePrintData()

        toolBar = self.CreateDefaultToolBar()
        self.SetToolBar(toolBar)
        menuBar = self.CreateDefaultMenuBar()
        statusBar = self.CreateDefaultStatusBar()

        config = wx.ConfigBase_Get()
        if config.ReadInt("MDIFrameMaximized", False):
            # wxBug: On maximize, statusbar leaves a residual that needs to be refereshed, happens even when user does it
            self.Maximize()

        self.CreateEmbeddedWindows(embeddedWindows, minSize)
        self._LayoutFrame()

        if wx.Platform == '__WXMAC__':
            self.SetMenuBar(menuBar)  # wxBug: Have to set the menubar at the very end or the automatic MDI "window" menu doesn't get put in the right place when the services add new menus to the menubar

        wx.GetApp().SetTopWindow(self)  # Need to do this here in case the services are looking for wx.GetApp().GetTopWindow()
        for service in wx.GetApp().GetServices():
            service.InstallControls(self, menuBar = menuBar, toolBar = toolBar, statusBar = statusBar)
            if hasattr(service, "ShowWindow"):
                service.ShowWindow()  # instantiate service windows for correct positioning, we'll hide/show them later based on user preference

        if wx.Platform != '__WXMAC__':
            self.SetMenuBar(menuBar)  # wxBug: Have to set the menubar at the very end or the automatic MDI "window" menu doesn't get put in the right place when the services add new menus to the menubar


    def ProcessEvent(self, event):
        """
        Processes an event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        id = event.GetId()
        if id == SAVEALL_ID:
            self.OnFileSaveAll(event)
            return True

        return wx.GetApp().ProcessEvent(event)


    def ProcessUpdateUIEvent(self, event):
        """
        Processes a UI event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        id = event.GetId()
        if id == wx.ID_CUT:
            event.Enable(False)
            return True
        elif id == wx.ID_COPY:
            event.Enable(False)
            return True
        elif id == wx.ID_PASTE:
            event.Enable(False)
            return True
        elif id == wx.ID_CLEAR:
            event.Enable(False)
            return True
        elif id == wx.ID_SELECTALL:
            event.Enable(False)
            return True
        elif id == SAVEALL_ID:
            filesModified = False
            docs = wx.GetApp().GetDocumentManager().GetDocuments()
            for doc in docs:
                if doc.IsModified():
                    filesModified = True
                    break

            event.Enable(filesModified)
            return True
        else:
            return wx.GetApp().ProcessUpdateUIEvent(event)


    def CreateEmbeddedWindows(self, windows=0, minSize=20):
        """
        Create the specified embedded windows around the edges of the frame.
        """
        frameSize = self.GetSize()   # TODO: GetClientWindow.GetSize is still returning 0,0 since the frame isn't fully constructed yet, so using full frame size
        defaultHSize = max(minSize, int(frameSize[0] / 6))
        defaultVSize = max(minSize, int(frameSize[1] / 7))
        defaultSubVSize = int(frameSize[1] / 2)
        config = wx.ConfigBase_Get()
        if windows & (EMBEDDED_WINDOW_LEFT | EMBEDDED_WINDOW_TOPLEFT | EMBEDDED_WINDOW_BOTTOMLEFT):
            self._leftEmbWindow = self._CreateEmbeddedWindow(self, (max(minSize,config.ReadInt("MDIEmbedLeftSize", defaultHSize)), -1), wx.LAYOUT_VERTICAL, wx.LAYOUT_LEFT, visible = config.ReadInt("MDIEmbedLeftVisible", 1), sash = wx.SASH_RIGHT)
        else:
            self._leftEmbWindow = None
        if windows & EMBEDDED_WINDOW_TOPLEFT:
            self._topLeftEmbWindow = self._CreateEmbeddedWindow(self._leftEmbWindow, (-1, config.ReadInt("MDIEmbedTopLeftSize", defaultSubVSize)), wx.LAYOUT_HORIZONTAL, wx.LAYOUT_TOP, visible = config.ReadInt("MDIEmbedTopLeftVisible", 1), sash = wx.SASH_BOTTOM)
        else:
            self._topLeftEmbWindow = None
        if windows & EMBEDDED_WINDOW_BOTTOMLEFT:
            self._bottomLeftEmbWindow = self._CreateEmbeddedWindow(self._leftEmbWindow, (-1, config.ReadInt("MDIEmbedBottomLeftSize", defaultSubVSize)), wx.LAYOUT_HORIZONTAL, wx.LAYOUT_BOTTOM, visible = config.ReadInt("MDIEmbedBottomLeftVisible", 1))
        else:
            self._bottomLeftEmbWindow = None
        if windows & (EMBEDDED_WINDOW_RIGHT | EMBEDDED_WINDOW_TOPRIGHT | EMBEDDED_WINDOW_BOTTOMRIGHT):
            self._rightEmbWindow = self._CreateEmbeddedWindow(self, (max(minSize,config.ReadInt("MDIEmbedRightSize", defaultHSize)), -1), wx.LAYOUT_VERTICAL, wx.LAYOUT_RIGHT, visible = config.ReadInt("MDIEmbedRightVisible", 1), sash = wx.SASH_LEFT)
        else:
            self._rightEmbWindow = None
        if windows & EMBEDDED_WINDOW_TOPRIGHT:
            self._topRightEmbWindow = self._CreateEmbeddedWindow(self._rightEmbWindow, (-1, config.ReadInt("MDIEmbedTopRightSize", defaultSubVSize)), wx.LAYOUT_HORIZONTAL, wx.LAYOUT_TOP, visible = config.ReadInt("MDIEmbedTopRightVisible", 1), sash = wx.SASH_BOTTOM)
        else:
            self._topRightEmbWindow = None
        if windows & EMBEDDED_WINDOW_BOTTOMRIGHT:
            self._bottomRightEmbWindow = self._CreateEmbeddedWindow(self._rightEmbWindow, (-1, config.ReadInt("MDIEmbedBottomRightSize", defaultSubVSize)), wx.LAYOUT_HORIZONTAL, wx.LAYOUT_BOTTOM, visible = config.ReadInt("MDIEmbedBottomRightVisible", 1))
        else:
            self._bottomRightEmbWindow = None
        if windows & EMBEDDED_WINDOW_TOP:
            self._topEmbWindow = self._CreateEmbeddedWindow(self, (-1, max(minSize,config.ReadInt("MDIEmbedTopSize", defaultVSize))), wx.LAYOUT_HORIZONTAL, wx.LAYOUT_TOP, visible = config.ReadInt("MDIEmbedTopVisible", 1), sash = wx.SASH_BOTTOM)
        else:
            self._topEmbWindow = None
        if windows & EMBEDDED_WINDOW_BOTTOM:
            self._bottomEmbWindow = self._CreateEmbeddedWindow(self, (-1, max(minSize,config.ReadInt("MDIEmbedBottomSize", defaultVSize))), wx.LAYOUT_HORIZONTAL, wx.LAYOUT_BOTTOM, visible = config.ReadInt("MDIEmbedBottomVisible", 1), sash = wx.SASH_TOP)
        else:
            self._bottomEmbWindow = None


    def SaveEmbeddedWindowSizes(self):
        """
        Saves the sizes of the embedded windows.
        """
        config = wx.ConfigBase_Get()
        if not self.IsMaximized():
            config.WriteInt("MDIFrameXLoc", self.GetPositionTuple()[0])
            config.WriteInt("MDIFrameYLoc", self.GetPositionTuple()[1])
            config.WriteInt("MDIFrameXSize", self.GetSizeTuple()[0])
            config.WriteInt("MDIFrameYSize", self.GetSizeTuple()[1])
        config.WriteInt("MDIFrameMaximized", self.IsMaximized())
        config.WriteInt("ViewToolBar", self._toolBar.IsShown())
        config.WriteInt("ViewStatusBar", self.GetStatusBar().IsShown())

        if self._leftEmbWindow:
            config.WriteInt("MDIEmbedLeftSize", self._leftEmbWindow.GetSize()[0])
            config.WriteInt("MDIEmbedLeftVisible", self._leftEmbWindow.IsShown())
        if self._topLeftEmbWindow:
            if self._topLeftEmbWindow._sizeBeforeHidden:
                size = self._topLeftEmbWindow._sizeBeforeHidden[1]
            else:
                size = self._topLeftEmbWindow.GetSize()[1]
            config.WriteInt("MDIEmbedTopLeftSize", size)
            config.WriteInt("MDIEmbedTopLeftVisible", self._topLeftEmbWindow.IsShown())
        if self._bottomLeftEmbWindow:
            if self._bottomLeftEmbWindow._sizeBeforeHidden:
                size = self._bottomLeftEmbWindow._sizeBeforeHidden[1]
            else:
                size = self._bottomLeftEmbWindow.GetSize()[1]
            config.WriteInt("MDIEmbedBottomLeftSize", size)
            config.WriteInt("MDIEmbedBottomLeftVisible", self._bottomLeftEmbWindow.IsShown())
        if self._rightEmbWindow:
            config.WriteInt("MDIEmbedRightSize", self._rightEmbWindow.GetSize()[0])
            config.WriteInt("MDIEmbedRightVisible", self._rightEmbWindow.IsShown())
        if self._topRightEmbWindow:
            if self._topRightEmbWindow._sizeBeforeHidden:
                size = self._topRightEmbWindow._sizeBeforeHidden[1]
            else:
                size = self._topRightEmbWindow.GetSize()[1]
            config.WriteInt("MDIEmbedTopRightSize", size)
            config.WriteInt("MDIEmbedTopRightVisible", self._topRightEmbWindow.IsShown())
        if self._bottomRightEmbWindow:
            if self._bottomRightEmbWindow._sizeBeforeHidden:
                size = self._bottomRightEmbWindow._sizeBeforeHidden[1]
            else:
                size = self._bottomRightEmbWindow.GetSize()[1]
            config.WriteInt("MDIEmbedBottomRightSize", size)
            config.WriteInt("MDIEmbedBottomRightVisible", self._bottomRightEmbWindow.IsShown())
        if self._topEmbWindow:
            config.WriteInt("MDIEmbedTopSize", self._topEmbWindow.GetSize()[1])
            config.WriteInt("MDIEmbedTopVisible", self._topEmbWindow.IsShown())
        if self._bottomEmbWindow:
            config.WriteInt("MDIEmbedBottomSize", self._bottomEmbWindow.GetSize()[1])
            config.WriteInt("MDIEmbedBottomVisible", self._bottomEmbWindow.IsShown())


    def GetEmbeddedWindow(self, loc):
        """
        Returns the instance of the embedded window specified by the embedded window location constant.
        """
        if loc == EMBEDDED_WINDOW_TOP:
            return self._topEmbWindow
        elif loc == EMBEDDED_WINDOW_BOTTOM:
            return self._bottomEmbWindow
        elif loc == EMBEDDED_WINDOW_LEFT:
            return self._leftEmbWindow
        elif loc == EMBEDDED_WINDOW_RIGHT:
            return self._rightEmbWindow
        elif loc == EMBEDDED_WINDOW_TOPLEFT:
            return self._topLeftEmbWindow
        elif loc == EMBEDDED_WINDOW_BOTTOMLEFT:
            return self._bottomLeftEmbWindow
        elif loc == EMBEDDED_WINDOW_TOPRIGHT:
            return self._topRightEmbWindow
        elif loc == EMBEDDED_WINDOW_BOTTOMRIGHT:
            return self._bottomRightEmbWindow
        return None


    def _CreateEmbeddedWindow(self, parent, size, orientation, alignment, visible=True, sash=None):
        """
        Creates the embedded window with the specified size, orientation, and alignment.  If the
        window is not visible it will retain the size with which it was last viewed.
        """
        window = wx.SashLayoutWindow(parent, wx.NewId(), style = wx.NO_BORDER | wx.SW_3D)
        window.SetDefaultSize(size)
        window.SetOrientation(orientation)
        window.SetAlignment(alignment)
        if sash != None:  # wx.SASH_TOP is 0 so check for None instead of just doing "if sash:"
            window.SetSashVisible(sash, True)
        ####
        def OnEmbeddedWindowSashDrag(event):
            if event.GetDragStatus() == wx.SASH_STATUS_OUT_OF_RANGE:
                return
            sashWindow = event.GetEventObject()
            if sashWindow.GetAlignment() == wx.LAYOUT_TOP or sashWindow.GetAlignment() == wx.LAYOUT_BOTTOM:
                size = wx.Size(-1, event.GetDragRect().height)
            else:
                size = wx.Size(event.GetDragRect().width, -1)
            event.GetEventObject().SetDefaultSize(size)
            self._LayoutFrame()
            sashWindow.Refresh()
            if isinstance(sashWindow.GetParent(), wx.SashLayoutWindow):
                sashWindow.Show()
                parentSashWindow = sashWindow.GetParent()  # Force a refresh
                parentSashWindow.Layout()
                parentSashWindow.Refresh()
                parentSashWindow.SetSize((parentSashWindow.GetSize().width + 1, parentSashWindow.GetSize().height + 1))
        ####
        wx.EVT_SASH_DRAGGED(window, window.GetId(), OnEmbeddedWindowSashDrag)
        window._sizeBeforeHidden = None
        if not visible:
            window.Show(False)
            if isinstance(parent, wx.SashLayoutWindow): # It's a window embedded in another sash window so remember its actual size to show it again
                window._sizeBeforeHidden = size
        return window


    def ShowEmbeddedWindow(self, window, show=True):
        """
        Shows or hides the embedded window specified by the embedded window location constant.
        """
        window.Show(show)
        if isinstance(window.GetParent(), wx.SashLayoutWindow):  # It is a parent sashwindow with multiple embedded sashwindows
            parentSashWindow = window.GetParent()
            if show:  # Make sure it is visible in case all of the subwindows were hidden
                parentSashWindow.Show()
            if show and window._sizeBeforeHidden:
                if window._sizeBeforeHidden[1] == parentSashWindow.GetClientSize()[1]:
                    if window == self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMLEFT) and self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPLEFT).IsShown():
                        window.SetDefaultSize((window._sizeBeforeHidden[0], window._sizeBeforeHidden[0] - self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPLEFT).GetSize()[1]))
                    elif window == self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPLEFT) and self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMLEFT).IsShown():
                        window.SetDefaultSize((window._sizeBeforeHidden[0], window._sizeBeforeHidden[0] - self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMLEFT).GetSize()[1]))
                    elif window == self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMRIGHT) and self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPRIGHT).IsShown():
                        window.SetDefaultSize((window._sizeBeforeHidden[0], window._sizeBeforeHidden[0] - self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPRIGHT).GetSize()[1]))
                    elif window == self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPRIGHT) and self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMRIGHT).IsShown():
                        window.SetDefaultSize((window._sizeBeforeHidden[0], window._sizeBeforeHidden[0] - self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMRIGHT).GetSize()[1]))
                else:
                    window.SetDefaultSize(window._sizeBeforeHidden)
                    # If it is not the size of the full parent sashwindow set the other window's size so that if it gets shown it will have a cooresponding size
                    if window._sizeBeforeHidden[1] < parentSashWindow.GetClientSize()[1]:
                        otherWindowSize = (-1, parentSashWindow.GetClientSize()[1] - window._sizeBeforeHidden[1])
                        if window == self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMLEFT):
                            self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPLEFT).SetDefaultSize(otherWindowSize)
                        elif window == self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPLEFT):
                            self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMLEFT).SetDefaultSize(otherWindowSize)
                        elif window == self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMRIGHT):
                            self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPRIGHT).SetDefaultSize(otherWindowSize)
                        elif window == self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPRIGHT):
                            self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMRIGHT).SetDefaultSize(otherWindowSize)

            if not show:
                if window == self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMRIGHT) and not self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPRIGHT).IsShown() \
                    or window == self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPRIGHT) and not self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMRIGHT).IsShown() \
                    or window == self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMLEFT) and not self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPLEFT).IsShown() \
                    or window == self.GetEmbeddedWindow(EMBEDDED_WINDOW_TOPLEFT) and not self.GetEmbeddedWindow(EMBEDDED_WINDOW_BOTTOMLEFT).IsShown():
                    parentSashWindow.Hide()  # Hide the parent sashwindow if all of the children are hidden
            parentSashWindow.Layout()   # Force a refresh
            parentSashWindow.Refresh()
            parentSashWindow.SetSize((parentSashWindow.GetSize().width + 1, parentSashWindow.GetSize().height + 1))
        self._LayoutFrame()


    def HideEmbeddedWindow(self, window):
        """
        Hides the embedded window specified by the embedded window location constant.
        """
        self.ShowEmbeddedWindow(window, show=False)


class DocTabbedChildFrame(wx.Panel):
    """
    The wxDocMDIChildFrame class provides a default frame for displaying
    documents on separate windows. This class can only be used for MDI child
    frames.

    The class is part of the document/view framework supported by wxWindows,
    and cooperates with the wxView, wxDocument, wxDocManager and wxDocTemplate
    classes.
    """


    def __init__(self, doc, view, frame, id, title, pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE, name="frame"):
        """
        Constructor.  Note that the event table must be rebuilt for the
        frame since the EvtHandler is not virtual.
        """
        wx.Panel.__init__(self, frame.GetNotebook(), id)
        self._childDocument = doc
        self._childView = view
        frame.AddNotebookPage(self, doc.GetPrintableName())
        if view:
            view.SetFrame(self)


    def GetIcon(self):
        """
        Dummy method since the icon of tabbed frames are managed by the notebook.
        """
        return None


    def SetIcon(self, icon):
        """
        Dummy method since the icon of tabbed frames are managed by the notebook.
        """
        pass


    def Destroy(self):
        """
        Removes the current notebook page.
        """
        wx.GetApp().GetTopWindow().RemoveNotebookPage(self)


    def SetFocus(self):
        """
        Activates the current notebook page.
        """
        wx.GetApp().GetTopWindow().ActivateNotebookPage(self)


    def Activate(self):  # Need this in case there are embedded sash windows and such, OnActivate is not getting called
        """
        Activates the current view.
        """
        # Called by Project Editor
        if self._childView:
            self._childView.Activate(True)


    def GetTitle(self):
        """
        Returns the frame's title.
        """
        return wx.GetApp().GetTopWindow().GetNotebookPageTitle(self)


    def SetTitle(self, title):
        """
        Sets the frame's title.
        """
        wx.GetApp().GetTopWindow().SetNotebookPageTitle(self, title)


    def OnTitleIsModified(self):
        """
        Add/remove to the frame's title an indication that the document is dirty.
        If the document is dirty, an '*' is appended to the title
        """
        title = self.GetTitle()
        if title:
            if self.GetDocument().IsModified():
                if not title.endswith("*"):
                    title = title + "*"
                    self.SetTitle(title)
            else:
                if title.endswith("*"):
                    title = title[:-1]
                    self.SetTitle(title)
        

    def ProcessEvent(event):
        """
        Processes an event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        if not self._childView or not self._childView.ProcessEvent(event):
            if not isinstance(event, wx.CommandEvent) or not self.GetParent() or not self.GetParent().ProcessEvent(event):
                return False
            else:
                return True
        else:
            return True


    def GetDocument(self):
        """
        Returns the document associated with this frame.
        """
        return self._childDocument


    def SetDocument(self, document):
        """
        Sets the document for this frame.
        """
        self._childDocument = document


    def GetView(self):
        """
        Returns the view associated with this frame.
        """
        return self._childView


    def SetView(self, view):
        """
        Sets the view for this frame.
        """
        self._childView = view


class DocTabbedParentFrame(wx.Frame, DocFrameMixIn, DocMDIParentFrameMixIn):
    """
    The DocTabbedParentFrame class provides a default top-level frame for
    applications using the document/view framework. This class can only be
    used for MDI parent frames that use a tabbed interface.

    It cooperates with the wxView, wxDocument, wxDocManager and wxDocTemplate
    classes.
    """


    def __init__(self, docManager, frame, id, title, pos = wx.DefaultPosition, size = wx.DefaultSize, style = wx.DEFAULT_FRAME_STYLE, name = "DocTabbedParentFrame", embeddedWindows = 0, minSize=20):
        """
        Constructor.  Note that the event table must be rebuilt for the
        frame since the EvtHandler is not virtual.
        """
        pos, size = self._GetPosSizeFromConfig(pos, size)
        wx.Frame.__init__(self, frame, id, title, pos, size, style, name)

        # From docview.MDIParentFrame
        self._docManager = docManager

        wx.EVT_CLOSE(self, self.OnCloseWindow)

        wx.EVT_MENU(self, wx.ID_EXIT, self.OnExit)
        wx.EVT_MENU_RANGE(self, wx.ID_FILE1, wx.ID_FILE9, self.OnMRUFile)

        wx.EVT_MENU(self, wx.ID_NEW, self.ProcessEvent)
        wx.EVT_MENU(self, wx.ID_OPEN, self.ProcessEvent)
        wx.EVT_MENU(self, wx.ID_CLOSE_ALL, self.ProcessEvent)
        wx.EVT_MENU(self, wx.ID_CLOSE, self.ProcessEvent)
        wx.EVT_MENU(self, wx.ID_REVERT, self.ProcessEvent)
        wx.EVT_MENU(self, wx.ID_SAVE, self.ProcessEvent)
        wx.EVT_MENU(self, wx.ID_SAVEAS, self.ProcessEvent)
        wx.EVT_MENU(self, wx.ID_UNDO, self.ProcessEvent)
        wx.EVT_MENU(self, wx.ID_REDO, self.ProcessEvent)
        wx.EVT_MENU(self, wx.ID_PRINT, self.ProcessEvent)
        wx.EVT_MENU(self, wx.ID_PRINT_SETUP, self.ProcessEvent)
        wx.EVT_MENU(self, wx.ID_PREVIEW, self.ProcessEvent)
        wx.EVT_MENU(self, wx.ID_ABOUT, self.OnAbout)

        wx.EVT_UPDATE_UI(self, wx.ID_NEW, self.ProcessUpdateUIEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_OPEN, self.ProcessUpdateUIEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_CLOSE_ALL, self.ProcessUpdateUIEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_CLOSE, self.ProcessUpdateUIEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_REVERT, self.ProcessUpdateUIEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_SAVE, self.ProcessUpdateUIEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_SAVEAS, self.ProcessUpdateUIEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_UNDO, self.ProcessUpdateUIEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_REDO, self.ProcessUpdateUIEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_PRINT, self.ProcessUpdateUIEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_PRINT_SETUP, self.ProcessUpdateUIEvent)
        wx.EVT_UPDATE_UI(self, wx.ID_PREVIEW, self.ProcessUpdateUIEvent)
        # End From docview.MDIParentFrame

        self.CreateNotebook()
        self._InitFrame(embeddedWindows, minSize)


    def _LayoutFrame(self):
        """
        Lays out the frame.
        """
        wx.LayoutAlgorithm().LayoutFrame(self, self._notebook)


    def CreateNotebook(self):
        """
        Creates the notebook to use for the tabbed document interface.
        """
        if wx.Platform != "__WXMAC__":
            self._notebook = wx.Notebook(self, wx.NewId())
        else:
            self._notebook = wx.Listbook(self, wx.NewId(), style=wx.LB_LEFT)
        # self._notebook.SetSizer(wx.NotebookSizer(self._notebook))
        if wx.Platform != "__WXMAC__":
            wx.EVT_NOTEBOOK_PAGE_CHANGED(self, self._notebook.GetId(), self.OnNotebookPageChanged)
        else:
            wx.EVT_LISTBOOK_PAGE_CHANGED(self, self._notebook.GetId(), self.OnNotebookPageChanged)
        wx.EVT_RIGHT_DOWN(self._notebook, self.OnNotebookRightClick)
        wx.EVT_MIDDLE_DOWN(self._notebook, self.OnNotebookMiddleClick)

        # wxBug: wx.Listbook does not implement HitTest the same way wx.Notebook
        # does, so for now don't fire MouseOver events.
        if wx.Platform != "__WXMAC__":
            wx.EVT_MOTION(self._notebook, self.OnNotebookMouseOver)

        templates = wx.GetApp().GetDocumentManager().GetTemplates()
        iconList = wx.ImageList(16, 16, initialCount = len(templates))
        self._iconIndexLookup = []
        for template in templates:
            icon = template.GetIcon()
            if icon:
                if icon.GetHeight() != 16 or icon.GetWidth() != 16:
                    icon.SetHeight(16)
                    icon.SetWidth(16)
                    if wx.GetApp().GetDebug():
                        print "Warning: icon for '%s' isn't 16x16, not crossplatform" % template._docTypeName
                iconIndex = iconList.AddIcon(icon)
                self._iconIndexLookup.append((template, iconIndex))

        icon = Blank.GetIcon()
        if icon.GetHeight() != 16 or icon.GetWidth() != 16:
            icon.SetHeight(16)
            icon.SetWidth(16)
            if wx.GetApp().GetDebug():
                print "Warning: getBlankIcon isn't 16x16, not crossplatform"
        self._blankIconIndex = iconList.AddIcon(icon)
        self._notebook.AssignImageList(iconList)


    def GetNotebook(self):
        """
        Returns the notebook used by the tabbed document interface.
        """
        return self._notebook


    def GetActiveChild(self):
        """
        Returns the active notebook page, which to the framework is treated as
        a document frame.
        """
        index = self._notebook.GetSelection()
        if index == -1:
            return None
        return self._notebook.GetPage(index)


    def OnNotebookPageChanged(self, event):
        """
        Activates a notebook page's view when it is selected.
        """
        index = self._notebook.GetSelection()
        if index > -1:
            self._notebook.GetPage(index).GetView().Activate()


    def OnNotebookMouseOver(self, event):
        # wxBug: On Windows XP the tooltips don't automatically disappear when you move the mouse and it is on a notebook tab, has nothing to do with this code!!!
        index, type = self._notebook.HitTest(event.GetPosition())

        if index > -1:
            doc = self._notebook.GetPage(index).GetView().GetDocument()
            # wxBug: Tooltips no longer appearing on tabs except on
            # about a 2 pixel area between tab top and contents that will show tip.
            self._notebook.GetParent().SetToolTip(wx.ToolTip(doc.GetFilename()))
        else:
            self._notebook.SetToolTip(wx.ToolTip(""))
        event.Skip()


    def OnNotebookMiddleClick(self, event):
        """
        Handles middle clicks for the notebook, closing the document whose tab was
        clicked on.
        """
        index, type = self._notebook.HitTest(event.GetPosition())
        if index > -1:
            doc = self._notebook.GetPage(index).GetView().GetDocument()
            if doc:
                doc.DeleteAllViews()

    def OnNotebookRightClick(self, event):
        """
        Handles right clicks for the notebook, enabling users to either close
        a tab or select from the available documents if the user clicks on the
        notebook's white space.
        """
        index, type = self._notebook.HitTest(event.GetPosition())
        menu = wx.Menu()
        x, y = event.GetX(), event.GetY()
        if index > -1:
            doc = self._notebook.GetPage(index).GetView().GetDocument()
            id = wx.NewId()
            menu.Append(id, _("Close"))
            def OnRightMenuSelect(event):
                doc.DeleteAllViews()
            wx.EVT_MENU(self, id, OnRightMenuSelect)
            if self._notebook.GetPageCount() > 1:
                id = wx.NewId()
                menu.Append(id, _("Close All but \"%s\"" % doc.GetPrintableName()))
                def OnRightMenuSelect(event):
                    for i in range(self._notebook.GetPageCount()-1, -1, -1): # Go from len-1 to 0
                        if i != index:
                            doc = self._notebook.GetPage(i).GetView().GetDocument()
                            if not self.GetDocumentManager().CloseDocument(doc, False):
                                return
                wx.EVT_MENU(self, id, OnRightMenuSelect)
                menu.AppendSeparator()
                tabsMenu = wx.Menu()
                menu.AppendMenu(wx.NewId(), _("Select Tab"), tabsMenu)
        else:
            y = y - 25  # wxBug: It is offsetting click events in the blank notebook area
            tabsMenu = menu

        if self._notebook.GetPageCount() > 1:
            selectIDs = {}
            for i in range(0, self._notebook.GetPageCount()):
                id = wx.NewId()
                selectIDs[id] = i
                tabsMenu.Append(id, self._notebook.GetPageText(i))
                def OnRightMenuSelect(event):
                    self._notebook.SetSelection(selectIDs[event.GetId()])
                wx.EVT_MENU(self, id, OnRightMenuSelect)

        self._notebook.PopupMenu(menu, wx.Point(x, y))
        menu.Destroy()


    def AddNotebookPage(self, panel, title):
        """
        Adds a document page to the notebook.
        """
        self._notebook.AddPage(panel, title)
        index = self._notebook.GetPageCount() - 1
        self._notebook.SetSelection(index)

        found = False  # Now set the icon
        template = panel.GetDocument().GetDocumentTemplate()
        if template:
            for t, iconIndex in self._iconIndexLookup:
                if t is template:
                    self._notebook.SetPageImage(index, iconIndex)
                    found = True
                    break
        if not found:
            self._notebook.SetPageImage(index, self._blankIconIndex)

        # wxBug: the wxListbook used on Mac needs its tabs list resized
        # whenever a new tab is added, but the only way to do this is
        # to resize the entire control
        if wx.Platform == "__WXMAC__":
            content_size = self._notebook.GetSize()
            self._notebook.SetSize((content_size.x+2, -1))
            self._notebook.SetSize((content_size.x, -1))

        self._notebook.Layout()

        windowMenuService = wx.GetApp().GetService(WindowMenuService)
        if windowMenuService:
            windowMenuService.BuildWindowMenu(wx.GetApp().GetTopWindow())  # build file menu list when we open a file


    def RemoveNotebookPage(self, panel):
        """
        Removes a document page from the notebook.
        """
        index = self.GetNotebookPageIndex(panel)
        if index > -1:
            if self._notebook.GetPageCount() == 1 or index < 2:
                pass
            elif index >= 1:
                self._notebook.SetSelection(index - 1)
            elif index < self._notebook.GetPageCount():
                self._notebook.SetSelection(index + 1)
            self._notebook.DeletePage(index)
            self._notebook.GetParent().SetToolTip(wx.ToolTip(""))

        windowMenuService = wx.GetApp().GetService(WindowMenuService)
        if windowMenuService:
            windowMenuService.BuildWindowMenu(wx.GetApp().GetTopWindow())  # build file menu list when we open a file


    def ActivateNotebookPage(self, panel):
        """
        Sets the notebook to the specified panel.
        """
        index = self.GetNotebookPageIndex(panel)
        if index > -1:
            self._notebook.SetFocus()
            self._notebook.SetSelection(index)


    def GetNotebookPageTitle(self, panel):
        index = self.GetNotebookPageIndex(panel)
        if index != -1:
            return self._notebook.GetPageText(self.GetNotebookPageIndex(panel))
        else:
            return None


    def SetNotebookPageTitle(self, panel, title):
        self._notebook.SetPageText(self.GetNotebookPageIndex(panel), title)


    def GetNotebookPageIndex(self, panel):
        """
        Returns the index of particular notebook panel.
        """
        index = -1
        for i in range(self._notebook.GetPageCount()):
            if self._notebook.GetPage(i) == panel:
                index = i
                break
        return index


    def ProcessEvent(self, event):
        """
        Processes an event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        if wx.GetApp().ProcessEventBeforeWindows(event):
            return True
        if self._docManager and self._docManager.ProcessEvent(event):
            return True
        return DocMDIParentFrameMixIn.ProcessEvent(self, event)


    def ProcessUpdateUIEvent(self, event):
        """
        Processes a UI event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        if wx.GetApp().ProcessUpdateUIEventBeforeWindows(event):
            return True
        if self._docManager and self._docManager.ProcessUpdateUIEvent(event):
            return True
        return DocMDIParentFrameMixIn.ProcessUpdateUIEvent(self, event)


    def OnExit(self, event):
        """
        Called when File/Exit is chosen and closes the window.
        """
        self.Close()


    def OnMRUFile(self, event):
        """
        Opens the appropriate file when it is selected from the file history
        menu.
        """
        n = event.GetId() - wx.ID_FILE1
        filename = self._docManager.GetHistoryFile(n)
        if filename:
            self._docManager.CreateDocument(filename, wx.lib.docview.DOC_SILENT)
        else:
            self._docManager.RemoveFileFromHistory(n)
            msgTitle = wx.GetApp().GetAppName()
            if not msgTitle:
                msgTitle = _("File Error")
            wx.MessageBox("The file '%s' doesn't exist and couldn't be opened.\nIt has been removed from the most recently used files list" % FileNameFromPath(file),
                          msgTitle,
                          wx.OK | wx.ICON_EXCLAMATION,
                          self)


    def OnSize(self, event):
        """
        Called when the frame is resized and lays out the client window.
        """
        # Needed in case there are splitpanels around the mdi frame
        self._LayoutFrame()


    def OnCloseWindow(self, event):
        """
        Called when the frame is closed.  Remembers the frame size.
        """
        self.SaveEmbeddedWindowSizes()

        # save and close services last
        for service in wx.GetApp().GetServices():
            if not service.OnCloseFrame(event):
                return

        # From docview.MDIParentFrame
        if self._docManager.Clear(not event.CanVeto()):
            self.Destroy()
        else:
            event.Veto()


class DocMDIChildFrame(wx.MDIChildFrame):
    """
    The wxDocMDIChildFrame class provides a default frame for displaying
    documents on separate windows. This class can only be used for MDI child
    frames.

    The class is part of the document/view framework supported by wxWindows,
    and cooperates with the wxView, wxDocument, wxDocManager and wxDocTemplate
    classes.
    """


    def __init__(self, doc, view, frame, id, title, pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE, name="frame"):
        """
        Constructor.  Note that the event table must be rebuilt for the
        frame since the EvtHandler is not virtual.
        """
        wx.MDIChildFrame.__init__(self, frame, id, title, pos, size, style, name)
        self._childDocument = doc
        self._childView = view
        if view:
            view.SetFrame(self)
        # self.Create(doc, view, frame, id, title, pos, size, style, name)
        self._activeEvent = None
        self._activated = 0
        wx.EVT_ACTIVATE(self, self.OnActivate)
        wx.EVT_CLOSE(self, self.OnCloseWindow)

        if frame:  # wxBug: For some reason the EVT_ACTIVATE event is not getting triggered for the first mdi client window that is opened so we have to do it manually
            mdiChildren = filter(lambda x: isinstance(x, wx.MDIChildFrame), frame.GetChildren())
            if len(mdiChildren) == 1:
                self.Activate()


##    # Couldn't get this to work, but seems to work fine with single stage construction
##    def Create(self, doc, view, frame, id, title, pos, size, style, name):
##        self._childDocument = doc
##        self._childView = view
##        if wx.MDIChildFrame.Create(self, frame, id, title, pos, size, style, name):
##            if view:
##                view.SetFrame(self)
##                return True
##        return False



    def Activate(self):  # Need this in case there are embedded sash windows and such, OnActivate is not getting called
        """
        Activates the current view.
        """
        if self._childView:
            self._childView.Activate(True)


    def OnTitleIsModified(self):
        """
        Add/remove to the frame's title an indication that the document is dirty.
        If the document is dirty, an '*' is appended to the title
        """
        title = self.GetTitle()
        if title:
            if self.GetDocument().IsModified():
                if title.endswith("*"):
                    return
                else:
                    title = title + "*"
                    self.SetTitle(title)
            else:
                if title.endswith("*"):
                    title = title[:-1]
                    self.SetTitle(title)                
                else:
                    return


    def ProcessEvent(event):
        """
        Processes an event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        if self._activeEvent == event:
            return False

        self._activeEvent = event  # Break recursion loops

        if self._childView:
            self._childView.Activate(True)

        if not self._childView or not self._childView.ProcessEvent(event):
            if not isinstance(event, wx.CommandEvent) or not self.GetParent() or not self.GetParent().ProcessEvent(event):
                ret = False
            else:
                ret = True
        else:
            ret = True

        self._activeEvent = None
        return ret


    def OnActivate(self, event):
        """
        Sets the currently active view to be the frame's view. You may need to
        override (but still call) this function in order to set the keyboard
        focus for your subwindow.
        """
        event.Skip()
        if self._activated != 0:
            return True
        self._activated += 1
        wx.MDIChildFrame.Activate(self)
        if event.GetActive() and self._childView:
            self._childView.Activate(event.GetActive())
        self._activated = 0


    def OnCloseWindow(self, event):
        """
        Closes and deletes the current view and document.
        """
        if self._childView:
            ans = False
            if not event.CanVeto():
                ans = True
            else:
                ans = self._childView.Close(deleteWindow = False)

            if ans:
                self._childView.Activate(False)
                self._childView.Destroy()
                self._childView = None
                if self._childDocument:
                    self._childDocument.Destroy()  # This isn't in the wxWindows codebase but the document needs to be disposed of somehow
                self._childDocument = None
                self.Destroy()
            else:
                event.Veto()
        else:
            event.Veto()


    def GetDocument(self):
        """
        Returns the document associated with this frame.
        """
        return self._childDocument


    def SetDocument(self, document):
        """
        Sets the document for this frame.
        """
        self._childDocument = document


    def GetView(self):
        """
        Returns the view associated with this frame.
        """
        return self._childView


    def SetView(self, view):
        """
        Sets the view for this frame.
        """
        self._childView = view


class DocService(wx.EvtHandler):
    """
    An abstract class used to add reusable services to a docview application.
    """


    def __init__(self):
        """Initializes the DocService."""
        pass


    def GetDocumentManager(self):
        """Returns the DocManager for the docview application."""
        return self._docManager


    def SetDocumentManager(self, docManager):
        """Sets the DocManager for the docview application."""
        self._docManager = docManager


    def InstallControls(self, frame, menuBar=None, toolBar=None, statusBar=None, document=None):
        """Called to install controls into the menubar and toolbar of a SDI or MDI window.  Override this method for a particular service."""
        pass


    def ProcessEventBeforeWindows(self, event):
        """
        Processes an event before the main window has a chance to process the window.
        Override this method for a particular service.
        """
        return False


    def ProcessUpdateUIEventBeforeWindows(self, event):
        """
        Processes a UI event before the main window has a chance to process the window.
        Override this method for a particular service.
        """
        return False


    def ProcessEvent(self, event):
        """
        Processes an event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        return False


    def ProcessUpdateUIEvent(self, event):
        """
        Processes a UI event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        return False


    def OnCloseFrame(self, event):
        """
        Called when the a docview frame is being closed.  Override this method
        so a service can either do cleanup or veto the frame being closed by
        returning false.
        """
        return True


    def OnExit(self):
        """
        Called when the the docview application is being closed.  Override this method
        so a service can either do cleanup or veto the frame being closed by
        returning false.
        """
        pass


    def GetMenuItemPos(self, menu, id):
        """
        Utility method used to find the position of a menu item so that services can
        easily find where to insert a menu item in InstallControls.
        """
        menuItems = menu.GetMenuItems()
        for i, menuItem in enumerate(menuItems):
            if menuItem.GetId() == id:
                return i
        return i


    def GetView(self):
        """
        Called by WindowMenuService to get views for services that don't
        have dedicated documents such as the Outline Service.
        """
        return None


class DocOptionsService(DocService):
    """
    A service that implements an options menu item and an options dialog with
    notebook tabs.  New tabs can be added by other services by calling the
    "AddOptionsPanel" method.
    """


    def __init__(self, showGeneralOptions=True, supportedModes=wx.lib.docview.DOC_SDI & wx.lib.docview.DOC_MDI):
        """
        Initializes the options service with the option of suppressing the default
        general options pane that is included with the options service by setting
        showGeneralOptions to False.  It allowModeChanges is set to False, the
        default general options pane will allow users to change the document
        interface mode between SDI and MDI modes.
        """
        DocService.__init__(self)
        self.ClearOptionsPanels()
        self._supportedModes = supportedModes
        self._toolOptionsID = wx.ID_PREFERENCES
        if showGeneralOptions:
            self.AddOptionsPanel(GeneralOptionsPanel)


    def InstallControls(self, frame, menuBar=None, toolBar=None, statusBar=None, document=None):
        """
        Installs a "Tools" menu with an "Options" menu item.
        """
        toolsMenuIndex = menuBar.FindMenu(_("&Tools"))
        if toolsMenuIndex > -1:
            toolsMenu = menuBar.GetMenu(toolsMenuIndex)
        else:
            toolsMenu = wx.Menu()
        if toolsMenuIndex == -1:
            formatMenuIndex = menuBar.FindMenu(_("&Format"))
            menuBar.Insert(formatMenuIndex + 1, toolsMenu, _("&Tools"))
        if toolsMenu:
            if toolsMenu.GetMenuItemCount():
                toolsMenu.AppendSeparator()
            toolsMenu.Append(self._toolOptionsID, _("&Options..."), _("Sets options"))
            wx.EVT_MENU(frame, self._toolOptionsID, frame.ProcessEvent)


    def ProcessEvent(self, event):
        """
        Checks to see if the "Options" menu item has been selected.
        """
        id = event.GetId()
        if id == self._toolOptionsID:
            self.OnOptions(event)
            return True
        else:
            return False


    def GetSupportedModes(self):
        """
        Return the modes supported by the application.  Use docview.DOC_SDI and
        docview.DOC_MDI flags to check if SDI and/or MDI modes are supported.
        """
        return self._supportedModes


    def SetSupportedModes(self, _supportedModessupportedModes):
        """
        Sets the modes supported by the application.  Use docview.DOC_SDI and
        docview.DOC_MDI flags to set if SDI and/or MDI modes are supported.
        """
        self._supportedModes = supportedModes


    def ClearOptionsPanels(self):
        """
        Clears all of the options panels that have been added into the
        options dialog.
        """
        self._optionsPanels = []


    def AddOptionsPanel(self, optionsPanel):
        """
        Adds an options panel to the options dialog.
        """
        self._optionsPanels.append(optionsPanel)


    def OnOptions(self, event):
        """
        Shows the options dialog, called when the "Options" menu item is selected.
        """
        if len(self._optionsPanels) == 0:
            return
        optionsDialog = OptionsDialog(wx.GetApp().GetTopWindow(), self._optionsPanels, self._docManager)
        optionsDialog.CenterOnParent()
        if optionsDialog.ShowModal() == wx.ID_OK:
            optionsDialog.OnOK(optionsDialog)  # wxBug: wxDialog should be calling this automatically but doesn't
        optionsDialog.Destroy()


class OptionsDialog(wx.Dialog):
    """
    A default options dialog used by the OptionsService that hosts a notebook
    tab of options panels.
    """


    def __init__(self, parent, optionsPanelClasses, docManager):
        """
        Initializes the options dialog with a notebook page that contains new
        instances of the passed optionsPanelClasses.
        """
        wx.Dialog.__init__(self, parent, -1, _("Options"))

        self._optionsPanels = []
        self._docManager = docManager

        HALF_SPACE = 5
        SPACE = 10

        sizer = wx.BoxSizer(wx.VERTICAL)

        if wx.Platform == "__WXMAC__":
            optionsNotebook = wx.Listbook(self, wx.NewId(), style=wx.LB_DEFAULT)
        else:
            optionsNotebook = wx.Notebook(self, wx.NewId(), style=wx.NB_MULTILINE)  # NB_MULTILINE is windows platform only
        sizer.Add(optionsNotebook, 0, wx.ALL | wx.EXPAND, SPACE)

        if wx.Platform == "__WXMAC__":
            iconList = wx.ImageList(16, 16, initialCount = len(optionsPanelClasses))
            self._iconIndexLookup = []

            for optionsPanelClass in optionsPanelClasses:
                optionsPanel = optionsPanelClass(optionsNotebook, -1)
                self._optionsPanels.append(optionsPanel)

                # We need to populate the image list before setting notebook images
                if hasattr(optionsPanel, "GetIcon"):
                    icon = optionsPanel.GetIcon()
                else:
                    icon = None
                if icon:
                    if icon.GetHeight() != 16 or icon.GetWidth() != 16:
                        icon.SetHeight(16)
                        icon.SetWidth(16)
                        if wx.GetApp().GetDebug():
                            print "Warning: icon for '%s' isn't 16x16, not crossplatform" % template._docTypeName
                    iconIndex = iconList.AddIcon(icon)
                    self._iconIndexLookup.append((optionsPanel, iconIndex))

                else:
                    # use -1 to represent that this panel has no icon
                    self._iconIndexLookup.append((optionsPanel, -1))

            optionsNotebook.AssignImageList(iconList)

            # Add icons to notebook
            for index in range(0, len(optionsPanelClasses)-1):
                iconIndex = self._iconIndexLookup[index][1]
                if iconIndex >= 0:
                    optionsNotebook.SetPageImage(index, iconIndex)
        else:
            for optionsPanelClass in optionsPanelClasses:
                optionsPanel = optionsPanelClass(optionsNotebook, -1)
                self._optionsPanels.append(optionsPanel)

        sizer.Add(self.CreateButtonSizer(wx.OK | wx.CANCEL), 0, wx.ALIGN_RIGHT | wx.RIGHT | wx.BOTTOM, HALF_SPACE)
        self.SetSizer(sizer)
        self.Layout()
        self.Fit()
        wx.CallAfter(self.DoRefresh)


    def DoRefresh(self):
        """
        wxBug: On Windows XP when using a multiline notebook the default page doesn't get
        drawn, but it works when using a single line notebook.
        """
        self.Refresh()


    def GetDocManager(self):
        """
        Returns the document manager passed to the OptionsDialog constructor.
        """
        return self._docManager


    def OnOK(self, event):
        """
        Calls the OnOK method of all of the OptionDialog's embedded panels
        """
        for optionsPanel in self._optionsPanels:
            optionsPanel.OnOK(event)


class GeneralOptionsPanel(wx.Panel):
    """
    A general options panel that is used in the OptionDialog to configure the
    generic properties of a pydocview application, such as "show tips at startup"
    and whether to use SDI or MDI for the application.
    """


    def __init__(self, parent, id):
        """
        Initializes the panel by adding an "Options" folder tab to the parent notebook and
        populating the panel with the generic properties of a pydocview application.
        """
        wx.Panel.__init__(self, parent, id)
        SPACE = 10
        HALF_SPACE = 5
        config = wx.ConfigBase_Get()
        self._showTipsCheckBox = wx.CheckBox(self, -1, _("Show tips at start up"))
        self._showTipsCheckBox.SetValue(config.ReadInt("ShowTipAtStartup", True))
        if self._AllowModeChanges():
            supportedModes = wx.GetApp().GetService(DocOptionsService).GetSupportedModes()
            choices = []
            self._sdiChoice = _("Show each document in its own window")
            self._mdiChoice = _("Show all documents in a single window with tabs")
            self._winMdiChoice = _("Show all documents in a single window with child windows")
            if supportedModes & wx.lib.docview.DOC_SDI:
                choices.append(self._sdiChoice)
            choices.append(self._mdiChoice)
            if wx.Platform == "__WXMSW__":
                choices.append(self._winMdiChoice)
            self._documentRadioBox = wx.RadioBox(self, -1, _("Document Display Style"),
                                          choices = choices,
                                          majorDimension=1,
                                          )
            if config.ReadInt("UseWinMDI", False):
                self._documentRadioBox.SetStringSelection(self._winMdiChoice)
            elif config.ReadInt("UseMDI", True):
                self._documentRadioBox.SetStringSelection(self._mdiChoice)
            else:
                self._documentRadioBox.SetStringSelection(self._sdiChoice)
            def OnDocumentInterfaceSelect(event):
                if not self._documentInterfaceMessageShown:
                    msgTitle = wx.GetApp().GetAppName()
                    if not msgTitle:
                        msgTitle = _("Document Options")
                    wx.MessageBox(_("Document interface changes will not appear until the application is restarted."),
                                  msgTitle,
                                  wx.OK | wx.ICON_INFORMATION,
                                  self.GetParent())
                    self._documentInterfaceMessageShown = True
            wx.EVT_RADIOBOX(self, self._documentRadioBox.GetId(), OnDocumentInterfaceSelect)
        optionsBorderSizer = wx.BoxSizer(wx.VERTICAL)
        optionsSizer = wx.BoxSizer(wx.VERTICAL)
        if self._AllowModeChanges():
            optionsSizer.Add(self._documentRadioBox, 0, wx.ALL, HALF_SPACE)
        optionsSizer.Add(self._showTipsCheckBox, 0, wx.ALL, HALF_SPACE)
        optionsBorderSizer.Add(optionsSizer, 0, wx.ALL, SPACE)
        self.SetSizer(optionsBorderSizer)
        self.Layout()
        self._documentInterfaceMessageShown = False
        parent.AddPage(self, _("General"))


    def _AllowModeChanges(self):
        supportedModes = wx.GetApp().GetService(DocOptionsService).GetSupportedModes()
        return supportedModes & wx.lib.docview.DOC_SDI and supportedModes & wx.lib.docview.DOC_MDI or wx.Platform == "__WXMSW__" and supportedModes & wx.lib.docview.DOC_MDI  # More than one mode is supported, allow selection


    def OnOK(self, optionsDialog):
        """
        Updates the config based on the selections in the options panel.
        """
        config = wx.ConfigBase_Get()
        config.WriteInt("ShowTipAtStartup", self._showTipsCheckBox.GetValue())
        if self._AllowModeChanges():
            config.WriteInt("UseMDI", (self._documentRadioBox.GetStringSelection() == self._mdiChoice))
            config.WriteInt("UseWinMDI", (self._documentRadioBox.GetStringSelection() == self._winMdiChoice))


    def GetIcon(self):
        """ Return icon for options panel on the Mac. """
        return wx.GetApp().GetDefaultIcon()


class DocApp(wx.PySimpleApp):
    """
    The DocApp class serves as the base class for pydocview applications and offers
    functionality such as services, creation of SDI and MDI frames, show tips,
    and a splash screen.
    """


    def OnInit(self):
        """
        Initializes the DocApp.
        """
        self._services = []
        self._defaultIcon = None
        self._registeredCloseEvent = False
        self._useTabbedMDI = True

        if not hasattr(self, "_debug"):  # only set if not already initialized
            self._debug = False
        if not hasattr(self, "_singleInstance"):  # only set if not already initialized
            self._singleInstance = True

        # if _singleInstance is TRUE only allow one single instance of app to run.
        # When user tries to run a second instance of the app, abort startup,
        # But if user also specifies files to open in command line, send message to running app to open those files
        if self._singleInstance:
            # create shared memory temporary file
            if wx.Platform == '__WXMSW__':
                tfile = tempfile.TemporaryFile(prefix="ag", suffix="tmp")
                fno = tfile.fileno()
                self._sharedMemory = mmap.mmap(fno, 1024, "shared_memory")
            else:
                tfile = file(os.path.join(tempfile.gettempdir(), tempfile.gettempprefix() + self.GetAppName() + '-' + wx.GetUserId() + "AGSharedMemory"), 'w+b')
                tfile.write("*")
                tfile.seek(1024)
                tfile.write(" ")
                tfile.flush()
                fno = tfile.fileno()
                self._sharedMemory = mmap.mmap(fno, 1024)

            self._singleInstanceChecker = wx.SingleInstanceChecker(self.GetAppName() + '-' + wx.GetUserId(), tempfile.gettempdir())
            if self._singleInstanceChecker.IsAnotherRunning():
                # have running single instance open file arguments
                data = pickle.dumps(sys.argv[1:])
                while 1:
                    self._sharedMemory.seek(0)
                    marker = self._sharedMemory.read_byte()
                    if marker == '\0' or marker == '*':        # available buffer
                        self._sharedMemory.seek(0)
                        self._sharedMemory.write_byte('-')     # set writing marker
                        self._sharedMemory.write(data)  # write files we tried to open to shared memory
                        self._sharedMemory.seek(0)
                        self._sharedMemory.write_byte('+')     # set finished writing marker
                        self._sharedMemory.flush()
                        break
                    else:
                        time.sleep(1)  # give enough time for buffer to be available

                return False
            else:
                self._timer = wx.PyTimer(self.DoBackgroundListenAndLoad)
                self._timer.Start(250)

        return True


    def OpenMainFrame(self):
        docManager = self.GetDocumentManager()
        if docManager.GetFlags() & wx.lib.docview.DOC_MDI:
            if self.GetUseTabbedMDI():
                frame = wx.lib.pydocview.DocTabbedParentFrame(docManager, None, -1, self.GetAppName())
            else:
                frame = wx.lib.pydocview.DocMDIParentFrame(docManager, None, -1, self.GetAppName())
            frame.Show(True)

    def MacOpenFile(self, filename):
        self.GetDocumentManager().CreateDocument(os.path.normpath(filename), wx.lib.docview.DOC_SILENT)

        # force display of running app
        topWindow = wx.GetApp().GetTopWindow()
        if topWindow.IsIconized():
            topWindow.Iconize(False)
        else:
            topWindow.Raise()

    def DoBackgroundListenAndLoad(self):
        """
        Open any files specified in the given command line argument passed in via shared memory
        """
        self._timer.Stop()

        self._sharedMemory.seek(0)
        if self._sharedMemory.read_byte() == '+':  # available data
            data = self._sharedMemory.read(1024-1)
            self._sharedMemory.seek(0)
            self._sharedMemory.write_byte("*")     # finished reading, set buffer free marker
            self._sharedMemory.flush()
            args = pickle.loads(data)
            for arg in args:
                if (wx.Platform != "__WXMSW__" or arg[0] != "/") and arg[0] != '-' and os.path.exists(arg):
                    self.GetDocumentManager().CreateDocument(os.path.normpath(arg), wx.lib.docview.DOC_SILENT)

            # force display of running app
            topWindow = wx.GetApp().GetTopWindow()
            if topWindow.IsIconized():
                topWindow.Iconize(False)
            else:
                topWindow.Raise()


        self._timer.Start(1000) # 1 second interval


    def OpenCommandLineArgs(self):
        """
        Called to open files that have been passed to the application from the
        command line.
        """
        args = sys.argv[1:]
        for arg in args:
            if (wx.Platform != "__WXMSW__" or arg[0] != "/") and arg[0] != '-' and os.path.exists(arg):
                self.GetDocumentManager().CreateDocument(os.path.normpath(arg), wx.lib.docview.DOC_SILENT)


    def GetDocumentManager(self):
        """
        Returns the document manager associated to the DocApp.
        """
        return self._docManager


    def SetDocumentManager(self, docManager):
        """
        Sets the document manager associated with the DocApp and loads the
        DocApp's file history into the document manager.
        """
        self._docManager = docManager
        config = wx.ConfigBase_Get()
        self.GetDocumentManager().FileHistoryLoad(config)


    def ProcessEventBeforeWindows(self, event):
        """
        Enables services to process an event before the main window has a chance to
        process the window.
        """
        for service in self._services:
            if service.ProcessEventBeforeWindows(event):
                return True
        return False


    def ProcessUpdateUIEventBeforeWindows(self, event):
        """
        Enables services to process a UI event before the main window has a chance
        to process the window.
        """
        for service in self._services:
            if service.ProcessUpdateUIEventBeforeWindows(event):
                return True
        return False


    def ProcessEvent(self, event):
        """
        Processes an event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        for service in self._services:
            if service.ProcessEvent(event):
                return True
        return False


    def ProcessUpdateUIEvent(self, event):
        """
        Processes a UI event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        for service in self._services:
            if service.ProcessUpdateUIEvent(event):
                return True
        return False


    def InstallService(self, service):
        """
        Installs an instance of a DocService into the DocApp.
        """
        service.SetDocumentManager(self._docManager)
        self._services.append(service)
        return service


    def GetServices(self):
        """
        Returns the DocService instances that have been installed into the DocApp.
        """
        return self._services


    def GetService(self, type):
        """
        Returns the instance of a particular type of service that has been installed
        into the DocApp.  For example, "wx.GetApp().GetService(pydocview.OptionsService)"
        returns the isntance of the OptionsService that is running within the DocApp.
        """
        for service in self._services:
            if isinstance(service, type):
                return service
        return None


    def OnExit(self):
        """
        Called when the DocApp is exited, enables the installed DocServices to exit
        and saves the DocManager's file history.
        """
        for service in self._services:
            service.OnExit()
        config = wx.ConfigBase_Get()
        self._docManager.FileHistorySave(config)

        if hasattr(self, "_singleInstanceChecker"):
            del self._singleInstanceChecker


    def GetDefaultDocManagerFlags(self):
        """
        Returns the default flags to use when creating the DocManager.
        """
        config = wx.ConfigBase_Get()
        if config.ReadInt("UseMDI", True) or config.ReadInt("UseWinMDI", False):
            flags = wx.lib.docview.DOC_MDI | wx.lib.docview.DOC_OPEN_ONCE
            if config.ReadInt("UseWinMDI", False):
                self.SetUseTabbedMDI(False)
        else:
            flags = wx.lib.docview.DOC_SDI | wx.lib.docview.DOC_OPEN_ONCE
        return flags


    def ShowTip(self, frame, tipProvider):
        """
        Shows the tip window, generally this is called when an application starts.
        A wx.TipProvider must be passed.
        """
        config = wx.ConfigBase_Get()
        showTip = config.ReadInt("ShowTipAtStartup", 1)
        if showTip:
            index = config.ReadInt("TipIndex", 0)
            showTipResult = wx.ShowTip(wx.GetApp().GetTopWindow(), tipProvider, showAtStartup = showTip)
            if showTipResult != showTip:
                config.WriteInt("ShowTipAtStartup", showTipResult)


    def GetEditMenu(self, frame):
        """
        Utility method that finds the Edit menu within the menubar of a frame.
        """
        menuBar = frame.GetMenuBar()
        if not menuBar:
            return None
        editMenuIndex = menuBar.FindMenu(_("&Edit"))
        if editMenuIndex == -1:
            return None
        return menuBar.GetMenu(editMenuIndex)


    def GetUseTabbedMDI(self):
        """
        Returns True if Windows MDI should use folder tabs instead of child windows.
        """
        return self._useTabbedMDI


    def SetUseTabbedMDI(self, useTabbedMDI):
        """
        Set to True if Windows MDI should use folder tabs instead of child windows.
        """
        self._useTabbedMDI = useTabbedMDI


    def CreateDocumentFrame(self, view, doc, flags, id = -1, title = "", pos = wx.DefaultPosition, size = wx.DefaultSize, style = wx.DEFAULT_FRAME_STYLE):
        """
        Called by the DocManager to create and return a new Frame for a Document.
        Chooses whether to create an MDIChildFrame or SDI Frame based on the
        DocManager's flags.
        """
        docflags = self.GetDocumentManager().GetFlags()
        if docflags & wx.lib.docview.DOC_SDI:
            frame = self.CreateSDIDocumentFrame(doc, view, id, title, pos, size, style)
            frame.Show()

            # wxBug: operating system bug, first window is set to the position of last window closed, ignoring passed in position on frame creation
            #        also, initial size is incorrect for the same reasons
            if frame.GetPosition() != pos:
                frame.Move(pos)
            if frame.GetSize() != size:
                frame.SetSize(size)

            if doc and doc.GetCommandProcessor():
                doc.GetCommandProcessor().SetEditMenu(self.GetEditMenu(frame))
        elif docflags & wx.lib.docview.DOC_MDI:
            if self.GetUseTabbedMDI():
                frame = self.CreateTabbedDocumentFrame(doc, view, id, title, pos, size, style)
            else:
                frame = self.CreateMDIDocumentFrame(doc, view, id, title, pos, size, style)
                if doc:
                    if doc.GetDocumentTemplate().GetIcon():
                        frame.SetIcon(doc.GetDocumentTemplate().GetIcon())
                    elif wx.GetApp().GetTopWindow().GetIcon():
                        frame.SetIcon(wx.GetApp().GetTopWindow().GetIcon())
            if doc and doc.GetCommandProcessor():
                doc.GetCommandProcessor().SetEditMenu(self.GetEditMenu(wx.GetApp().GetTopWindow()))
        if not frame.GetIcon() and self._defaultIcon:
            frame.SetIcon(self.GetDefaultIcon())
        view.SetFrame(frame)
        return frame


    def CreateSDIDocumentFrame(self, doc, view, id=-1, title="", pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE):
        """
        Creates and returns an SDI Document Frame.
        """
        frame = DocSDIFrame(doc, view, None, id, title, pos, size, style)
        return frame


    def CreateTabbedDocumentFrame(self, doc, view, id=-1, title="", pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE):
        """
        Creates and returns an MDI Document Frame for a Tabbed MDI view
        """
        frame = DocTabbedChildFrame(doc, view, wx.GetApp().GetTopWindow(), id, title, pos, size, style)
        return frame


    def CreateMDIDocumentFrame(self, doc, view, id=-1, title="", pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE):
        """
        Creates and returns an MDI Document Frame.
        """
        # if any child windows are maximized, then user must want any new children maximized
        # if no children exist, then use the default value from registry
        # wxBug:  Only current window is maximized, so need to check every child frame
        parentFrame = wx.GetApp().GetTopWindow()
        childrenMaximized = filter(lambda child: isinstance(child, wx.MDIChildFrame) and child.IsMaximized(), parentFrame.GetChildren())
        if childrenMaximized:
            maximize = True
        else:
            children = filter(lambda child: isinstance(child, wx.MDIChildFrame), parentFrame.GetChildren())
            if children:
                # other windows exist and none are maximized
                maximize = False
            else:
                # get default setting from registry
                maximize = wx.ConfigBase_Get().ReadInt("MDIChildFrameMaximized", False)

        frame = wx.lib.docview.DocMDIChildFrame(doc, view, wx.GetApp().GetTopWindow(), id, title, pos, size, style)
        if maximize:  # wxBug: Should already be maximizing new child frames if one is maximized but it's not so we have to force it to
            frame.Maximize(True)

##        wx.EVT_MAXIMIZE(frame, self.OnMaximize) # wxBug: This doesn't work, need to save MDIChildFrameMaximized state on close of windows instead
        wx.EVT_CLOSE(frame, self.OnCloseChildWindow)
        if not self._registeredCloseEvent:
            wx.EVT_CLOSE(parentFrame, self.OnCloseMainWindow) # need to check on this, but only once
            self._registeredCloseEvent = True

        return frame


    def SaveMDIDocumentFrameMaximizedState(self, maximized):
        """
        Remember in the config whether the MDI Frame is maximized so that it can be restored
        on open.
        """
        config = wx.ConfigBase_Get()
        maximizeFlag = config.ReadInt("MDIChildFrameMaximized", False)
        if maximized != maximizeFlag:
            config.WriteInt("MDIChildFrameMaximized", maximized)


    def OnCloseChildWindow(self, event):
        """
        Called when an MDI Child Frame is closed.  Calls SaveMDIDocumentFrameMaximizedState to
        remember whether the MDI Frame is maximized so that it can be restored on open.
        """
        self.SaveMDIDocumentFrameMaximizedState(event.GetEventObject().IsMaximized())
        event.Skip()


    def OnCloseMainWindow(self, event):
        """
        Called when the MDI Parent Frame is closed.  Remembers whether the MDI Parent Frame is
        maximized.
        """
        children = event.GetEventObject().GetChildren()
        childrenMaximized = filter(lambda child: isinstance(child, wx.MDIChildFrame)and child.IsMaximized(), children)
        if childrenMaximized:
            self.SaveMDIDocumentFrameMaximizedState(True)
        else:
            childrenNotMaximized = filter(lambda child: isinstance(child, wx.MDIChildFrame), children)

            if childrenNotMaximized:
                # other windows exist and none are maximized
                self.SaveMDIDocumentFrameMaximizedState(False)

        event.Skip()


    def GetDefaultIcon(self):
        """
        Returns the application's default icon.
        """
        return self._defaultIcon


    def SetDefaultIcon(self, icon):
        """
        Sets the application's default icon.
        """
        self._defaultIcon = icon


    def GetDebug(self):
        """
        Returns True if the application is in debug mode.
        """
        return self._debug


    def SetDebug(self, debug):
        """
        Sets the application's debug mode.
        """
        self._debug = debug


    def GetSingleInstance(self):
        """
        Returns True if the application is in single instance mode.  Used to determine if multiple instances of the application is allowed to launch.
        """
        return self._singleInstance


    def SetSingleInstance(self, singleInstance):
        """
        Sets application's single instance mode.
        """
        self._singleInstance = singleInstance



    def CreateChildDocument(self, parentDocument, documentType, objectToEdit, path=''):
        """
        Creates a child window of a document that edits an object.  The child window
        is managed by the parent document frame, so it will be prompted to close if its
        parent is closed, etc.  Child Documents are useful when there are complicated
        Views of a Document and users will need to tunnel into the View.
        """
        for document in self.GetDocumentManager().GetDocuments()[:]:  # Cloning list to make sure we go through all docs even as they are deleted
            if isinstance(document, ChildDocument) and document.GetParentDocument() == parentDocument:
                if document.GetData() == objectToEdit:
                    if hasattr(document.GetFirstView().GetFrame(), "SetFocus"):
                        document.GetFirstView().GetFrame().SetFocus()
                    return document
        for temp in wx.GetApp().GetDocumentManager().GetTemplates():
            if temp.GetDocumentType() == documentType:
                break
            temp = None
        newDoc = temp.CreateDocument(path, 0, data = objectToEdit, parentDocument = parentDocument)
        newDoc.SetDocumentName(temp.GetDocumentName())
        newDoc.SetDocumentTemplate(temp)
        if path == '':
            newDoc.OnNewDocument()
        else:
            if not newDoc.OnOpenDocument(path):
                newDoc.DeleteAllViews()  # Implicitly deleted by DeleteAllViews
                return None
        return newDoc


    def CloseChildDocuments(self, parentDocument):
        """
        Closes the child windows of a Document.
        """
        for document in self.GetDocumentManager().GetDocuments()[:]:  # Cloning list to make sure we go through all docs even as they are deleted
            if isinstance(document, ChildDocument) and document.GetParentDocument() == parentDocument:
                if document.GetFirstView().GetFrame():
                    document.GetFirstView().GetFrame().SetFocus()
                if not document.GetFirstView().OnClose():
                    return False
        return True


    def IsMDI(self):
        """
        Returns True if the application is in MDI mode.
        """
        return self.GetDocumentManager().GetFlags() & wx.lib.docview.DOC_MDI


    def IsSDI(self):
        """
        Returns True if the application is in SDI mode.
        """
        return self.GetDocumentManager().GetFlags() & wx.lib.docview.DOC_SDI


    def ShowSplash(self, image):
        """
        Shows a splash window with the given image.  Input parameter 'image' can either be a wx.Bitmap or a filename.
        """
        if isinstance(image, wx.Bitmap):
            splash_bmp = image
        else:
            splash_bmp = wx.Image(image).ConvertToBitmap()
        self._splash = wx.SplashScreen(splash_bmp, wx.SPLASH_CENTRE_ON_SCREEN|wx.SPLASH_NO_TIMEOUT, 0, None, -1, style=wx.SIMPLE_BORDER|wx.FRAME_NO_TASKBAR)
        self._splash.Show()


    def CloseSplash(self):
        """
        Closes the splash window.
        """
        if self._splash:
            self._splash.Close(True)


class _DocFrameFileDropTarget(wx.FileDropTarget):
    """
    Class used to handle drops into the document frame.
    """

    def __init__(self, docManager, docFrame):
        """
        Initializes the FileDropTarget class with the active docManager and the docFrame.
        """
        wx.FileDropTarget.__init__(self)
        self._docManager = docManager
        self._docFrame = docFrame


    def OnDropFiles(self, x, y, filenames):
        """
        Called when files are dropped in the drop target and tells the docManager to open
        the files.
        """
        try:
            for file in filenames:
                self._docManager.CreateDocument(file, wx.lib.docview.DOC_SILENT)
        except:
            msgTitle = wx.GetApp().GetAppName()
            if not msgTitle:
                msgTitle = _("File Error")
            wx.MessageBox("Could not open '%s'.  '%s'" % (wx.lib.docview.FileNameFromPath(file), sys.exc_value),
                          msgTitle,
                          wx.OK | wx.ICON_EXCLAMATION,
                          self._docManager.FindSuitableParent())


class DocMDIParentFrame(wx.lib.docview.DocMDIParentFrame, DocFrameMixIn, DocMDIParentFrameMixIn):
    """
    The DocMDIParentFrame is the primary frame which the DocApp uses to host MDI child windows.  It offers
    features such as a default menubar, toolbar, and status bar, and a mechanism to manage embedded windows
    on the edges of the DocMDIParentFrame.
    """


    def __init__(self, docManager, parent, id, title, pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE, name="DocMDIFrame", embeddedWindows=0, minSize=20):
        """
        Initializes the DocMDIParentFrame with the default menubar, toolbar, and status bar.  Use the
        optional embeddedWindows parameter with the embedded window constants to create embedded
        windows around the edges of the DocMDIParentFrame.
        """
        pos, size = self._GetPosSizeFromConfig(pos, size)
        wx.lib.docview.DocMDIParentFrame.__init__(self, docManager, parent, id, title, pos, size, style, name)
        self._InitFrame(embeddedWindows, minSize)


    def _LayoutFrame(self):
        """
        Lays out the frame.
        """
        wx.LayoutAlgorithm().LayoutMDIFrame(self)
        self.GetClientWindow().Refresh()


    def ProcessEvent(self, event):
        """
        Processes an event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        if wx.GetApp().ProcessEventBeforeWindows(event):
            return True
        if wx.lib.docview.DocMDIParentFrame.ProcessEvent(self, event):
            return True
        return DocMDIParentFrameMixIn.ProcessEvent(self, event)


    def ProcessUpdateUIEvent(self, event):
        """
        Processes a UI event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        if wx.GetApp().ProcessUpdateUIEventBeforeWindows(event):
            return True
        if wx.lib.docview.DocMDIParentFrame.ProcessUpdateUIEvent(self, event):  # Let the views handle the event before the services
            return True
        if event.GetId() == wx.ID_ABOUT:  # Using ID_ABOUT to update the window menu, the window menu items are not triggering
            self.UpdateWindowMenu()
            return True
        return DocMDIParentFrameMixIn.ProcessUpdateUIEvent(self, event)


    def UpdateWindowMenu(self):
        """
        Updates the WindowMenu on Windows platforms.
        """
        if wx.Platform == '__WXMSW__':
            children = filter(lambda child: isinstance(child, wx.MDIChildFrame), self.GetChildren())
            windowCount = len(children)
            hasWindow = windowCount >= 1
            has2OrMoreWindows = windowCount >= 2

            windowMenu = self.GetWindowMenu()
            if windowMenu:
                windowMenu.Enable(wx.IDM_WINDOWTILE, hasWindow)
                windowMenu.Enable(wx.IDM_WINDOWTILEHOR, hasWindow)
                windowMenu.Enable(wx.IDM_WINDOWCASCADE, hasWindow)
                windowMenu.Enable(wx.IDM_WINDOWICONS, hasWindow)
                windowMenu.Enable(wx.IDM_WINDOWTILEVERT, hasWindow)
                wx.IDM_WINDOWPREV = 4006  # wxBug: Not defined for some reason
                windowMenu.Enable(wx.IDM_WINDOWPREV, has2OrMoreWindows)
                windowMenu.Enable(wx.IDM_WINDOWNEXT, has2OrMoreWindows)



    def OnSize(self, event):
        """
        Called when the DocMDIParentFrame is resized and lays out the MDI client window.
        """
        # Needed in case there are splitpanels around the mdi frame
        self._LayoutFrame()


    def OnCloseWindow(self, event):
        """
        Called when the DocMDIParentFrame is closed.  Remembers the frame size.
        """
        self.SaveEmbeddedWindowSizes()

        # save and close services last.
        for service in wx.GetApp().GetServices():
            if not service.OnCloseFrame(event):
                return

        # save and close documents
        # documents with a common view, e.g. project view, should save the document, but not close the window
        # and let the service close the window.
        wx.lib.docview.DocMDIParentFrame.OnCloseWindow(self, event)


class DocSDIFrame(wx.lib.docview.DocChildFrame, DocFrameMixIn):
    """
    The DocSDIFrame host DocManager Document windows.  It offers features such as a default menubar,
    toolbar, and status bar.
    """


    def __init__(self, doc, view, parent, id, title, pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE, name="DocSDIFrame"):
        """
        Initializes the DocSDIFrame with the default menubar, toolbar, and status bar.
        """
        wx.lib.docview.DocChildFrame.__init__(self, doc, view, parent, id, title, pos, size, style, name)
        self._fileMenu = None
        if doc:
            self._docManager = doc.GetDocumentManager()
        else:
            self._docManager = None
        self.SetDropTarget(_DocFrameFileDropTarget(self._docManager, self))

        wx.EVT_MENU(self, wx.ID_ABOUT, self.OnAbout)
        wx.EVT_MENU(self, wx.ID_EXIT, self.OnExit)
        wx.EVT_MENU_RANGE(self, wx.ID_FILE1, wx.ID_FILE9, self.OnMRUFile)

        self.InitializePrintData()

        menuBar = self.CreateDefaultMenuBar(sdi=True)
        toolBar = self.CreateDefaultToolBar()
        self.SetToolBar(toolBar)
        statusBar = self.CreateDefaultStatusBar()

        for service in wx.GetApp().GetServices():
            service.InstallControls(self, menuBar = menuBar, toolBar = toolBar, statusBar = statusBar, document = doc)

        self.SetMenuBar(menuBar)  # wxBug: Need to do this in SDI to mimic MDI... because have to set the menubar at the very end or the automatic MDI "window" menu doesn't get put in the right place when the services add new menus to the menubar


    def _LayoutFrame(self):
        """
        Lays out the Frame.
        """
        self.Layout()


    def OnExit(self, event):
        """
        Called when the application is exitting.
        """
        self._childView.GetDocumentManager().Clear(force = False)


    def OnMRUFile(self, event):
        """
        Opens the appropriate file when it is selected from the file history
        menu.
        """
        n = event.GetId() - wx.ID_FILE1
        filename = self._docManager.GetHistoryFile(n)
        if filename:
            self._docManager.CreateDocument(filename, wx.lib.docview.DOC_SILENT)
        else:
            self._docManager.RemoveFileFromHistory(n)
            msgTitle = wx.GetApp().GetAppName()
            if not msgTitle:
                msgTitle = _("File Error")
            wx.MessageBox("The file '%s' doesn't exist and couldn't be opened.\nIt has been removed from the most recently used files list" % docview.FileNameFromPath(file),
                          msgTitle,
                          wx.OK | wx.ICON_EXCLAMATION,
                          self)


    def ProcessEvent(self, event):
        """
        Processes an event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        if wx.GetApp().ProcessEventBeforeWindows(event):
            return True
        if self._childView:
            self._childView.Activate(True)

        id = event.GetId()
        if id == SAVEALL_ID:
            self.OnFileSaveAll(event)
            return True

        if hasattr(self._childView, "GetDocumentManager") and self._childView.GetDocumentManager().ProcessEvent(event):  # Need to call docmanager here since super class relies on DocParentFrame which we are not using
            return True
        else:
            return wx.GetApp().ProcessEvent(event)


    def ProcessUpdateUIEvent(self, event):
        """
        Processes a UI event, searching event tables and calling zero or more
        suitable event handler function(s).  Note that the ProcessEvent
        method is called from the wxPython docview framework directly since
        wxPython does not have a virtual ProcessEvent function.
        """
        if wx.GetApp().ProcessUpdateUIEventBeforeWindows(event):
            return True
        if self._childView:
            if hasattr(self._childView, "GetDocumentManager"):
                docMgr = self._childView.GetDocumentManager()
                if docMgr:
                    if docMgr.GetCurrentDocument() != self._childView.GetDocument():
                        return False
                    if docMgr.ProcessUpdateUIEvent(event):  # Let the views handle the event before the services
                        return True
        id = event.GetId()
        if id == wx.ID_CUT:
            event.Enable(False)
            return True
        elif id == wx.ID_COPY:
            event.Enable(False)
            return True
        elif id == wx.ID_PASTE:
            event.Enable(False)
            return True
        elif id == wx.ID_CLEAR:
            event.Enable(False)
            return True
        elif id == wx.ID_SELECTALL:
            event.Enable(False)
            return True
        elif id == SAVEALL_ID:
            filesModified = False
            docs = wx.GetApp().GetDocumentManager().GetDocuments()
            for doc in docs:
                if doc.IsModified():
                    filesModified = True
                    break

            event.Enable(filesModified)
            return True
        else:
            return wx.GetApp().ProcessUpdateUIEvent(event)


    def OnCloseWindow(self, event):
        """
        Called when the window is saved.  Enables services to help close the frame.
        """
        for service in wx.GetApp().GetServices():
            service.OnCloseFrame(event)
        wx.lib.docview.DocChildFrame.OnCloseWindow(self, event)
        if self._fileMenu and self._docManager:
            self._docManager.FileHistoryRemoveMenu(self._fileMenu)


class AboutService(DocService):
    """
    About Dialog Service that installs under the Help menu to show the properties of the current application.
    """

    def __init__(self, aboutDialog=None, image=None):
        """
        Initializes the AboutService.
        """
        if aboutDialog:
            self._dlg = aboutDialog
            self._image = None
        else:
            self._dlg = AboutDialog  # use default AboutDialog
            self._image = image


    def ShowAbout(self):
        """
        Show the AboutDialog
        """
        if self._image:
            dlg = self._dlg(wx.GetApp().GetTopWindow(), self._image)
        else:
            dlg = self._dlg(wx.GetApp().GetTopWindow())
        dlg.CenterOnParent()
        dlg.ShowModal()
        dlg.Destroy()


    def SetAboutDialog(self, dlg):
        """
        Customize the AboutDialog
        """
        self._dlg = dlg


class AboutDialog(wx.Dialog):
    """
    Opens an AboutDialog.  Shared by DocMDIParentFrame and DocSDIFrame.
    """

    def __init__(self, parent, image=None):
        """
        Initializes the about dialog.
        """
        wx.Dialog.__init__(self, parent, -1, _("About ") + wx.GetApp().GetAppName(), style = wx.DEFAULT_DIALOG_STYLE)

        sizer = wx.BoxSizer(wx.VERTICAL)
        if image:
            imageItem = wx.StaticBitmap(self, -1, image.ConvertToBitmap(), (0,0), (image.GetWidth(), image.GetHeight()))
            sizer.Add(imageItem, 0, wx.ALIGN_CENTER|wx.ALL, 0)
        sizer.Add(wx.StaticText(self, -1, wx.GetApp().GetAppName()), 0, wx.ALIGN_CENTRE|wx.ALL, 5)

        btn = wx.Button(self, wx.ID_OK)
        sizer.Add(btn, 0, wx.ALIGN_CENTRE|wx.ALL, 5)

        self.SetSizer(sizer)
        sizer.Fit(self)



class FilePropertiesService(DocService):
    """
    Service that installs under the File menu to show the properties of the file associated
    with the current document.
    """

    PROPERTIES_ID = wx.NewId()


    def __init__(self):
        """
        Initializes the PropertyService.
        """
        self._customEventHandlers = []


    def InstallControls(self, frame, menuBar=None, toolBar=None, statusBar=None, document=None):
        """
        Installs a File/Properties menu item.
        """
        fileMenu = menuBar.GetMenu(menuBar.FindMenu(_("&File")))
        exitMenuItemPos = self.GetMenuItemPos(fileMenu, wx.ID_EXIT)
        fileMenu.InsertSeparator(exitMenuItemPos)
        fileMenu.Insert(exitMenuItemPos, FilePropertiesService.PROPERTIES_ID, _("&Properties"), _("Show file properties"))
        wx.EVT_MENU(frame, FilePropertiesService.PROPERTIES_ID, self.ProcessEvent)
        wx.EVT_UPDATE_UI(frame, FilePropertiesService.PROPERTIES_ID, self.ProcessUpdateUIEvent)


    def ProcessEvent(self, event):
        """
        Detects when the File/Properties menu item is selected.
        """
        id = event.GetId()
        if id == FilePropertiesService.PROPERTIES_ID:
            for eventHandler in self._customEventHandlers:
                if eventHandler.ProcessEvent(event):
                    return True

            self.ShowPropertiesDialog()
            return True
        else:
            return False


    def ProcessUpdateUIEvent(self, event):
        """
        Updates the File/Properties menu item.
        """
        id = event.GetId()
        if id == FilePropertiesService.PROPERTIES_ID:
            for eventHandler in self._customEventHandlers:
                if eventHandler.ProcessUpdateUIEvent(event):
                    return True

            event.Enable(wx.GetApp().GetDocumentManager().GetCurrentDocument() != None)
            return True
        else:
            return False


    def ShowPropertiesDialog(self, filename=None):
        """
        Shows the PropertiesDialog for the specified file.
        """
        if not filename:
            filename = wx.GetApp().GetDocumentManager().GetCurrentDocument().GetFilename()

        filePropertiesDialog = FilePropertiesDialog(wx.GetApp().GetTopWindow(), filename)
        filePropertiesDialog.CenterOnParent()
        if filePropertiesDialog.ShowModal() == wx.ID_OK:
            pass  # Handle OK
        filePropertiesDialog.Destroy()


    def GetCustomEventHandlers(self):
        """
        Returns the custom event handlers for the PropertyService.
        """
        return self._customEventHandlers


    def AddCustomEventHandler(self, handler):
        """
        Adds a custom event handlers for the PropertyService.  A custom event handler enables
        a different dialog to be provided for a particular file.
        """
        self._customEventHandlers.append(handler)


    def RemoveCustomEventHandler(self, handler):
        """
        Removes a custom event handler from the PropertyService.
        """
        self._customEventHandlers.remove(handler)


    def chopPath(self, text, length=36):
        """
        Simple version of textwrap.  textwrap.fill() unfortunately chops lines at spaces
        and creates odd word boundaries.  Instead, we will chop the path without regard to
        spaces, but pay attention to path delimiters.
        """
        chopped = ""
        textLen = len(text)
        start = 0

        while start < textLen:
            end = start + length
            if end > textLen:
                end = textLen

            # see if we can find a delimiter to chop the path
            if end < textLen:
                lastSep = text.rfind(os.sep, start, end + 1)
                if lastSep != -1 and lastSep != start:
                    end = lastSep

            if len(chopped):
                chopped = chopped + '\n' + text[start:end]
            else:
                chopped = text[start:end]

            start = end

        return chopped


class FilePropertiesDialog(wx.Dialog):
    """
    Dialog that shows the properties of a file.  Invoked by the PropertiesService.
    """


    def __init__(self, parent, filename):
        """
        Initializes the properties dialog.
        """
        wx.Dialog.__init__(self, parent, -1, _("File Properties"), size=(310, 330))

        HALF_SPACE = 5
        SPACE = 10

        filePropertiesService = wx.GetApp().GetService(FilePropertiesService)

        fileExists = os.path.exists(filename)

        notebook = wx.Notebook(self, -1)
        tab = wx.Panel(notebook, -1)

        gridSizer = RowColSizer()

        gridSizer.Add(wx.StaticText(tab, -1, _("Filename:")), flag=wx.RIGHT, border=HALF_SPACE, row=0, col=0)
        gridSizer.Add(wx.StaticText(tab, -1, os.path.basename(filename)), row=0, col=1)

        gridSizer.Add(wx.StaticText(tab, -1, _("Location:")), flag=wx.RIGHT, border=HALF_SPACE, row=1, col=0)
        gridSizer.Add(wx.StaticText(tab, -1, filePropertiesService.chopPath(os.path.dirname(filename))), flag=wx.BOTTOM, border=SPACE, row=1, col=1)

        gridSizer.Add(wx.StaticText(tab, -1, _("Size:")), flag=wx.RIGHT, border=HALF_SPACE, row=2, col=0)
        if fileExists:
            gridSizer.Add(wx.StaticText(tab, -1, str(os.path.getsize(filename)) + ' ' + _("bytes")), row=2, col=1)

        lineSizer = wx.BoxSizer(wx.VERTICAL)    # let the line expand horizontally without vertical expansion
        lineSizer.Add(wx.StaticLine(tab, -1, size = (10,-1)), 0, wx.EXPAND)
        gridSizer.Add(lineSizer, flag=wx.EXPAND|wx.ALIGN_CENTER_VERTICAL|wx.TOP, border=HALF_SPACE, row=3, col=0, colspan=2)

        gridSizer.Add(wx.StaticText(tab, -1, _("Created:")), flag=wx.RIGHT, border=HALF_SPACE, row=4, col=0)
        if fileExists:
            gridSizer.Add(wx.StaticText(tab, -1, time.ctime(os.path.getctime(filename))), row=4, col=1)

        gridSizer.Add(wx.StaticText(tab, -1, _("Modified:")), flag=wx.RIGHT, border=HALF_SPACE, row=5, col=0)
        if fileExists:
            gridSizer.Add(wx.StaticText(tab, -1, time.ctime(os.path.getmtime(filename))), row=5, col=1)

        gridSizer.Add(wx.StaticText(tab, -1, _("Accessed:")), flag=wx.RIGHT, border=HALF_SPACE, row=6, col=0)
        if fileExists:
            gridSizer.Add(wx.StaticText(tab, -1, time.ctime(os.path.getatime(filename))), row=6, col=1)

        # add a border around the inside of the tab
        spacerGrid = wx.BoxSizer(wx.VERTICAL)
        spacerGrid.Add(gridSizer, 0, wx.ALL, SPACE);
        tab.SetSizer(spacerGrid)
        notebook.AddPage(tab, _("General"))

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(notebook, 0, wx.ALL | wx.EXPAND, SPACE)
        sizer.Add(self.CreateButtonSizer(wx.OK), 0, wx.ALIGN_RIGHT | wx.RIGHT | wx.BOTTOM, HALF_SPACE)

        sizer.Fit(self)
        self.SetDimensions(-1, -1, 310, -1, wx.SIZE_USE_EXISTING)
        self.SetSizer(sizer)
        self.Layout()


class ChildDocument(wx.lib.docview.Document):
    """
    A ChildDocument is a document that represents a portion of a Document.  The child
    document is managed by the parent document, so it will be prompted to close if its
    parent is closed, etc.  Child Documents are useful when there are complicated
    Views of a Document and users will need to tunnel into the View.
    """


    def GetData(self):
        """
        Returns the data that the ChildDocument contains.
        """
        return self._data


    def SetData(self, data):
        """
        Sets the data that the ChildDocument contains.
        """
        self._data = data


    def GetParentDocument(self):
        """
        Returns the parent Document of the ChildDocument.
        """
        return self._parentDocument


    def SetParentDocument(self, parentDocument):
        """
        Sets the parent Document of the ChildDocument.
        """
        self._parentDocument = parentDocument


    def OnSaveDocument(self, filename):
        """
        Called when the ChildDocument is saved and does the minimum such that the
        ChildDocument looks like a real Document to the framework.
        """
        self.SetFilename(filename, True)
        self.Modify(False)
        self.SetDocumentSaved(True)
        return True


    def OnOpenDocument(self, filename):
        """
        Called when the ChildDocument is opened and does the minimum such that the
        ChildDocument looks like a real Document to the framework.
        """
        self.SetFilename(filename, True)
        self.Modify(False)
        self.SetDocumentSaved(True)
        self.UpdateAllViews()
        return True


    def Save(self):
        """
        Called when the ChildDocument is saved and does the minimum such that the
        ChildDocument looks like a real Document to the framework.
        """
        return self.OnSaveDocument(self._documentFile)


    def SaveAs(self):
        """
        Called when the ChildDocument is saved and does the minimum such that the
        ChildDocument looks like a real Document to the framework.
        """
        return self.OnSaveDocument(self._documentFile)


class ChildDocTemplate(wx.lib.docview.DocTemplate):
    """
    A ChildDocTemplate is a DocTemplate subclass that enables the creation of ChildDocuments
    that represents a portion of a Document.  The child document is managed by the parent document,
    so it will be prompted to close if its parent is closed, etc.  Child Documents are useful
    when there are complicated  Views of a Document and users will need to tunnel into the View.
    """


    def __init__(self, manager, description, filter, dir, ext, docTypeName, viewTypeName, docType, viewType, flags=wx.lib.docview.TEMPLATE_INVISIBLE, icon=None):
        """
        Initializes the ChildDocTemplate.
        """
        wx.lib.docview.DocTemplate.__init__(self, manager, description, filter, dir, ext, docTypeName, viewTypeName, docType, viewType, flags=flags, icon=icon)


    def CreateDocument(self, path, flags, data=None, parentDocument=None):
        """
        Called when a ChildDocument is to be created and does the minimum such that the
        ChildDocument looks like a real Document to the framework.
        """
        doc = self._docType()
        doc.SetFilename(path)
        doc.SetData(data)
        doc.SetParentDocument(parentDocument)
        doc.SetDocumentTemplate(self)
        self.GetDocumentManager().AddDocument(doc)
        doc.SetCommandProcessor(doc.OnCreateCommandProcessor())
        if doc.OnCreate(path, flags):
            return doc
        else:
            if doc in self.GetDocumentManager().GetDocuments():
                doc.DeleteAllViews()
            return None


class WindowMenuService(DocService):
    """
    The WindowMenuService is a service that implements a standard Window menu that is used
    by the DocSDIFrame.  The MDIFrame automatically includes a Window menu and does not use
    the WindowMenuService.
    """

    #----------------------------------------------------------------------------
    # Constants
    #----------------------------------------------------------------------------
    ARRANGE_WINDOWS_ID = wx.NewId()
    SELECT_MORE_WINDOWS_ID = wx.NewId()
    SELECT_NEXT_WINDOW_ID = wx.NewId()
    SELECT_PREV_WINDOW_ID = wx.NewId()
    CLOSE_CURRENT_WINDOW_ID = wx.NewId()


    def __init__(self):
        """
        Initializes the WindowMenu and its globals.
        """
        self._selectWinIds = []
        for i in range(0, 9):
            self._selectWinIds.append(wx.NewId())


    def InstallControls(self, frame, menuBar=None, toolBar=None, statusBar=None, document=None):
        """
        Installs the Window menu.
        """

        windowMenu = None
        if hasattr(frame, "GetWindowMenu"):
            windowMenu = frame.GetWindowMenu()
        if not windowMenu:
            needWindowMenu = True
            windowMenu = wx.Menu()
        else:
            needWindowMenu = False
            
        if self.GetDocumentManager().GetFlags() & wx.lib.docview.DOC_SDI:
            if not _WINDOWS:  # Arrange All and window navigation doesn't work on Linux
                return
                
            item = windowMenu.Append(self.ARRANGE_WINDOWS_ID, _("&Arrange All"), _("Arrange the open windows"))
            wx.EVT_MENU(frame, self.ARRANGE_WINDOWS_ID, frame.ProcessEvent)
            wx.EVT_UPDATE_UI(frame, self.ARRANGE_WINDOWS_ID, frame.ProcessUpdateUIEvent)
            windowMenu.AppendSeparator()

            for i, id in enumerate(self._selectWinIds):
                wx.EVT_MENU(frame, id, frame.ProcessEvent)
            wx.EVT_MENU(frame, self.SELECT_MORE_WINDOWS_ID, frame.ProcessEvent)
        elif wx.GetApp().GetUseTabbedMDI():
            item = windowMenu.Append(self.SELECT_PREV_WINDOW_ID, _("Previous"), _("Previous Tab"))
            wx.EVT_MENU(frame, self.SELECT_PREV_WINDOW_ID, frame.ProcessEvent)
            wx.EVT_UPDATE_UI(frame, self.SELECT_PREV_WINDOW_ID, frame.ProcessUpdateUIEvent)
            item = windowMenu.Append(self.SELECT_NEXT_WINDOW_ID, _("Next"), _("Next Tab"))
            wx.EVT_MENU(frame, self.SELECT_NEXT_WINDOW_ID, frame.ProcessEvent)
            wx.EVT_UPDATE_UI(frame, self.SELECT_NEXT_WINDOW_ID, frame.ProcessUpdateUIEvent)
            item = windowMenu.Append(self.CLOSE_CURRENT_WINDOW_ID, _("Close Current\tCtrl+F4"), _("Close Current Tab"))
            wx.EVT_MENU(frame, self.CLOSE_CURRENT_WINDOW_ID, frame.ProcessEvent)
            wx.EVT_UPDATE_UI(frame, self.CLOSE_CURRENT_WINDOW_ID, frame.ProcessUpdateUIEvent)
            self._sep = None

            for i, id in enumerate(self._selectWinIds):
                wx.EVT_MENU(frame, id, self.OnCtrlKeySelect)

        if needWindowMenu:
            helpMenuIndex = menuBar.FindMenu(_("&Help"))
            menuBar.Insert(helpMenuIndex, windowMenu, _("&Window"))

        self._lastFrameUpdated = None


    def OnCtrlKeySelect(self, event):
        i = self._selectWinIds.index(event.GetId())
        notebook = wx.GetApp().GetTopWindow()._notebook
        if i < notebook.GetPageCount():
            notebook.SetSelection(i)


    def ProcessEvent(self, event):
        """
        Processes a Window menu event.
        """
        id = event.GetId()
        if id == self.ARRANGE_WINDOWS_ID:
            self.OnArrangeWindows(event)
            return True
        elif id == self.SELECT_MORE_WINDOWS_ID:
            self.OnSelectMoreWindows(event)
            return True
        elif id in self._selectWinIds:
            self.OnSelectWindowMenu(event)
            return True
        elif wx.GetApp().GetUseTabbedMDI():
            if id == self.SELECT_NEXT_WINDOW_ID:
                notebook = wx.GetApp().GetTopWindow()._notebook
                i = notebook.GetSelection()
                notebook.SetSelection(i+1)
                return True
            elif id == self.SELECT_PREV_WINDOW_ID:
                notebook = wx.GetApp().GetTopWindow()._notebook
                i = notebook.GetSelection()
                notebook.SetSelection(i-1)
                return True
            elif id == self.CLOSE_CURRENT_WINDOW_ID:
                notebook = wx.GetApp().GetTopWindow()._notebook
                i = notebook.GetSelection()
                if i != -1:
                    doc = notebook.GetPage(i).GetView().GetDocument()
                    wx.GetApp().GetDocumentManager().CloseDocument(doc, False)
                return True
        else:
            return False


    def ProcessUpdateUIEvent(self, event):
        """
        Updates the Window menu items.
        """
        id = event.GetId()
        if id == self.ARRANGE_WINDOWS_ID:
            frame = event.GetEventObject()
            if not self._lastFrameUpdated or self._lastFrameUpdated != frame:
                self.BuildWindowMenu(frame)  # It's a new frame, so update the windows menu... this is as if the View::OnActivateMethod had been invoked
                self._lastFrameUpdated = frame
            return True
        elif wx.GetApp().GetUseTabbedMDI():
            if id == self.SELECT_NEXT_WINDOW_ID:
                self.BuildWindowMenu(event.GetEventObject())  # build file list only when we are updating the windows menu
                
                notebook = wx.GetApp().GetTopWindow()._notebook
                i = notebook.GetSelection()
                if i == -1:
                    event.Enable(False)
                    return True
                i += 1
                if i >= notebook.GetPageCount():
                    event.Enable(False)
                    return True
                event.Enable(True)
                return True
            elif id == self.SELECT_PREV_WINDOW_ID:
                notebook = wx.GetApp().GetTopWindow()._notebook
                i = notebook.GetSelection()
                if i == -1:
                    event.Enable(False)
                    return True
                i -= 1
                if i < 0:
                    event.Enable(False)
                    return True
                event.Enable(True)
                return True
            elif id == self.CLOSE_CURRENT_WINDOW_ID:
                event.Enable(wx.GetApp().GetTopWindow()._notebook.GetSelection() != -1)
                return True

            return False
        else:
            return False


    def BuildWindowMenu(self, currentFrame):
        """
        Builds the Window menu and adds menu items for all of the open documents in the DocManager.
        """
        if wx.GetApp().GetUseTabbedMDI():
            currentFrame = wx.GetApp().GetTopWindow()

        windowMenuIndex = currentFrame.GetMenuBar().FindMenu(_("&Window"))
        windowMenu = currentFrame.GetMenuBar().GetMenu(windowMenuIndex)

        if self.GetDocumentManager().GetFlags() & wx.lib.docview.DOC_SDI:
            frames = self._GetWindowMenuFrameList(currentFrame)
            max = WINDOW_MENU_NUM_ITEMS
            if max > len(frames):
                max = len(frames)
            i = 0
            for i in range(0, max):
                frame = frames[i]
                item = windowMenu.FindItemById(self._selectWinIds[i])
                label = '&' + str(i + 1) + ' ' + frame.GetTitle()
                if not item:
                    item = windowMenu.AppendCheckItem(self._selectWinIds[i], label)
                else:
                    windowMenu.SetLabel(self._selectWinIds[i], label)
                windowMenu.Check(self._selectWinIds[i], (frame == currentFrame))
            if len(frames) > WINDOW_MENU_NUM_ITEMS:  # Add the more items item
                if not windowMenu.FindItemById(self.SELECT_MORE_WINDOWS_ID):
                    windowMenu.Append(self.SELECT_MORE_WINDOWS_ID, _("&More Windows..."))
            else:  # Remove any extra items
                if windowMenu.FindItemById(self.SELECT_MORE_WINDOWS_ID):
                    windowMenu.Remove(self.SELECT_MORE_WINDOWS_ID)

                for j in range(i + 1, WINDOW_MENU_NUM_ITEMS):
                    if windowMenu.FindItemById(self._selectWinIds[j]):
                        windowMenu.Remove(self._selectWinIds[j])

        elif wx.GetApp().GetUseTabbedMDI():
            notebook = wx.GetApp().GetTopWindow()._notebook
            numPages = notebook.GetPageCount()

            for id in self._selectWinIds:
                item = windowMenu.FindItemById(id)
                if item:
                    windowMenu.DeleteItem(item)
            if numPages == 0 and self._sep:
                windowMenu.DeleteItem(self._sep)
                self._sep = None

            if numPages > len(self._selectWinIds):
                for i in range(len(self._selectWinIds), numPages):
                    self._selectWinIds.append(wx.NewId())
                    wx.EVT_MENU(currentFrame, self._selectWinIds[i], self.OnCtrlKeySelect)                    
            
            for i in range(0, numPages):
                if i == 0 and not self._sep:
                    self._sep = windowMenu.AppendSeparator()
                if i < 9:
                    menuLabel = "%s\tCtrl+%s" % (notebook.GetPageText(i), i+1)
                else:
                    menuLabel = notebook.GetPageText(i)
                windowMenu.Append(self._selectWinIds[i], menuLabel)


    def _GetWindowMenuIDList(self):
        """
        Returns a list of the Window menu item IDs.
        """
        return self._selectWinIds


    def _GetWindowMenuFrameList(self, currentFrame=None):
        """
        Returns the Frame associated with each menu item in the Window menu.
        """
        frameList = []
        # get list of windows for documents
        for doc in self._docManager.GetDocuments():
            for view in doc.GetViews():
                frame = view.GetFrame()
                if frame not in frameList:
                    if frame == currentFrame and len(frameList) >= WINDOW_MENU_NUM_ITEMS:
                        frameList.insert(WINDOW_MENU_NUM_ITEMS - 1, frame)
                    else:
                        frameList.append(frame)
        # get list of windows for general services
        for service in wx.GetApp().GetServices():
            view = service.GetView()
            if view:
                frame = view.GetFrame()
                if frame not in frameList:
                    if frame == currentFrame and len(frameList) >= WINDOW_MENU_NUM_ITEMS:
                        frameList.insert(WINDOW_MENU_NUM_ITEMS - 1, frame)
                    else:
                        frameList.append(frame)

        return frameList


    def OnArrangeWindows(self, event):
        """
        Called by Window/Arrange and tiles the frames on the desktop.
        """
        currentFrame = event.GetEventObject()

        tempFrame = wx.Frame(None, -1, "", pos = wx.DefaultPosition, size = wx.DefaultSize)
        sizex = tempFrame.GetSize()[0]
        sizey = tempFrame.GetSize()[1]
        tempFrame.Destroy()

        posx = 0
        posy = 0
        delta = 0
        frames = self._GetWindowMenuFrameList()
        frames.remove(currentFrame)
        frames.append(currentFrame) # Make the current frame the last frame so that it is the last one to appear
        for frame in frames:
            if delta == 0:
                delta = frame.GetClientAreaOrigin()[1]
            frame.SetPosition((posx, posy))
            frame.SetSize((sizex, sizey))
            # TODO: Need to loop around if posx + delta + size > displaysize
            frame.SetFocus()
            posx = posx + delta
            posy = posy + delta
            if posx + sizex > wx.DisplaySize()[0] or posy + sizey > wx.DisplaySize()[1]:
                posx = 0
                posy = 0
        currentFrame.SetFocus()


    def OnSelectWindowMenu(self, event):
        """
        Called when the Window menu item representing a Frame is selected and brings the selected
        Frame to the front of the desktop.
        """
        id = event.GetId()
        index = self._selectWinIds.index(id)
        if index > -1:
            currentFrame = event.GetEventObject()
            frame = self._GetWindowMenuFrameList(currentFrame)[index]
            if frame:
                wx.CallAfter(frame.Raise)


    def OnSelectMoreWindows(self, event):
        """
        Called when the "Window/Select More Windows..." menu item is selected and enables user to
        select from the Frames that do not in the Window list.  Useful when there are more than
        10 open frames in the application.
        """
        frames = self._GetWindowMenuFrameList()  # TODO - make the current window the first one
        strings = map(lambda frame: frame.GetTitle(), frames)
        # Should preselect the current window, but not supported by wx.GetSingleChoice
        res = wx.GetSingleChoiceIndex(_("Select a window to show:"),
                                      _("Select Window"),
                                      strings,
                                      self)
        if res == -1:
            return
        frames[res].SetFocus()



def getBlankIcon():
    return Blank.GetIcon()

#----------------------------------------------------------------------------
# File generated by encode_bitmaps.py
#----------------------------------------------------------------------------
from wx.lib.embeddedimage import PyEmbeddedImage

New = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAQRJ"
    "REFUOI2lk71qAkEQx397Z21l5WPkCYKFhYVvYeUr2FrYJ2X6gNjYWBlSxAeQBNFKCBcMclxW"
    "4eT8gLU499i9D6I4MOzsMr//zsyyQjgu91jJ3HxMl0rHvgwB8NYBAFJGdFp1kVEQjpv45PNb"
    "LXcq8a9t7M+DiRqtlOq+jJWZLxwX59pSyyV4aNRod1+VeW614MuQ6iUOT/G62cflvw/fWF3a"
    "KRQwQQ0DPDZrbE/wd4R+78nKL2xBw0AC55lVgbcOqOztBBPeHP4RkDKyQMjCi9m8WMAENazB"
    "IEpn5gjoKadv1bC/ywpkhngLnCtwCwypFn68X+ud0wPLM3Hvb7z6LxTZGR/7imGH8vcWAAAA"
    "AElFTkSuQmCC")

Open = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAWdJ"
    "REFUOI2lkz1LQlEYx39Xbyk1FTk0REMEIdELTS3R0FSBurf4AVqa2/oIQUMQDrWGukRDYENN"
    "YU7ZC1FGN+J6wWt6zcSX06DevFxvRf2nc87z/H/nec6LJLnc/EcyQPFiQwAYJc0SHF7cl34E"
    "GOdLom9mBYD+jkDDyPJ6ShOsK+b6eChhgcqid4qGkYWqbkX3DODzzwNQfXwzl5MRVcyF0yZE"
    "7vSIsvo1KasomSIAuYdjCzsZ8QuAuXBasgAA01TLpQAo5FVm17ZtvacO1onHosIEPF/f4xUa"
    "tVa/Y6ubNlM3yUZJQ7/L4BUavullfK1AQ791NJ3sblGZ3GkCAOofGhVdYahwY0uuvz85ggLB"
    "kCQbuoI86OuaXMsp3XwUchU8rbGrmZhiZGLUcafvZLsFp92PDi+dAYW8ClgrOEtcmeUCeBb2"
    "ugIkyeU2H0Zb2otunnKnAsGQ7W9I7d8Yj0XFbwyOgL/qE6gYiTXnTLM6AAAAAElFTkSuQmCC")

Copy = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAV9J"
    "REFUOI2Nkr9LAmEYxz93+ifY0KDkdtAkTY1CERGlQn9AQ9BSQ3trOEWDNLQ1ChX9gGhoaNKt"
    "SYKkIbiEytQ7pfJS4W3Q+/VeHn7h5V6ee76f9/u8d0qpogtGapjfANTqBgBm22Jvc1EhTKWK"
    "LqpfIrCOzsui/CnETv5UKGqEsatU0UUsGXeAjZdXdz9BItVu8iqWjBNLxtFSGlpKY2FpHoD0"
    "aprt/aLwAULnG6nZHT47A1jZWPdBVG9Ds+vGlmW2LR5u77k5OQPg6vJCAERrdYMpzYWMO31u"
    "OQ2A0YdivgCzM8MEZttymjsDv0mGGv3gAapt7AyGDfaNe832OwCzJwHeP5o+upxINrcseH6q"
    "Oj1Rn7nnH0Wut1y2H+CNFUgkmRs/EkDX39APCk4hkZie2AwQvT7eVexvCnD3+OtLFGZ2Rshk"
    "c87/vbZ1KLyJvBf2nxRFjQSK3kRhymRzyh8MSdTaiEWdxAAAAABJRU5ErkJggg==")

Paste = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAZBJ"
    "REFUOI2Nkz9IAmEYh59Td7GpIXHIsD9C0JgQRFMQmIJD0RIazQ1iJq0aFDRGYFvUIGhCUEtD"
    "g1CLNOSpdFCCmIhgJIKL2HDddael/ZZ7v+/9vc/3fnf3CoLBCEBN7HTRKHgSZWbKTi4vEVzc"
    "06Zweo2CEguCwUhN7HQtY/eqIRDJ4F1yUqy0AcjlJeK7RjVffAypEAM9CkQyACTvnsnlJXJ5"
    "Sd4/6Oh86atUF8DUC9gKbKhx/aMlt9l44a09wnG6wo77VefvAwCYbdbvp7IzydttBvuKj+B5"
    "isDEEADAzUVCjZfXfWq8sOYhfplk0TEEoC1SJF0nqDRNNMoFcDh/B9Q/WphtcgeVppz2b3uw"
    "zrsAsAPpw9jwK2g7ePn8y9ULaD5QencxPgtnpyldqlEuAGDbDA8AaE70b3t+PfmpDlmxindJ"
    "8w5qBbCYHwB4LdWwzsPRfkxXmBWrzE2P/q8DAHcwzFP9Z237w9f3Kyt31RYPkgnk6XpOhr6n"
    "Mar7TFmxOhAgKOMMPwPyX7lXPcIXHPiHmgMS17kAAAAASUVORK5CYII=")

Save = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAR1J"
    "REFUOI2dkz1Ow0AQhb/1oi3xEaCNQ4+CoOWnpuIEtHAAGvsCEVT0pKDBwVwBkFIixaSJUkQU"
    "BCSiQAGxBEPhyLFgHUNeNTs/782MdpRyNPHFp1CCYf+3b/1AK5SjuW8iMqhLGIZShMGHyN3z"
    "WHzfFxGRm2MkiiJx8oxaa6v60xge3xK67waA2xOVxRxrRQnWtvfsBEmSlKr/xFwd8NL9G0GZ"
    "OsBC/rG78UAQBDPFjzavAOh1wF2eEAz7QG2SsL84zR61rG3bOxi1wF0lOH3NhSuZ1Y6/Mvv8"
    "sGcfIVWs4Hle5jLGcNZoFI40c4nGpMtbqVb/R6C1zoqXalu047iQQCkn/b7X9eKD6nXsfncn"
    "mhIAXDbD0qss7GBefAO233xRLqcViQAAAABJRU5ErkJggg==")

SaveAll = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAVVJ"
    "REFUOI2dk79Lw0Acxd8lAWfrIB2Ki4NNEekWiov/QKDdFCcXF0Ey6SwIyRSCbi5ubpreP+BS"
    "O7g4mXboYIYKLiKBtrS0fh3SxF6SavUDx/1+9+5xxzwXhCmjHgSGM/3xMKy3j2UmLPJcEPkG"
    "cc4pSX9M9NonMk2TiIgeLkCcc2KSjLh4LqhYNtB6smPRPzmJHPzXiYIE6586WvVsJzs54PHq"
    "FMoSsOLraDggKSkAAMWygY7EsbVHQtk8IBT2CffvJipHYfaZAlkMJsDHCHjpAUEQoHnJFheY"
    "3RxR0Y2fBTzPEzbPIxVixMnuGyzLivtBEMTt88MBAMBvA0qpJrOGY9NyIVvkNxQgfBTPtxNK"
    "Tlo3q8Lp3W43nrs+ywGYyaBUk1mT20iiqio0TUO1Ws10IISY+ihT8vn83CswJsmpwYbzfR2/"
    "DaxthPXCAgBQd+9SmWTxBXqU0GLLIFPzAAAAAElFTkSuQmCC")

Print = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAadJ"
    "REFUOI2lUz1Lw1AUPe+1IA72F7gkWBCngrQKXQVBB+tkJf4Cl4LgoIOjiLVF/QdqC3HqBxgo"
    "ODo1FqJI8YtOEgl06YvdWp5DfGmT1A/wwOHmvtx73rnJe4TQEP6DsHg4bfa4afc9L/utB0+e"
    "XZ8lAQVCQyA0hB29x/14sh1emZwf5o55tVrlol6QCqGDuTGyeztw8PzhxFfbiePLGaiqikq5"
    "xIcN0OHEb/kvCAPAxfkZB4D7r53Fri02KDTtPppvQQF6cX7GFUWBoiju4tSEE+VIsME/BgWA"
    "YrHoFrzaDlvMoWn3Mfx38vm8R4QUCgUei8Wg1a6xvZWBZVkjZ2232558Y3MfdzeXJGxabcQA"
    "LC0uIJs/wXRUHinAGEMkEgFjzofpvuuolEucTkdlaLVrAIDRqCORSECW5QAlScLjSwuapkGS"
    "JFc4vJJaJevpNW406gGrnU7H4yI5H0dyPu5Zc49yOp0GYwyGYXgKTMs7ux+E0BAq5RJXVRXN"
    "N2Bm8sd6AICu68jlclhJrRIibmOlXOJ7Ryq67/qvAqLZdSDgP+ffQTQDwCfjgskYZ5COXQAA"
    "AABJRU5ErkJggg==")

PrintPreview = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAW1J"
    "REFUOI2Nkr1LAmEcxz93GoVFBDU4OFhIYdEUIUFCDS69B20tQnkuTf4DTgUO7fkSNAQ1BBXc"
    "2iY0tURDSoqBRQkugpE42PBwT3feGX3h4Xf3Oz7f38tzyt1DuUOXHotV+ZzYXVa6v5ulAvim"
    "xi1nZT0MQHApzMHhpa2AzaCXhvtgNbrzp4nbKVn/FvEieyVztzfXnc2tbds4NgMDDkXCBNtQ"
    "a8F5MkVkZtp5hObbk4SM2GiLU2v1atxkUGqOWGBDBpw/iuPvL6PrOrH9PdsuLCM02nYYLW0B"
    "YtDJ5k7lLlRzy05weg6ifpELeAAtbelELVc+JWjAH1+/Fe/rkK/B4hh4PSKCuBXZgXlZZhjg"
    "rCLAwBCExkQ0y/IjdcOybWB0ACYHRTTLXXguUUim7CQT+DNxXrQ0Xo+4pSJwnIijkeGdNQAU"
    "RXXJeZyk67rlfbaaAWDBB/M5l6IoqqsXK9Vd4PVkQ5r8y8DJ0DD5ATHJh/O00XSvAAAAAElF"
    "TkSuQmCC")

Cut = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAUhJ"
    "REFUOI2Fkk9LAlEUxX+jBX0bocCdtKpmNpNhEONeg2zfxqUbSUG0oJxN5SKEGP8QOLvwk7R2"
    "FWiprxavhbxhdF7Tgbc599zzznv3YiSShE+n05GbXPjU67W1egINnh4fpI5vNRsSYNDvBXWt"
    "QS6Xw3Xv10xazYa07WxEqzXQQQjBbDZlPl/EG4hvAUDeyQeRa9dVaZqW1nhrkygUzg1AHh4c"
    "RcSe55FKpTjOnhh/GgAsF6uYtp1FCCFN04pEVzCMRFJbULFVo++PIrdDzCfuvXfxPC9ozkyG"
    "Wp32CW8XuzJtOTB6xvchMxmSthzG3auINnaMacsJmhXCS6Q1ULcXbysALKafABRvK+yfFVi6"
    "l/EJVEMc/l1lgHapzLjrMvv44vWuQbtU1uoiYxz0e3L7ZfVZP6fVgFfRdwo3a6PU7oGKGBbq"
    "OIBfXKKLiCHSUwgAAAAASUVORK5CYII=")

Undo = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAadJ"
    "REFUOI2lkL9LW1EYhp9zI41/QnXI1snBoYPQFqFDQCqYxKIS2o41m4MEB0FwaBcpFt3UwaFa"
    "G8UmOaZQuneom5ugU7Dij1wlMV6N3vo5hHPx6qUGPHCGw/me57zvUcoK8ZDVdN9AuhCVcs0B"
    "YO71b9WwIF2ICkDXs14ALq/O2J0+EIAfQ1ueSAVVSBeichOsnV9QcY8BKDk23xbXWP+4rwIF"
    "BjagKzXmswsc7lcAeNHzFICvk7/Yy7jqTgXT17ya0d8ZjExApH6fmuyjs6cNgHwuKz7B+9Xn"
    "MvDyLVWn7INj8YTXOZ/LSmqpL/gTy+dVLq/OcKUGUI8d8SeMxRMqn8uKJ+hfbBeA5Tcb6tSu"
    "x3f+OZQc24MeDzQJwF7GVUZi7qxT22H41Qjd00/EdDeruG2TWvJ3vv1nSmsts8URxt6NU5WK"
    "17/k2Oyc7HKyU2Zt+Q8zyRXfy16CWDyhBiMTDH0aBiDc/Mg38D+4HsEKoawQWmvpGGuVn0dT"
    "8qU4Kp83U9KSDIvWWsxM0PYdtNbSkgzL6t8PDcF3BDcljcCBAiNpBFZWiGuCvt7CsLLqpwAA"
    "AABJRU5ErkJggg==")

Redo = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAYhJ"
    "REFUOI2lksFLAkEUxr9RCbwUdEr7DzIIukTdOuUh0q0kL4JGUAaZhwShg0QQC4XUqYwgKII8"
    "rc6p/8Bb0im8dQmy0KW1lBR6HWSHXRtCaODB8N77fjPvm2HM4cR/lkuW3Nb8ZO5PlDvWN8AU"
    "RmfXRa5+pVPj00Ah/igFMXOEbc1PViEANL8NAIDeruJCu5RCXAAQuZoS4ixXRXFjflPs15QY"
    "qq9ZKmVebBCH9bQsV7E8mBZxmFeht6sAgEanDgAoFjSyAsAcTii5cVJy48Q5J+Zwwhoz+6N0"
    "+7xL509bpFbC5EsO2/pcABDzHAAAAkFFapTRMVD/epOVuoBe4fTeCAFAKfPC3mstIa41dejP"
    "LQCAZ2mAbB70rsRqFGOJITFvramL2sb1ChZ2JrsWyH5isaCR+hBHNLJoO73R/hA9/OgeZ5G8"
    "/AaBoMLSE6c4Ob6R3s4UB4IKQ6/r1uCcky85TGolTKnyHKXKc+QNuW2v8CfAhHhDblIr4V/i"
    "vgBWiOyf9AUwIbL8D+DEtq2XUuV6AAAAAElFTkSuQmCC")

Blank = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAF1J"
    "REFUOI3tkzEOwDAIA22S//84cYe2QxGJqFg61BMDOhsBpDVU1O9Cc2jXSGvcAgBAihkkoTkU"
    "QSyV84JHKdMA8jT3kB52B+4e9DrBSj/gC4DHHfgdZ8TqN5ZHOACokRkohSNQfwAAAABJRU5E"
    "rkJggg==")

