###############################################################################
# Name: ed_statbar.py                                                         #
# Purpose: Custom statusbar with builtin progress indicator                   #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Custom StatusBar for Editra that contains a progress bar that responds to
messages from ed_msg to display progress of different actions.

@summary: Editra's StatusBar class

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_statbar.py 66482 2010-12-28 21:57:50Z CJP $"
__revision__ = "$Revision: 66482 $"

#--------------------------------------------------------------------------#
# Imports
import wx
import wx.stc

# Editra Libraries
import ed_glob
import util
import ed_msg
from syntax.synglob import GetDescriptionFromId
from eclib import ProgressStatusBar, EncodingDialog
from extern.decorlib import anythread

#--------------------------------------------------------------------------#
 
_ = wx.GetTranslation

#--------------------------------------------------------------------------#

class EdStatBar(ProgressStatusBar):
    """Custom status bar that handles dynamic field width adjustment and
    automatic expiration of status messages.

    """
    ID_CLEANUP_TIMER = wx.NewId()
    def __init__(self, parent):
        super(EdStatBar, self).__init__(parent, style=wx.ST_SIZEGRIP)

        # Attributes
        self._pid = parent.GetId() # Save parents id for filtering msgs
        self._widths = list()
        self._cleanup_timer = wx.Timer(self, EdStatBar.ID_CLEANUP_TIMER)
        self._eolmenu = wx.Menu()
        self._log = wx.GetApp().GetLog()

        # Setup
        self.SetFieldsCount(6) # Info, vi stuff, line/progress
        self.SetStatusWidths([-1, 90, 40, 40, 40, 155])
        self._eolmenu.Append(ed_glob.ID_EOL_MAC, u"CR",
                             _("Change line endings to %s") % u"CR",
                             kind=wx.ITEM_CHECK)
        self._eolmenu.Append(ed_glob.ID_EOL_WIN, u"CRLF",
                             _("Change line endings to %s") % u"CRLF",
                             kind=wx.ITEM_CHECK)
        self._eolmenu.Append(ed_glob.ID_EOL_UNIX, u"LF",
                             _("Change line endings to %s") % u"LF",
                             kind=wx.ITEM_CHECK)

        # Event Handlers
        self.Bind(wx.EVT_WINDOW_DESTROY, self.OnDestroy, self)
        self.Bind(wx.EVT_LEFT_DCLICK, self.OnLeftDClick)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_TIMER, self.OnExpireMessage,
                  id=EdStatBar.ID_CLEANUP_TIMER)

        # Messages
        ed_msg.Subscribe(self.OnProgress, ed_msg.EDMSG_PROGRESS_SHOW)
        ed_msg.Subscribe(self.OnProgress, ed_msg.EDMSG_PROGRESS_STATE)
        ed_msg.Subscribe(self.OnUpdateText, ed_msg.EDMSG_UI_SB_TXT)
        ed_msg.Subscribe(self.OnUpdateDoc, ed_msg.EDMSG_UI_NB_CHANGED)
        ed_msg.Subscribe(self.OnUpdateDoc, ed_msg.EDMSG_FILE_SAVED)
        ed_msg.Subscribe(self.OnUpdateDoc, ed_msg.EDMSG_FILE_OPENED)
        ed_msg.Subscribe(self.OnUpdateDoc, ed_msg.EDMSG_UI_STC_LEXER)

    def OnDestroy(self, evt):
        """Unsubscribe from messages"""
        if evt.GetId() == self.GetId():
            ed_msg.Unsubscribe(self.OnProgress)
            ed_msg.Unsubscribe(self.OnUpdateText)
            ed_msg.Unsubscribe(self.OnUpdateDoc)
        evt.Skip()

    def __SetStatusText(self, txt, field):
        """Safe method to use for setting status text with CallAfter.
        @param txt: string
        @param field: int

        """
        try:
            super(EdStatBar, self).SetStatusText(txt, field)
            self.AdjustFieldWidths()

            if field == ed_glob.SB_INFO and txt != u'':
                # Start the expiration countdown
                if self._cleanup_timer.IsRunning():
                    self._cleanup_timer.Stop()
                self._cleanup_timer.Start(10000, True)
        except wx.PyDeadObjectError, wx.PyAssertionError:
            # Getting some odd assertion errors on wxMac so just trap
            # and ignore them for now
            # glyphCount == (text.length()+1)" failed at graphics.cpp(2048)
            # in GetPartialTextExtents()
            pass
        except TypeError, err:
            self._log("[edstatbar][err] Bad status message: %s" % str(txt))
            self._log("[edstatbar][err] %s" % err)

    def AdjustFieldWidths(self):
        """Adjust each field width of status bar basing on the field text
        @return: None

        """
        widths = [-1]
        # Calculate required widths
        # NOTE: Order of fields is important
        for field in [ed_glob.SB_BUFF,
                      ed_glob.SB_LEXER,
                      ed_glob.SB_ENCODING,
                      ed_glob.SB_EOL,
                      ed_glob.SB_ROWCOL]:
            width = self.GetTextExtent(self.GetStatusText(field))[0] + 20
            if width == 20:
                width = 0
            widths.append(width)

        # Adjust widths
        if widths[-1] < 155:
            widths[-1] = 155

        # Only update if there are changes
        if widths != self._widths:
            self._widths = widths
            self.SetStatusWidths(self._widths)

    def GetMainWindow(self):
        """Method required for L{ed_msg.mwcontext}"""
        return self.GetParent()

    def OnExpireMessage(self, evt):
        """Handle Expiring the status message when the oneshot timer
        tells us it has expired.

        """
        if evt.GetId() == EdStatBar.ID_CLEANUP_TIMER:
            wx.CallAfter(self.__SetStatusText, u'', ed_glob.SB_INFO)
        else:
            evt.Skip()

    def OnLeftDClick(self, evt):
        """Handlers mouse left double click on status bar
        @param evt: Event fired that called this handler
        @type evt: 
        @note: Assumes parent is MainWindow instance

        """
        pt = evt.GetPosition()
        if self.GetFieldRect(ed_glob.SB_ROWCOL).Contains(pt):
            mw = self.GetParent()
            mpane = mw.GetEditPane()
            mpane.ShowCommandControl(ed_glob.ID_GOTO_LINE)
        else:
            evt.Skip()

    def OnLeftUp(self, evt):
        """Handle left clicks on the status bar
        @param evt: wx.MouseEvent

        """
        pt = evt.GetPosition()
        if self.GetFieldRect(ed_glob.SB_EOL).Contains(pt):
            rect = self.GetFieldRect(ed_glob.SB_EOL)
            self.PopupMenu(self._eolmenu, (rect.x, rect.y))
        elif self.GetFieldRect(ed_glob.SB_ENCODING).Contains(pt):
            nb = self.GetTopLevelParent().GetNotebook()
            buff = nb.GetCurrentCtrl()
            dlg = EncodingDialog(nb,
                                 msg=_("Change the encoding of the current document."),
                                 title=_("Change Encoding"),
                                 default=buff.GetEncoding())
            bmp = wx.ArtProvider.GetBitmap(str(ed_glob.ID_DOCPROP),
                                           wx.ART_OTHER)
            if bmp.IsOk():
                dlg.SetBitmap(bmp)
            dlg.CenterOnParent()

            # TODO: should add EdFile callbacks for modification events instead
            #       of using explicit statusbar refresh.
            if dlg.ShowModal() == wx.ID_OK:
                buff.SetEncoding(dlg.GetEncoding())
                self.UpdateFields()

            # NOTE: Got an error report about a PyDeadObject error here. The
            #       error does not make any sense since the dialog is not
            #       destroyed or deleted by anything before this. Add validity
            #       check to ensure reference is still valid.
            if dlg:
                dlg.Destroy()
        else:
            evt.Skip()

    def OnProgress(self, msg):
        """Set the progress bar's state
        @param msg: Message Object

        """
        mdata = msg.GetData()
        # Don't do anything if the message is not for this frame
        if self._pid != mdata[0]:
            return

        mtype = msg.GetType()
        if mtype == ed_msg.EDMSG_PROGRESS_STATE:
            # May be called from non gui thread so don't do anything with
            # the gui here.
            self.SetProgress(mdata[1])
            self.range = mdata[2]
            if sum(mdata[1:]) == 0:
                self.Stop()
        elif mtype == ed_msg.EDMSG_PROGRESS_SHOW:
            if mdata[1]:
                self.Start(75)
            else:
                # TODO: findout where stray stop event is coming from...
                self.Stop()

    @ed_msg.mwcontext
    def OnUpdateDoc(self, msg):
        """Update document related fields
        @param msg: Message Object

        """
        self.UpdateFields()
        if msg.GetType() == ed_msg.EDMSG_UI_NB_CHANGED:
            wx.CallAfter(self.__SetStatusText, u'', ed_glob.SB_INFO)

    @anythread
    def DoUpdateText(self, msg):
        """Thread safe update of status text. Proxy for OnUpdateText because
        pubsub seems to have issues with passing decorator methods for
        listeners.
        @param msg: Message Object

        """
        # Only process if this status bar is in the active window and shown
        parent = self.GetTopLevelParent()
        if (parent.IsActive() or wx.GetApp().GetTopWindow() == parent):
            field, txt = msg.GetData()
            self.UpdateFields()
            wx.CallAfter(self.__SetStatusText, txt, field)

    def OnUpdateText(self, msg):
        """Update the status bar text based on the received message
        @param msg: Message Object

        """
        self.DoUpdateText(msg)

    def PushStatusText(self, txt, field):
        """Set the status text
        @param txt: Text to put in bar
        @param field: int

        """
        wx.CallAfter(self.__SetStatusText, txt, field)

    def SetStatusText(self, txt, field):
        """Set the status text
        @param txt: Text to put in bar
        @param field: int

        """
        wx.CallAfter(self.__SetStatusText, txt, field)

    def UpdateFields(self):
        """Update document fields based on the currently selected
        document in the editor.
        @postcondition: encoding and lexer fields are updated
        @todo: update when readonly hooks are implemented

        """
        nb = self.GetParent().GetNotebook()
        if nb is None:
            return

        try:
            cbuff = nb.GetCurrentCtrl()
            doc = cbuff.GetDocument()
            wx.CallAfter(self.__SetStatusText, doc.GetEncoding(),
                         ed_glob.SB_ENCODING)
            wx.CallAfter(self.__SetStatusText,
                         GetDescriptionFromId(cbuff.GetLangId()),
                         ed_glob.SB_LEXER)

            eol = { wx.stc.STC_EOL_CR : u"CR",
                    wx.stc.STC_EOL_LF : u"LF",
                    wx.stc.STC_EOL_CRLF : u"CRLF" }
            wx.CallAfter(self.__SetStatusText,
                         eol[cbuff.GetEOLMode()],
                         ed_glob.SB_EOL)

        except wx.PyDeadObjectError:
            # May be called asyncronasly after the control is already dead
            return
