# Name:         view.py
# Purpose:      View classes
# Author:       Roman Rolinsky <rolinsky@femagsoft.com>
# Created:      07.06.2007
# RCS-ID:       $Id: view.py 64107 2010-04-22 14:05:36Z ROL $

import os
from XMLTree import XMLTree
from XMLTreeMenu import XMLTreeMenu
from AttributePanel import Panel, AttributePanel
from TestWin import TestWindow
from tools import *
import images
if wx.Platform == '__WXMAC__':
    # Substitute higher-res icons for Mac
    import images_32x32
    images.__dict__.update(images_32x32.__dict__)
import wx.aui
import wx.html

def create_view():
    '''
    Create all necessary view objects. Some of them are set as module
    global variables for convenience.
    '''

    # Load resources
    res = xrc.EmptyXmlResource()
    res.Load(os.path.join(g.basePath, 'xrced.xrc'))
    g.res = res

    global frame
    frame = Frame(g.conf.pos, g.conf.size)

    global toolFrame
    toolFrame = create_tools()

    global testWin
    testWin = TestWindow()

def create_tools():
    if g.useAUI:
        g.toolPanel = ToolPanel(frame)
        frame.mgr.AddPane(g.toolPanel, wx.aui.AuiPaneInfo().Name('tools').Caption("Tools").
                          MinSize(g.toolPanel.GetBestSize()).
                          Right().CloseButton(True).MaximizeButton(False))
        frame.mgr.Update()
        if g.conf.HasEntry('perspective'):
            frame.mgr.LoadPerspective(g.conf.Read('perspective'))
        return None
    else:
        # Tool panel on a MiniFrame
        toolFrame = wx.MiniFrame(frame, -1, 'Components', 
                                 g.conf.toolPanelPos,
                                 style=wx.CAPTION|wx.CLOSE_BOX|wx.RESIZE_BORDER)
                                     # This causes hiding on KDE
                                     # |wx.FRAME_TOOL_WINDOW)
        if wx.Platform != '__WXMAC__':
            toolFrame.SetIcons(frame.icns)
        g.toolPanel = toolFrame.panel = ToolPanel(toolFrame)
        if toolFrame.panel.panels:
            toolFrame.SetTitle(toolFrame.panel.panels[0].name)
        toolFrame.Fit()
        minSize = toolFrame.GetSize()
        if minSize[0] < 320: minSize[0] = 320
        toolFrame.SetMinSize(minSize)
        #toolFrame.SetPosition(g.conf.toolPanelPos)
        toolFrame.SetSize(g.conf.toolPanelSize)
        return toolFrame

#############################################################################

class Frame(wx.Frame):
    def __init__(self, pos, size):
        wx.Frame.__init__(self, None, -1, 'XRCed', pos, size)
        bar = self.CreateStatusBar(2)
        bar.SetStatusWidths([-1, 40])
        if wx.Platform != '__WXMAC__':
            self.icns = wx.IconBundleFromIcon(images.Icon.GetIcon())
            self.SetIcons(self.icns)

        self.InitMenuBar()

        self.ID_TOOL_PASTE = wx.NewId()
        self.ID_TOOL_LOCATE = wx.NewId()

        # Init HTML help
        wx.FileSystem.AddHandler(wx.ZipFSHandler())
        self.htmlCtrl = wx.html.HtmlHelpController()
        programPath = os.path.dirname(__file__)
        if not (self.htmlCtrl.AddBook(os.path.join(programPath, "xrced.htb"))) :
            print >> sys.stderr, "Cannot load help file \"xrced.htb\""
            self.GetMenuBar().Enable(wx.ID_HELP_CONTENTS, False)        

        # Create toolbar
        self.tb = tb = wx.ToolBar(self, -1, style=wx.TB_FLAT | wx.TB_NODIVIDER)
        # Use bigger icon size on Mac
        if wx.Platform == '__WXMAC__':
            tb.SetToolBitmapSize((32,32))
        elif wx.Platform == '__WXMSW__':
            tb.SetToolBitmapSize((22,22))

        self.InitToolBar(g.useAUI or g.conf.embedPanel) # add tools

        if g.useAUI:
            self.mgr = wx.aui.AuiManager()
            self.mgr.SetManagedWindow(self)
            parent = self
        else:
            splitter = wx.SplitterWindow(self, -1, style=wx.SP_3DSASH)
            self.splitter = splitter
            splitter.SetMinimumPaneSize(100)
            parent = splitter

        # always use the native toolbar on Mac, it looks too sucky otherwise
        if g.useAUI and wx.Platform != '__WXMAC__':  
            self.mgr.AddPane(tb, wx.aui.AuiPaneInfo().
                             Name("tb").Caption("Toolbar").
                             ToolbarPane().Top().LeftDockable(False).RightDockable(False))
        else:
            self.SetToolBar(tb)


        global tree
        tree = XMLTree(parent)

        global panel
        if g.useAUI:
            self.mgr.AddPane(tree, wx.aui.AuiPaneInfo().
                             Name("tree").Caption("XML Tree").
                             CenterPane())

            panel = Panel(self)
            self.mgr.AddPane(panel, wx.aui.AuiPaneInfo().
                             Name("attrpanel").Caption("Attributes").
                             MinSize(panel.GetBestSize()).
                             Right().CloseButton(True).MaximizeButton(False))

            self.miniFrame = None
        else:

            # Miniframe for split mode
            self.miniFrame = mf = wx.MiniFrame(self, -1, 'Attributes',
                                               g.conf.panelPos, g.conf.panelSize,
                                               style=wx.CAPTION|wx.RESIZE_BORDER)
                                               # This causes hiding on KDE
                                               # |wx.FRAME_TOOL_WINDOW)
            if wx.Platform != '__WXMAC__':
                mf.SetIcons(self.icns)
            mf.tb = mf.CreateToolBar(wx.TB_HORIZONTAL | wx.NO_BORDER | wx.TB_FLAT)

            # Use bigger icon size on Mac
            if wx.Platform == '__WXMAC__':
                mf.tb.SetToolBitmapSize((32,32))
            elif wx.Platform == '__WXMSW__':
                mf.tb.SetToolBitmapSize((22,22))
            self.InitMiniFrameToolBar(mf.tb)

            mfSizer = wx.BoxSizer()
            mf.SetMinSize((100,100))
            mf.SetSizer(mfSizer)
            if wx.Platform == '__WXMAC__': # mac don't respect pos and size exactly
                mf.SetPosition(g.conf.panelPos)
                mf.SetSize(g.conf.panelSize)

            # Create attribute panel
            if g.conf.embedPanel:
                panel = Panel(splitter)
                # Set plitter windows
                splitter.SplitVertically(tree, panel, g.conf.sashPos)
            else:
                panel = Panel(mf)
                mfSizer.Add(panel, 1, wx.EXPAND)
                splitter.Initialize(tree)
            
        if wx.Platform == '__WXMAC__':
            self.SetClientSize(size)

    def Clear(self):
        pass

    def InitMenuBar(self):
        # Make menus
        menuBar = wx.MenuBar()

        menu = wx.Menu()                # File menu
        menu.Append(wx.ID_NEW, '&New\tCtrl-N', 'New file')
        menu.AppendSeparator()
        menu.Append(wx.ID_OPEN, '&Open...\tCtrl-O', 'Open XRC file')
        
        self.recentMenu = wx.Menu()
        g.fileHistory.UseMenu(self.recentMenu)
        g.fileHistory.AddFilesToMenu()

        menu.AppendMenu(-1, 'Open &Recent', self.recentMenu, 'Open a recent file')
        
        menu.AppendSeparator()
        menu.Append(wx.ID_SAVE, '&Save\tCtrl-S', 'Save XRC file')
        menu.Append(wx.ID_SAVEAS, 'Save &As...', 'Save XRC file under different name')
        self.ID_GENERATE_PYTHON = wx.NewId()
        menu.Append(self.ID_GENERATE_PYTHON, '&Generate Python...', 
                    'Generate a Python module that uses this XRC')
        menu.AppendSeparator()
        menu.Append(wx.ID_PREFERENCES, 'Preferences...', 'Change XRCed settings')
        menu.AppendSeparator()
        menu.Append(wx.ID_EXIT, '&Quit\tCtrl-Q', 'Exit application')

        menuBar.Append(menu, '&File')

        menu = wx.Menu()                # Edit menu
        menu.Append(wx.ID_UNDO, '&Undo\tCtrl-Z', 'Undo')
        menu.Append(wx.ID_REDO, '&Redo\tCtrl-Y', 'Redo')
        menu.AppendSeparator()
        menu.Append(wx.ID_CUT, 'Cut\tCtrl-X', 'Cut to the clipboard')
        menu.Append(wx.ID_COPY, '&Copy\tCtrl-C', 'Copy to the clipboard')
        menu.Append(ID.PASTE, '&Paste\tCtrl-V', 'Paste from the clipboard')
        menu.Append(ID.PASTE_SIBLING, '&Paste Sibling\tAlt-Ctrl-V', 
                    'Paste clipboard as a sibling')
        menu.Append(wx.ID_DELETE, '&Delete\tCtrl-D', 'Delete object')
        menu.AppendSeparator()
        self.ID_UNSELECT = wx.NewId()
        menu.Append(self.ID_UNSELECT, '&Unselect All', 'Clear tree selection')
        menu.Append(ID.COLLAPSE_ALL, '&Collapse All', 'Collapse tree')
        menu.AppendSeparator()
        menu.Append(wx.ID_FIND, '&Find\tCtrl-F', 'Find a named control')
        self.ID_FINDAGAIN = wx.NewId()
        menu.Append(self.ID_FINDAGAIN, 'Find A&gain\tCtrl-G', 'Repeat last search')
        self.ART_LOCATE = 'ART_LOCATE'
        self.ID_LOCATE = wx.NewId()
        menu.Append(self.ID_LOCATE, '&Locate\tCtrl-L', 'Locate control in test window and select it')
        
        menuBar.Append(menu, '&Edit')
        
        menu = wx.Menu()                # View menu
        self.ID_EMBED_PANEL = wx.NewId()
        menu.Append(self.ID_EMBED_PANEL, '&Embed Panel',
                    'Toggle embedding properties panel in the main window', True)
        menu.Check(self.ID_EMBED_PANEL, g.conf.embedPanel)
        self.ID_SHOW_TOOLS = wx.NewId()
        menu.Append(self.ID_SHOW_TOOLS, 'Show &Components', 'Show components')
        menu.AppendSeparator()
        self.ID_TEST = wx.NewId()
        self.ART_TEST = 'ART_TEST'
        menu.Append(self.ID_TEST, '&Test\tF5', 'Show test window')
        self.ID_TEST_HIDE = wx.NewId()
        menu.Append(self.ID_TEST_HIDE, '&End testing\tF6', 'Close test window')
        self.ART_REFRESH = 'ART_REFRESH'
        menu.Append(wx.ID_REFRESH, '&Refresh\tCtrl-R', 'Refresh test window')
        self.ID_AUTO_REFRESH = wx.NewId()
        self.ART_AUTO_REFRESH = 'ART_AUTO_REFRESH'
        menu.Append(self.ID_AUTO_REFRESH, '&Auto-refresh\tAlt-A',
                    'Toggle auto-refresh mode', True)
        menu.Check(self.ID_AUTO_REFRESH, g.conf.autoRefresh)
        menu.AppendSeparator()
        self.ID_SHOW_XML = wx.NewId()
        menu.Append(self.ID_SHOW_XML, 'Show &XML...', 
                    'Display XML source for the selected subtree')
        
        menuBar.Append(menu, '&View')

        menu = wx.Menu()                # Move menu
        self.ID_MOVEUP = wx.NewId()
        self.ART_MOVEUP = 'ART_MOVEUP'
        menu.Append(self.ID_MOVEUP, '&Up', 'Move before previous sibling')
        self.ID_MOVEDOWN = wx.NewId()
        self.ART_MOVEDOWN = 'ART_MOVEDOWN'
        menu.Append(self.ID_MOVEDOWN, '&Down', 'Move after next sibling')
        self.ID_MOVELEFT = wx.NewId()
        self.ART_MOVELEFT = 'ART_MOVELEFT'
        menu.Append(self.ID_MOVELEFT, '&Make sibling', 'Make sibling of parent')
        self.ID_MOVERIGHT = wx.NewId()
        self.ART_MOVERIGHT = 'ART_MOVERIGHT'
        menu.Append(self.ID_MOVERIGHT, '&Make child', 'Make child of previous sibling')
        
        menuBar.Append(menu, '&Move')

        menu = wx.Menu()                # Help menu
        menu.Append(wx.ID_HELP_CONTENTS, '&Contents...', 'On-line help contents')
        menu.Append(wx.ID_ABOUT, '&About...', 'About XCRed')
        self.ID_README = wx.NewId()
        menu.Append(self.ID_README, '&Readme...\tF1', 'View the README file')
        if get_debug():
            self.ID_DEBUG_CMD = wx.NewId()
            menu.Append(self.ID_DEBUG_CMD, 'CMD', 'Python command line')
            
        menuBar.Append(menu, '&Help')

        self.menuBar = menuBar
        self.SetMenuBar(menuBar)

    def InitToolBar(self, long):
        '''Initialize toolbar, long is boolean.'''
        tb = self.tb
        tb.ClearTools()
        new_bmp  = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE, wx.ART_TOOLBAR)
        open_bmp = wx.ArtProvider.GetBitmap(wx.ART_FILE_OPEN, wx.ART_TOOLBAR)
        save_bmp = wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE, wx.ART_TOOLBAR)
        undo_bmp = wx.ArtProvider.GetBitmap(wx.ART_UNDO, wx.ART_TOOLBAR)
        redo_bmp = wx.ArtProvider.GetBitmap(wx.ART_REDO, wx.ART_TOOLBAR)
        cut_bmp  = wx.ArtProvider.GetBitmap(wx.ART_CUT, wx.ART_TOOLBAR)
        copy_bmp = wx.ArtProvider.GetBitmap(wx.ART_COPY, wx.ART_TOOLBAR)
        paste_bmp= wx.ArtProvider.GetBitmap(wx.ART_PASTE, wx.ART_TOOLBAR)
        if g.conf.TB_file:
            tb.AddSimpleTool(wx.ID_NEW, new_bmp, 'New', 'New file')
            tb.AddSimpleTool(wx.ID_OPEN, open_bmp, 'Open', 'Open file')
            tb.AddSimpleTool(wx.ID_SAVE, save_bmp, 'Save', 'Save file')
            tb.AddSeparator()
        if g.conf.TB_undo:
            tb.AddSimpleTool(wx.ID_UNDO, undo_bmp, 'Undo', 'Undo')
            tb.AddSimpleTool(wx.ID_REDO, redo_bmp, 'Redo', 'Redo')
            tb.AddSeparator()
        if g.conf.TB_copy:
            tb.AddSimpleTool(wx.ID_CUT, cut_bmp, 'Cut', 'Cut')
            tb.AddSimpleTool(wx.ID_COPY, copy_bmp, 'Copy', 'Copy')
            tb.AddSimpleTool(self.ID_TOOL_PASTE, paste_bmp, 'Paste', 'Paste')
            tb.AddSeparator()
        if g.conf.TB_move:
            bmp = wx.ArtProvider.GetBitmap(self.ART_MOVEUP, wx.ART_TOOLBAR)
            tb.AddSimpleTool(self.ID_MOVEUP, bmp,
                             'Up', 'Move before previous sibling')
            bmp = wx.ArtProvider.GetBitmap(self.ART_MOVEDOWN, wx.ART_TOOLBAR)
            tb.AddSimpleTool(self.ID_MOVEDOWN, bmp,
                             'Down', 'Move after next sibling')
            bmp = wx.ArtProvider.GetBitmap(self.ART_MOVELEFT, wx.ART_TOOLBAR)
            tb.AddSimpleTool(self.ID_MOVELEFT, bmp,
                             'Make Sibling', 'Make sibling of parent')
            bmp = wx.ArtProvider.GetBitmap(self.ART_MOVERIGHT, wx.ART_TOOLBAR)
            tb.AddSimpleTool(self.ID_MOVERIGHT, bmp,
                             'Make Child', 'Make child of previous sibling')
        if long:
            tb.AddSeparator()
            bmp = wx.ArtProvider.GetBitmap(self.ART_LOCATE, wx.ART_TOOLBAR)
            tb.AddSimpleTool(self.ID_TOOL_LOCATE, bmp,
                             'Locate', 'Locate control in test window and select it', True)
            bmp = wx.ArtProvider.GetBitmap(self.ART_TEST, wx.ART_TOOLBAR)
            tb.AddSimpleTool(self.ID_TEST, bmp, 'Test', 'Test window')
            bmp = wx.ArtProvider.GetBitmap(self.ART_REFRESH, wx.ART_TOOLBAR)
            tb.AddSimpleTool(wx.ID_REFRESH, bmp, 'Refresh', 'Refresh view')
            bmp = wx.ArtProvider.GetBitmap(self.ART_AUTO_REFRESH, wx.ART_TOOLBAR)
            tb.AddSimpleTool(self.ID_AUTO_REFRESH, bmp,
                             'Auto-refresh', 'Toggle auto-refresh mode', True)
            tb.ToggleTool(self.ID_AUTO_REFRESH, g.conf.autoRefresh)
        tb.Realize()
        self.minWidth = tb.GetSize()[0] # minimal width is the size of toolbar 

    def InitMiniFrameToolBar(self, tb):
        bmp = wx.ArtProvider.GetBitmap(self.ART_LOCATE, wx.ART_TOOLBAR)
        tb.AddSimpleTool(self.ID_TOOL_LOCATE, bmp,
                         'Locate', 'Locate control in test window and select it', True)
        bmp = wx.ArtProvider.GetBitmap(self.ART_TEST, wx.ART_TOOLBAR)
        tb.AddSimpleTool(self.ID_TEST, bmp, 'Test', 'Test window')
        bmp = wx.ArtProvider.GetBitmap(self.ART_REFRESH, wx.ART_TOOLBAR)
        tb.AddSimpleTool(wx.ID_REFRESH, bmp, 'Refresh', 'Refresh view')
        bmp = wx.ArtProvider.GetBitmap(self.ART_AUTO_REFRESH, wx.ART_TOOLBAR)
        tb.AddSimpleTool(self.ID_AUTO_REFRESH, bmp,
                         'Auto-refresh', 'Toggle auto-refresh mode', True)
        tb.ToggleTool(self.ID_AUTO_REFRESH, g.conf.autoRefresh)
        tb.Realize() 

    def EmbedUnembed(self, embedPanel):
        conf = g.conf
        conf.embedPanel = embedPanel
        if conf.embedPanel:
            # Remember last dimentions
            conf.panelPos = self.miniFrame.GetPosition()
            conf.panelSize = self.miniFrame.GetSize()
            conf.size = self.GetSize()
            conf.sashPos = self.GetClientSize()[0]
            panelWidth = panel.GetSize()[0] # client size
            panel.Reparent(self.splitter)
            self.miniFrame.GetSizer().Remove(panel)
            # Widen
            conf.size.width += panelWidth
            self.SetSize(conf.size)
            self.splitter.SplitVertically(tree, panel, conf.sashPos)
            self.miniFrame.Show(False)
        else:
            conf.sashPos = self.splitter.GetSashPosition()
            conf.size = self.GetSize()
            conf.panelSize[0] = panel.GetSize()[0]
            self.splitter.Unsplit(panel)
            sizer = self.miniFrame.GetSizer()
            panel.Reparent(self.miniFrame)
            panel.Show(True)
            sizer.Add(panel, 1, wx.EXPAND)
            self.miniFrame.Show(True)
            self.miniFrame.SetPosition(conf.panelPos)
            self.miniFrame.SetSize(conf.panelSize)
            self.miniFrame.Layout()
            # Reduce width
            conf.size.width -= conf.panelSize.width
            self.SetSize(conf.size)
        
        # Set long or short toolbar
        self.InitToolBar(embedPanel)

    def ShowReadme(self):
        text = open(os.path.join(g.basePath, 'README.txt'), 'r').read()
        dlg = ScrolledMessageDialog(frame, text, "XRCed README")
        dlg.ShowModal()
        dlg.Destroy()

    def ShowPrefs(self):
        dlg = PrefsDialog(self)
        conf = g.conf
        toolPanelType = conf.toolPanelType
        if dlg.ShowModal() == wx.ID_OK:
            # Fetch new preferences
            for id,cdp in dlg.checkControls.items():
                c,d,p = cdp
                if dlg.FindWindowById(id).IsChecked():
                    d[p] = str(c.GetValue())
                elif p in d: del d[p]
            conf.useAUI = dlg.check_AUI.GetValue()
            conf.toolPanelType = ['TB','FPB'][dlg.radio_toolPanelType.GetSelection()]
            conf.toolThumbSize = dlg.slider_thumbSize.GetValue()
            conf.toolIconScale = dlg.slider_iconScale.GetValue()
            conf.expandOnOpen = dlg.check_expandOnOpen.GetValue()
            conf.fitTestWin = dlg.check_fitTestWin.GetValue()
            conf.autoRefreshPolicy = dlg.radio_autoRefreshPolicy.GetSelection()
            conf.TB_file = dlg.check_TB_file.GetValue()
            conf.TB_undo = dlg.check_TB_undo.GetValue()
            conf.TB_copy = dlg.check_TB_copy.GetValue()
            conf.TB_move = dlg.check_TB_move.GetValue()
            conf.useSubclassing = dlg.check_useSubclassing.GetValue()
            conf.allowExec = ('ask', 'yes', 'no')[dlg.radio_allowExec.GetSelection()]
            wx.LogMessage('Restart may be needed for some settings to take effect.')
        dlg.Destroy()

#############################################################################

# ScrolledMessageDialog - modified from wxPython lib to set fixed-width font
class ScrolledMessageDialog(wx.Dialog):
    def __init__(self, parent, msg, caption, textSize=(80,40), centered=True):
        from wx.lib.layoutf import Layoutf
        wx.Dialog.__init__(self, parent, -1, caption)
        text = wx.TextCtrl(self, -1, msg, wx.DefaultPosition,
                             wx.DefaultSize, wx.TE_MULTILINE | wx.TE_READONLY)
        text.SetFont(g.modernFont())
        dc = wx.WindowDC(text)
        w, h = dc.GetFullTextExtent(' ', g.modernFont())[:2]
        ok = wx.Button(self, wx.ID_OK, "OK")
        ok.SetDefault()
        text.SetConstraints(Layoutf('t=t5#1;b=t5#2;l=l5#1;r=r5#1', (self,ok)))
        text.SetSize((w * textSize[0] + 30, h * textSize[1]))
        text.ShowPosition(1)            # scroll to the first line
        ok.SetConstraints(Layoutf('b=b5#1;x%w50#1;w!80;h!35', (self,)))
        self.SetAutoLayout(True)
        self.Fit()
        if centered:
            self.CenterOnScreen(wx.BOTH)

################################################################################

class PrefsDialog(wx.Dialog): #(wx.PropertySheetDialog): !!! not wrapper yed - wrap by hand

    def __init__(self, parent):
        pre = g.res.LoadObject(parent, 'DIALOG_PREFS', 'wxPropertySheetDialog')
        self.PostCreate(pre)
        
        self.Fit()
        
        self.checkControls = {} # map of check IDs to (control,dict,param)
        conf = g.conf

        # Defaults

        self.check_proportionContainer = xrc.XRCCTRL(self, 'check_proportionContainer')
        id = self.check_proportionContainer.GetId()
        wx.EVT_CHECKBOX(self, id, self.OnCheck)
        self.checkControls[id] = (xrc.XRCCTRL(self, 'spin_proportionContainer'),
                                  conf.defaultsContainer, 'option')

        self.check_flagContainer = xrc.XRCCTRL(self, 'check_flagContainer')
        id = self.check_flagContainer.GetId()
        wx.EVT_CHECKBOX(self, id, self.OnCheck)
        self.checkControls[id] = (xrc.XRCCTRL(self, 'text_flagContainer'),
                                  conf.defaultsContainer, 'flag')

        self.check_proportionControl = xrc.XRCCTRL(self, 'check_proportionControl')
        id = self.check_proportionControl.GetId()
        wx.EVT_CHECKBOX(self, id, self.OnCheck)
        self.checkControls[id] = (xrc.XRCCTRL(self, 'spin_proportionControl'),
                                  conf.defaultsControl, 'option')

        self.check_flagControl = xrc.XRCCTRL(self, 'check_flagControl')
        id = self.check_flagControl.GetId()
        wx.EVT_CHECKBOX(self, id, self.OnCheck)
        self.checkControls[id] = (xrc.XRCCTRL(self, 'text_flagControl'),
                                  conf.defaultsControl, 'flag')

        for id,cdp in self.checkControls.items():
            c,d,p = cdp
            try:
                if isinstance(c, wx.SpinCtrl):
                    c.SetValue(int(d[p]))
                else:
                    c.SetValue(d[p])
                self.FindWindowById(id).SetValue(True)
            except KeyError:
                c.Enable(False)

        # Appearance

        self.check_AUI = xrc.XRCCTRL(self, 'check_AUI')
        self.check_AUI.SetValue(conf.useAUI)
        self.radio_toolPanelType = xrc.XRCCTRL(self, 'radio_toolPanelType')
        self.radio_toolPanelType.SetSelection(['TB','FPB'].index(conf.toolPanelType))
        self.slider_thumbSize = xrc.XRCCTRL(self, 'slider_thumbSize')
        self.slider_thumbSize.SetValue(conf.toolThumbSize)
        self.slider_iconScale = xrc.XRCCTRL(self, 'slider_iconScale')
        self.slider_iconScale.SetValue(conf.toolIconScale)

        self.check_expandOnOpen = xrc.XRCCTRL(self, 'check_expandOnOpen')
        self.check_expandOnOpen.SetValue(conf.expandOnOpen)
        self.check_fitTestWin = xrc.XRCCTRL(self, 'check_fitTestWin')
        self.check_fitTestWin.SetValue(conf.fitTestWin)
        self.radio_autoRefreshPolicy = xrc.XRCCTRL(self, 'radio_autoRefreshPolicy')
        self.radio_autoRefreshPolicy.SetSelection(conf.autoRefreshPolicy)

        self.check_TB_file = xrc.XRCCTRL(self, 'check_TB_file')
        self.check_TB_file.SetValue(conf.TB_file)
        self.check_TB_undo = xrc.XRCCTRL(self, 'check_TB_undo')
        self.check_TB_undo.SetValue(conf.TB_undo)
        self.check_TB_copy = xrc.XRCCTRL(self, 'check_TB_copy')
        self.check_TB_copy.SetValue(conf.TB_copy)
        self.check_TB_move = xrc.XRCCTRL(self, 'check_TB_move')
        self.check_TB_move.SetValue(conf.TB_move)

        self.check_useSubclassing = xrc.XRCCTRL(self, 'check_useSubclassing')
        self.check_useSubclassing.SetValue(conf.useSubclassing)

        self.radio_allowExec = xrc.XRCCTRL(self, 'radio_allowExec')
        try:
            index = {'ask': 0, 'yes':1, 'no': 2}[g.conf.allowExec]
        except KeyError:
            index = 0
        self.radio_allowExec.SetSelection(index)

    def OnCheck(self, evt):
        self.checkControls[evt.GetId()][0].Enable(evt.IsChecked())
        evt.Skip()

#############################################################################

# ArtProvider for toolbar icons
class ToolArtProvider(wx.ArtProvider):
    def __init__(self):
        wx.ArtProvider.__init__(self)
        self.images = {
            'ART_LOCATE': images.Locate.GetImage(),
            'ART_TEST': images.Test.GetImage(),
            'ART_REFRESH': images.Refresh.GetImage(),
            'ART_AUTO_REFRESH': images.AutoRefresh.GetImage(),
            'ART_MOVEUP': images.MoveUp.GetImage(),
            'ART_MOVEDOWN': images.MoveDown.GetImage(),
            'ART_MOVELEFT': images.MoveLeft.GetImage(),
            'ART_MOVERIGHT': images.MoveRight.GetImage(),
            'ART_REMOVE': images.Remove.GetImage()
            }
        if wx.Platform in ['__WXMAC__', '__WXMSW__']:
            self.images.update({
                    wx.ART_NORMAL_FILE: images.New.GetImage(),
                    wx.ART_FILE_OPEN: images.Open.GetImage(),
                    wx.ART_FILE_SAVE: images.Save.GetImage(),
                    wx.ART_UNDO: images.Undo.GetImage(),
                    wx.ART_REDO: images.Redo.GetImage(),
                    wx.ART_CUT: images.Cut.GetImage(),
                    wx.ART_COPY: images.Copy.GetImage(),
                    wx.ART_PASTE: images.Paste.GetImage()
                    })

    def CreateBitmap(self, id, client, size):
        bmp = wx.NullBitmap
        if id in self.images:
            bmp = wx.BitmapFromImage(self.images[id])
        return bmp

