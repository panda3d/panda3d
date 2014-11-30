###############################################################################
# Name: ed_main.py                                                            #
# Purpose: Editra's Main Window                                               #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
This module provides the L{MainWindow} class for Editra. The MainWindow is
main Ui component of the editor that contains all the other components.

@summary: MainWindow Component

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_main.py 67857 2011-06-05 00:16:24Z CJP $"
__revision__ = "$Revision: 67857 $"

#--------------------------------------------------------------------------#
# Dependencies
import os
import sys
import time
import wx
import wx.aui

# Editra Libraries
from ed_glob import *
import util
import profiler
import ed_toolbar
import ed_mpane
import ed_event
import ed_msg
import ed_menu
import ed_print
import ed_shelf
import ed_statbar
import ed_mdlg
import prefdlg
import syntax.syntax as syntax
import generator
import plugin
import perspective as viewmgr
import iface
import ebmlib
import eclib

# Function Aliases
_ = wx.GetTranslation
_PGET = profiler.Profile_Get
_PSET = profiler.Profile_Set

#--------------------------------------------------------------------------#

class MainWindow(wx.Frame, viewmgr.PerspectiveManager):
    """Editras Main Window"""
    # Clipboard ring is limited to 25, why? Because any more is a waste of
    # memory and an inefficient waste of your time to cycle through.
    CLIPBOARD = util.EdClipboard(25)
    PRINTER = None

    def __init__(self, parent, id_, wsize, title):
        """Initialiaze the Frame and Event Handlers.
        @param wsize: Windows initial size
        @param title: Windows Title

        """
        wx.Frame.__init__(self, parent, id_, title, size=wsize,
                          style=wx.DEFAULT_FRAME_STYLE)
        viewmgr.PerspectiveManager.__init__(self, CONFIG['CACHE_DIR'])

        # Setup app icon and title
        util.SetWindowIcon(self)

        # Attributes
        self._loaded = False
        self._initialized = False # for GTK OnActivate HACK
        self._mlock = ebmlib.CallLock()
        self._last_save = u''
        self.LOG = wx.GetApp().GetLog()
        self._exiting = False
        self._handlers = dict(menu=list(), ui=list())

        #---- Setup File History ----#
        self.filehistory = ebmlib.EFileHistory(_PGET('FHIST_LVL', 'int', 9))

        #---- Status bar on bottom of window ----#
        self.SetStatusBar(ed_statbar.EdStatBar(self))
        self.GetStatusBar().Show(_PGET('STATBAR', default=True))
        #---- End Statusbar Setup ----#

        #---- Notebook that contains the editing buffers ----#
        self._mpane = ed_mpane.MainPanel(self)
        self.nb = self._mpane.GetWindow()
        self.PanelMgr.AddPane(self._mpane, wx.aui.AuiPaneInfo(). \
                              Name("EditPane").Center().Layer(1).Dockable(False). \
                              CloseButton(False).MaximizeButton(False). \
                              CaptionVisible(False))
        self._mpane.InitCommandBar() # <- required due to nb dependencies...

        #---- Command Bar ----#
        self._mpane.HideCommandBar()

        #---- Pane Navigator ----#
        self._paneNavi = None

        # Printer Setup
        if MainWindow.PRINTER is None:
            MainWindow.PRINTER = ed_print.EdPrinter(self)

        #---- Setup Toolbar ----#
        self.SetupToolBar()
        #---- End Toolbar Setup ----#

        #---- Menus ----#
        menbar = ed_menu.EdMenuBar()

        # Todo this should not be hard coded
        menbar.GetMenuByName("view").InsertMenu(5, ID_PERSPECTIVES,
                             _("Perspectives"), self.GetPerspectiveControls())

        ## Setup additional menu items
        self.filehistory.UseMenu(menbar.GetMenuByName("filehistory"))

        # On mac, do this to make help menu appear in correct location
        # Note it must be done before setting the menu bar and after the
        # menus have been created.
        if wx.Platform == '__WXMAC__':
            wx.GetApp().SetMacHelpMenuTitleName(_("&Help"))

        #---- Menu Bar ----#
        self.SetMenuBar(menbar)

        #---- Actions to take on menu events ----#

        # Collect Menu Event handler pairs
        self._handlers['menu'].extend([# File Menu
                                       (ID_NEW, self.OnNew),
                                       (ID_OPEN, self.OnOpen),
                                       (ID_CLOSE, self.OnClosePage),
                                       (ID_CLOSEALL, self.OnClosePage),
                                       (ID_SAVE, self.OnSave),
                                       (ID_SAVEAS, self.OnSaveAs),
                                       (ID_SAVEALL, self.OnSave),
                                       (ID_REVERT_FILE, self.DispatchToControl),
                                       (ID_RELOAD_ENC, self.OnReloadWithEnc),
                                       (ID_SAVE_PROFILE, self.OnSaveProfile),
                                       (ID_LOAD_PROFILE, self.OnLoadProfile),
                                       (ID_SAVE_SESSION, self.OnSaveSession),
                                       (ID_LOAD_SESSION, self.OnLoadSession),
                                       (ID_EXIT, wx.GetApp().OnExit),
                                       (ID_PRINT, self.OnPrint),
                                       (ID_PRINT_PRE, self.OnPrint),
                                       (ID_PRINT_SU, self.OnPrint),

                                       # Edit Menu
                                       (ID_PASTE_AFTER, self.DispatchToControl),
                                       (ID_CYCLE_CLIPBOARD, self.DispatchToControl),
                                       (ID_COLUMN_MODE, self.DispatchToControl),
                                       (ID_TOGGLE_FOLD, self.DispatchToControl),
                                       (ID_TOGGLE_ALL_FOLDS, self.DispatchToControl),
                                       (ID_SHOW_AUTOCOMP, self.DispatchToControl),
                                       (ID_SHOW_CALLTIP, self.DispatchToControl),
                                       (ID_QUICK_FIND, self.OnCommandBar),
                                       (ID_PREF, OnPreferences),

                                       # View Menu
                                       (ID_GOTO_LINE, self.OnCommandBar),
                                       (ID_GOTO_MBRACE, self.DispatchToControl),
                                       (ID_SHOW_SB, self.OnShowStatusBar),
                                       (ID_VIEW_TOOL, self.OnViewTb),
                                       (ID_PANELIST, self.OnPaneList),
                                       (ID_MAXIMIZE_EDITOR, self.OnMaximizeEditor),

                                       # Format Menu
                                       (ID_FONT, self.OnFont),

                                       # Settings menu
                                       (ID_LEXER_CUSTOM, self.OnCustomizeLangMenu),

                                       # Tool Menu
                                       (ID_COMMAND, self.OnCommandBar),
                                       (ID_STYLE_EDIT, self.OnStyleEdit),
                                       (ID_PLUGMGR, self.OnPluginMgr),

                                       # Help Menu
                                       (ID_ABOUT, OnAbout),
                                       (ID_HOMEPAGE, self.OnHelp),
                                       (ID_DOCUMENTATION, self.OnHelp),
                                       (ID_TRANSLATE, self.OnHelp),
                                       (ID_CONTACT, self.OnHelp),
                                       (ID_BUG_TRACKER, self.OnHelp)])

        self._handlers['menu'].extend([(l_id, self.DispatchToControl)
                                       for l_id in syntax.SYNTAX_IDS])

        # Extra menu handlers (need to work these into above system yet)
        self.Bind(wx.EVT_MENU, self.DispatchToControl)
        self.Bind(wx.EVT_MENU, self.OnGenerate)
        self.Bind(wx.EVT_MENU_RANGE, self.OnFileHistory,
                  id=wx.ID_FILE1, id2=wx.ID_FILE9)

        # Update UI Handlers
        self._handlers['ui'].extend([# File Menu
                                     (ID_REVERT_FILE, self.OnUpdateFileUI),
                                     (ID_RELOAD_ENC, self.OnUpdateFileUI),
                                     # Edit Menu
                                     (ID_COPY, self.OnUpdateClipboardUI),
                                     (ID_CUT, self.OnUpdateClipboardUI),
                                     (ID_PASTE, self.OnUpdateClipboardUI),
                                     (ID_PASTE_AFTER, self.OnUpdateClipboardUI),
                                     (ID_CYCLE_CLIPBOARD, self.OnUpdateClipboardUI),
                                     (ID_UNDO, self.OnUpdateClipboardUI),
                                     (ID_REDO, self.OnUpdateClipboardUI),
                                     (ID_COLUMN_MODE, self.OnUpdateClipboardUI),
                                     # Format Menu
                                     (ID_INDENT, self.OnUpdateFormatUI),
                                     (ID_USE_SOFTTABS, self.OnUpdateFormatUI),
                                     (ID_TO_UPPER, self.OnUpdateFormatUI),
                                     (ID_TO_LOWER, self.OnUpdateFormatUI),
                                     (ID_WORD_WRAP, self.OnUpdateFormatUI),
                                     (ID_EOL_MAC, self.OnUpdateFormatUI),
                                     (ID_EOL_WIN, self.OnUpdateFormatUI),
                                     (ID_EOL_UNIX, self.OnUpdateFormatUI),
                                     # Settings Menu
                                     (ID_AUTOCOMP, self.OnUpdateSettingsUI),
                                     (ID_AUTOINDENT, self.OnUpdateSettingsUI),
                                     (ID_SYNTAX, self.OnUpdateSettingsUI),
                                     (ID_FOLDING, self.OnUpdateSettingsUI),
                                     (ID_BRACKETHL, self.OnUpdateSettingsUI)])

        # View Menu
        self._handlers['ui'].extend([(m_id, self.OnUpdateViewUI)
                                      for m_id in [ID_ZOOM_NORMAL, ID_ZOOM_IN,
                                                   ID_ZOOM_OUT, ID_GOTO_MBRACE,
                                                   ID_HLCARET_LINE, ID_SHOW_SB,
                                                   ID_VIEW_TOOL, ID_SHOW_WS,
                                                   ID_SHOW_EDGE, ID_SHOW_EOL,
                                                   ID_SHOW_LN, ID_INDENT_GUIDES,
                                                   ID_MAXIMIZE_EDITOR]])

        # Lexer Menu
        self._handlers['ui'].extend([(l_id, self.OnUpdateLexerUI)
                                     for l_id in syntax.SYNTAX_IDS])

        # Perspectives
        self._handlers['ui'].extend(self.GetPersectiveHandlers())

        #---- End Menu Setup ----#

        #---- Other Event Handlers ----#
        # Frame
        self.Bind(wx.EVT_ACTIVATE, self.OnActivate)
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.Bind(wx.EVT_WINDOW_DESTROY, self.OnDestroy, self)
        self.Bind(ed_event.EVT_STATUS, self.OnStatus)

        # Find Dialog
        self._handlers['menu'].extend(self.nb.GetMenuHandlers())
        self._handlers['ui'].extend(self.nb.GetUiHandlers())

        #---- End other event actions ----#

        #---- Final Setup Calls ----#
        self.LoadFileHistory(_PGET('FHIST_LVL', fmt='int'))

        # Call add on plugins
        self.LOG("[ed_main][info] Loading MainWindow Plugins")
        plgmgr = wx.GetApp().GetPluginManager()
        addons = MainWindowAddOn(plgmgr)
        addons.Init(self)
        self._handlers['menu'].extend(addons.GetEventHandlers())
        self._handlers['ui'].extend(addons.GetEventHandlers(ui_evt=True))
        shelf = ed_shelf.Shelf(plgmgr)
        self._shelf = shelf.Init(self)
        self._handlers['ui'].extend(shelf.GetUiHandlers(self._shelf))

        self.LOG("[ed_main][info] Loading Generator plugins")
        generator.Generator(plgmgr).InstallMenu(menbar.GetMenuByName("tools"))

        # Set Perspective and other UI settings
        self.SetPerspective(_PGET('DEFAULT_VIEW'))
        self.PanelMgr.Update()
        # Make sure all clients are updated to user specified display font.
        ed_msg.PostMessage(ed_msg.EDMSG_DSP_FONT,
                           _PGET('FONT3', 'font', wx.NORMAL_FONT))

        # Message Handlers
        ed_msg.Subscribe(self.OnUpdateFileHistory, ed_msg.EDMSG_ADD_FILE_HISTORY)

        # HACK: for gtk as most linux window managers manage the windows alpha
        #       and set it when its created.
        wx.CallAfter(self.InitWindowAlpha)

    __name__ = u"MainWindow"

    def OnDestroy(self, evt):
        """Disconnect Message Handlers"""
        if evt.GetId() == self.GetId():
            ed_msg.Unsubscribe(self.OnUpdateFileHistory)
        evt.Skip()

    #---- End Private Member Functions/Variables ----#

    #---- Begin Public Member Function ----#
    def OnActivate(self, evt):
        """Activation Event Handler
        @param evt: event that called this handler
        @type evt: wx.ActivateEvent

        """
        if self._mlock.IsLocked():
            # Deactivated for popup, leave handlers hooked up
            wx.UpdateUIEvent.SetMode(wx.UPDATE_UI_PROCESS_ALL)
            evt.Skip()
            return

        app = wx.GetApp()
        active = evt.GetActive()

        # Add or remove handlers from the event stack
        if active and not self._loaded:
            self._loaded = True
            app.SetTopWindow(self)

            # Slow the update interval to reduce overhead
            wx.UpdateUIEvent.SetUpdateInterval(215)
            wx.UpdateUIEvent.SetMode(wx.UPDATE_UI_PROCESS_SPECIFIED)
            self.SetExtraStyle(wx.WS_EX_PROCESS_UI_UPDATES)

            for handler in self._handlers['menu']:
                app.AddHandlerForID(*handler)

            for handler in self._handlers['ui']:
                app.AddUIHandlerForID(*handler)

            # HACK find better way to do this later. It seems that on gtk the
            #      window doesn't get activated until later than it does on the
            #      other platforms. So for panels that depend on updating their
            #      initial state we need to send out a fake update message here.
            if wx.Platform == '__WXGTK__' and not self._initialized:
                self._initialized = True
                nb = self.GetNotebook()
                ed_msg.PostMessage(ed_msg.EDMSG_UI_NB_CHANGED,
                                   (nb, nb.GetSelection()))
        elif not active:
            self._loaded = False
            self.DeActivate()

        evt.Skip()

    def OnUpdateFileHistory(self, msg):
        """Update filehistory menu for new files that were opened
        @param msg: Message object (data == filename)

        """
        # May get notified after/during delete
        if self:
            try:
                self.filehistory.AddFileToHistory(msg.GetData())
            except wx.PyAssertionError:
                # ignore errors that wxMac sometimes raises about unicode data
                pass

    def AddFileToHistory(self, fname):
        """Add a file to the windows file history as well as any
        other open windows history.
        @param fname: name of file to add
        @todo: change the file history to a centrally managed object that
               all windows pull from to avoid this quick solution.

        """
        if _PGET('FHIST_LVL', 'int', 9) > 0:
            ed_msg.PostMessage(ed_msg.EDMSG_ADD_FILE_HISTORY, fname)

    def AddMenuHandler(self, menu_id, handler):
        """Add a menu event handler to the handler stack
        @param menu_id: Menu item id
        @param handler: Handler callable
        @postcondition: handler is added only if its not already in the set

        """
        for item in self._handlers['menu']:
            if item[0] == menu_id:
                return
        else:
            self._handlers['menu'].append((menu_id, handler))

    def AddUIHandler(self, menu_id, handler):
        """Add a UpdateUI event handler to the handler stack
        @param menu_id: Menu item id
        @param handler: Handler callable
        @postcondition: handler is added only if its not already in the set

        """
        for item in self._handlers['ui']:
            if item[0] == menu_id:
                return
        else:
            self._handlers['ui'].append((menu_id, handler))

    def DoOpen(self, evt, fname=u'', lnum=-1):
        """ Do the work of opening a file and placing it
        in a new notebook page.
        @keyword fname: can be optionally specified to open
                        a file without opening a FileDialog
        @type fname: string
        @keyword lnum: Explicitly set the line number to open the file to
        @type lnum: int

        """
        try:
            e_id = evt.GetId()
        except AttributeError:
            e_id = evt

        if e_id == ID_OPEN:
            fdir = self.GetNotebook().GetCurrentCtrl().GetFileName()
            if len(fdir):
                fdir = os.path.dirname(fdir)
            elif not hasattr(sys, 'frozen'):
                fdir = os.curdir

            dlg = wx.FileDialog(self, _("Editra: Open"), fdir, "",
                                ''.join(syntax.GenFileFilters()),
                                wx.OPEN | wx.MULTIPLE | wx.CHANGE_DIR)
            dlg.SetFilterIndex(_PGET('FFILTER', 'int', 0))

            if ebmlib.LockCall(self._mlock, dlg.ShowModal) == wx.ID_OK:
                _PSET('FFILTER', dlg.GetFilterIndex())
                for path in dlg.GetPaths():
                    if _PGET('OPEN_NW', default=False):
                        wx.GetApp().OpenNewWindow(path)
                    else:
                        self.nb.OpenPage(ebmlib.GetPathName(path),
                                         ebmlib.GetFileName(path))
                        self.nb.GoCurrentPage()

            dlg.Destroy()
        else:
            self.LOG("[ed_main][info] CMD Open File: %s" % fname)
            self.nb.OpenPage(ebmlib.GetPathName(fname),
                             ebmlib.GetFileName(fname), quiet=True)
            self.nb.GoCurrentPage()

            # lnum arg is only used with command line open
            if lnum >= 0:
                buff = self.nb.GetCurrentCtrl()
                buff.GotoLine(lnum)

            self.Raise()

    def DeActivate(self):
        """Helper method for the App to tell this window to remove
        all its event handlers.

        """
        self.SetExtraStyle(0)

        # HACK set update ui events back to process all here in case
        # opened dialog needs them. Not sure why this is necessary but it
        # is the only solution I could find to fix the external find
        # dialogs so that their buttons become enabled when typing in the
        # text control.
        #
        # If the windows that took the active position is another mainwindow
        # it will set the events back to UPDATE_UI_PROCESS_SPECIFIED to
        # prevent all the toolbars/menu items of each window from updating
        # when they dont need to.
        wx.UpdateUIEvent.SetMode(wx.UPDATE_UI_PROCESS_ALL)
        self.FlushEventStack()

    def FlushEventStack(self):
        """Clear the Menu and UpdateUI event handler stack
        @note: only unregisters this frames handlers from the app

        """
        app = wx.GetApp()
        for handler in self._handlers['menu']:
            app.RemoveHandlerForID(handler[0])

        for handler in self._handlers['ui']:
            app.RemoveUIHandlerForID(handler[0])

    def GetCommandbar(self):
        """Get this windows command bar
        @return: ed_cmdbar.CommandBarBase

        """
        return self._mpane.GetControlBar(wx.BOTTOM)

    def GetEditPane(self):
        """Get the editor notebook/command bar control
        @return: ed_mpane.MainPane

        """
        return self._mpane

    def GetNotebook(self):
        """Get the windows main notebook that contains the editing buffers
        @return: reference to L{extern.flatnotebook.FlatNotebook} instance

        """
        return getattr(self, 'nb', None)

    def GetShelf(self):
        """Get this windows Shelf
        @return: reference to L{iface.Shelf} instance
        @note: returns the plugin instance not the actual notebook, if
               a reference to the notebook is needed for parenting call
               GetWindow on the object returned by this function.

        """
        return self._shelf

    def IsExiting(self):
        """Returns whether the windows is in the process of exiting
        or not.
        @return: boolean stating if the window is exiting or not

        """
        return self._exiting

    def IsTopWindow(self):
        """Is this main window 'the' current top window
        @return: bool

        """
        return wx.GetApp().GetTopWindow() == self

    def LoadFileHistory(self, size):
        """Loads file history from profile
        @return: None

        """
        try:
            hist_list = _PGET('FHIST', default=list())
            if len(hist_list) > size:
                hist_list = hist_list[:size]
            self.filehistory.History = hist_list
        except UnicodeEncodeError, msg:
            self.LOG("[ed_main][err] Filehistory load failed: %s" % msg)

    def OnNew(self, evt):
        """Start a New File in a new tab
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        if evt.GetId() == ID_NEW:
            self.nb.NewPage()
            self.nb.GoCurrentPage()
        else:
            evt.Skip()

    def OnOpen(self, evt):
        """Open a File
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        if evt.GetId() == ID_OPEN:
            self.DoOpen(evt)
        else:
            evt.Skip()

    def OnFileHistory(self, evt):
        """Open a File from the File History
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        fnum = evt.GetId() - wx.ID_FILE1
        fname = self.filehistory.GetHistoryFile(fnum)

        # Check if file still exists
        if not os.path.exists(fname):
            mdlg = wx.MessageDialog(self, _("%s could not be found.\nPerhaps "
                                            "it's been moved or deleted.") % \
                                    fname, _("File Not Found"),
                                    wx.OK | wx.ICON_WARNING)
            mdlg.CenterOnParent()
            ebmlib.LockCall(self._mlock, mdlg.ShowModal)
            mdlg.Destroy()
            # Remove offending file from history
            self.filehistory.RemoveFileFromHistory(fnum)
        else:
            self.DoOpen(evt, fname)

    def OnClosePage(self, evt):
        """Close a page
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        if not self.IsActive():
            evt.Skip()
            return

        e_id = evt.GetId()
        if e_id == ID_CLOSE:
            self.nb.ClosePage()
        elif e_id == ID_CLOSEALL:
            self.nb.CloseAllPages()
        else:
            evt.Skip()

    def SaveFile(self, tablbl, buf):
        """Save the given page in the notebook
        @param tablbl: main notebook tab label
        @param buf: EdEditView instance
        @note: intended for internal use! method signature may change

        """
        fname = ebmlib.GetFileName(buf.GetFileName())
        if fname != u'':
            fpath = buf.GetFileName()
            result = buf.SaveFile(fpath)
            self._last_save = fpath
            if result:
                self.PushStatusText(_("Saved File: %s") % fname, SB_INFO)
            else:
                err = buf.GetDocument().GetLastError()
                self.PushStatusText(_("ERROR: %s") % err, SB_INFO)
                ed_mdlg.SaveErrorDlg(self, fname, err)
                buf.GetDocument().ResetAll()
        else:
            self.OnSaveAs(ID_SAVEAS, tablbl, buf)

    def SaveCurrentBuffer(self):
        """Save the file in the currently selected editor buffer"""
        page = self.nb.GetSelection()
        self.SaveFile(self.nb.GetPageText(page), self.nb.GetCurrentCtrl())

    def SaveAllBuffers(self):
        """Save all open editor buffers"""
        for page in xrange(self.nb.GetPageCount()):
            buff = self.nb.GetPage(page)
            if isinstance(buff, wx.stc.StyledTextCtrl):
                if buff.GetModify():
                    self.SaveFile(self.nb.GetPageText(page), buff)

    def OnSave(self, evt):
        """Save Current or All Buffers
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        e_id = evt.GetId()
        if e_id == ID_SAVE:
            self.SaveCurrentBuffer()
        elif e_id == ID_SAVEALL:
            self.SaveAllBuffers()
        else:
            evt.Skip()
            return

    def OnSaveAs(self, evt, title=u'', page=None):
        """Save File Using a new/different name
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        if page:
            ctrl = page
        else:
            ctrl = self.nb.GetCurrentCtrl()

        if title == u'':
            title = os.path.split(ctrl.GetFileName())[1]

        sdir = ctrl.GetFileName()
        if sdir is None or not len(sdir):
            sdir = self._last_save

        dlg = wx.FileDialog(self, _("Choose a Save Location"),
                            os.path.dirname(sdir),
                            title.lstrip(u"*"),
                            u''.join(syntax.GenFileFilters()),
                            wx.SAVE | wx.OVERWRITE_PROMPT)

        if ebmlib.LockCall(self._mlock, dlg.ShowModal) == wx.ID_OK:
            path = dlg.GetPath()
            dlg.Destroy()

            result = ctrl.SaveFile(path)
            fname = ebmlib.GetFileName(ctrl.GetFileName())
            if not result:
                err = ctrl.GetDocument().GetLastError()
                ed_mdlg.SaveErrorDlg(self, fname, err)
                ctrl.GetDocument().ResetAll()
                self.PushStatusText(_("ERROR: Failed to save %s") % fname, SB_INFO)
            else:
                self._last_save = path
                self.PushStatusText(_("Saved File As: %s") % fname, SB_INFO)
                self.SetTitle("%s - file://%s" % (fname, ctrl.GetFileName()))
                self.nb.SetPageText(self.nb.GetSelection(), fname)
                self.nb.GetCurrentCtrl().FindLexer()
                self.nb.UpdatePageImage()
                self.AddFileToHistory(ctrl.GetFileName())
        else:
            dlg.Destroy()

    def OnSaveProfile(self, evt):
        """Saves current settings as a profile
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        if evt.GetId() == ID_SAVE_PROFILE:
            dlg = wx.FileDialog(self, _("Where to Save Profile?"), \
                               CONFIG['PROFILE_DIR'], "default.ppb", \
                               _("Profile") + " (*.ppb)|*.ppb",
                                wx.SAVE | wx.OVERWRITE_PROMPT)

            if ebmlib.LockCall(self._mlock, dlg.ShowModal) == wx.ID_OK:
                profiler.TheProfile.Write(dlg.GetPath())
                self.PushStatusText(_("Profile Saved as: %s") % \
                                    dlg.GetFilename(), SB_INFO)
            dlg.Destroy()
        else:
            evt.Skip()

    def OnLoadProfile(self, evt):
        """Loads a profile and refreshes the editors state to match
        the settings found in the profile file.
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        if evt.GetId() == ID_LOAD_PROFILE:
            dlg = wx.FileDialog(self, _("Load a Custom Profile"),
                                CONFIG['PROFILE_DIR'], "default.ppb",
                                _("Profile") + " (*.ppb)|*.ppb", wx.OPEN)

            if ebmlib.LockCall(self._mlock, dlg.ShowModal) == wx.ID_OK:
                profiler.TheProfile.Load(dlg.GetPath())
                self.PushStatusText(_("Loaded Profile: %s") % \
                                    dlg.GetFilename(), SB_INFO)
            dlg.Destroy()

            # Update editor to reflect loaded profile
            for win in wx.GetApp().GetMainWindows():
                win.nb.UpdateTextControls()
        else:
            evt.Skip()

    def OnSaveSession(self, evt):
        """Save the current session of open files.
        @todo: Save all windows and what the active tabs are as well

        """
        if evt.GetId() == ID_SAVE_SESSION:
            # TODO: set current profile as default
            dlg = wx.FileDialog(self, _("Where to Save Session?"), \
                               CONFIG['SESSION_DIR'], u"", \
                               _("Session") + " (*.session)|*.session",
                                wx.SAVE | wx.OVERWRITE_PROMPT)

            if ebmlib.LockCall(self._mlock, dlg.ShowModal) == wx.ID_OK:
                fname = dlg.GetPath()
                if fname is None or not len(fname):
                    return

                if not fname.endswith('.session'):
                    fname = fname.rstrip(u'.') + u'.session'
                rval = self.nb.SaveSessionFile(fname)
                if rval is not None:
                    wx.MessageBox(rval[1], rval[0], wx.OK|wx.ICON_ERROR)
                    return

                self.PushStatusText(_("Session Saved as: %s") % fname, SB_INFO)
                _PSET('LAST_SESSION', fname)
            dlg.Destroy()
        else:
            evt.Skip()

    def OnLoadSession(self, evt):
        """Load a saved session."""
        if evt.GetId() == ID_LOAD_SESSION:
            # TODO: set current file as default
            dlg = wx.FileDialog(self, _("Load a Session file"),
                                CONFIG['SESSION_DIR'], u"",
                                _("Session") + " (*.session)|*.session", wx.OPEN)

            if ebmlib.LockCall(self._mlock, dlg.ShowModal) == wx.ID_OK:
                fname = dlg.GetPath()
                nbook = self.GetNotebook()
                rval = nbook.LoadSessionFile(fname)

                # Check for an error during load
                if rval is not None:
                    wx.MessageBox(rval[1], rval[0], wx.OK|wx.ICON_WARNING)
                    return
                
                self.PushStatusText(_("Loaded Session: %s") % fname, SB_INFO)
                _PSET('LAST_SESSION', fname)

            dlg.Destroy()
        else:
            evt.Skip()

    def OnStatus(self, evt):
        """Update status text with messages from other controls
        @param evt: event that called this handler

        """
        self.SetStatusText(evt.GetMessage(), evt.GetSection())

    def OnPrint(self, evt):
        """Handles sending the current document to the printer,
        showing print previews, and opening the printer settings
        dialog.
        @todo: is any manual cleanup required for the printer objects?
        @param evt: wxMenuEvent

        """
        e_id = evt.GetId()
        printer = MainWindow.PRINTER
        printer.SetStc(self.nb.GetCurrentCtrl())
        printer.SetColourMode(_PGET('PRINT_MODE'))
        if e_id == ID_PRINT:
            printer.Print()
        elif e_id == ID_PRINT_PRE:
            printer.Preview()
        elif e_id == ID_PRINT_SU:
            printer.PageSetup()
        else:
            evt.Skip()

    def Close(self, force=False):
        """Close the window
        @param force: force the closer by vetoing the event handler

        """
        if force:
            return wx.Frame.Close(self, True)
        else:
            result = self.OnClose()
            return not result

    def OnClose(self, evt=None):
        """Close this frame and unregister it from the applications
        mainloop.
        @note: Closing the frame will write out all session data to the
               users configuration directory.
        @keyword evt: Event fired that called this handler
        @type evt: wxMenuEvent
        @return: None on destroy, or True on cancel

        """
        # Save session files
        self.nb.SaveCurrentSession()

        # Cleanup Controls
        self._exiting = True
        controls = self.nb.GetPageCount()
        self.LOG("[ed_main][evt] OnClose: Number of controls: %d" % controls)
        self.Freeze()
        while controls:
            if controls <= 0:
                self.Close(True) # Force exit since there is a problem

            self.LOG("[ed_main][evt] OnClose: Requesting Page Close")
            if not self.nb.ClosePage():
                self.Thaw()
                self._exiting = False
                ed_msg.PostMessage(ed_msg.EDMSG_UI_NB_CHANGED,
                                   (self.nb, self.nb.GetSelection()))
                return True
            controls -= 1
        self.Thaw()

        ### If we get to here there is no turning back so cleanup
        ### additional items and save user settings

        # Write out saved document information
        self.nb.DocMgr.WriteBook()
        syntax.SyntaxMgr().SaveState()

        # Save Shelf contents
        _PSET('SHELF_ITEMS', self._shelf.GetItemStack())
        _PSET('SHELF_LAYOUT', self._shelf.GetPerspective())
        _PSET('SHELF_SELECTION', self._shelf.GetSelection())

        # Save Window Size/Position for next launch
        self.UpdateAutoPerspective()

        # XXX On wxMac the window size doesnt seem to take the toolbar
        #     into account so destroy it so that the window size is accurate.
        if wx.Platform == '__WXMAC__' and self.GetToolBar():
            self.GetToolBar().Destroy()

        # Raise the window from being iconized so that the size and position is
        # correct for the next launch (msw).
        if self.IsIconized():
            self.Iconize(False)

        _PSET('WSIZE', self.GetSizeTuple())
        _PSET('MAXIMIZED', self.IsMaximized())
        _PSET('WPOS', self.GetPositionTuple())

        self.LOG("[ed_main][evt] OnClose: Closing editor at pos=%s size=%s" % \
                 (_PGET('WPOS', 'str'), _PGET('WSIZE', 'str')))

        # Cleanup file history
        # TODO: Find out why filehistory can be undefined by this point
        #       sometimes.
        try:
            _PSET('FHIST', self.filehistory.History)
        except AttributeError:
            self.LOG("[ed_main][err] OnClose: Trapped AttributeError OnExit")

        # Update profile
        ppath = _PGET('MYPROFILE')
        profiler.TheProfile.Write(ppath)
        self.LOG("[ed_main][info] Saving profile to %s" % ppath)

        # Post exit notice to all aui panes
        panes = self.PanelMgr.GetAllPanes()
        exit_evt = ed_event.MainWindowExitEvent(ed_event.edEVT_MAINWINDOW_EXIT,
                                                wx.ID_ANY)
        for pane in panes:
            wx.PostEvent(pane.window, exit_evt)

        # Finally close the window
        self.LOG("[ed_main][evt] OnClose: Closing Main Frame")
        wx.GetApp().UnRegisterWindow(repr(self))

        # Ensure that event handlers have been un registered from the app
        wx.UpdateUIEvent.SetMode(wx.UPDATE_UI_PROCESS_ALL)

        # Cleanup
        self.Destroy()

    #---- End File Menu Functions ----#

    #---- View Menu Functions ----#
    def OnShowStatusBar(self, evt):
        """Toggles visibility of status bar
        @param evt: wxMenuEvent

        """
        if evt.GetId() == ID_SHOW_SB:
            show = not self.GetStatusBar().IsShown()
            _PSET('STATBAR', show)
            self.GetStatusBar().Show(show)
            self.SendSizeEvent()
        else:
            evt.Skip()

    def OnViewTb(self, evt):
        """Toggles visibility of toolbar
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        if evt.GetId() == ID_VIEW_TOOL:
            size = self.GetSize()
            toolbar = self.GetToolBar()
            if _PGET('TOOLBAR', 'bool', False) or toolbar.IsShown():
                _PSET('TOOLBAR', False)
                toolbar.Hide()
                if wx.Platform != '__WXMAC__':
                    self.SetSize((size[0], size[1] - toolbar.GetSize()[1]))
            else:
                _PSET('TOOLBAR', True)
                toolbar.Show()
                if wx.Platform != '__WXMAC__':
                    self.SetSize((size[0], size[1] + toolbar.GetSize()[1]))

            self.SendSizeEvent()
            self.Refresh()
            self.Update()
        else:
            evt.Skip()

    def OnMaximizeEditor(self, evt):
        """Maximize the editor and hide the other panes. If the editor
        is already maximized, it is un-maximized and the other panes are restored  
        @param evt: CommandEvent instance

        """
        paneInfo = self.PanelMgr.GetPane("EditPane")
        if paneInfo.IsMaximized():
            self.PanelMgr.RestorePane(paneInfo)
            ed_msg.PostMessage(ed_msg.EDMSG_UI_STC_RESTORE, context=self.GetId())
        else:
            self.PanelMgr.RestoreMaximizedPane()
            self.PanelMgr.MaximizePane(paneInfo)
        self.PanelMgr.Update()
        
    #---- End View Menu Functions ----#

    #---- Format Menu Functions ----#
    def OnFont(self, evt):
        """Open Font Settings Dialog for changing fonts on a per document
        basis.
        @status: This currently does not allow for font settings to stick
                 from one session to the next.
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        if evt.GetId() == ID_FONT:
            ctrl = self.nb.GetCurrentCtrl()
            fdata = wx.FontData()
            fdata.SetInitialFont(ctrl.GetDefaultFont())
            dlg = wx.FontDialog(self, fdata)
            result = ebmlib.LockCall(self._mlock, dlg.ShowModal)
            data = dlg.GetFontData()
            dlg.Destroy()
            if result == wx.ID_OK:
                font = data.GetChosenFont()
                ctrl.SetGlobalFont(self.nb.control.FONT_PRIMARY, \
                                   font.GetFaceName(), font.GetPointSize())
                ctrl.SetGlobalFont(self.nb.control.FONT_SECONDARY, \
                                   font.GetFaceName(), font.GetPointSize())
                ctrl.UpdateAllStyles()
        else:
            evt.Skip()

    #---- End Format Menu Functions ----#

    #---- Tools Menu Functions ----#
    def OnStyleEdit(self, evt):
        """Opens the style editor
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        if evt.GetId() == ID_STYLE_EDIT:
            import style_editor
            dlg = style_editor.StyleEditor(self)
            dlg.CenterOnParent()
            ebmlib.LockCall(self._mlock, dlg.ShowModal)
            dlg.Destroy()
        else:
            evt.Skip()

    def OnPluginMgr(self, evt):
        """Opens and shows Plugin Manager window
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        if evt.GetId() == ID_PLUGMGR:
            import plugdlg
            win = wx.GetApp().GetWindowInstance(plugdlg.PluginDialog)
            if win is not None:
                win.Raise()
                return
            dlg = plugdlg.PluginDialog(self, wx.ID_ANY, PROG_NAME + " " \
                                       + _("Plugin Manager"), \
                                       size=wx.Size(550, 450))
            dlg.CenterOnParent()
            dlg.Show()
        else:
            evt.Skip()

    def OnGenerate(self, evt):
        """Generates a given document type
        @requires: PluginMgr must be initialized and have active
                   plugins that implement the Generator Interface
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        e_id = evt.GetId()
        gen = generator.Generator(wx.GetApp().GetPluginManager())
        doc = gen.GenerateText(e_id, self.nb.GetCurrentCtrl())
        if doc:
            self.nb.NewPage()
            ctrl = self.nb.GetCurrentCtrl()
            ctrl.SetText(doc[1])
            ctrl.FindLexer(doc[0])
        else:
            evt.Skip()

    #---- Misc Function Definitions ----#
    def DispatchToControl(self, evt):
        """Catches events that need to be passed to the current
        text control for processing.
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        if not self.IsActive():
            evt.Skip()
            return

        e_id = evt.GetId()
        ctrl = self.nb.GetCurrentCtrl()
        active_only = [ ID_ZOOM_IN, ID_ZOOM_OUT, ID_ZOOM_NORMAL,
                        ID_JOIN_LINES, ID_CUT_LINE, ID_COPY_LINE, ID_INDENT,
                        ID_UNINDENT, ID_TRANSPOSE, ID_TOGGLECOMMENT,
                        ID_LINE_MOVE_UP, ID_LINE_MOVE_DOWN,
                        ID_SELECTALL, ID_UNDO, ID_REDO, ID_CUT, ID_COPY,
                        ID_PASTE, ID_LINE_BEFORE, ID_LINE_AFTER, ID_DUP_LINE,
                        ID_PASTE_AFTER, ID_COLUMN_MODE, ID_TOGGLE_FOLD,
                        ID_CYCLE_CLIPBOARD,
                        ID_TOGGLE_ALL_FOLDS, ID_DELETE_LINE,
                        ID_SHOW_AUTOCOMP, ID_SHOW_CALLTIP ]

        # Special handling for common clipboard related actions
        has_focus = self.FindFocus()
        is_stc = isinstance(has_focus, wx.stc.StyledTextCtrl)
        if has_focus is not None:
            if e_id == ID_PASTE and hasattr(has_focus, 'Paste'):
                has_focus.Paste()
                return
            elif e_id == ID_CYCLE_CLIPBOARD:
                start, end = has_focus.GetSelection()
                start, end = min(start, end), max(start, end)

                if is_stc:
                    txt = has_focus.GetTextRange(start, end)
                elif hasattr(has_focus, 'GetRange'):
                    txt = has_focus.GetRange(start, end)
                else:
                    self.LOG("[ed_main][warn] no range meth in cycle clipboard")
                    return

                if not MainWindow.CLIPBOARD.IsAtIndex(txt):
                    MainWindow.CLIPBOARD.Reset()

                next = MainWindow.CLIPBOARD.GetNext()
                if is_stc:
                    has_focus.ReplaceSelection(next)
                elif hasattr(has_focus, 'Replace'):
                    has_focus.Replace(start, end, next)
                else:
                    return

                has_focus.SetSelection(start, start+len(next))
                return
            elif e_id == ID_CUT and hasattr(has_focus, 'Cut'):
                start, end = has_focus.GetSelection()
                if is_stc:
                    txt = has_focus.GetTextRange(start, end)
                elif hasattr(has_focus, 'GetRange'):
                    txt = has_focus.GetRange(start, end)
                MainWindow.CLIPBOARD.Put(txt)
                has_focus.Cut()
                return
            elif e_id == ID_COPY and hasattr(has_focus, 'Copy'):
                start, end = has_focus.GetSelection()
                if is_stc:
                    txt = has_focus.GetTextRange(start, end)
                elif hasattr(has_focus, 'GetRange'):
                    txt = has_focus.GetRange(start, end)
                MainWindow.CLIPBOARD.Put(txt)
                has_focus.Copy()
                return
            elif e_id == ID_REDO and hasattr(has_focus, 'Redo'):
                has_focus.Redo()
                return
            elif e_id == ID_UNDO and hasattr(has_focus, 'Undo'):
                has_focus.Undo()
                return

        menu_ids = list(syntax.SYNTAX_IDS)
        menu_ids.extend([ID_SHOW_EOL, ID_SHOW_WS, ID_INDENT_GUIDES, ID_SYNTAX,
                         ID_WORD_WRAP, ID_BRACKETHL, ID_EOL_MAC, ID_EOL_UNIX,
                         ID_EOL_WIN, ID_NEXT_MARK, ID_PRE_MARK, ID_ADD_BM,
                         ID_DEL_ALL_BM, ID_FOLDING, ID_AUTOCOMP, ID_SHOW_LN,
                         ID_AUTOINDENT, ID_TAB_TO_SPACE, ID_SPACE_TO_TAB,
                         ID_TRIM_WS, ID_SHOW_EDGE, ID_MACRO_START,
                         ID_MACRO_STOP, ID_MACRO_PLAY, ID_TO_LOWER,
                         ID_TO_UPPER, ID_USE_SOFTTABS,
                         ID_GOTO_MBRACE, ID_HLCARET_LINE, ID_REVERT_FILE,
                         ])
        menu_ids.extend(active_only)

        if e_id in menu_ids:
            ctrl.ControlDispatch(evt)
        else:
            evt.Skip()
        return

    def OnReloadWithEnc(self, evt):
        """Reload the current file with a specified encoding
        @param evt: wx.MenuEvent

        """
        if evt.GetId() == ID_RELOAD_ENC:
            ctrl = self.nb.GetCurrentCtrl()
            doc = ctrl.GetDocument()
            cenc = doc.GetEncoding()
            dlg = eclib.EncodingDialog(self.GetNotebook(),
                                        msg=_("Select an encoding to reload the file with"),
                                        title=_("Reload with Encoding"),
                                        default=cenc)
            bmp = wx.ArtProvider.GetBitmap(str(ID_DOCPROP), wx.ART_OTHER)
            if bmp.IsOk():
                dlg.SetBitmap(bmp)
            dlg.CenterOnParent()

            if ebmlib.LockCall(self._mlock, dlg.ShowModal) == wx.ID_OK:
                nenc = dlg.GetEncoding()
                doc.SetEncoding(nenc)
                success = ctrl.ReloadFile()[0]
                if not success:
                    msg = _("Failed to reload the file with: %(encoding)s") % dict(encoding=nenc)
                    wx.MessageBox(msg, style=wx.OK|wx.ICON_ERROR)
                    # Revert to previous encoding
                    doc.SetEncoding(cenc)
                    ctrl.ReloadFile()
            dlg.Destroy()
        else:
            evt.Skip()

    def OnPaneList(self, evt):
        """Navigates through panes
        @param evt: CommandEvent instance

        """
        if evt.GetId() == ID_PANELIST:
            if self._paneNavi is not None:
                return

            bmp = wx.ArtProvider.GetBitmap(str(ID_NEW_WINDOW), wx.ART_MENU)
            self._paneNavi = eclib.AuiPaneNavigator(self, self.PanelMgr, bmp,
                                                      _("Aui Pane Navigator"))
            self._paneNavi.SetReturnCode(wx.ID_OK)
            ebmlib.LockCall(self._mlock, self._paneNavi.ShowModal)

            sel = self._paneNavi.GetSelection()
            self._paneNavi.Destroy()
            self._paneNavi = None

            if isinstance(sel, basestring):
                paneInfo = self.PanelMgr.GetPane(sel)
                if paneInfo.IsOk():
                    if not paneInfo.IsShown():
                        paneInfo.Show()
                        self.PanelMgr.Update()
                        # Notify activation if the window supports it
                        if hasattr(paneInfo.window, "OnShowAUIPane"):
                            paneInfo.window.OnShowAUIPane()
                    paneInfo.window.SetFocus()
        else:
            evt.Skip()

    # Menu Update Handlers
    def OnUpdateFileUI(self, evt):
        """Update filemenu items
        @param evt: EVT_UPDATE_UI

        """
        if not self.IsActive():
            return

        e_id = evt.GetId()
        ctrl = self.nb.GetCurrentCtrl()
        if e_id == ID_REVERT_FILE:
            evt.Enable(ctrl.GetModify())
        elif e_id == ID_RELOAD_ENC:
            evt.Enable(len(ctrl.GetFileName()))
        else:
            evt.Skip()

    def OnUpdateClipboardUI(self, evt):
        """Update clipboard related menu/toolbar items
        @param evt: EVT_UPDATE_UI

        """
        if not self.IsActive():
            return

        e_id = evt.GetId()
        focus = self.FindFocus()
        enable = False
        if e_id == ID_UNDO:
            if hasattr(focus, 'CanUndo'):
                enable = focus.CanUndo()
            evt.Enable(enable)
        elif e_id == ID_REDO:
            if hasattr(focus, 'CanRedo'):
                enable = focus.CanRedo()
            evt.Enable(enable)
        elif e_id in (ID_PASTE, ID_PASTE_AFTER, ID_CYCLE_CLIPBOARD):
            if hasattr(focus, 'CanPaste'):
                enable = focus.CanPaste()
            evt.Enable(enable)
        elif e_id == ID_COPY:
            if hasattr(focus, 'CanCopy'):
                enable = focus.CanCopy()
            evt.Enable(enable)
        elif e_id == ID_CUT:
            if hasattr(focus, 'CanCut'):
                enable = focus.CanCut()
            evt.Enable(enable)
        elif e_id == ID_COLUMN_MODE:
            if hasattr(focus, 'IsColumnMode'):
                evt.Enable(True)
            else:
                evt.Enable(False)
            evt.Check(enable)
        else:
            evt.Skip()

    def OnUpdateFormatUI(self, evt):
        """Update status of format menu items
        @param evt: wxEVT_UPDATE_UI

        """
        if not self.IsActive():
            return

        e_id = evt.GetId()
        ctrl = self.nb.GetCurrentCtrl()
        if e_id == ID_USE_SOFTTABS:
            evt.Check(not bool(ctrl.GetUseTabs()))
        elif e_id == ID_WORD_WRAP:
            evt.Check(bool(ctrl.GetWrapMode()))
        elif e_id in [ID_EOL_MAC, ID_EOL_WIN, ID_EOL_UNIX]:
            evt.Check(ctrl.GetEOLModeId() == e_id)
        elif e_id in [ID_INDENT, ID_TO_UPPER, ID_TO_LOWER]:
            evt.Enable(ctrl.GetSelectionStart() != ctrl.GetSelectionEnd())
        else:
            evt.Skip()

    def OnUpdateLexerUI(self, evt):
        """Update status of lexer menu
        @param evt: wxEVT_UPDATE_UI

        """
        if not self.IsActive():
            return

        e_id = evt.GetId()
        if e_id in syntax.SYNTAX_IDS:
            lang = self.nb.GetCurrentCtrl().GetLangId()
            evt.Check(lang == evt.GetId())
        else:
            evt.Skip()

    def OnUpdateSettingsUI(self, evt):
        """Update settings menu items
        @param evt: wxEVT_UPDATE_UI

        """
        if not self.IsActive():
            return

        e_id = evt.GetId()
        ctrl = self.nb.GetCurrentCtrl()
        if e_id == ID_AUTOCOMP:
            evt.Check(ctrl.GetAutoComplete())
        elif e_id == ID_AUTOINDENT:
            evt.Check(ctrl.GetAutoIndent())
        elif e_id == ID_SYNTAX:
            evt.Check(ctrl.IsHighlightingOn())
        elif e_id == ID_FOLDING:
            evt.Check(ctrl.IsFoldingOn())
        elif e_id == ID_BRACKETHL:
            evt.Check(ctrl.IsBracketHlOn())
        else:
            evt.Skip()

    def OnUpdateViewUI(self, evt):
        """Update status of view menu items
        @param evt: wxEVT_UPDATE_UI

        """
        if not self.IsActive():
            return

        e_id = evt.GetId()
        ctrl = self.nb.GetCurrentCtrl()
        zoom = ctrl.GetZoom()
        if e_id == ID_ZOOM_NORMAL:
            evt.Enable(zoom)
        elif e_id == ID_ZOOM_IN:
            evt.Enable(zoom < 18)
        elif e_id == ID_ZOOM_OUT:
            evt.Enable(zoom > -8)
        elif e_id == ID_GOTO_MBRACE:
            evt.Enable(-1 not in ctrl.GetBracePair())
        elif e_id == ID_HLCARET_LINE:
            evt.Check(ctrl.GetCaretLineVisible())
        elif e_id == ID_SHOW_SB:
            evt.Check(self.GetStatusBar().IsShown())
        elif e_id == ID_VIEW_TOOL:
            evt.Check(self.GetToolBar().IsShown())
        elif e_id == ID_SHOW_WS:
            evt.Check(bool(ctrl.GetViewWhiteSpace()))
        elif e_id == ID_SHOW_EDGE:
            evt.Check(bool(ctrl.GetEdgeMode()))
        elif e_id == ID_SHOW_EOL:
            evt.Check(bool(ctrl.GetViewEOL()))
        elif e_id == ID_SHOW_LN:
            evt.Check(bool(ctrl.GetMarginWidth(1)))
        elif e_id == ID_INDENT_GUIDES:
            evt.Check(bool(ctrl.GetIndentationGuides()))
        elif e_id == ID_MAXIMIZE_EDITOR:
            paneInfo = self.PanelMgr.GetPane("EditPane")
            binder = self.MenuBar.GetKeyBinder()
            binding = binder.GetBinding(ID_MAXIMIZE_EDITOR)
            txt = _("Maximize Editor")
            if paneInfo.IsMaximized():
                txt = _("Restore Editor")
            evt.SetText(txt + binding)
        else:
            evt.Skip()

    def OnCommandBar(self, evt):
        """Open the Commandbar
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        e_id = evt.GetId()
        if e_id in (ID_QUICK_FIND, ID_GOTO_LINE, ID_COMMAND):
            self._mpane.ShowCommandControl(e_id)
        else:
            evt.Skip()

    def OnCustomizeLangMenu(self, evt):
        """Show the lexer menu customization dialog"""
        if evt.GetId() == ID_LEXER_CUSTOM:
            dlg = eclib.FilterDialog(self, title=_("Customize Menu"),
                                     style=wx.DEFAULT_DIALOG_STYLE|wx.RESIZE_BORDER)
            mconfig = _PGET("LEXERMENU", default=list())
            flters = dict()
            for item in syntax.SyntaxNames():
                if item in mconfig:
                    flters[item] = True
                else:
                    flters[item] = False
            dlg.SetListValues(flters)
            dlg.SetInitialSize()
            dlg.CenterOnParent()

            if ebmlib.LockCall(self._mlock, dlg.ShowModal) == wx.ID_OK:
                includes = dlg.GetIncludes()
                includes.sort()
                _PSET("LEXERMENU", includes)
                ed_msg.PostMessage(ed_msg.EDMSG_CREATE_LEXER_MENU)
            dlg.Destroy()
        else:
            evt.Skip()

    def OnHelp(self, evt):
        """Handles help related menu events
        @param evt: Event fired that called this handler
        @type evt: wxMenuEvent

        """
        import webbrowser
        e_id = evt.GetId()
        if e_id == ID_HOMEPAGE:
            page = HOME_PAGE
        elif e_id == ID_DOCUMENTATION:
            page = HOME_PAGE + "/documentation"
        elif e_id == ID_TRANSLATE:
            page = I18N_PAGE
        elif e_id == ID_CONTACT:
            webbrowser.open("mailto:%s" % CONTACT_MAIL)
            return
        elif e_id == ID_BUG_TRACKER:
            page = "http://code.google.com/p/editra/issues/list"
        else:
            evt.Skip()
            return

        # It seems under some cases when running under windows the call to
        # subprocess in webbrowser will fail and raise an exception here. So
        # simply trap and ignore it.
        try:
            self.PushStatusText(_("Opening %s") % page, SB_INFO)
            webbrowser.open(page, 1)
        except:
            self.PushStatusText(_("Error: Unable to open %s") % page, SB_INFO)

    def PushStatusText(self, txt, field):
        """Override so that our custom status bar's method gets called
        do to these wxFrame methods not being exposed as virtuals.

        """
        sb = self.GetStatusBar()
        if sb:
            sb.PushStatusText(txt, field)

    SetStatusText = PushStatusText

    def SetTitle(self, title=u''):
        """Sets the windows title
        @param title: The text to tag on to the default frame title
        @type title: string

        """
        name = u"%s v%s" % (PROG_NAME, VERSION)
        if len(title):
            name = u" - " + name
        super(MainWindow, self).SetTitle(title + name)

    def SetupToolBar(self):
        """Setup or reinitialize the windows ToolBar"""
        tb = self.GetToolBar()
        if tb:
            tb.Destroy()
        self.SetToolBar(ed_toolbar.EdToolBar(self))
        self.GetToolBar().Show(_PGET('TOOLBAR'))
        self.Layout()

    @classmethod
    def UpdateClipboardRing(cls):
        """Update the clipboard ring to sync it with the
        system clipboard.
        @note: for internal use only

        """
        txt = util.GetClipboardText()
        if txt is None or cls.CLIPBOARD.IsAtIndex(txt):
            return

        # Something new has come in from an external program
        cls.CLIPBOARD.Reset()
        cls.CLIPBOARD.Put(txt)

#-----------------------------------------------------------------------------#
# Event handlers that don't need to be part of the class

def OnAbout(evt):
    """Show the About Dialog
    @param evt: Event fired that called this handler
    @type evt: wxMenuEvent

    """
    if evt.GetId() == ID_ABOUT:
        info = wx.AboutDialogInfo()
        year = time.localtime()
        desc = [_("Editra is a programmers text editor."),
                _("Written in 100%% Python."),
                _("Homepage") + ": " + HOME_PAGE + "\n",
                _("Platform Info") + ": (%s,%s)",
                _("License: wxWindows (see COPYING.txt for full license)")]
        desc = "\n".join(desc)
        py_version = sys.platform + ", python " + sys.version.split()[0]
        platform = list(wx.PlatformInfo[1:])
        platform[0] += (" " + wx.VERSION_STRING)
        wx_info = ", ".join(platform)
        info.SetCopyright(_("Copyright") + "(C) 2005-%d Cody Precord" % year[0])
        info.SetName(PROG_NAME.title())
        info.SetDescription(desc % (py_version, wx_info))
        info.SetVersion(VERSION)
        wx.AboutBox(info)
    else:
        evt.Skip()

def OnPreferences(evt):
    """Open the Preference Panel
    @param evt: Event fired that called this handler
    @type evt: wxMenuEvent

    """
    if evt.GetId() == ID_PREF:
        cursor = wx.BusyCursor()
        win = wx.GetApp().GetWindowInstance(prefdlg.PreferencesDialog)
        if win is not None:
            win.Raise()
        else:
            dlg = prefdlg.PreferencesDialog(None)
            dlg.CenterOnParent()
            dlg.Show()
        del cursor
    else:
        evt.Skip()

#-----------------------------------------------------------------------------#
# Plugin interface to the MainWindow

class MainWindowAddOn(plugin.Plugin):
    """Plugin that Extends the L{MainWindowI}"""
    observers = plugin.ExtensionPoint(iface.MainWindowI)
    def Init(self, window):
        """Call all observers once to initialize
        @param window: window that observers become children of

        """
        for observer in self.observers:
            try:
                observer.PlugIt(window)
            except Exception, msg:
                util.Log("[ed_main][err] MainWindowAddOn.Init: %s" % str(msg))

    def GetEventHandlers(self, ui_evt=False):
        """Get Event handlers and Id's from all observers
        @keyword ui_evt: Get Update Ui handlers (default get menu handlers)
        @return: list [(ID_FOO, foo.OnFoo), (ID_BAR, bar.OnBar)]

        """
        handlers = list()
        for observer in self.observers:
            try:
                items = None
                if ui_evt:
                    if hasattr(observer, 'GetUIHandlers'):
                        items = observer.GetUIHandlers()
                        assert isinstance(items, list), "Must be a list()!"
                else:
                    if hasattr(observer, 'GetMenuHandlers'):
                        items = observer.GetMenuHandlers()
                        assert isinstance(items, list), "Must be a list()!"
            except Exception, msg:
                util.Log("[ed_main][err] MainWindowAddOn.GetEventHandlers: %s" % str(msg))
                continue

            if items is not None:
                handlers.extend(items)

        return handlers
