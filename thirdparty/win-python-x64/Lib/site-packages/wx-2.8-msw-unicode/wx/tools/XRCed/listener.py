# Name:         listener.py
# Purpose:      Listener for dispatching events from view to presenter
# Author:       Roman Rolinsky <rolinsky@femagsoft.com>
# Created:      07.06.2007
# RCS-ID:       $Id: listener.py 64627 2010-06-18 18:17:45Z ROL $

import wx
import os,sys,shutil,tempfile
from globals import *
from presenter import Presenter
from component import Manager
from model import Model
import view
import undo
from generate import PythonOptions

class _Listener:
    '''
    Installs event handlers to view objects and delegates some events
    to Presenter.
    '''
    def Install(self, frame, tree, panel, toolFrame, testWin):
        '''Set event handlers.'''
        self.frame = frame
        self.tree = tree
        self.panel = panel
        self.toolFrame = toolFrame
        self.testWin = testWin
        self.lastSearch = None

        self.dataElem = wx.CustomDataObject('XRCED_elem')
        self.dataNode = wx.CustomDataObject('XRCED_node')

        # Some local members
        self.inUpdateUI = self.inIdle = False
        self.clipboardHasData = False
        
        # Component events
        wx.EVT_MENU_RANGE(frame, Manager.firstId, Manager.lastId,
                          self.OnComponentCreate)
        wx.EVT_MENU_RANGE(frame, Manager.firstId + ID.SHIFT, Manager.lastId + ID.SHIFT,
                          self.OnComponentReplace)

        wx.EVT_MENU(frame, ID.REF, self.OnReference)
        wx.EVT_MENU(frame, ID.COMMENT, self.OnComment)

        # Other events
        frame.Bind(wx.EVT_IDLE, self.OnIdle)
        frame.Bind(wx.EVT_CLOSE, self.OnCloseWindow)
#        frame.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
#        wx.EVT_KEY_UP(frame, tools.OnKeyUp)
#        wx.EVT_ICONIZE(frame, self.OnIconize)

        frame.Bind(wx.EVT_ACTIVATE, self.OnFrameActivate)
        if toolFrame:
            toolFrame.Bind(wx.EVT_ACTIVATE, self.OnFrameActivate)
        if frame.miniFrame:
            frame.miniFrame.Bind(wx.EVT_ACTIVATE, self.OnFrameActivate)
        
        # Menubar events
        # File
        frame.Bind(wx.EVT_MENU, self.OnRecentFile, id=wx.ID_FILE1, id2=wx.ID_FILE9)
        wx.EVT_MENU(frame, wx.ID_NEW, self.OnNew)
        wx.EVT_MENU(frame, wx.ID_OPEN, self.OnOpen)
        wx.EVT_MENU(frame, wx.ID_SAVE, self.OnSaveOrSaveAs)
        wx.EVT_MENU(frame, wx.ID_SAVEAS, self.OnSaveOrSaveAs)
        wx.EVT_MENU(frame, frame.ID_GENERATE_PYTHON, self.OnGeneratePython)
        wx.EVT_MENU(frame, wx.ID_PREFERENCES, self.OnPrefs)
        wx.EVT_MENU(frame, wx.ID_EXIT, self.OnExit)
        if frame.miniFrame:
            wx.EVT_MENU(frame.miniFrame, wx.ID_EXIT, self.OnExit)

        # Edit
        wx.EVT_MENU(frame, wx.ID_UNDO, self.OnUndo)
        wx.EVT_MENU(frame, wx.ID_REDO, self.OnRedo)
        wx.EVT_MENU(frame, wx.ID_CUT, self.OnCut)
        wx.EVT_MENU(frame, wx.ID_COPY, self.OnCopy)
        wx.EVT_MENU(frame, wx.ID_PASTE, self.OnMenuPaste)
        wx.EVT_MENU(frame, ID.PASTE, self.OnCmdPaste)
        wx.EVT_MENU(frame, ID.PASTE_SIBLING, self.OnPasteSibling)
        wx.EVT_MENU(frame, wx.ID_DELETE, self.OnDelete)
        wx.EVT_MENU(frame, frame.ID_UNSELECT, self.OnUnselect)
        wx.EVT_MENU(frame, frame.ID_TOOL_PASTE, self.OnToolPaste)
        wx.EVT_MENU(frame, wx.ID_FIND, self.OnFind)
        wx.EVT_MENU(frame, frame.ID_FINDAGAIN, self.OnFindAgain)
        wx.EVT_MENU(frame, frame.ID_LOCATE, self.OnLocate)
        wx.EVT_MENU(frame, frame.ID_TOOL_LOCATE, self.OnLocate)
        # View
        wx.EVT_MENU(frame, frame.ID_EMBED_PANEL, self.OnEmbedPanel)
        wx.EVT_MENU(frame, frame.ID_SHOW_TOOLS, self.OnShowTools)
        wx.EVT_MENU(frame, frame.ID_TEST, self.OnTest)
        wx.EVT_MENU(frame, wx.ID_REFRESH, self.OnRefresh)
        wx.EVT_MENU(frame, frame.ID_AUTO_REFRESH, self.OnAutoRefresh)
        wx.EVT_MENU(frame, frame.ID_TEST_HIDE, self.OnTestHide)
        wx.EVT_MENU(frame, frame.ID_SHOW_XML, self.OnShowXML)
        # Move
        wx.EVT_MENU(frame, frame.ID_MOVEUP, self.OnMoveUp)
        wx.EVT_MENU(frame, frame.ID_MOVEDOWN, self.OnMoveDown)
        wx.EVT_MENU(frame, frame.ID_MOVELEFT, self.OnMoveLeft)
        wx.EVT_MENU(frame, frame.ID_MOVERIGHT, self.OnMoveRight)        
        # Help
        wx.EVT_MENU(frame, wx.ID_ABOUT, self.OnHelpAbout)
        wx.EVT_MENU(frame, wx.ID_HELP_CONTENTS, self.OnHelpContents)
        wx.EVT_MENU(frame, frame.ID_README, self.OnHelpReadme)
        if get_debug():
            wx.EVT_MENU(frame, frame.ID_DEBUG_CMD, self.OnDebugCMD)

        # Pulldown menu commands
        wx.EVT_MENU(frame, ID.SUBCLASS, self.OnSubclass)
        wx.EVT_MENU(frame, ID.COLLAPSE, self.OnCollapse)
        wx.EVT_MENU(frame, ID.COLLAPSE_ALL, self.OnCollapseAll)
        wx.EVT_MENU(frame, ID.EXPAND, self.OnExpand)

        # Update events
        wx.EVT_UPDATE_UI(frame, wx.ID_SAVE, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, wx.ID_CUT, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, wx.ID_COPY, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, wx.ID_PASTE, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, wx.ID_DELETE, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, frame.ID_LOCATE, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, frame.ID_FINDAGAIN, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, frame.ID_TOOL_LOCATE, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, frame.ID_TOOL_PASTE, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, wx.ID_UNDO, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, wx.ID_REDO, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, frame.ID_TEST, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, frame.ID_MOVEUP, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, frame.ID_MOVEDOWN, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, frame.ID_MOVELEFT, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, frame.ID_MOVERIGHT, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, wx.ID_REFRESH, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, frame.ID_SHOW_XML, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, ID.COLLAPSE, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, ID.EXPAND, self.OnUpdateUI)
        wx.EVT_UPDATE_UI(frame, ID.SUBCLASS, self.OnUpdateUI)

        wx.EVT_MENU_HIGHLIGHT_ALL(self.frame, self.OnMenuHighlight)

        # XMLTree events
        tree.Bind(wx.EVT_LEFT_DOWN, self.OnTreeLeftDown)
        tree.Bind(wx.EVT_RIGHT_DOWN, self.OnTreeRightDown)
        tree.Bind(wx.EVT_TREE_SEL_CHANGING, self.OnTreeSelChanging)
        tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.OnTreeSelChanged)
        tree.Bind(wx.EVT_TREE_ITEM_COLLAPSED, self.OnTreeItemCollapsed)

        # AttributePanel events
        panel.nb.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGING, self.OnPanelPageChanging)
        panel.nb.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGED, self.OnPanelPageChanged)
        panel.pinButton.Bind(wx.EVT_BUTTON, self.OnPanelTogglePin)

        # Make important keys work when focus is in the panel frame
        self.accels = wx.AcceleratorTable([
            (wx.ACCEL_NORMAL, wx.WXK_F5, frame.ID_TEST),
            (wx.ACCEL_NORMAL, wx.WXK_F6, frame.ID_TEST_HIDE),
            (wx.ACCEL_CTRL, ord('r'), wx.ID_REFRESH),
            ])
        if frame.miniFrame:
            self.frame.miniFrame.SetAcceleratorTable(self.accels)
            # Propagate all menu commands to the frame
            self.frame.miniFrame.Bind(wx.EVT_MENU, lambda evt: frame.ProcessEvent(evt))

        # Tool panel events
        toolPanel = g.toolPanel
        toolPanel.tp.Bind(wx.EVT_TOOLBOOK_PAGE_CHANGED, self.OnToolPanelPageChanged)
        wx.EVT_COMMAND_RANGE(toolPanel.tp, Manager.firstId, Manager.lastId,
                             wx.wxEVT_COMMAND_BUTTON_CLICKED,
                             self.OnComponentTool)
        if toolFrame:
            toolFrame.Bind(wx.EVT_CLOSE, self.OnCloseToolFrame)

    def InstallTestWinEvents(self):
        self.idleAfterSizeBound = False
        frame = self.testWin.GetFrame()
        frame.Bind(wx.EVT_CLOSE, self.OnCloseTestWin)
        frame.Bind(wx.EVT_SIZE, self.OnSizeTestWin)
        frame.SetAcceleratorTable(self.accels)
        frame.Bind(wx.EVT_MENU, self.OnTestWinEvent)
        frame.Bind(wx.EVT_BUTTON, self.OnTestWinEvent)

    def OnTestWinEvent(self, evt):
        TRACE('Test window event: %s', evt)

    def Uninstall(self):
        '''Unbind some event before destroying.'''
        self.frame.Unbind(wx.EVT_IDLE)

    def OnComponentCreate(self, evt):
        '''Hadnler for creating new elements.'''
        state = self.tree.GetFullState() # state just before
        comp = Manager.findById(evt.GetId())
        if comp.groups[0] == 'component':
            node = Model.createComponentNode('Component')
            item = Presenter.create(comp, node)
        else:
            item = Presenter.create(comp)
        itemIndex = self.tree.ItemFullIndex(item)
        g.undoMan.RegisterUndo(undo.UndoPasteCreate(itemIndex, state))

    def OnComponentReplace(self, evt):
        '''Hadnler for creating new elements.'''
        comp = Manager.findById(evt.GetId() - ID.SHIFT)
        item = self.tree.GetSelection()
        index = self.tree.ItemFullIndex(item)
        oldComp = Presenter.comp
        oldNode = Presenter.replace(comp)
        g.undoMan.RegisterUndo(undo.UndoReplace(index, oldComp, oldNode))

    def OnReference(self, evt):
        '''Create reference to an existing object.'''
        ref = wx.GetTextFromUser('Create reference to:', 'Create reference')
        if not ref: return
        Presenter.createRef(ref)

    def OnComment(self, evt):
        '''Create comment node.'''
        Presenter.createComment()

    def OnNew(self, evt):
        '''wx.ID_NEW hadndler.'''
        if not self.AskSave(): return
        if self.testWin.IsShown(): self.testWin.Destroy()
        Presenter.init()

    def OnOpen(self, evt):
        '''wx.ID_OPEN handler.'''
        if not self.AskSave(): return
        exts = 'XRC files (*.xrc)|*.xrc'
        if g.useMeta: exts += '|CRX files (*.crx)|*.crx'
        dlg = wx.FileDialog(self.frame, 'Open', os.path.dirname(Presenter.path),
                            '', exts, wx.OPEN | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            if self.testWin.IsShown(): self.testWin.Destroy()
            # Clear old undo data
            g.undoMan.Clear()
            path = dlg.GetPath()
            wx.BeginBusyCursor()
            try:
                Presenter.open(path)
                self.frame.SetStatusText('Data loaded')
                self.SaveRecent(path)
            finally:
                wx.EndBusyCursor()
        dlg.Destroy()

    def OnRecentFile(self, evt):
        '''wx.ID_FILE<n> handler.'''
        if not self.AskSave(): return

        # get the pathname based on the menu ID
        fileNum = evt.GetId() - wx.ID_FILE1
        path = g.fileHistory.GetHistoryFile(fileNum)
            
        wx.BeginBusyCursor()
        try:
            if self.testWin.IsShown(): self.testWin.Destroy()
            Presenter.open(path)
            self.frame.SetStatusText('Data loaded')
            # add it back to the history so it will be moved up the list
            self.SaveRecent(path)
        finally:
            wx.EndBusyCursor()

    def OnSaveOrSaveAs(self, evt):
        '''wx.ID_SAVE and wx.ID_SAVEAS handler'''
        path = Presenter.path
        if evt.GetId() == wx.ID_SAVEAS or not path:
            dirname = os.path.abspath(os.path.dirname(path))
            exts = 'XRC files (*.xrc)|*.xrc'
            if g.useMeta: exts += '|CRX files (*.crx)|*.crx'
            dlg = wx.FileDialog(self.frame, 'Save As', dirname, '', exts,
                               wx.SAVE | wx.OVERWRITE_PROMPT | wx.CHANGE_DIR)
            if dlg.ShowModal() == wx.ID_OK:
                path = dlg.GetPath()
                if isinstance(path, unicode):
                    path = path.encode(sys.getfilesystemencoding())
                if not os.path.splitext(path)[1]:
                    if g.useMeta:
                        path += '.crx'
                    else:
                        path += '.xrc'
                dlg.Destroy()
            else:
                dlg.Destroy()
                return

        if g.conf.localconf:
            # if we already have a localconf then it needs to be
            # copied to a new config with the new name
            lc = g.conf.localconf
            nc = Presenter.createLocalConf(path)
            flag, key, idx = lc.GetFirstEntry()
            while flag:
                nc.Write(key, lc.Read(key))
                flag, key, idx = lc.GetNextEntry(idx)
            g.conf.localconf = nc
        else:
            # otherwise create a new one
            g.conf.localconf = Presenter.createLocalConf(path)
        wx.BeginBusyCursor()
        try:
            Presenter.save(path) # save temporary file first
            if g.conf.localconf.ReadBool("autogenerate", False):
                pypath = g.conf.localconf.Read("filename")
                embed = g.conf.localconf.ReadBool("embedResource", False)
                genGettext = g.conf.localconf.ReadBool("genGettext", False)
                Presenter.generatePython(path, pypath, embed, genGettext)
            self.frame.SetStatusText('Data saved')
            self.SaveRecent(path)
        finally:
            wx.EndBusyCursor()        

    def OnPrefs(self, evt):
        self.frame.ShowPrefs()

    def OnExit(self, evt):
        '''wx.ID_EXIT handler'''
        self.frame.Close()
        
    def OnGeneratePython(self, evt):
        if Presenter.modified or not g.conf.localconf:
            wx.MessageBox("Save the XRC file first!", "Error")
            return
        
        dlg = PythonOptions(view.frame, g.conf.localconf, Presenter.path)
        dlg.ShowModal()
        dlg.Destroy()
        
    def SaveRecent(self, path):
        '''Append path to recently used files.'''
        g.fileHistory.AddFileToHistory(path)

    def AskSave(self):
        '''Show confirmation dialog.'''
        if not Presenter.modified: return True
        flags = wx.ICON_EXCLAMATION | wx.YES_NO | wx.CANCEL | wx.CENTRE
        dlg = wx.MessageDialog(self.frame, 'File is modified. Save before exit?',
                               'Save before too late?', flags)
        say = dlg.ShowModal()
        dlg.Destroy()
        wx.Yield()
        if say == wx.ID_YES:
            self.OnSaveOrSaveAs(wx.CommandEvent(wx.EVT_MENU.typeId, wx.ID_SAVE))
            # If save was successful, modified flag is unset
            if not Presenter.modified: return True
        elif say == wx.ID_NO:
            Presenter.setModified(False)
            return True
        return False

    def OnCloseWindow(self, evt):
        '''wx.EVT_CLOSE handler'''
        if not self.AskSave(): return
        if self.testWin.object: self.testWin.Destroy()
        self.panel.undo = False # prevent undo
        g.undoMan.Clear()        
        # Remember sizes and position
        conf = g.conf
        if g.useAUI:
            conf.perspective = view.frame.mgr.SavePerspective()
        if not self.frame.IsIconized():
            conf.pos = self.frame.GetPosition()
            if wx.Platform == '__WXMAC__':
                conf.size = self.frame.GetClientSize()
            else:
                conf.size = self.frame.GetSize()
            if not g.useAUI:
                if conf.embedPanel:
                    conf.sashPos = self.frame.splitter.GetSashPosition()
                else:
                    if self.frame.miniFrame:
                        conf.panelPos = self.frame.miniFrame.GetPosition()
                        conf.panelSize = self.frame.miniFrame.GetSize()
                if conf.showToolPanel and self.toolFrame:
                    conf.toolPanelPos = self.toolFrame.GetPosition()
                    conf.toolPanelSize = self.toolFrame.GetSize()
        #self.tree.UnselectAll()
        g.undoMan.Clear()
        #self.panel.Destroy()            # destroy panel before tree
        self.Uninstall()
        self.frame.Destroy()

    def OnUndo(self, evt):
        if g.undoMan.CanUndo():
            g.undoMan.Undo()

    def OnRedo(self, evt):
        if g.undoMan.CanRedo():
            g.undoMan.Redo()

    def OnCut(self, evt):
        '''wx.ID_CUT handler.'''
        item = self.tree.GetSelection()
        index = self.tree.ItemFullIndex(item)
        state = self.tree.GetFullState()        
        node = Presenter.cut()
        g.undoMan.RegisterUndo(undo.UndoCutDelete(index, state, node))

    def OnDelete(self, evt):
        '''wx.ID_DELETE handler.'''
        if len(self.tree.GetSelections()) == 1:
            item = self.tree.GetSelection()
            index = self.tree.ItemFullIndex(item)
            state = self.tree.GetFullState()
            node = Presenter.delete(self.tree.GetSelection())
            g.undoMan.RegisterUndo(undo.UndoCutDelete(index, state, node))
        else:
            # Save all if multiselection
            g.undoMan.RegisterUndo(undo.UndoGlobal())
            Presenter.deleteMany(self.tree.GetSelections())

    def OnCopy(self, evt):
        '''wx.ID_COPY handler.'''
        Presenter.copy()

    def OnMenuPaste(self, evt):
        '''wx.ID_PASTE handler (for XMLTreeMenu).'''
        state = self.tree.GetFullState() # state just before
        item = Presenter.paste()
        if not item: return     # error in paste()
        itemIndex = self.tree.ItemFullIndex(item)
        g.undoMan.RegisterUndo(undo.UndoPasteCreate(itemIndex, state))

    def OnCmdPaste(self, evt):
        '''ID.PASTE handler (for Edit menu and shortcuts).'''
        TRACE('OnCmdPaste')
        state = wx.GetMouseState()
        forceSibling = state.AltDown()
        forceInsert = state.ShiftDown()
        g.Presenter.updateCreateState(forceSibling, forceInsert)
        state = self.tree.GetFullState() # state just before
        item = Presenter.paste()
        if not item: return     # error in paste()
        itemIndex = self.tree.ItemFullIndex(item)
        g.undoMan.RegisterUndo(undo.UndoPasteCreate(itemIndex, state))

    def OnToolPaste(self, evt):
        '''frame.ID_TOOL_PASTE handler.'''
        state = wx.GetMouseState()
        # Ctrl+click does not work with tools on Mac, Alt+click often
        # bound to window move on wxGTK
        if wx.Platform == '__WXMAC__':
            forceSibling = state.AltDown()
        else:
            forceSibling = state.ControlDown()
        forceInsert = state.ShiftDown()
        g.Presenter.updateCreateState(forceSibling, forceInsert)
        treeState = self.tree.GetFullState() # state just before
        item = Presenter.paste()
        if not item: return     # error in paste()
        itemIndex = self.tree.ItemFullIndex(item)
        g.undoMan.RegisterUndo(undo.UndoPasteCreate(itemIndex, treeState)) 

    def OnPasteSibling(self, evt):
        '''ID.PASTE_SIBLING handler.'''
        forceSibling = True
        state = wx.GetMouseState()
        forceInsert = state.ShiftDown()
        g.Presenter.updateCreateState(forceSibling, forceInsert)
        treeState = self.tree.GetFullState() # state just before
        item = Presenter.paste()
        itemIndex = self.tree.ItemFullIndex(item)
        g.undoMan.RegisterUndo(undo.UndoPasteCreate(itemIndex, treeState))

    def OnUnselect(self, evt):
        self.tree.UnselectAll()
        if not Presenter.applied: Presenter.update()
        Presenter.setData(self.tree.root)

    def ItemsAreCompatible(self, parent, child):
        raise NotImplementedError

    def OnMoveUp(self, evt):
        self.inIdle = True
        g.undoMan.RegisterUndo(undo.UndoGlobal())
        Presenter.moveUp()
        self.inIdle = False

    def OnMoveDown(self, evt):
        self.inIdle = True
        g.undoMan.RegisterUndo(undo.UndoGlobal()) 
        Presenter.moveDown()
        self.inIdle = False
    
    def OnMoveLeft(self, evt):
        self.inIdle = True
        g.undoMan.RegisterUndo(undo.UndoGlobal()) 
        Presenter.moveLeft()
        self.inIdle = False

    def OnMoveRight(self, evt):
        self.inIdle = True
        g.undoMan.RegisterUndo(undo.UndoGlobal()) 
        Presenter.moveRight()
        self.inIdle = False

    def OnFind(self, evt):
        name = wx.GetTextFromUser('Find name:', caption='Find')
        if not name: return
        self.lastSearch = name
        self.frame.SetStatusText('Looking for "%s"' % name)
        if Presenter.item == self.tree.root:
            item = self.tree.Find(self.tree.root, name)
        else:
            # Find from current position
            item = Presenter.item
            while item:
                found = self.tree.Find(item, name)
                if found: 
                    item = found
                    break
                # Search the rest of the current subtree, then go up
                next = self.tree.GetNextSibling(item)
                while not next:
                    next = self.tree.GetItemParent(item)
                    if next == self.tree.root:
                        next = None
                        break
                    item = next
                    next = self.tree.GetNextSibling(next)
                item = next
            if not item: 
                ask = wx.MessageBox('Search failed. Search from the root?', 
                                    'Question', wx.YES_NO)
                if ask == wx.YES:
                    item = self.tree.Find(self.tree.root, name)
                else:
                    self.frame.SetStatusText('')
                    return
        if not item: 
            self.frame.SetStatusText('Search failed')
            wx.LogError('No such name')
            return
        self.frame.SetStatusText('Search succeded')
        Presenter.unselect()
        self.tree.EnsureVisible(item)
        self.tree.SelectItem(item)        

    def OnFindAgain(self, evt):
        self.frame.SetStatusText('Looking for "%s"' % self.lastSearch)
        if Presenter.item == self.tree.root:
            item = self.tree.Find(self.tree.root, self.lastSearch)
        else:
            # Find from current position
            item = Presenter.item        
            while item:
                # Search the rest of the current subtree, then go up
                next = self.tree.GetNextSibling(item)
                while not next:
                    next = self.tree.GetItemParent(item)
                    if next == self.tree.root:
                        next = None
                        break
                    item = next
                    next = self.tree.GetNextSibling(next)
                item = next
                if item:
                    found = self.tree.Find(item, self.lastSearch)
                    if found: 
                        item = found
                        break
            if not item: 
                ask = wx.MessageBox('Search failed. Search from the root?', 
                                    'Question', wx.YES_NO)
                if ask == wx.YES:
                    item = self.tree.Find(self.tree.root, name)
                    if not item: 
                        self.frame.SetStatusText('Search failed')
                        wx.LogError('Search from the root failed.')
                        return
                else:
                    self.frame.SetStatusText('')
                    return
        self.lastFoundItem = item
        self.frame.SetStatusText('Search succeded')
        Presenter.unselect()
        self.tree.EnsureVisible(item)
        self.tree.SelectItem(item)                

    def OnLocate(self, evt):
        frame = self.testWin.GetFrame()
        frame.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDownTestWin)
        frame.Bind(wx.EVT_MOUSE_CAPTURE_LOST, self.OnCaptureLostTestWin)
        frame.CaptureMouse()
        wx.SetCursor(wx.CROSS_CURSOR)

    def OnRefresh(self, evt):
        if self.testWin.IsShown():
            self.testWin.isDirty = True
            Presenter.refreshTestWin()

    def OnAutoRefresh(self, evt):
        g.conf.autoRefresh = evt.IsChecked()
        self.frame.menuBar.Check(self.frame.ID_AUTO_REFRESH, g.conf.autoRefresh)
        if g.conf.embedPanel:
            self.frame.tb.ToggleTool(self.frame.ID_AUTO_REFRESH, g.conf.autoRefresh)
        else:
            self.frame.miniFrame.tb.ToggleTool(self.frame.ID_AUTO_REFRESH, g.conf.autoRefresh)

    def OnHelpAbout(self, evt):
        str = '''\
XRCed version %s

(c) Roman Rolinsky <rollrom@users.sourceforge.net>
Homepage: http://xrced.sourceforge.net\
''' % version
        dlg = wx.MessageDialog(self.frame, str, 'About XRCed', wx.OK | wx.CENTRE)
        dlg.ShowModal()
        dlg.Destroy()

    def OnHelpContents(self, evt):
      self.frame.htmlCtrl.DisplayContents()
          
    def OnHelpReadme(self, evt):
        self.frame.ShowReadme()

    # Simple emulation of python command line
    def OnDebugCMD(self, evt):
        while 1:
            try:
                exec raw_input('C:\> ')
            except EOFError:
                print '^D'
                break
            except:
                import traceback
                (etype, value, tb) =sys.exc_info()
                tblist =traceback.extract_tb(tb)[1:]
                msg =' '.join(traceback.format_exception_only(etype, value)
                        +traceback.format_list(tblist))
                print msg

    def OnEmbedPanel(self, evt):
        self.frame.EmbedUnembed(evt.IsChecked())

    def OnShowTools(self, evt):
        conf = g.conf
        self.toolFrame.Show()
        conf.showToolPanel = True
        
    def OnTest(self, evt):
        if not Presenter.item: return
        Presenter.createTestWin(Presenter.item)

    # Test window events

    def OnCloseTestWin(self, evt):
        TRACE('OnCloseTestWin')
        Presenter.closeTestWin()

    def OnSizeTestWin(self, evt):
        TRACE('OnSizeTestWin')
        if view.testWin.hl and not self.idleAfterSizeBound:
            self.idleAfterSizeBound = True
            frame = self.testWin.GetFrame()
            frame.Bind(wx.EVT_IDLE, self.OnIdleAfterSize)
        evt.Skip()

    def OnIdleAfterSize(self, evt):
        frame = self.testWin.GetFrame()
        frame.Unbind(wx.EVT_IDLE)
        self.idleAfterSizeBound = False
        TRACE('OnIdleAfterSize')
        Presenter.highlight(Presenter.item)

    def OnTestHide(self, evt):
        Presenter.closeTestWin()

    def OnCaptureLostTestWin(self, evt):
        frame = self.testWin.GetFrame()
        wx.SetCursor(wx.NullCursor)
        frame.ReleaseMouse()
        frame.Unbind(wx.EVT_LEFT_DOWN)
        self.frame.tb.ToggleTool(view.frame.ID_TOOL_LOCATE, False)
        self.frame.miniFrame.tb.ToggleTool(view.frame.ID_TOOL_LOCATE, False)

    def OnLeftDownTestWin(self, evt):
        frame = self.testWin.GetFrame()
        wx.SetCursor(wx.NullCursor)
        frame.ReleaseMouse()
        frame.Unbind(wx.EVT_LEFT_DOWN)
        self.frame.tb.ToggleTool(view.frame.ID_TOOL_LOCATE, False)
        self.frame.miniFrame.tb.ToggleTool(view.frame.ID_TOOL_LOCATE, False)

        scrPos = view.testWin.object.ClientToScreen(evt.GetPosition())
        obj = wx.FindWindowAtPoint(scrPos)
        if not obj: return
        item = self.testWin.FindObjectItem(self.testWin.item, obj)
        if not item: return
        # If window has a sizer use it as parent
        if obj.GetSizer():
            obj = obj.GetSizer()
            item = self.testWin.FindObjectItem(self.testWin.item, obj)
        Presenter.unselect()
        self.tree.EnsureVisible(item)
        self.tree.SelectItem(item)

    def OnShowXML(self, evt):
        Presenter.showXML()

    def OnMenuHighlight(self, evt):
        menuId = evt.GetMenuId()
        if menuId != -1:
            menu = evt.GetEventObject()
            try:
                help = menu.GetHelpString(menuId)
                if menuId == wx.ID_UNDO:
                    help += ' ' + g.undoMan.GetUndoLabel()
                elif menuId == wx.ID_REDO:
                    help += ' ' + g.undoMan.GetRedoLabel()
                self.frame.SetStatusText(help)
            except:
                self.frame.SetStatusText('')
        else:
            self.frame.SetStatusText('')

    def OnUpdateUI(self, evt):
        if self.inUpdateUI: return          # Recursive call protection
        self.inUpdateUI = True
        container = Presenter.container
        comp = Presenter.comp
        treeNode = self.tree.GetPyData(Presenter.item)
        isComment = treeNode and treeNode.nodeType == treeNode.COMMENT_NODE
        # Wokraround for wxMSW: view.tree.GetPrevSibling crashes
        if evt.GetId() in [self.frame.ID_MOVEUP, self.frame.ID_MOVERIGHT,
                           self.frame.ID_MOVEDOWN, self.frame.ID_MOVELEFT] and \
            Presenter.item is view.tree.root:
            pass
        elif evt.GetId() in [wx.ID_CUT, wx.ID_COPY, wx.ID_DELETE]:
            evt.Enable(bool(self.tree.GetSelection()))
        elif evt.GetId() in [self.frame.ID_MOVEUP, self.frame.ID_MOVERIGHT]:
            evt.Enable(view.tree.GetPrevSibling(Presenter.item).IsOk())
        elif evt.GetId() == self.frame.ID_MOVEDOWN:
            evt.Enable(view.tree.GetNextSibling(Presenter.item).IsOk())
        elif evt.GetId() == self.frame.ID_MOVELEFT:
            evt.Enable(container is not Manager.rootComponent and \
                       view.tree.GetItemParent(Presenter.item).IsOk())
        elif evt.GetId() == wx.ID_SAVE:
            evt.Enable(Presenter.modified)
#        elif evt.GetId() in [self.frame.ID_SHOW_XML]:
#            evt.Enable(len(self.tree.GetSelections()) == 1)
        elif evt.GetId() in [wx.ID_PASTE, self.frame.ID_TOOL_PASTE]:
            evt.Enable(self.clipboardHasData)
        elif evt.GetId() in [self.frame.ID_TEST,
                             self.frame.ID_MOVEUP, self.frame.ID_MOVEDOWN,
                             self.frame.ID_MOVELEFT, self.frame.ID_MOVERIGHT]:
            evt.Enable(bool(self.tree.GetSelection()))
        elif evt.GetId() in [self.frame.ID_LOCATE, self.frame.ID_TOOL_LOCATE,
                             wx.ID_REFRESH]:
            evt.Enable(self.testWin.IsShown())
        elif evt.GetId() == self.frame.ID_FINDAGAIN:
            evt.Enable(self.lastSearch is not None)
        elif evt.GetId() == wx.ID_UNDO:  evt.Enable(g.undoMan.CanUndo())
        elif evt.GetId() == wx.ID_REDO:  evt.Enable(g.undoMan.CanRedo())
        elif evt.GetId() in [ID.COLLAPSE, ID.EXPAND]:
            evt.Enable(not self.tree.GetSelection() or
                       len(self.tree.GetSelections()) == 1 and \
                           self.tree.ItemHasChildren(self.tree.GetSelection()))
        elif evt.GetId() == ID.SUBCLASS:
            evt.Enable(not isComment)
        self.inUpdateUI = False

    def OnIdle(self, evt):
        if self.inIdle: return          # Recursive call protection
        self.inIdle = True
        if not Presenter.applied:
            item = self.tree.GetSelection()
            if item: Presenter.update(item)

        # Check clipboard
        if not wx.TheClipboard.IsOpened():
            self.clipboardHasData = False
            if wx.TheClipboard.IsSupported(self.dataElem.GetFormat()):
                self.clipboardHasData = True
            elif wx.TheClipboard.IsSupported(self.dataNode.GetFormat()):
                self.clipboardHasData = True

        self.inIdle = False

    def OnIconize(self, evt):
        conf = g.conf
        if evt.Iconized():
            conf.pos = self.frame.GetPosition()
            conf.size = self.frame.GetSize()
            if conf.embedPanel:
                conf.sashPos = self.frame.splitter.GetSashPosition()
            elif self.miniFrame:
                conf.panelPos = self.miniFrame.GetPosition()
                conf.panelSize = self.miniFrame.GetSize()
                self.miniFrame.Show(False)
        else:
            if not conf.embedPanel and self.miniFrame:
                self.miniFrame.Show(True)
        evt.Skip()

    def OnSubclass(self, evt):
        node = self.tree.GetPyData(Presenter.item)
        subclass = node.getAttribute('subclass')
        dlg = wx.TextEntryDialog(self.frame, 'Subclass:', defaultValue=subclass)
        if dlg.ShowModal() == wx.ID_OK:
            subclass = dlg.GetValue()
            Presenter.subclass(Presenter.item, subclass)
        dlg.Destroy()        

    # Expand/collapse subtree
    def OnExpand(self, evt):
        if self.tree.GetSelection(): 
            map(self.tree.ExpandAllChildren, self.tree.GetSelections())
        else: 
            self.tree.ExpandAll()

    def OnCollapse(self, evt):
        # Prevent multiple calls to setData
        self.tree.Unbind(wx.EVT_TREE_ITEM_COLLAPSED)
        if self.tree.GetSelection(): 
            map(self.tree.CollapseAllChildren, self.tree.GetSelections())
        else: 
            self.tree.CollapseAll()
        self.tree.Bind(wx.EVT_TREE_ITEM_COLLAPSED, self.OnTreeItemCollapsed)
        if not self.tree.GetSelection():
            if not Presenter.applied: Presenter.update()
            Presenter.setData(self.tree.root)

    def OnCollapseAll(self, evt):
        # Prevent multiple calls to setData
        self.tree.Unbind(wx.EVT_TREE_ITEM_COLLAPSED)
        self.tree.UnselectAll()
        self.tree.CollapseAll()
        self.tree.Bind(wx.EVT_TREE_ITEM_COLLAPSED, self.OnTreeItemCollapsed)
        if not Presenter.applied: Presenter.update()
        Presenter.setData(self.tree.root)

    #
    # XMLTree event handlers
    #
    
    def OnTreeLeftDown(self, evt):
        pt = evt.GetPosition();
        item, flags = self.tree.HitTest(pt)
        if flags & wx.TREE_HITTEST_NOWHERE or not item:
            # Unselecting seems to be broken on wxGTK!!!
            Presenter.unselect()
        evt.Skip()

    def OnTreeRightDown(self, evt):
        if wx.Platform == '__WXMAC__':
            forceSibling = evt.AltDown()
        else:
            forceSibling = evt.ControlDown()
        forceInsert = evt.ShiftDown()
        Presenter.popupMenu(forceSibling, forceInsert, evt.GetPosition())

    def OnTreeSelChanging(self, evt):
        #TRACE('OnTreeSelChanging: %s=>%s', evt.GetOldItem(), evt.GetItem())
        #TRACE('Selection: %s', self.tree.GetSelections())
        if not self.tree.GetSelections(): return
        # Permit multiple selection for same level only
        state = wx.GetMouseState()
        oldItem = evt.GetOldItem()
        if oldItem and (state.ShiftDown() or state.ControlDown()) and \
           self.tree.GetItemParent(oldItem) != self.tree.GetItemParent(evt.GetItem()):
            evt.Veto()
            self.frame.SetStatusText('Veto selection (not same level)')
            return
        # If panel has a pending undo, register it
        if Presenter.panelIsDirty():
            Presenter.registerUndoEdit()
        evt.Skip()

    def OnTreeSelChanged(self, evt):
        TRACE('OnTreeSelChanged: %s=>%s', evt.GetOldItem(), evt.GetItem())
        TRACE('Selection: %s', self.tree.GetSelections())
        # On wxMSW (at least) two selection events are generated
        if not self.tree.GetSelections(): return
        if evt.GetOldItem():
            if not Presenter.applied:
                Presenter.update(evt.GetOldItem())
            # Refresh test window after finishing
            if g.conf.autoRefresh and self.testWin.IsDirty():
                wx.CallAfter(Presenter.refreshTestWin)
        # Tell presenter to update current data and view
        item = evt.GetItem()
        if not item: item = self.tree.root
        wx.CallAfter(Presenter.setData, item)
        # Set initial sibling/insert modes
        Presenter.createSibling = not Presenter.comp.isContainer()
        Presenter.insertBefore = False
        evt.Skip()

    def OnTreeItemCollapsed(self, evt):
        # If no selection, reset panel
        if not self.tree.GetSelection():
            if not Presenter.applied: Presenter.update()
            Presenter.setData(self.tree.root)
        evt.Skip()

    def OnPanelPageChanging(self, evt):
        TRACE('OnPanelPageChanging: %d=>%d', evt.GetOldSelection(), evt.GetSelection())
        # Register undo if something was changed
        i = evt.GetOldSelection()
        if i >= 0 and Presenter.panelIsDirty():
            g.undoMan.RegisterUndo(self.panel.undo)
        evt.Skip()

    def OnPanelPageChanged(self, evt):
        TRACE('OnPanelPageChanged: %d=>%d', evt.GetOldSelection(), evt.GetSelection())
        # Register new undo 
        if Presenter.panelIsDirty():
            Presenter.createUndoEdit(page=evt.GetSelection())
        # Refresh test window after finishing
        if g.conf.autoRefresh and self.testWin.IsDirty():
            wx.CallAfter(Presenter.refreshTestWin)
        evt.Skip()

    def OnPanelTogglePin(self, evt):
        g.conf.panelPinState = evt.GetIsDown()
        evt.Skip()

    # Tool panel

    def OnToolPanelPageChanged(self, evt):
        TRACE('OnToolPanelPageChanged: %d > %d', evt.GetOldSelection(), evt.GetSelection())
        # Update tool frame (if exists)
        panel = g.toolPanel.panels[evt.GetSelection()]
        if self.toolFrame:
            self.toolFrame.SetTitle(panel.name)
        evt.Skip()

    def OnComponentTool(self, evt):
        '''Hadnler for creating new elements.'''
        comp = Manager.findById(evt.GetId())
        # Check compatibility
        if Presenter.checkCompatibility(comp):
            state = self.tree.GetFullState() # state just before
            if comp.groups[0] == 'component':
                node = Model.createComponentNode('Component')
                item = Presenter.create(comp, node)
            else:
                item = Presenter.create(comp)
            itemIndex = self.tree.ItemFullIndex(item)
            g.undoMan.RegisterUndo(undo.UndoPasteCreate(itemIndex, state))
        evt.Skip()

    def OnCloseToolFrame(self, evt):
        '''wx.EVT_CLOSE handler'''
        conf = g.conf
        if not self.toolFrame.IsIconized():
            if conf.showToolPanel:
                conf.toolPanelPos = self.toolFrame.GetPosition()
                conf.toolPanelSize = self.toolFrame.GetSize()
        self.toolFrame.Show(False)
        conf.showToolPanel = False

    def OnFrameActivate(self, evt):
        if evt.GetActive():
            TRACE('Setting active frame')
            g.lastActiveFrame = evt.GetEventObject()
        evt.Skip()

        
# Singleton class
Listener = g.Listener = _Listener()
