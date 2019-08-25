import wx
from wx.lib.agw import fourwaysplitter as FWS

from panda3d.core import *
from direct.showbase.ShowBase import *
from direct.directtools.DirectGlobals import *

try:
    base
except NameError:
    base = ShowBase(False, windowType = 'none')

from .WxAppShell import *
from .ViewPort import *

ID_FOUR_VIEW = 401
ID_TOP_VIEW = 402
ID_FRONT_VIEW = 403
ID_LEFT_VIEW = 404
ID_PERSP_VIEW = 405

class WxPandaShell(WxAppShell):
    """ Class for Panda3D LevelEditor """
    frameWidth = 800
    frameHeight = 600
    appversion      = '1.0'
    appname         = 'Panda3D Generic WX Frame'
    copyright       = ('Copyright 2010 Disney Online Studios.' +
                       '\nAll Rights Reserved.')

    MENU_TEXTS = {
        ID_FOUR_VIEW : ("Four Views", None),
        ID_TOP_VIEW : ("Top View", None),
        ID_FRONT_VIEW : ("Front View", None),
        ID_LEFT_VIEW : ("Left View", None),
        ID_PERSP_VIEW : ("Persp View", None),
        }

    def __init__(self, fStartDirect = False):
        fDirect = (base.config.GetBool('want-directtools', 0) or
                   (base.config.GetString("cluster-mode", '') != ''))

        self.fStartDirect = fStartDirect or fDirect

        # Create the Wx app
        self.wxApp = wx.App(redirect = False)
        self.wxApp.SetAppName(self.appname)
        WxAppShell.__init__(self, size=wx.Size(self.frameWidth, self.frameHeight))
        self.initialize()

    def createMenu(self):
        self.menuView = wx.Menu()
        self.menuBar.Insert(self.menuBar.GetMenuCount() - 1, self.menuView, "&View")

        menuItem = self.menuView.AppendRadioItem(ID_FOUR_VIEW, self.MENU_TEXTS[ID_FOUR_VIEW][0])
        self.Bind(wx.EVT_MENU, lambda p0=None, p1=-1:self.onViewChange(p0, p1), menuItem)

        menuItem = self.menuView.AppendRadioItem(ID_TOP_VIEW, self.MENU_TEXTS[ID_TOP_VIEW][0])
        self.Bind(wx.EVT_MENU, lambda p0=None, p1=0:self.onViewChange(p0, p1), menuItem)

        menuItem = self.menuView.AppendRadioItem(ID_FRONT_VIEW, self.MENU_TEXTS[ID_FRONT_VIEW][0])
        self.Bind(wx.EVT_MENU, lambda p0=None, p1=1:self.onViewChange(p0, p1), menuItem)

        menuItem = self.menuView.AppendRadioItem(ID_LEFT_VIEW, self.MENU_TEXTS[ID_LEFT_VIEW][0])
        self.Bind(wx.EVT_MENU, lambda p0=None, p1=2:self.onViewChange(p0, p1), menuItem)

        self.perspViewMenuItem = self.menuView.AppendRadioItem(ID_PERSP_VIEW, self.MENU_TEXTS[ID_PERSP_VIEW][0])
        self.Bind(wx.EVT_MENU, lambda p0=None, p1=3:self.onViewChange(p0, p1), self.perspViewMenuItem)

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

        self.leftFrame.SetSashGravity(0.5)
        self.rightFrame.SetSashGravity(0.5)
        self.baseFrame.SetSashGravity(1.0)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.mainFrame, 1, wx.EXPAND, 0)
        self.SetSizer(sizer); self.Layout()

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

        # to make persp view as default
        self.perspViewMenuItem.Toggle()
        self.onViewChange(None, 3)

        # initializing direct
        if self.fStartDirect:
            base.startDirect(fWantTk = 0, fWantWx = 0)

            base.direct.disableMouseEvents()
            newMouseEvents = ["_le_per_%s"%x for x in base.direct.mouseEvents] +\
                             ["_le_fro_%s"%x for x in base.direct.mouseEvents] +\
                             ["_le_lef_%s"%x for x in base.direct.mouseEvents] +\
                             ["_le_top_%s"%x for x in base.direct.mouseEvents]
            base.direct.mouseEvents = newMouseEvents
            base.direct.enableMouseEvents()

            base.direct.disableKeyEvents()
            keyEvents = ["_le_per_%s"%x for x in base.direct.keyEvents] +\
                             ["_le_fro_%s"%x for x in base.direct.keyEvents] +\
                             ["_le_lef_%s"%x for x in base.direct.keyEvents] +\
                             ["_le_top_%s"%x for x in base.direct.keyEvents]
            base.direct.keyEvents = keyEvents
            base.direct.enableKeyEvents()

            base.direct.disableModifierEvents()
            modifierEvents = ["_le_per_%s"%x for x in base.direct.modifierEvents] +\
                             ["_le_fro_%s"%x for x in base.direct.modifierEvents] +\
                             ["_le_lef_%s"%x for x in base.direct.modifierEvents] +\
                             ["_le_top_%s"%x for x in base.direct.modifierEvents]
            base.direct.modifierEvents = modifierEvents
            base.direct.enableModifierEvents()

            base.direct.cameraControl.lockRoll = True
            base.direct.setFScaleWidgetByCam(1)

            unpickables = [
                "z-guide",
                "y-guide",
                "x-guide",
                "x-disc-geom",
                "x-ring-line",
                "x-post-line",
                "y-disc-geom",
                "y-ring-line",
                "y-post-line",
                "z-disc-geom",
                "z-ring-line",
                "z-post-line",
                "centerLines",
                "majorLines",
                "minorLines",
                "Sphere",]

            for unpickable in unpickables:
                base.direct.addUnpickable(unpickable)

            base.direct.manipulationControl.optionalSkipFlags |= SKIP_UNPICKABLE
            base.direct.manipulationControl.fAllowMarquee = 1
            base.direct.manipulationControl.supportMultiView()
            base.direct.cameraControl.useMayaCamControls = 1
            base.direct.cameraControl.perspCollPlane = self.perspView.collPlane
            base.direct.cameraControl.perspCollPlane2 = self.perspView.collPlane2

            for widget in base.direct.manipulationControl.widgetList:
                widget.setBin('gui-popup', 0)
                widget.setDepthTest(0)

            # [gjeon] to intercept messages here
            base.direct.ignore('DIRECT-delete')
            base.direct.ignore('DIRECT-select')
            base.direct.ignore('DIRECT-preDeselectAll')
            base.direct.ignore('DIRECT-toggleWidgetVis')
            base.direct.fIgnoreDirectOnlyKeyMap = 1

            # [gjeon] do not use the old way of finding current DR
            base.direct.drList.tryToGetCurrentDr = False

        else:
            base.direct=None
        #base.closeWindow(base.win)
        base.win = base.winList[3]

    def wxStep(self, task = None):
        """A step in the WX event loop. You can either call this yourself or use as task."""
        while self.evtLoop.Pending():
          self.evtLoop.Dispatch()
        self.evtLoop.ProcessIdle()
        if task != None: return task.cont

    def appInit(self):
        """Overridden from WxAppShell.py."""
        # Create a new event loop (to overide default wxEventLoop)
        self.evtLoop = wx.GUIEventLoop()
        self.oldLoop = wx.GUIEventLoop.GetActive()
        wx.GUIEventLoop.SetActive(self.evtLoop)
        taskMgr.add(self.wxStep, "evtLoopTask")

    def onViewChange(self, evt, viewIdx):
        for i in range(4):
            if viewIdx >=0 and\
               i != viewIdx:
                base.winList[i].setActive(0)
            else:
                base.winList[i].setActive(1)

        self.viewFrame.SetExpanded(viewIdx)

    def getCurrentView(self):
        """Function for get the current Viewport"""
        if self.viewFrame._expanded == -1: #four view
            self.currentView = None
        if self.viewFrame._expanded == 0: #top view
            self.currentView = self.topView
        if self.viewFrame._expanded == 1: #front view
            self.currentView = self.frontView
        if self.viewFrame._expanded == 2: #left view
            self.currentView = self.leftView
        if self.viewFrame._expanded == 3: #perspect view
            self.currentView = self.perspView

        return self.currentView


