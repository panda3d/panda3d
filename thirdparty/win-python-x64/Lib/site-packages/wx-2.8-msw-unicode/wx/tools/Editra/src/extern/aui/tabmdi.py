__author__ = "Andrea Gavana <andrea.gavana@gmail.com>"
__date__ = "31 March 2009"


import wx

import auibook
from aui_constants import *

_ = wx.GetTranslation

#-----------------------------------------------------------------------------
# AuiMDIParentFrame
#-----------------------------------------------------------------------------

class AuiMDIParentFrame(wx.Frame):

    def __init__(self, parent, id=wx.ID_ANY, title="", pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE|wx.VSCROLL|wx.HSCROLL,
                 name="AuiMDIParentFrame"):

        wx.Frame.__init__(self, parent, id, title, pos, size, style, name=name)
        self.Init()

        self.Bind(wx.EVT_MENU, self.DoHandleMenu, id=wx.ID_ANY)

        # this style can be used to prevent a window from having the standard MDI
        # "Window" menu
        if not style & wx.FRAME_NO_WINDOW_MENU:
        
            self._pWindowMenu = wx.Menu()
            self._pWindowMenu.Append(wxWINDOWCLOSE,    _("Cl&ose"))
            self._pWindowMenu.Append(wxWINDOWCLOSEALL, _("Close All"))
            self._pWindowMenu.AppendSeparator()
            self._pWindowMenu.Append(wxWINDOWNEXT,     _("&Next"))
            self._pWindowMenu.Append(wxWINDOWPREV,     _("&Previous"))
    
        self._pClientWindow = self.OnCreateClient()


    def SetArtProvider(self, provider):

        if self._pClientWindow:
            self._pClientWindow.SetArtProvider(provider)
    

    def GetArtProvider(self):

        if not self._pClientWindow:
            return None

        return self._pClientWindow.GetArtProvider()


    def GetNotebook(self):

        return self._pClientWindow


    def SetWindowMenu(self, pMenu):

        # Replace the window menu from the currently loaded menu bar.
        pMenuBar = self.GetMenuBar()

        if self._pWindowMenu:
            self.RemoveWindowMenu(pMenuBar)
            del self._pWindowMenu
            self._pWindowMenu = None

        if pMenu:
            self._pWindowMenu = pMenu
            self.AddWindowMenu(pMenuBar)
        

    def GetWindowMenu(self):

        return self._pWindowMenu
    

    def SetMenuBar(self, pMenuBar):

        # Remove the Window menu from the old menu bar
        self.RemoveWindowMenu(self.GetMenuBar())

        # Add the Window menu to the new menu bar.
        self.AddWindowMenu(pMenuBar)

        wx.Frame.SetMenuBar(self, pMenuBar)


    def SetChildMenuBar(self, pChild):

        if not pChild:
        
            # No Child, set Our menu bar back.
            if self._pMyMenuBar:
                self.SetMenuBar(self._pMyMenuBar)
            else:
                self.SetMenuBar(self.GetMenuBar())

            # Make sure we know our menu bar is in use
            self._pMyMenuBar = None
        
        else:
        
            if pChild.GetMenuBar() == None:
                return

            # Do we need to save the current bar?
            if self._pMyMenuBar == None:
                self._pMyMenuBar = self.GetMenuBar()

            self.SetMenuBar(pChild.GetMenuBar())
    

    def ProcessEvent(self, event):

        # stops the same event being processed repeatedly
        if self._pLastEvt == event:
            return False
        
        self._pLastEvt = event

        # let the active child (if any) process the event first.
        res = False
        if self._pActiveChild and event.IsCommandEvent() and \
           event.GetEventObject() != self._pClientWindow and \
           event.GetEventType() not in [wx.wxEVT_ACTIVATE, wx.wxEVT_SET_FOCUS,
                                        wx.wxEVT_KILL_FOCUS, wx.wxEVT_CHILD_FOCUS,
                                        wx.wxEVT_COMMAND_SET_FOCUS, wx.wxEVT_COMMAND_KILL_FOCUS]:
        
            res = self._pActiveChild.GetEventHandler().ProcessEvent(event)
        
        if not res:
        
            # if the event was not handled this frame will handle it,
            # which is why we need the protection code at the beginning
            # of this method
            res = self.GetEventHandler().ProcessEvent(event)
        
        self._pLastEvt = None

        return res


    def GetActiveChild(self):

        return self._pActiveChild


    def SetActiveChild(self, pChildFrame):

        self._pActiveChild = pChildFrame


    def GetClientWindow(self):

        return self._pClientWindow


    def OnCreateClient(self):

        return AuiMDIClientWindow(self)


    def ActivateNext(self):

        if self._pClientWindow and self._pClientWindow.GetSelection() != wx.NOT_FOUND:
        
            active = self._pClientWindow.GetSelection() + 1
            if active >= self._pClientWindow.GetPageCount():
                active = 0

            self._pClientWindow.SetSelection(active)
        

    def ActivatePrevious(self):

        if self._pClientWindow and self._pClientWindow.GetSelection() != wx.NOT_FOUND:
        
            active = self._pClientWindow.GetSelection() - 1
            if active < 0:
                active = self._pClientWindow.GetPageCount() - 1

            self._pClientWindow.SetSelection(active)
    

    def Init(self):

        self._pLastEvt = None

        self._pClientWindow = None
        self._pActiveChild = None
        self._pWindowMenu = None
        self._pMyMenuBar = None


    def RemoveWindowMenu(self, pMenuBar):

        if pMenuBar and self._pWindowMenu:
        
            # Remove old window menu
            pos = pMenuBar.FindMenu(_("&Window"))
            if pos != wx.NOT_FOUND:            
                pMenuBar.Remove(pos)
            

    def AddWindowMenu(self, pMenuBar):

        if pMenuBar and self._pWindowMenu:
        
            pos = pMenuBar.FindMenu(wx.GetStockLabel(wx.ID_HELP, wx.STOCK_NOFLAGS))
            if pos == wx.NOT_FOUND:
                pMenuBar.Append(self._pWindowMenu, _("&Window"))
            else:
                pMenuBar.Insert(pos, self._pWindowMenu, _("&Window"))
    

    def DoHandleMenu(self, event):

        evId = event.GetId()
        
        if evId == wxWINDOWCLOSE:
            if self._pActiveChild:
                self._pActiveChild.Close()

        elif evId == wxWINDOWCLOSEALL:
            
            while self._pActiveChild:            
                if not self._pActiveChild.Close():
                    return # failure
                
        elif evId == wxWINDOWNEXT:
            self.ActivateNext()

        elif evId == wxWINDOWPREV:
            self.ActivatePrevious()

        else:
            event.Skip()

    
    def Tile(self, orient=wx.HORIZONTAL):

        client_window = self.GetClientWindow()
        if not client_window:
            raise Exception("Missing MDI Client Window")

        cur_idx = client_window.GetSelection()
        if cur_idx == -1:
            return

        if orient == wx.VERTICAL:
        
            client_window.Split(cur_idx, wx.LEFT)
        
        elif orient == wx.HORIZONTAL:
        
            client_window.Split(cur_idx, wx.TOP)
    

#-----------------------------------------------------------------------------
# AuiMDIChildFrame
#-----------------------------------------------------------------------------

class AuiMDIChildFrame(wx.PyPanel):

    def __init__(self, parent, id=wx.ID_ANY, title="", pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE, name="AuiMDIChildFrame"):

        pClientWindow = parent.GetClientWindow()
        if pClientWindow is None:
            raise Exception("Missing MDI client window.")

        self.Init()
        
        # see comment in constructor
        if style & wx.MINIMIZE:
            self._activate_on_create = False

        cli_size = pClientWindow.GetClientSize()

        # create the window off-screen to prevent flicker
        wx.PyPanel.__init__(self, pClientWindow, id, wx.Point(cli_size.x+1, cli_size.y+1),
                            size, wx.NO_BORDER, name=name)

        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        self.Show(False)
        self.SetMDIParentFrame(parent)

        # this is the currently active child
        parent.SetActiveChild(self)
        self._title = title

        pClientWindow.AddPage(self, title, self._activate_on_create)
        pClientWindow.Refresh()

        self.Bind(wx.EVT_MENU_HIGHLIGHT_ALL, self.OnMenuHighlight)
        self.Bind(wx.EVT_ACTIVATE, self.OnActivate)
        self.Bind(wx.EVT_CLOSE, self.OnCloseWindow)


    def Init(self):

        # There are two ways to create an tabbed mdi child fram without
        # making it the active document.  Either Show(False) can be called
        # before Create() (as is customary on some ports with wxFrame-type
        # windows), or wx.MINIMIZE can be passed in the style flags.  Note that
        # AuiMDIChildFrame is not really derived from wxFrame, as MDIChildFrame
        # is, but those are the expected symantics.  No style flag is passed
        # onto the panel underneath.

        self._activate_on_create = True

        self._pMDIParentFrame = None
        self._pMenuBar = None
        
        self._mdi_currect = None
        self._mdi_newrect = wx.Rect()
        self._icon = None
        self._icon_bundle = None


    def Destroy(self):

        pParentFrame = self.GetMDIParentFrame()
        if not pParentFrame:
            raise Exception("Missing MDI Parent Frame")

        pClientWindow = pParentFrame.GetClientWindow()
        if not pClientWindow:
            raise Exception("Missing MDI Client Window")

        if pParentFrame.GetActiveChild() == self:
        
            # deactivate ourself
            event = wx.ActivateEvent(wx.wxEVT_ACTIVATE, False, self.GetId())
            event.SetEventObject(self)
            self.GetEventHandler().ProcessEvent(event)

            pParentFrame.SetActiveChild(None)
            pParentFrame.SetChildMenuBar(None)
        
        for pos in xrange(pClientWindow.GetPageCount()):
            if pClientWindow.GetPage(pos) == self:
                return pClientWindow.DeletePage(pos)

        return False


    def SetMenuBar(self, menu_bar):

        pOldMenuBar = self._pMenuBar
        self._pMenuBar = menu_bar

        if self._pMenuBar:
        
            pParentFrame = self.GetMDIParentFrame()
            if not pParentFrame:
                raise Exception("Missing MDI Parent Frame")

            self._pMenuBar.Reparent(pParentFrame)
            if pParentFrame.GetActiveChild() == self:
            
                # replace current menu bars
                if pOldMenuBar:
                    pParentFrame.SetChildMenuBar(None)
                    
                pParentFrame.SetChildMenuBar(self)
            

    def GetMenuBar(self):

        return self._pMenuBar


    def SetTitle(self, title):

        self._title = title

        pParentFrame = self.GetMDIParentFrame()
        if not pParentFrame:
            raise Exception("Missing MDI Parent Frame")
        
        pClientWindow = pParentFrame.GetClientWindow()
        if pClientWindow is not None:
        
            for pos in xrange(pClientWindow.GetPageCount()):
                if pClientWindow.GetPage(pos) == self:
                    pClientWindow.SetPageText(pos, self._title)
                    break


    def GetTitle(self):

        return self._title


    def SetIcons(self, icons):

        # get icon with the system icon size
        self.SetIcon(icons.GetIcon(-1))
        self._icon_bundle = icons


    def GetIcons(self):

        return self._icon_bundle


    def SetIcon(self, icon):

        pParentFrame = self.GetMDIParentFrame()
        if not pParentFrame:
            raise Exception("Missing MDI Parent Frame")

        self._icon = icon

        bmp = wx.BitmapFromIcon(self._icon)

        pClientWindow = pParentFrame.GetClientWindow()
        if pClientWindow is not None:
            idx = pClientWindow.GetPageIndex(self)
            if idx != -1:
                pClientWindow.SetPageBitmap(idx, bmp)
        

    def GetIcon(self):

        return self._icon


    def Activate(self):

        pParentFrame = self.GetMDIParentFrame()
        if not pParentFrame:
            raise Exception("Missing MDI Parent Frame")
        
        pClientWindow = pParentFrame.GetClientWindow()
        if pClientWindow is not None:
        
            for pos in xrange(pClientWindow.GetPageCount()):
                if pClientWindow.GetPage(pos) == self:
                    pClientWindow.SetSelection(pos)
                    break
            

    def OnMenuHighlight(self, event):

        if self._pMDIParentFrame:
    
            # we don't have any help text for this item,
            # but may be the MDI frame does?
            self._pMDIParentFrame.OnMenuHighlight(event)


    def OnActivate(self, event):

        # do nothing
        pass


    def OnCloseWindow(self, event):

        pParentFrame = self.GetMDIParentFrame()
        if pParentFrame:
            if pParentFrame.GetActiveChild() == self:
            
                pParentFrame.SetActiveChild(None)
                pParentFrame.SetChildMenuBar(None)
            
            pClientWindow = pParentFrame.GetClientWindow()
            idx = pClientWindow.GetPageIndex(self)
            
            if idx != wx.NOT_FOUND:
                pClientWindow.RemovePage(idx)

        self.Destroy()


    def SetMDIParentFrame(self, parentFrame):

        self._pMDIParentFrame = parentFrame


    def GetMDIParentFrame(self):

        return self._pMDIParentFrame


    def CreateStatusBar(self, number=1, style=1, winid=1, name=""):
        
        return None


    def GetStatusBar(self):

        return None
    

    def SetStatusText(self, text, number=0):

        pass

    
    def SetStatusWidths(self, widths_field):

        pass
    

    # no toolbar bars
    def CreateToolBar(self, style=1, winid=-1, name=""):
        
        return None

    
    def GetToolBar(self):

        return None
    

    # no maximize etc
    def Maximize(self, maximize=True):

        pass


    def Restore(self):
    
        pass

    
    def Iconize(self, iconize=True):

        pass

    
    def IsMaximized(self):

        return True

    
    def IsIconized(self):

        return False

    
    def ShowFullScreen(self, show=True, style=0):

        return False

    
    def IsFullScreen(self):

        return False        


    def IsTopLevel(self):

        return False


    # renamed from Show().
    def ActivateOnCreate(self, activate_on_create):

        self._activate_on_create = activate_on_create
        return True

    
    def Show(self, show=True):

        wx.PyPanel.Show(self, show)


    def ApplyMDIChildFrameRect(self):

        if self._mdi_currect != self._mdi_newrect:
            self.SetDimensions(*self._mdi_newrect)
            self._mdi_currect = wx.Rect(*self._mdi_newrect)


#-----------------------------------------------------------------------------
# AuiMDIClientWindow
#-----------------------------------------------------------------------------

class AuiMDIClientWindow(auibook.AuiNotebook):

    def __init__(self, parent, agwStyle=0):

        auibook.AuiNotebook.__init__(self, parent, wx.ID_ANY, wx.Point(0, 0), wx.Size(100, 100),
                                     agwStyle=AUI_NB_DEFAULT_STYLE|wx.NO_BORDER)

        caption_icon_size = wx.Size(wx.SystemSettings.GetMetric(wx.SYS_SMALLICON_X),
                                    wx.SystemSettings.GetMetric(wx.SYS_SMALLICON_Y))
        self.SetUniformBitmapSize(caption_icon_size)

        bkcolour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_APPWORKSPACE)
        self.SetOwnBackgroundColour(bkcolour)

        self._mgr.GetArtProvider().SetColour(AUI_DOCKART_BACKGROUND_COLOUR, bkcolour)

        self.Bind(auibook.EVT_AUINOTEBOOK_PAGE_CHANGED, self.OnPageChanged)
        self.Bind(auibook.EVT_AUINOTEBOOK_PAGE_CLOSE, self.OnPageClose)
        self.Bind(wx.EVT_SIZE, self.OnSize)


    def SetSelection(self, nPage):

        return auibook.AuiNotebook.SetSelection(self, nPage)


    def PageChanged(self, old_selection, new_selection):

        # don't do anything if the page doesn't actually change
        if old_selection == new_selection:
            return

        # notify old active child that it has been deactivated
        if old_selection != -1 and old_selection < self.GetPageCount():
        
            old_child = self.GetPage(old_selection)
            if not old_child:
                raise Exception("AuiMDIClientWindow.PageChanged - null page pointer")

            event = wx.ActivateEvent(wx.wxEVT_ACTIVATE, False, old_child.GetId())
            event.SetEventObject(old_child)
            old_child.GetEventHandler().ProcessEvent(event)
        
        # notify new active child that it has been activated
        if new_selection != -1:
        
            active_child = self.GetPage(new_selection)
            if not active_child:
                raise Exception("AuiMDIClientWindow.PageChanged - null page pointer")

            event = wx.ActivateEvent(wx.wxEVT_ACTIVATE, True, active_child.GetId())
            event.SetEventObject(active_child)
            active_child.GetEventHandler().ProcessEvent(event)

            if active_child.GetMDIParentFrame():
                active_child.GetMDIParentFrame().SetActiveChild(active_child)
                active_child.GetMDIParentFrame().SetChildMenuBar(active_child)


    def OnPageClose(self, event):

        wnd = self.GetPage(event.GetSelection())
        wnd.Close()

        # regardless of the result of wnd.Close(), we've
        # already taken care of the close operations, so
        # suppress further processing
        event.Veto()


    def OnPageChanged(self, event):

        self.PageChanged(event.GetOldSelection(), event.GetSelection())


    def OnSize(self, event):

        auibook.AuiNotebook.OnSize(self, event)

        for pos in xrange(self.GetPageCount()):
            self.GetPage(pos).ApplyMDIChildFrameRect()
