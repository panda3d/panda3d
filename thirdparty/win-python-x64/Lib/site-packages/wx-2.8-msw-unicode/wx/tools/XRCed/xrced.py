# Name:         xrced.py
# Purpose:      XRC editor, main module
# Author:       Roman Rolinsky <rolinsky@mema.ucl.ac.be>
# Created:      20.08.2001
# RCS-ID:       $Id: xrced.py 64107 2010-04-22 14:05:36Z ROL $

"""

usage: xrced [options] [XRC file]

options:
  --version      show program's version number and exit
  -h, --help     show this help message and exit
  -d, --debug    add Debug command to Help menu
  -m, --meta     activate meta-components
  -v, --verbose  verbose messages

"""

import os
from optparse import OptionParser
from globals import *
import params
from presenter import Presenter
from listener import Listener
from component import Manager
import view
import undo
import plugin

# for helping to attach to the process with gdb...
#print "%s\nPID: %d\n" % (wx.version(), os.getpid()); #raw_input("Press Enter...")

# to enable the widget inspector & its hot-key
USE_INSPECTOR = False

# Parse string in form var1=val1[,var2=val2]* as dictionary
def ReadDictFromString(s):
    d = {}
    for vv in s.split(','):
        var,val = vv.split(':')
        d[var.strip()] = val
    return d

# Transform dictionary with strings into one string
def DictToString(d):
    return ','.join(map(':'.join, d.items()))


if USE_INSPECTOR:
    import wx.lib.mixins.inspection
    AppBase = wx.lib.mixins.inspection.InspectableApp
else:
    AppBase = wx.App
    

class App(AppBase):
    def OnInit(self):
        if USE_INSPECTOR:
            self.Init()

        # Check version
        if wx.VERSION[:3] < MinWxVersion:
            wx.LogWarning('''\
This version of XRCed may not work correctly on your version of wxWidgets. \
Please upgrade wxWidgets to %d.%d.%d or higher.''' % MinWxVersion)

        g.undoMan = undo.UndoManager()
        Manager.init()

        parser = OptionParser(prog=progname, 
                              version='%s version %s' % (ProgName, version),
                              usage='%prog [options] [XRC file]')
        parser.add_option('-d', '--debug', action='store_true',
                          help='add Debug command to Help menu')
        parser.add_option('-m', '--meta', action='store_true',
                          dest = 'meta',
                          help='activate meta-components')
        parser.add_option('-v', '--verbose', action='store_true',
                          help='verbose messages')

        # Process command-line arguments
        options, args = parser.parse_args()
        if options.debug:
            set_debug(True)
        if options.verbose:
            set_verbose(True)
        if options.meta:
            g.useMeta = True
            import meta
            Manager.register(meta.Component)
            Manager.setMenu(meta.Component, 'TOP_LEVEL', 'component', 'component plugin')
            
        self.SetAppName(progname)

        self.ReadConfig()
        
        # Add handlers
        wx.FileSystem.AddHandler(wx.MemoryFSHandler())
        self.toolArtProvider = view.ToolArtProvider()
        wx.ArtProvider.Push(self.toolArtProvider)

        # Load standard plugins first
        plugin.load_plugins(os.path.join(g.basePath, 'plugins'))
        # ... and then from user-defined dirs
        plugin.load_plugins_from_dirs()

        # Setup MVP
        view.create_view()
        Presenter.init()
        Listener.Install(view.frame, view.tree, view.panel,
                         view.toolFrame, view.testWin)

        if args:
            path = args[0]
            # Change current directory
            dir = os.path.dirname(path)
            if dir:
                os.chdir(dir)
                path = os.path.basename(path)
            if os.path.isfile(path):
                Presenter.open(path)
            else:
                # Future name
                Presenter.path = path
                # Force update title
                Presenter.setModified(False)
        view.frame.Show()
        if not g.useAUI:
            if not g.conf.embedPanel:
                view.frame.miniFrame.Show()
            if g.conf.showToolPanel:
                Listener.toolFrame.Show()

        return True

    def OnExit(self):
        self.WriteConfig()

    def ReadConfig(self):
        # Settings
        conf = g.conf = wx.Config(style = wx.CONFIG_USE_LOCAL_FILE)
        conf.localconf = None
        conf.autoRefresh = conf.ReadBool('autoRefresh', True)
        conf.autoRefreshPolicy = conf.ReadInt('autoRefreshPolicy', 0)
        conf.pos = wx.Point(conf.ReadInt('x', -1), conf.ReadInt('y', -1))
        conf.size = wx.Size(conf.ReadInt('width', 800), conf.ReadInt('height', 600))
        g.useAUI = conf.useAUI = conf.ReadBool('useAUI', False)
        conf.embedPanel = conf.ReadBool('embedPanel', True)
        conf.panelPinState = conf.ReadBool('panelPinState', False)
        conf.showToolPanel = conf.ReadBool('showToolPanel', True)
        conf.sashPos = conf.ReadInt('sashPos', 200)

        # read recently used files
        g.fileHistory = wx.FileHistory()
        g.fileHistory.Load(conf)

        conf.panelPos = wx.Point(conf.ReadInt('panelX', -1), 
                                 conf.ReadInt('panelY', -1))
        conf.panelSize = wx.Size(conf.ReadInt('panelWidth', 200),
                                 conf.ReadInt('panelHeight', 200))
        conf.sashPos = conf.ReadInt('sashPos', 200)
        
        conf.toolPanelPos = wx.Point(conf.ReadInt('toolPanelX', -1), 
                                     conf.ReadInt('toolPanelY', -1))
        if wx.Platform == '__WXMAC__':
            conf.toolPanelPos.y += 4  # OSX has some issues with miniframe
        conf.toolPanelSize = wx.Size(conf.ReadInt('toolPanelWidth', -1), 
                                     conf.ReadInt('toolPanelHeight', -1))
        
        # Preferences
        conf.toolPanelType = conf.Read('Prefs/toolPanelType', 'TB')
        conf.toolThumbSize = conf.ReadInt('Prefs/toolThumbSize', 48)
        conf.toolIconScale = conf.ReadInt('Prefs/toolIconScale', 100)
        conf.expandOnOpen = conf.ReadBool('Prefs/expandOnOpen', True)
        conf.fitTestWin = conf.ReadBool('Prefs/fitTestWin', True)
        conf.useSubclassing = conf.ReadBool('Prefs/useSubclassing', False)
        conf.allowExec = conf.Read('Prefs/allowExec', 'ask')
        # Toolbar configuration
        conf.TB_file = conf.ReadBool('Prefs/TB_file', True)
        conf.TB_undo = conf.ReadBool('Prefs/TB_undo', True)
        conf.TB_copy = conf.ReadBool('Prefs/TB_copy', True)
        conf.TB_move = conf.ReadBool('Prefs/TB_move', True)
        p = 'Prefs/Defaults/Container'
        if conf.HasEntry(p):
            conf.defaultsContainer = ReadDictFromString(conf.Read(p))
        else:
            conf.defaultsContainer = {}
        p = 'Prefs/Defaults/Control'
        if conf.HasEntry(p):
            conf.defaultsControl = ReadDictFromString(conf.Read(p))
        else:
            conf.defaultsControl = {}
        conf.SetPath('/')
            
    def WriteConfig(self):
        
        # Write config
        conf = g.conf
        conf.SetPath('/')
        conf.WriteBool('autoRefresh', conf.autoRefresh)
        conf.WriteInt('autoRefreshPolicy', conf.autoRefreshPolicy)
        conf.WriteBool('useAUI', conf.useAUI)
        if g.useAUI:
            conf.Write('perspective', conf.perspective)
        conf.WriteInt('x', conf.pos.x)
        conf.WriteInt('y', conf.pos.y)
        conf.WriteInt('width', conf.size.x)
        conf.WriteInt('height', conf.size.y)
        conf.WriteInt('embedPanel', conf.embedPanel)
        conf.WriteInt('panelPinState', conf.panelPinState)
        conf.WriteInt('showTools', conf.showToolPanel)
        if not conf.embedPanel:
            conf.WriteInt('panelX', conf.panelPos.x)
            conf.WriteInt('panelY', conf.panelPos.y)
        conf.WriteInt('sashPos', conf.sashPos)
        conf.WriteInt('panelWidth', conf.panelSize.x)
        conf.WriteInt('panelHeight', conf.panelSize.y)
        conf.WriteInt('showToolPanel', conf.showToolPanel)
        conf.WriteInt('toolPanelX', conf.toolPanelPos.x)
        conf.WriteInt('toolPanelY', conf.toolPanelPos.y)
        conf.WriteInt('toolPanelWidth', conf.toolPanelSize.x)
        conf.WriteInt('toolPanelHeight', conf.toolPanelSize.y)
        g.fileHistory.Save(conf)
        # Preferences
        conf.DeleteGroup('Prefs')
        conf.Write('Prefs/toolPanelType', conf.toolPanelType)
        conf.WriteInt('Prefs/toolThumbSize', conf.toolThumbSize)
        conf.WriteInt('Prefs/toolIconScale', conf.toolIconScale)
        conf.WriteInt('Prefs/expandOnOpen', conf.expandOnOpen)
        conf.WriteInt('Prefs/fitTestWin', conf.fitTestWin)
        conf.WriteInt('Prefs/TB_file', conf.TB_file)
        conf.WriteInt('Prefs/TB_undo', conf.TB_undo)
        conf.WriteInt('Prefs/TB_copy', conf.TB_copy)
        conf.WriteInt('Prefs/TB_move', conf.TB_move)
        conf.WriteInt('Prefs/useSubclassing', conf.useSubclassing)
        conf.Write('Prefs/allowExec', conf.allowExec)
        v = conf.defaultsContainer
        if v: conf.Write('Prefs/Defaults/Container', DictToString(v))
        v = conf.defaultsControl
        if v: conf.Write('Prefs/Defaults/Control', DictToString(v))
        
        conf.Flush()

def main():
    if 'wxMac' in wx.PlatformInfo:
        # if running from an App bundle on Mac then there could be an
        # unexpected -psb_blahblah command-line arg.  Blow it away.
        for idx,arg in enumerate(sys.argv):
            if arg.startswith('-psn'):
                del sys.argv[idx]
                break
            
    app = App(0, useBestVisual=False)
    #app.SetAssertMode(wx.PYAPP_ASSERT_LOG)
    app.MainLoop()

if __name__ == '__main__':
    # Substitute wx.tools.XRCed with local module
    try:
        from XRCed.xrced import main
    except ImportError:
        print >>sys.stderr, 'XRCed parent directory must be in PYTHONPATH for local running'
        raise
    sys.modules['wx.tools.XRCed'] = sys.modules['XRCed']
    main()
