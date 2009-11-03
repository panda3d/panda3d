import wx
import os
from wx.lib.agw import fourwaysplitter as FWS

from pandac.PandaModules import *
from direct.wxwidgets.WxAppShell import *

from ViewPort import *
from ObjectPaletteUI import *
from ObjectPropertyUI import *
from SceneGraphUI import *

class PandaTextDropTarget(wx.TextDropTarget):
    def __init__(self, editor):
        wx.TextDropTarget.__init__(self)
        self.editor = editor

    def OnDropText(self, x, y, text):
        self.editor.objectMgr.addNewObject(text)

class LevelEditorUI(WxAppShell):
    """ Class for Panda3D LevelEditor """ 
    frameWidth = 800
    frameHeight = 600
    appversion      = '0.1'
    appname         = 'Panda3D Level Editor'
    
    def __init__(self, editor, *args, **kw):
        # Create the Wx app
        self.wxApp = wx.App(redirect = False)
        self.wxApp.SetAppName("Panda3D LevelEditor")
        self.wxApp.SetClassName("P3DLevelEditor")
        self.editor = editor

        if not kw.get('size'):
            kw['size'] = wx.Size(self.frameWidth, self.frameHeight)
        WxAppShell.__init__(self, *args, **kw)        

        self.initialize()
        self.Bind(wx.EVT_SET_FOCUS, self.onSetFocus)
        self.Bind(wx.EVT_KEY_DOWN, self.onSetFocus)

    def createMenu(self):
        menuItem = self.menuFile.Insert(0, -1 , "&New")
        self.Bind(wx.EVT_MENU, self.onNew, menuItem)
        
        menuItem = self.menuFile.Insert(1, -1 , "&Load")
        self.Bind(wx.EVT_MENU, self.onLoad, menuItem)

        menuItem = self.menuFile.Insert(2, -1 , "&Save")
        self.Bind(wx.EVT_MENU, self.onSave, menuItem)

        menuItem = self.menuFile.Insert(3, -1 , "Save &As")
        self.Bind(wx.EVT_MENU, self.onSaveAs, menuItem)

        self.menuEdit = wx.Menu()
        self.menuBar.Insert(1, self.menuEdit, "&Edit")

        menuItem = self.menuEdit.Append(-1, "&Duplicate")
        self.Bind(wx.EVT_MENU, self.onDuplicate, menuItem)

        self.gridSnapMenuItem = self.menuEdit.Append(-1, "&Grid Snap", kind = wx.ITEM_CHECK)
        self.Bind(wx.EVT_MENU, self.toggleGridSnap, self.gridSnapMenuItem)

        self.menuOptions = wx.Menu()
        self.menuBar.Insert(2, self.menuOptions, "&Options")

        self.gridSizeMenuItem = self.menuOptions.Append(-1, "&Grid Size ")
        self.Bind(wx.EVT_MENU, self.onGridSize, self.gridSizeMenuItem)
        
    def createInterface(self):
        self.createMenu()
        
        self.mainFrame = wx.SplitterWindow(self, style = wx.SP_3D | wx.SP_BORDER)
        self.leftFrame = wx.SplitterWindow(self.mainFrame, style = wx.SP_3D | wx.SP_BORDER)
        self.baseFrame = wx.SplitterWindow(self.mainFrame, style = wx.SP_3D | wx.SP_BORDER)
        self.viewFrame = FWS.FourWaySplitter(self.baseFrame, style=wx.SP_LIVE_UPDATE)
        self.rightFrame = wx.SplitterWindow(self.baseFrame, style = wx.SP_3D | wx.SP_BORDER)

        self.topView = Viewport.makeTop(self.viewFrame)
        self.viewFrame.AppendWindow(self.topView)

        self.frontView = Viewport.makeFront(self.viewFrame)
        self.viewFrame.AppendWindow(self.frontView)

        self.leftView = Viewport.makeLeft(self.viewFrame)
        self.viewFrame.AppendWindow(self.leftView)

        self.perspView = Viewport.makePerspective(self.viewFrame)
        self.viewFrame.AppendWindow(self.perspView)

        self.leftBarUpPane = wx.Panel(self.leftFrame)
        self.leftBarDownPane = wx.Panel(self.leftFrame)
        self.rightBarUpPane = wx.Panel(self.rightFrame)
        self.rightBarDownPane = wx.Panel(self.rightFrame)

        self.leftFrame.SplitHorizontally(self.leftBarUpPane, self.leftBarDownPane)
        self.rightFrame.SplitHorizontally(self.rightBarUpPane, self.rightBarDownPane)
        self.mainFrame.SplitVertically(self.leftFrame, self.baseFrame, 200)
        self.baseFrame.SplitVertically(self.viewFrame, self.rightFrame, 600)
        
        self.viewFrame.SetDropTarget(PandaTextDropTarget(self.editor))

        self.leftFrame.SetSashGravity(0.5)
        self.rightFrame.SetSashGravity(0.5)        
        self.baseFrame.SetSashGravity(1.0)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.mainFrame, 1, wx.EXPAND, 0)
        self.SetSizer(sizer); self.Layout()

        self.objectPaletteUI = ObjectPaletteUI(self.leftBarUpPane, self.editor)
        self.objectPropertyUI = ObjectPropertyUI(self.rightBarUpPane, self.editor)
        self.sceneGraphUI = SceneGraphUI(self.leftBarDownPane, self.editor)

    def onSetFocus(self):
        print 'wx got focus'

    def appInit(self):
        """Overridden from WxAppShell.py."""
        # Create a new event loop (to overide default wxEventLoop)
        self.evtLoop = wx.EventLoop()
        self.oldLoop = wx.EventLoop.GetActive()
        wx.EventLoop.SetActive(self.evtLoop)
        taskMgr.add(self.wxStep, "evtLoopTask")

    def initialize(self):
        """Initializes the viewports and editor."""
        self.Update()
        ViewportManager.updateAll()
        self.wxStep()
        ViewportManager.initializeAll()
        # Position the camera
        if base.trackball != None:
          base.trackball.node().setPos(0, 30, 0)
          base.trackball.node().setHpr(0, 15, 0)

    def wxStep(self, task = None):
        """A step in the WX event loop. You can either call this yourself or use as task."""
        while self.evtLoop.Pending():
          self.evtLoop.Dispatch()
        self.wxApp.ProcessIdle()
        if task != None: return task.cont

    def onNew(self, evt):
        self.editor.reset()

    def onLoad(self, evt):
        dialog = wx.FileDialog(None, "Choose a file", os.getcwd(), "", "*.py", wx.OPEN)
        if dialog.ShowModal() == wx.ID_OK:
            self.editor.load(dialog.GetPath())
        dialog.Destroy()

    def onSave(self, evt):
        if self.editor.currentFile is None:
            self.onSaveAs(evt)
        else:
            self.editor.save()

    def onSaveAs(self, evt):
        dialog = wx.FileDialog(None, "Choose a file", os.getcwd(), "", "*.py", wx.SAVE)
        if dialog.ShowModal() == wx.ID_OK:
            self.editor.saveAs(dialog.GetPath())
        dialog.Destroy()

    def onDuplicate(self, evt):
        self.editor.objectMgr.duplicateSelected()

    def toggleGridSnap(self, evt):
        if self.gridSnapMenuItem.IsChecked():
            base.direct.manipulationControl.fGridSnap = 1
        else:
            base.direct.manipulationControl.fGridSnap = 0            

    def onGridSize(self, evt):
        gridSizeUI = GridSizeUI(self, -1, 'Change Grid Size', self.perspView.grid.gridSize, self.perspView.grid.gridSpacing)
        gridSizeUI.ShowModal()
        gridSizeUI.Destroy()        

class GridSizeUI(wx.Dialog):
    def __init__(self, parent, id, title, gridSize, gridSpacing):
        wx.Dialog.__init__(self, parent, id, title, size=(250, 240))

        self.parent = parent
        panel = wx.Panel(self, -1)
        vbox = wx.BoxSizer(wx.VERTICAL)

        wx.StaticBox(panel, -1, 'Grid Size', (5, 5), (235, 80))

        self.gridSizeSlider = WxSlider(panel, -1, float(gridSize), 10.0, 1000.0,
                           pos = (10, 25), size=(220, -1),
                           style=wx.SL_HORIZONTAL | wx.SL_LABELS)
        self.gridSizeSlider.Enable()

        wx.StaticBox(panel, -1, 'Grid Space', (5, 90), (235, 80))

        self.gridSpacingSlider = WxSlider(panel, -1, float(gridSpacing), 0.01, 20.0,
                           pos = (10, 115), size=(220, -1),
                           style=wx.SL_HORIZONTAL | wx.SL_LABELS)
        self.gridSpacingSlider.Enable()
        
        okButton = wx.Button(self, -1, 'Apply', size=(70, 20))
        okButton.Bind(wx.EVT_BUTTON, self.onApply)
        vbox.Add(panel)
        vbox.Add(okButton, 1, wx.ALIGN_CENTER | wx.TOP | wx.BOTTOM, 5)

        self.SetSizer(vbox)

    def onApply(self, evt):
        newSize = self.gridSizeSlider.GetValue()
        newSpacing = self.gridSpacingSlider.GetValue()

        self.parent.perspView.grid.gridSize = newSize
        self.parent.perspView.grid.gridSpacing = newSpacing
        self.parent.perspView.grid.updateGrid()

        self.parent.topView.grid.gridSize = newSize
        self.parent.topView.grid.gridSpacing = newSpacing
        self.parent.topView.grid.updateGrid()

        self.parent.frontView.grid.gridSize = newSize
        self.parent.frontView.grid.gridSpacing = newSpacing
        self.parent.frontView.grid.updateGrid()

        self.parent.leftView.grid.gridSize = newSize
        self.parent.leftView.grid.gridSpacing = newSpacing
        self.parent.leftView.grid.updateGrid()

        self.Destroy()
