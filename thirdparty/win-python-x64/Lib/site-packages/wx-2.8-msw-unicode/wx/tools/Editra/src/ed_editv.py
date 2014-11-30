###############################################################################
# Name: ed_editv.py                                                           #
# Purpose: Editor view notebook tab implementation                            #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Text editor buffer view control for the main notebook

@summary: Editor view

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_editv.py 67834 2011-06-02 02:39:41Z CJP $"
__revision__ = "$Revision: 67834 $"

#--------------------------------------------------------------------------#
# Imports
import wx
import os

# Editra Libraries
import ed_glob
import ed_menu
import ed_msg
import ed_stc
import ed_tab
from doctools import DocPositionMgr
from profiler import Profile_Get
from util import Log, SetClipboardText
import syntax.synglob as synglob
from ebmlib import GetFileModTime, ContextMenuManager, GetFileName

# External libs
from extern.stcspellcheck import STCSpellCheck

#--------------------------------------------------------------------------#

ID_SPELL_1 = wx.NewId()
ID_SPELL_2 = wx.NewId()
ID_SPELL_3 = wx.NewId()

_ = wx.GetTranslation

def modalcheck(func):
    """Decorator method to add extra modality guards to functions that
    show modal dialogs. Arg 0 must be a Window instance.

    """
    def WrapModal(*args, **kwargs):
        """Wrapper method to guard against multiple dialogs being shown"""
        self = args[0]
        self._has_dlg = True
        func(*args, **kwargs)
        self._has_dlg = False

    WrapModal.__name__ = func.__name__
    WrapModal.__doc__ = func.__doc__
    return WrapModal

#--------------------------------------------------------------------------#

class EdEditorView(ed_stc.EditraStc, ed_tab.EdTabBase):
    """Tab editor view for main notebook control."""
    ID_NO_SUGGEST = wx.NewId()
    ID_CLOSE_TAB = wx.NewId()
    ID_CLOSE_ALL_TABS = wx.NewId()
    DOCMGR = DocPositionMgr()

    def __init__(self, parent, id_=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=0, use_dt=True):
        """Initialize the editor view"""
        ed_stc.EditraStc.__init__(self, parent, id_, pos, size, style, use_dt)
        ed_tab.EdTabBase.__init__(self)

        # Attributes
        self._ro_img = False
        self._ignore_del = False
        self._has_dlg = False
        self._lprio = 0     # Idle event priority counter
        self._menu = ContextMenuManager()
        self._spell = STCSpellCheck(self, check_region=self.IsNonCode)
        self._caret_w = 1
        self._focused = True
        spref = Profile_Get('SPELLCHECK', default=dict())
        self._spell_data = dict(choices=list(),
                                word=('', -1, -1),
                                enabled=spref.get('auto', False))

        # Initialize the classes position manager for the first control
        # that is created only.
        if not EdEditorView.DOCMGR.IsInitialized():
            EdEditorView.DOCMGR.InitPositionCache(ed_glob.CONFIG['CACHE_DIR'] + \
                                                  os.sep + u'positions')

        self._spell.clearAll()
        self._spell.setDefaultLanguage(spref.get('dict', 'en_US'))
        self._spell.startIdleProcessing()

        # Context Menu Events
        self.Bind(wx.EVT_CONTEXT_MENU, self.OnContextMenu)

        # Need to relay the menu events from the context menu to the top level
        # window to be handled on gtk. Other platforms don't require this.
        self.Bind(wx.EVT_MENU, self.OnMenuEvent)

        # Hide autocomp/calltips when window looses focus
        # TODO: decide on whether this belongs in base class or not
        self.Bind(wx.EVT_KILL_FOCUS, lambda evt: self.HidePopups())
        self.Bind(wx.EVT_LEFT_UP, self.OnSetFocus)
        self.Bind(wx.EVT_WINDOW_DESTROY, self.OnDestroy, self)

        ed_msg.Subscribe(self.OnConfigMsg,
                         ed_msg.EDMSG_PROFILE_CHANGE + ('SPELLCHECK',))
        ed_msg.Subscribe(self.OnConfigMsg,
                         ed_msg.EDMSG_PROFILE_CHANGE + ('AUTOBACKUP',))
        ed_msg.Subscribe(self.OnConfigMsg,
                         ed_msg.EDMSG_PROFILE_CHANGE + ('SYNTHEME',))
        ed_msg.Subscribe(self.OnConfigMsg,
                         ed_msg.EDMSG_PROFILE_CHANGE + ('SYNTAX',))

    def OnDestroy(self, evt):
        if evt.GetId() == self.GetId():
            ed_msg.Unsubscribe(self.OnConfigMsg)
        evt.Skip()

    #---- EdTab Methods ----#

    def DoDeactivateTab(self):
        """Deactivate any active popups when the tab is no longer
        the active tab.

        """
        self._menu.Clear()
        self.HidePopups()

    def DoOnIdle(self):
        """Check if the file has been modified and prompt a warning"""
        # Don't check while the file is loading
        if self.IsLoading():
            return

        # Handle hiding and showing the caret when the window gets loses focus
        cfocus = self.FindFocus()
        if not self._focused and cfocus is self:
            # Focus has just returned to the window
            self.SetCaretWidth(self._caret_w)
            self._focused = True
        elif self._focused and cfocus is not self:
            cwidth = self.GetCaretWidth()
            if cwidth > 0:
                self._caret_w = cwidth
            self.SetCaretWidth(0) # Hide the caret when not active
            self._focused = False
            self.CallTipCancel()

        # Check for changes to on disk file
        if not self._has_dlg and Profile_Get('CHECKMOD'):
            cfile = self.GetFileName()
            lmod = GetFileModTime(cfile)
            mtime = self.GetModTime()
            if mtime and not lmod and not os.path.exists(cfile):
                # File was deleted since last check
                wx.CallAfter(self.PromptToReSave, cfile)
            elif mtime < lmod:
                # Check if we should automatically reload the file or not
                if Profile_Get('AUTO_RELOAD', default=False) and \
                   not self.GetModify():
                    wx.CallAfter(self.DoReloadFile)
                else:
                    wx.CallAfter(self.AskToReload, cfile)

        # Check for changes to permissions
        if self.File.IsReadOnly() != self._ro_img:
            self._nb.SetPageBitmap(self.GetTabIndex(), self.GetTabImage())
            self._nb.Refresh()
        else:
            pass

        # Handle Low(er) priority idle events
        self._lprio += 1
        if self._lprio == 2:
            self._lprio = 0 # Reset counter
            # Do spell checking
            # TODO: Add generic subscriber hook and move spell checking and
            #       and other low priority idle handling there
            if self.IsShown():
                if self._spell_data['enabled']:
                    self._spell.processCurrentlyVisibleBlock()
            else:
                # Ensure calltips are not shown when this is a background tab.
                self.CallTipCancel()

    @modalcheck
    def DoReloadFile(self):
        """Reload the current file"""
        ret, rmsg = self.ReloadFile()
        if not ret:
            cfile = self.GetFileName()
            errmap = dict(filename=cfile, errmsg=rmsg)
            mdlg = wx.MessageDialog(self,
                                    _("Failed to reload %(filename)s:\n"
                                      "Error: %(errmsg)s") % errmap,
                                    _("Error"),
                                    wx.OK | wx.ICON_ERROR)
            mdlg.ShowModal()
            mdlg.Destroy()

    def DoTabClosing(self):
        """Save the current position in the buffer to reset on next load"""
        if len(self.GetFileName()) > 1:
            EdEditorView.DOCMGR.AddRecord([self.GetFileName(),
                                           self.GetCurrentPos()])

    def DoTabOpen(self, ):
        """Called to open a new tab"""
        pass

    def DoTabSelected(self):
        """Performs updates that need to happen when this tab is selected"""
        Log("[ed_editv][info] Tab has file: %s" % self.GetFileName())
        self.PostPositionEvent()

    def GetName(self):
        """Gets the unique name for this tab control.
        @return: (unicode) string

        """
        return u"EditraTextCtrl"

    def GetTabImage(self):
        """Get the Bitmap to use for the tab
        @return: wx.Bitmap (16x16)

        """
        if self.GetDocument().ReadOnly:
            self._ro_img = True
            bmp = wx.ArtProvider.GetBitmap(str(ed_glob.ID_READONLY), wx.ART_MENU)
        else:
            self._ro_img = False
            lang_id = str(self.GetLangId())
            bmp = wx.ArtProvider.GetBitmap(lang_id, wx.ART_MENU)
            if bmp.IsNull():
                bmp = wx.ArtProvider.GetBitmap(str(synglob.ID_LANG_TXT), wx.ART_MENU)
        return bmp

    def GetTabMenu(self):
        """Get the tab menu
        @return: wx.Menu
        @todo: move logic from notebook to here
        @todo: generalize generic actions to base class (close, new, etc..)

        """
        ptxt = self.GetTabLabel()

        menu = ed_menu.EdMenu()
        menu.Append(ed_glob.ID_NEW, _("New Tab"))
        menu.Append(ed_glob.ID_MOVE_TAB, _("Move Tab to New Window"))
        menu.AppendSeparator()
        menu.Append(ed_glob.ID_SAVE, _("Save \"%s\"") % ptxt)
        menu.Append(EdEditorView.ID_CLOSE_TAB, _("Close \"%s\"") % ptxt)
        menu.Append(ed_glob.ID_CLOSE_OTHERS, _("Close Other Tabs"))
        menu.Append(EdEditorView.ID_CLOSE_ALL_TABS, _("Close All"))
        menu.AppendSeparator()
        menu.Append(ed_glob.ID_COPY_FILE, _("Copy Filename"))
        menu.Append(ed_glob.ID_COPY_PATH, _("Copy Full Path"))
        return menu

    def GetTitleString(self):
        """Get the title string to display in the MainWindows title bar
        @return: (unicode) string

        """
        fname = self.GetFileName()
        title = os.path.split(fname)[-1]

        # Its an unsaved buffer
        if not len(title):
            title = fname = self.GetTabLabel()

        if self.GetModify() and not title.startswith(u'*'):
            title = u"*" + title
        return u"%s - file://%s" % (title, fname)

    def CanCloseTab(self):
        """Called when checking if tab can be closed or not
        @return: bool

        """
        if self._ignore_del:
            self._ignore_del = False
            return True

        result = True
        if self.GetModify():
            result = self.ModifySave()
            result = result in (wx.ID_YES, wx.ID_OK, wx.ID_NO)
            if result:
                self._ignore_del = True

        return result

    def OnSetFocus(self, evt):
        """Make sure that the currently selected tab is this one"""
        evt.Skip()
        parent = self.GetParent()
        csel = parent.GetSelection()
        idx = self.GetTabIndex()
        if csel != idx:
            parent.SetSelection(idx)

    def OnSpelling(self, buff, evt):
        """Context menu subscriber callback
        @param buff: buffer menu event happened in
        @param evt: MenuEvent

        """
        e_id = evt.GetId()
        replace = None
        for choice in self._spell_data['choices']:
            if e_id == choice[0]:
                replace = choice[1]
                break

        if replace is not None:
            buff.SetTargetStart(self._spell_data['word'][1])
            buff.SetTargetEnd(self._spell_data['word'][2])
            buff.ReplaceTarget(replace)

    def OnTabMenu(self, evt):
        """Tab menu event handler"""
        e_id = evt.GetId()
        if e_id in (ed_glob.ID_COPY_PATH, ed_glob.ID_COPY_FILE):
            path = self.GetFileName()
            if path is not None:
                if e_id == ed_glob.ID_COPY_FILE:
                    path = GetFileName(path)
                SetClipboardText(path)
        elif e_id == ed_glob.ID_MOVE_TAB:
            frame = wx.GetApp().OpenNewWindow()
            nbook = frame.GetNotebook()
            parent = self.GetParent()
            pg_txt = parent.GetRawPageText(parent.GetSelection())
            nbook.OpenDocPointer(self.GetDocPointer(),
                                 self.GetDocument(), pg_txt)
            self._ignore_del = True
            wx.CallAfter(parent.ClosePage)
        elif e_id == ed_glob.ID_CLOSE_OTHERS:
            parent = self.GetParent()
            if hasattr(parent, 'CloseOtherPages'):
                parent.CloseOtherPages()
        elif e_id in (EdEditorView.ID_CLOSE_TAB, EdEditorView.ID_CLOSE_ALL_TABS):
            # Need to relay events up to toplevel window on GTK for them to
            # be processed. On other platforms the propagate by themselves.
            evt.SetId({ EdEditorView.ID_CLOSE_TAB : ed_glob.ID_CLOSE,
                        EdEditorView.ID_CLOSE_ALL_TABS : ed_glob.ID_CLOSEALL}.get(e_id))
            wx.PostEvent(self.GetTopLevelParent(), evt)
        else:
            evt.Skip()

    #---- End EdTab Methods ----#

    def IsNonCode(self, pos):
        """Is the passed in position in a non code region
        @param pos: buffer position
        @return: bool

        """
        return self.IsComment(pos) or self.IsString(pos)

    def OnConfigMsg(self, msg):
        """Update config based on profile changes"""
        mtype = msg.GetType()[-1]
        mdata = msg.GetData()
        if mtype == 'SPELLCHECK':
            self._spell_data['enabled'] = mdata.get('auto', False)
            self._spell.setDefaultLanguage(mdata.get('dict', 'en_US'))
            if not self._spell_data['enabled']:
                self._spell.clearAll()
        elif mtype == 'AUTOBACKUP':
            self.EnableAutoBackup(Profile_Get('AUTOBACKUP'))
        elif mtype == 'SYNTHEME':
            self.UpdateAllStyles(Profile_Get('SYNTHEME'))
        elif mtype == 'SYNTAX':
            self.SyntaxOnOff(Profile_Get('SYNTAX'))
        elif mtype == 'AUTO_COMP_EX':
            self.ConfigureAutoComp()

    def OnContextMenu(self, evt):
        """Handle right click menu events in the buffer"""
        self._menu.Clear()

        menu = ed_menu.EdMenu()
        menu.Append(ed_glob.ID_UNDO, _("Undo"))
        menu.Append(ed_glob.ID_REDO, _("Redo"))
        menu.AppendSeparator()
        menu.Append(ed_glob.ID_CUT, _("Cut"))
        menu.Append(ed_glob.ID_COPY, _("Copy"))
        menu.Append(ed_glob.ID_PASTE, _("Paste"))
        menu.AppendSeparator()
        menu.Append(ed_glob.ID_TO_UPPER, _("To Uppercase"))
        menu.Append(ed_glob.ID_TO_LOWER, _("To Lowercase"))
        menu.AppendSeparator()
        menu.Append(ed_glob.ID_SELECTALL, _("Select All"))

        # Allow clients to customize the context menu
        self._menu.SetMenu(menu)
        pos = evt.GetPosition()
        bpos = self.PositionFromPoint(self.ScreenToClient(pos))
        self._menu.SetPosition(bpos)
        self._menu.SetUserData('buffer', self)
        ed_msg.PostMessage(ed_msg.EDMSG_UI_STC_CONTEXT_MENU,
                           self._menu, self.GetId())

        # Spell checking
        # TODO: de-couple to the forthcoming buffer service interface
        menu.InsertSeparator(0)
        words = self.GetWordFromPosition(bpos)
        self._spell_data['word'] = words
        sugg = self._spell.getSuggestions(words[0])

        # Don't give suggestions if the selected word is in the suggestions list
        if words[0] in sugg:
            sugg = list()

        if not len(sugg):
            item = menu.Insert(0, EdEditorView.ID_NO_SUGGEST, _("No Suggestions"))
            item.Enable(False)
        else:
            sugg = reversed(sugg[:min(len(sugg), 3)])
            ids = (ID_SPELL_1, ID_SPELL_2, ID_SPELL_3)
            del self._spell_data['choices']
            self._spell_data['choices'] = list()
            for idx, sug in enumerate(sugg):
                id_ = ids[idx] 
                self._menu.AddHandler(id_, self.OnSpelling)
                self._spell_data['choices'].append((id_, sug))
                menu.Insert(0, id_, sug)

        self.PopupMenu(self._menu.Menu)
        evt.Skip()

    def OnMenuEvent(self, evt):
        """Handle context menu events"""
        e_id = evt.GetId()
        handler = self._menu.GetHandler(e_id)

        # Handle custom menu items
        if handler is not None:
            handler(self, evt)
        else:
            self.ControlDispatch(evt)
            if evt.GetSkipped():
                evt.Skip()

    def OnModified(self, evt):
        """Overrides EditraBaseStc.OnModified"""
        super(EdEditorView, self).OnModified(evt)

        # Handle word changes to update spell checking
        # TODO: limit via preferences and move to buffer service once
        #       implemented.
        mod = evt.GetModificationType() 
        if mod & wx.stc.STC_MOD_INSERTTEXT or mod & wx.stc.STC_MOD_DELETETEXT: 
            pos = evt.GetPosition() 
            last = pos + evt.GetLength() 
            self._spell.addDirtyRange(pos, last, evt.GetLinesAdded(),
                                      mod & wx.stc.STC_MOD_DELETETEXT) 

    @modalcheck
    def PromptToReSave(self, cfile):
        """Show a dialog prompting to resave the current file
        @param cfile: the file in question

        """
        mdlg = wx.MessageDialog(self,
                                _("%s has been deleted since its "
                                  "last save point.\n\nWould you "
                                  "like to save it again?") % cfile,
                                _("Resave File?"),
                                wx.YES_NO | wx.ICON_INFORMATION)
        mdlg.CenterOnParent()
        result = mdlg.ShowModal()
        mdlg.Destroy()
        if result == wx.ID_YES:
            result = self.SaveFile(cfile)
        else:
            self.SetModTime(0)

    @modalcheck
    def AskToReload(self, cfile):
        """Show a dialog asking if the file should be reloaded
        @param cfile: the file to prompt for a reload of

        """
        mdlg = wx.MessageDialog(self,
                                _("%s has been modified by another "
                                  "application.\n\nWould you like "
                                  "to reload it?") % cfile,
                                  _("Reload File?"),
                                  wx.YES_NO | wx.ICON_INFORMATION)
        mdlg.CenterOnParent()
        result = mdlg.ShowModal()
        mdlg.Destroy()
        if result == wx.ID_YES:
            self.DoReloadFile()
        else:
            self.SetModTime(GetFileModTime(cfile))

    def SetLexer(self, lexer):
        """Override to toggle spell check context"""
        super(EdEditorView, self).SetLexer(lexer)

        if lexer == wx.stc.STC_LEX_NULL:
            self._spell.setCheckRegion(lambda p: True)
        else:
            self._spell.setCheckRegion(self.IsNonCode)

#-----------------------------------------------------------------------------#

    def ModifySave(self):
        """Called when document has been modified prompting
        a message dialog asking if the user would like to save
        the document before closing.
        @return: Result value of whether the file was saved or not

        """
        name = self.GetFileName()
        if name == u"":
            name = self.GetTabLabel()

        dlg = wx.MessageDialog(self,
                                _("The file: \"%s\" has been modified since "
                                  "the last save point.\n\nWould you like to "
                                  "save the changes?") % name,
                               _("Save Changes?"),
                               wx.YES_NO | wx.YES_DEFAULT | wx.CANCEL | \
                               wx.ICON_INFORMATION)
        result = dlg.ShowModal()
        dlg.Destroy()

        # HACK
        if result == wx.ID_YES:
            evt = wx.MenuEvent(wx.wxEVT_COMMAND_MENU_SELECTED, ed_glob.ID_SAVE)
            tlw = self.GetTopLevelParent()
            if hasattr(tlw, 'OnSave'):
                tlw.OnSave(evt)

        return result
