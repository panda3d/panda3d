#----------------------------------------------------------------------
# Name:        wx.lib.iewin
# Purpose:     A class that allows the use of the IE web browser
#              ActiveX control
#
# Author:      Robin Dunn
#
# Created:     22-March-2004
# RCS-ID:      $Id: iewin.py 54195 2008-06-13 17:25:38Z RD $
# Copyright:   (c) 2008 by Total Control Software
# Licence:     wxWindows license
#----------------------------------------------------------------------

import wx
import wx.lib.activex
import comtypes.client as cc

import sys
if not hasattr(sys, 'frozen'):
    cc.GetModule('shdocvw.dll')  # IWebBrowser2 and etc.
from comtypes.gen import SHDocVw


clsID = '{8856F961-340A-11D0-A96B-00C04FD705A2}'
progID = 'Shell.Explorer.2'


# Flags to be used with the RefreshPage method
REFRESH_NORMAL = 0
REFRESH_IFEXPIRED = 1
REFRESH_CONTINUE = 2
REFRESH_COMPLETELY = 3

# Flags to be used with LoadUrl, Navigate, Navigate2 methods
NAV_OpenInNewWindow = 0x1
NAV_NoHistory = 0x2
NAV_NoReadFromCache = 0x4
NAV_NoWriteToCache = 0x8
NAV_AllowAutosearch = 0x10
NAV_BrowserBar = 0x20
NAV_Hyperlink = 0x40
NAV_EnforceRestricted = 0x80,
NAV_NewWindowsManaged = 0x0100,
NAV_UntrustedForDownload = 0x0200,
NAV_TrustedForActiveX = 0x0400,
NAV_OpenInNewTab = 0x0800,
NAV_OpenInBackgroundTab = 0x1000,
NAV_KeepWordWheelText = 0x2000


#----------------------------------------------------------------------

class IEHtmlWindow(wx.lib.activex.ActiveXCtrl):
    def __init__(self, parent, id=-1, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=0, name='IEHtmlWindow'):
        wx.lib.activex.ActiveXCtrl.__init__(self, parent, progID,
                                            id, pos, size, style, name)

        self._canGoBack = False
        self._canGoForward = False
        

    def LoadString(self, html):
        """Load the html document from a string"""
        if self.ctrl.Document is None:
            self.LoadUrl('about:blank')
        doc = self.ctrl.Document
        doc.write(html)
        doc.close()


    def LoadStream(self, stream):
        """
        Load the html document from a Python file-like object.
        """
        if self.ctrl.Document is None:
            self.LoadUrl('about:blank')
        doc = self.ctrl.Document
        for line in stream:
            doc.write(line)
        doc.close()
        

    def LoadUrl(self, URL, Flags=0):
        """Load the document from url."""
        return self.ctrl.Navigate2(URL, Flags)


    def GetStringSelection(self, asHTML=True):
        """
        Returns the contents of the selected portion of the document as
        either html or plain text.
        """
        if self.ctrl.Document is None:
            return ""
        if not hasattr(sys, 'frozen'): cc.GetModule('mshtml.tlb')
        from comtypes.gen import MSHTML
        doc = self.ctrl.Document.QueryInterface(MSHTML.IHTMLDocument2)
        sel = doc.selection
        range = sel.createRange()
        if asHTML:
            return range.htmlText
        else:
            return range.text
        
    
    def GetText(self, asHTML=True):
        """
        Returns the contents of the the html document as either html or plain text.
        """
        if self.ctrl.Document is None:
            return ""
        if not hasattr(sys, 'frozen'): cc.GetModule('mshtml.tlb')
        from comtypes.gen import MSHTML
        doc = self.ctrl.Document.QueryInterface(MSHTML.IHTMLDocument2)
        if not asHTML:
            # if just fetching the text then get it from the body property
            return doc.body.innerText
        
        # otherwise look in the all property
        for idx in range(doc.all.length):
            # the first item with content should be the <html> tag and all its
            # children.
            item = doc.all.item(idx)
            if item is None:
                continue
            return item.outerHTML
        return ""


    def Print(self, showDialog=False):
        if showDialog:
            prompt = SHDocVw.OLECMDEXECOPT_PROMPTUSER
        else:
            prompt = SHDocVw.OLECMDEXECOPT_DONTPROMPTUSER
        self.ctrl.ExecWB(SHDocVw.OLECMDID_PRINT, prompt)


    def PrintPreview(self):
        self.ctrl.ExecWB( SHDocVw.OLECMDID_PRINTPREVIEW,
                          SHDocVw.OLECMDEXECOPT_DODEFAULT)
        

        
    def GoBack(self):
        if self.CanGoBack():
            return self.ctrl.GoBack()

    def GoForward(self):
        if self.CanGoForward():
            return self.ctrl.GoForward()

    def CanGoBack(self):
        return self._canGoBack

    def CanGoForward(self):
        return self._canGoForward

    def GoHome(self):
        return self.ctrl.GoHome()

    def GoSearch(self):
        return self.ctrl.GoSearch()

    def Navigate(self, URL, Flags=0, TargetFrameName=None, PostData=None, Headers=None):
        return self.ctrl.Navigate2( URL, Flags, TargetFrameName, PostData, Headers)

    def RefreshPage(self, Level=REFRESH_NORMAL):
        return self.ctrl.Refresh2(Level)

    def Stop(self):
        return self.ctrl.Stop()

    def Quit(self):
        return self.ctrl.Quit()


    # COM Event handlers
    def CommandStateChange(self, this, command, enable):
        # watch the command states to know when it is possible to use
        # GoBack or GoForward
        if command == SHDocVw.CSC_NAVIGATEFORWARD:
            self._canGoForward = enable
        if command == SHDocVw.CSC_NAVIGATEBACK:
            self._canGoBack = enable
            

    # Getters, Setters and properties
    def _get_Busy(self):
        return self.ctrl.Busy
    busy = property(_get_Busy, None)

    def _get_Document(self):
        return self.ctrl.Document
    document = property(_get_Document, None)

    def _get_LocationName(self):
        return self.ctrl.LocationName
    locationname = property(_get_LocationName, None)

    def _get_LocationURL(self):
        return self.ctrl.LocationURL
    locationurl = property(_get_LocationURL, None)

    def _get_ReadyState(self):
        return self.ctrl.ReadyState
    readystate = property(_get_ReadyState, None)

    def _get_Offline(self):
        return self.ctrl.Offline
    def _set_Offline(self, Offline):
        self.ctrl.Offline = Offline
    offline = property(_get_Offline, _set_Offline)

    def _get_Silent(self):
        return self.ctrl.Silent
    def _set_Silent(self, Silent):
        self.ctrl.Silent = Silent
    silent = property(_get_Silent, _set_Silent)

    def _get_RegisterAsBrowser(self):
        return self.ctrl.RegisterAsBrowser
    def _set_RegisterAsBrowser(self, RegisterAsBrowser):
        self.ctrl.RegisterAsBrowser = RegisterAsBrowser
    registerasbrowser = property(_get_RegisterAsBrowser, _set_RegisterAsBrowser)

    def _get_RegisterAsDropTarget(self):
        return self.ctrl.RegisterAsDropTarget
    def _set_RegisterAsDropTarget(self, RegisterAsDropTarget):
        self.ctrl.RegisterAsDropTarget = RegisterAsDropTarget
    registerasdroptarget = property(_get_RegisterAsDropTarget, _set_RegisterAsDropTarget)

    def _get_Type(self):
        return self.ctrl.Type
    type = property(_get_Type, None)



if __name__ == '__main__':
    app = wx.App(False)
    frm = wx.Frame(None, title="AX Test Window")
    
    ie = IEHtmlWindow(frm)
    
    frm.Show()
    import wx.lib.inspection
    wx.lib.inspection.InspectionTool().Show()
    app.MainLoop()
                                 

