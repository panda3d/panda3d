"""
Contains classes useful for 3D viewports.

Originally written by pro-rsoft,
Modified by gjeon.
"""

__all__ = ["Viewport", "ViewportManager", "ViewportMenu"]

from direct.showbase.DirectObject import DirectObject
from direct.directtools.DirectGrid import DirectGrid
from direct.showbase.ShowBase import WindowControls
from direct.directtools.DirectGlobals import *
from pandac.PandaModules import WindowProperties, OrthographicLens, Point3
import wx

HORIZONTAL = wx.SPLIT_HORIZONTAL
VERTICAL   = wx.SPLIT_VERTICAL
CREATENEW  = 99
VPLEFT     = 10
VPFRONT    = 11
VPTOP      = 12
VPPERSPECTIVE = 13

class ViewportManager:
  """Manages the global viewport stuff."""
  viewports = []
  gsg = None

  @staticmethod
  def initializeAll(*args, **kwargs):
    """Calls initialize() on all the viewports."""
    for v in ViewportManager.viewports:
      v.initialize(*args, **kwargs)
  
  @staticmethod
  def updateAll(*args, **kwargs):
    """Calls Update() on all the viewports."""
    for v in ViewportManager.viewports:
      v.Update(*args, **kwargs)
  
  @staticmethod
  def layoutAll(*args, **kwargs):
    """Calls Layout() on all the viewports."""
    for v in ViewportManager.viewports:
      v.Layout(*args, **kwargs)

class Viewport(wx.Panel, DirectObject):
  """Class representing a 3D Viewport."""
  CREATENEW  = CREATENEW
  VPLEFT     = VPLEFT
  VPFRONT    = VPFRONT
  VPTOP      = VPTOP
  VPPERSPECTIVE = VPPERSPECTIVE
  def __init__(self, name, *args, **kwargs):
    self.name = name
    DirectObject.__init__(self)
    wx.Panel.__init__(self, *args, **kwargs)

    ViewportManager.viewports.append(self)
    self.win = None
    self.camera = None
    self.lens = None
    self.camPos = None
    self.camLookAt = None
    self.initialized = False
    self.grid = None
    #self.Bind(wx.EVT_RIGHT_DOWN, self.onRightDown)

  def initialize(self):
    self.Update()
    wp = WindowProperties()
    wp.setOrigin(0, 0)
    wp.setSize(self.ClientSize.GetWidth(), self.ClientSize.GetHeight())
    assert self.GetHandle() != 0
    wp.setParentWindow(self.GetHandle())

    # initializing panda window
    base.windowType = "onscreen"
    props = WindowProperties.getDefault()
    props.addProperties(wp)
    self.win = base.openWindow(props = props, gsg = ViewportManager.gsg)
    if self.win:
      self.cam2d = base.makeCamera2d(self.win)
      self.cam2d.node().setCameraMask(LE_CAM_MASKS[self.name])
      
    if ViewportManager.gsg == None:
      ViewportManager.gsg = self.win.getGsg()
    self.cam = base.camList[-1]
    self.camera = render.attachNewNode(self.name)
    #self.camera.setName(self.name)
    #self.camera.reparentTo(render)
    self.cam.reparentTo(self.camera)
    self.camNode = self.cam.node()

    self.camNode.setCameraMask(LE_CAM_MASKS[self.name])

    bt = base.setupMouse(self.win, True)
    bt.node().setPrefix('_le_%s_'%self.name[:3])    
    mw = bt.getParent()
    mk = mw.getParent()
    winCtrl = WindowControls(
                self.win, mouseWatcher=mw,
                cam=self.camera,
                camNode = self.camNode,
                cam2d=None,
                mouseKeyboard =mk)
    base.setupWindowControls(winCtrl)

    self.initialized = True
    if self.lens != None:      self.cam.node().setLens(self.lens)
    if self.camPos != None:    self.camera.setPos(self.camPos)
    if self.camLookAt != None: self.camera.lookAt(self.camLookAt)

    self.camLens = self.camNode.getLens()

    self.Bind(wx.EVT_SIZE, self.onSize)
##     self.accept("wheel_down", self.zoomOut)
##     self.accept("wheel_up", self.zoomIn)
##     self.accept("page_down", self.zoomOut)
##     self.accept("page_down-repeat", self.zoomOut)
##     self.accept("page_up", self.zoomIn)
##     self.accept("page_up-repeat", self.zoomIn)
    #self.accept("mouse3", self.onRightDown)
  
  def close(self):
    """Closes the viewport."""
    if self.initialized:
      Window.close(self)
    ViewportManager.viewports.remove(self)
  
  def onSize(self, evt):
    """Invoked when the viewport is resized."""
    if self.win != None:
      wp = WindowProperties()
      wp.setOrigin(0, 0)
      newWidth = self.ClientSize.GetWidth()
      newHeight = self.ClientSize.GetHeight()
      wp.setSize(newWidth, newHeight)
      self.win.requestProperties(wp)

      if hasattr(base, "direct") and base.direct:
        for dr in base.direct.drList:
          if dr.camNode == self.camNode:
            dr.updateFilmSize(newWidth, newHeight)
            break
      
  def onRightDown(self, evt = None):
    print "RightDown captured by wx"
    """Invoked when the viewport is right-clicked."""
    menu = ViewportMenu(self)
    if evt == None:
      mpos = wx.GetMouseState()
      mpos = self.ScreenToClient((mpos.x, mpos.y))
    else:
      mpos = evt.GetPosition()
    self.Update()
    self.PopupMenu(menu, mpos)
    menu.Destroy()
  
  def zoomOut(self):
    self.camera.setY(self.camera, -MOUSE_ZOO_SPEED)
  
  def zoomIn(self):
    self.camera.setY(self.camera,  MOUSE_ZOOM_SPEED)
  
  @staticmethod
  def make(parent, vpType = None):
    """Safe constructor that also takes CREATENEW, VPLEFT, VPTOP, etc."""
    if vpType == None or vpType == CREATENEW:
      return Viewport(parent)
    if isinstance(vpType, Viewport): return vpType
    if vpType == VPLEFT:  return Viewport.makeLeft(parent)
    if vpType == VPFRONT: return Viewport.makeFront(parent)
    if vpType == VPTOP:   return Viewport.makeTop(parent)
    if vpType == VPPERSPECTIVE:  return Viewport.makePerspective(parent)
    raise TypeError, "Unknown viewport type: %s" % vpType
  
  @staticmethod
  def makeOrthographic(parent, name, campos):
    v = Viewport(name, parent)
    v.lens = OrthographicLens()
    v.lens.setFilmSize(30)
    v.camPos = campos
    v.camLookAt = Point3(0, 0, 0)
    v.grid = DirectGrid(parent=render)
    if name == 'left':
      v.grid.setHpr(0, 0, 90)
      #v.grid.gridBack.findAllMatches("**/+GeomNode")[0].setName("_leftViewGridBack")
      LE_showInOneCam(v.grid, name)
    elif name == 'front':
      v.grid.setHpr(90, 0, 90)
      #v.grid.gridBack.findAllMatches("**/+GeomNode")[0].setName("_frontViewGridBack")
      LE_showInOneCam(v.grid, name)
    else:
      #v.grid.gridBack.findAllMatches("**/+GeomNode")[0].setName("_topViewGridBack")
      LE_showInOneCam(v.grid, name)
    return v
  
  @staticmethod
  def makePerspective(parent):
    v = Viewport('persp', parent)
    v.camPos = Point3(-30, -30, 30)
    v.camLookAt = Point3(0, 0, 0)

    v.grid = DirectGrid(parent=render)
    #v.grid.gridBack.findAllMatches("**/+GeomNode")[0].setName("_perspViewGridBack")
    LE_showInOneCam(v.grid, 'persp')
    return v
  
  @staticmethod
  def makeLeft(parent): return Viewport.makeOrthographic(parent, 'left', Point3(100, 0, 0))
  @staticmethod
  def makeFront(parent): return Viewport.makeOrthographic(parent, 'front', Point3(0, -100, 0))
  @staticmethod
  def makeTop(parent): return Viewport.makeOrthographic(parent, 'top', Point3(0, 0, 100))

class ViewportMenu(wx.Menu):
  """Represents a menu that appears when right-clicking a viewport."""
  def __init__(self, viewport):
    wx.Menu.__init__(self)
    self.viewport = viewport
    self.addItem("&Refresh", self.viewport.Update)
    self.addItem("&Background Color...", self.onChooseColor)
  
  def addItem(self, name, call = None, id = None):
    if id == None: id = wx.NewId()
    item = wx.MenuItem(self, id, name)
    self.AppendItem(item)
    if call != None:
      self.Bind(wx.EVT_MENU, call, item)
  
  def onChooseColor(self, evt = None):
    """Change the background color of the viewport."""
    data = wx.ColourData()
    bgcolor = self.viewport.win.getClearColor()
    bgcolor = bgcolor[0] * 255.0, bgcolor[1] * 255.0, bgcolor[2] * 255.0
    data.SetColour(bgcolor)
    dlg = wx.ColourDialog(self, data)
    try:
      if dlg.ShowModal() == wx.ID_OK:
        data = dlg.GetColourData().GetColour()
        data = data[0] / 255.0, data[1] / 255.0, data[2] / 255.0
        self.viewport.win.setClearColor(*data)
    finally:
      dlg.Destroy()

