import wx
import os
from wx.lib.agw import fourwaysplitter as FWS

from pandac.PandaModules import *
from direct.wxwidgets.WxAppShell import *
from direct.directtools.DirectSelection import SelectionRay

from ViewPort import *
from ObjectPaletteUI import *
from ObjectPropertyUI import *
from SceneGraphUI import *
from LayerEditorUI import *
from HotKeyUI import *
from ProtoPaletteUI import *
from ActionMgr import *

class PandaTextDropTarget(wx.TextDropTarget):
    def __init__(self, editor, view):
        wx.TextDropTarget.__init__(self)
        self.editor = editor
        self.view = view

    def OnDropText(self, x, y, text):
        # create new object
        action = ActionAddNewObj(self.editor, text)
        self.editor.actionMgr.push(action)
        newobj = action()
        if newobj is None:
            return

        # change window coordinate to mouse coordinate
        mx = 2 * (x/float(self.view.ClientSize.GetWidth()) - 0.5)
        my = -2 * (y/float(self.view.ClientSize.GetHeight()) - 0.5)

        # create ray from the camera to detect 3d position
        iRay = SelectionRay(self.view.camera)
        iRay.collider.setFromLens(self.view.camNode, mx, my)
        hitPt = None
        if self.editor.objectMgr.currLiveNP:
            iRay.collideWithGeom()
            iRay.ct.traverse(self.editor.objectMgr.currLiveNP)

            def isEntryBackfacing(iRay, entry):
                if not entry.hasSurfaceNormal():
                    # Well, no way to tell.  Assume we're not backfacing.
                    return 0

                fromNodePath = entry.getFromNodePath()
                v = Vec3(entry.getSurfacePoint(fromNodePath))
                n = entry.getSurfaceNormal(fromNodePath)
                # Convert to camera space for backfacing test
                p2cam = iRay.collisionNodePath.getParent().getMat(self.view.camera)
                v = Vec3(p2cam.xformPoint(v))
                n = p2cam.xformVec(n)
                # Normalize and check angle between to vectors
                v.normalize()
                return v.dot(n) >= 0                

            iRay.sortEntries()
            for entry in iRay.getEntries():
                if isEntryBackfacing(iRay, entry):
                    pass
                else:
                    hitPt = entry.getSurfacePoint(entry.getFromNodePath())
                    break

        if hitPt is None:
            if self.view.name == 'persp':
                iRay.collideWithBitMask(1)
            else:
                iRay.collideWithBitMask(BitMask32.bit(21))
            iRay.ct.traverse(self.view.collPlane)
            if iRay.getNumEntries() > 0:
                entry = iRay.getEntry(0)
                hitPt = entry.getSurfacePoint(entry.getFromNodePath())

        if hitPt:
            # create a temp nodePath to get the position
            np = NodePath('temp')
            np.setPos(self.view.camera, hitPt)
            # update temp nodePath's HPR and scale with newobj's
            np.setHpr(newobj.getHpr())
            np.setScale(newobj.getScale())

            # transform newobj to cursor position
            obj = self.editor.objectMgr.findObjectByNodePath(newobj)
            action = ActionTransformObj(self.editor, obj[OG.OBJ_UID], Mat4(np.getMat()))
            self.editor.actionMgr.push(action)
            np.remove()
            action()
        iRay.collisionNodePath.removeNode()
        del iRay

ID_NEW = 101
ID_OPEN = 102
ID_SAVE = 103
ID_SAVE_AS = 104

ID_DUPLICATE = 201
ID_MAKE_LIVE = 202
ID_UNDO = 203
ID_REDO = 204

ID_SHOW_GRID = 301
ID_GRID_SIZE = 302
ID_GRID_SNAP = 303
ID_SHOW_PANDA_OBJECT = 304
ID_HOT_KEYS = 305

ID_FOUR_VIEW = 401
ID_TOP_VIEW = 402
ID_FRONT_VIEW = 403
ID_LEFT_VIEW = 404
ID_PERSP_VIEW = 405

class LevelEditorUIBase(WxAppShell):
    """ Class for Panda3D LevelEditor """ 

    MENU_TEXTS = {
        ID_NEW : ("&New", "LE-NewScene"),
        ID_OPEN : ("&Open", "LE-OpenScene"),
        ID_SAVE : ("&Save", "LE-SaveScene"),
        ID_SAVE_AS : ("Save &As", None),
        wx.ID_EXIT : ("&Quit", "LE-Quit"),
        ID_DUPLICATE : ("&Duplicate", "LE-Duplicate"),
        ID_MAKE_LIVE : ("Make &Live", "LE-MakeLive"),
        ID_UNDO : ("&Undo", "LE-Undo"),
        ID_REDO : ("&Redo", "LE-Redo"),
        ID_SHOW_GRID : ("&Show Grid", None),
        ID_GRID_SIZE : ("&Grid Size", None),
        ID_GRID_SNAP : ("Grid S&nap", None),
        ID_SHOW_PANDA_OBJECT : ("Show &Panda Objects", None),
        ID_HOT_KEYS : ("&Hot Keys", None),
        ID_FOUR_VIEW : ("Four Views", None),
        ID_TOP_VIEW : ("Top View", None),        
        ID_FRONT_VIEW : ("Front View", None),
        ID_LEFT_VIEW : ("Left View", None),
        ID_PERSP_VIEW : ("Persp View", None),
        }
    
    def __init__(self, editor, *args, **kw):
        # Create the Wx app
        self.wxApp = wx.App(redirect = False)
        self.wxApp.SetAppName("Panda3D LevelEditor")
        self.wxApp.SetClassName("P3DLevelEditor")
        self.editor = editor
        self.contextMenu = ViewportMenu()

        if not kw.get('size'):
            kw['size'] = wx.Size(self.frameWidth, self.frameHeight)
        WxAppShell.__init__(self, *args, **kw)        
        self.bindKeyEvents(True)
        self.initialize()

    def bindKeyEvents(self, toBind=True):
        if toBind:
            self.wxApp.Bind(wx.EVT_CHAR, self.onKeyEvent)
            self.wxApp.Bind(wx.EVT_KEY_DOWN, self.onKeyDownEvent)
            self.wxApp.Bind(wx.EVT_KEY_UP, self.onKeyUpEvent)
        else:
            self.wxApp.Unbind(wx.EVT_CHAR)
            self.wxApp.Unbind(wx.EVT_KEY_DOWN)
            self.wxApp.Unbind(wx.EVT_KEY_UP)

    def createMenu(self):
        menuItem = self.menuFile.Insert(0, ID_NEW, self.MENU_TEXTS[ID_NEW][0])
        self.Bind(wx.EVT_MENU, self.onNew, menuItem)
        
        menuItem = self.menuFile.Insert(1, ID_OPEN, self.MENU_TEXTS[ID_OPEN][0])
        self.Bind(wx.EVT_MENU, self.onOpen, menuItem)

        menuItem = self.menuFile.Insert(2, ID_SAVE, self.MENU_TEXTS[ID_SAVE][0])
        self.Bind(wx.EVT_MENU, self.onSave, menuItem)

        menuItem = self.menuFile.Insert(3, ID_SAVE_AS, self.MENU_TEXTS[ID_SAVE_AS][0])
        self.Bind(wx.EVT_MENU, self.onSaveAs, menuItem)

        self.menuEdit = wx.Menu()
        self.menuBar.Insert(1, self.menuEdit, "&Edit")

        menuItem = self.menuEdit.Append(ID_DUPLICATE, self.MENU_TEXTS[ID_DUPLICATE][0])
        self.Bind(wx.EVT_MENU, self.onDuplicate, menuItem)

        menuItem = self.menuEdit.Append(ID_MAKE_LIVE, self.MENU_TEXTS[ID_MAKE_LIVE][0])
        self.Bind(wx.EVT_MENU, self.onMakeLive, menuItem)

        menuItem = self.menuEdit.Append(ID_UNDO, self.MENU_TEXTS[ID_UNDO][0])
        self.Bind(wx.EVT_MENU, self.editor.actionMgr.undo, menuItem)

        menuItem = self.menuEdit.Append(ID_REDO, self.MENU_TEXTS[ID_REDO][0])
        self.Bind(wx.EVT_MENU, self.editor.actionMgr.redo, menuItem)

        self.menuOptions = wx.Menu()
        self.menuBar.Insert(2, self.menuOptions, "&Options")

        self.showGridMenuItem = self.menuOptions.Append(ID_SHOW_GRID, self.MENU_TEXTS[ID_SHOW_GRID][0], kind = wx.ITEM_CHECK)
        self.Bind(wx.EVT_MENU, self.toggleGrid, self.showGridMenuItem)

        self.gridSizeMenuItem = self.menuOptions.Append(ID_GRID_SIZE, self.MENU_TEXTS[ID_GRID_SIZE][0])
        self.Bind(wx.EVT_MENU, self.onGridSize, self.gridSizeMenuItem)

        self.gridSnapMenuItem = self.menuOptions.Append(ID_GRID_SNAP, self.MENU_TEXTS[ID_GRID_SNAP][0], kind = wx.ITEM_CHECK)
        self.Bind(wx.EVT_MENU, self.toggleGridSnap, self.gridSnapMenuItem)

        self.showPandaObjectsMenuItem = self.menuOptions.Append(ID_SHOW_PANDA_OBJECT, self.MENU_TEXTS[ID_SHOW_PANDA_OBJECT][0], kind = wx.ITEM_CHECK)
        self.Bind(wx.EVT_MENU, self.onShowPandaObjects, self.showPandaObjectsMenuItem)

        self.hotKeysMenuItem = self.menuOptions.Append(ID_HOT_KEYS, self.MENU_TEXTS[ID_HOT_KEYS][0])
        self.Bind(wx.EVT_MENU, self.onHotKeys, self.hotKeysMenuItem)

        self.menuView = wx.Menu()
        self.menuBar.Insert(3, self.menuView, "&View")

        menuItem = self.menuView.AppendRadioItem(ID_FOUR_VIEW, self.MENU_TEXTS[ID_FOUR_VIEW][0])
        self.Bind(wx.EVT_MENU, lambda p0=None, p1=-1:self.onViewChange(p0, p1), menuItem)

        menuItem = self.menuView.AppendRadioItem(ID_TOP_VIEW, self.MENU_TEXTS[ID_TOP_VIEW][0])
        self.Bind(wx.EVT_MENU, lambda p0=None, p1=0:self.onViewChange(p0, p1), menuItem)

        menuItem = self.menuView.AppendRadioItem(ID_FRONT_VIEW, self.MENU_TEXTS[ID_FRONT_VIEW][0])
        self.Bind(wx.EVT_MENU, lambda p0=None, p1=1:self.onViewChange(p0, p1), menuItem)

        menuItem = self.menuView.AppendRadioItem(ID_LEFT_VIEW, self.MENU_TEXTS[ID_LEFT_VIEW][0])
        self.Bind(wx.EVT_MENU, lambda p0=None, p1=2:self.onViewChange(p0, p1), menuItem)

        menuItem = self.menuView.AppendRadioItem(ID_PERSP_VIEW, self.MENU_TEXTS[ID_PERSP_VIEW][0])
        self.Bind(wx.EVT_MENU, lambda p0=None, p1=3:self.onViewChange(p0, p1), menuItem)
        
    def updateMenu(self):
        hotKeyDict = {}
        for hotKey in base.direct.hotKeyMap.keys():
            desc = base.direct.hotKeyMap[hotKey]
            hotKeyDict[desc[1]] = hotKey
            
        for id in self.MENU_TEXTS.keys():
            desc = self.MENU_TEXTS[id]
            if desc[1]:
                menuItem = self.menuBar.FindItemById(id)
                hotKey = hotKeyDict.get(desc[1])
                if hotKey:
                    menuItem.SetText(desc[0] + "\t%s"%hotKey)

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
        self.leftBarUpNB = wx.Notebook(self.leftBarUpPane, style=wx.NB_BOTTOM)
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.leftBarUpNB, 1, wx.EXPAND)
        self.leftBarUpPane.SetSizer(sizer)
        self.leftBarUpPane0 = wx.Panel(self.leftBarUpNB, -1)
        self.leftBarUpNB.AddPage(self.leftBarUpPane0, 'Object Palette')
        self.leftBarUpPane1 = wx.Panel(self.leftBarUpNB, -1)
        self.leftBarUpNB.AddPage(self.leftBarUpPane1, 'Proto Palette')
        self.leftBarDownPane = wx.Panel(self.leftFrame)
        self.leftBarDownNB = wx.Notebook(self.leftBarDownPane)
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.leftBarDownNB, 1, wx.EXPAND)
        self.leftBarDownPane.SetSizer(sizer)
        self.leftBarDownPane0 = wx.Panel(self.leftBarDownNB, -1)
        self.leftBarDownNB.AddPage(self.leftBarDownPane0, 'Scene Graph')
        self.rightBarUpPane = wx.Panel(self.rightFrame)
        self.rightBarDownPane = wx.Panel(self.rightFrame)
        self.rightBarDownNB = wx.Notebook(self.rightBarDownPane)
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.rightBarDownNB, 1, wx.EXPAND)
        self.rightBarDownPane.SetSizer(sizer)
        self.rightBarDownPane0 = wx.Panel(self.rightBarDownNB, -1)
        self.rightBarDownNB.AddPage(self.rightBarDownPane0, 'Layers')

        self.leftFrame.SplitHorizontally(self.leftBarUpPane, self.leftBarDownPane)
        self.rightFrame.SplitHorizontally(self.rightBarUpPane, self.rightBarDownPane)
        self.mainFrame.SplitVertically(self.leftFrame, self.baseFrame, 200)
        self.baseFrame.SplitVertically(self.viewFrame, self.rightFrame, 600)
        
        self.topView.SetDropTarget(PandaTextDropTarget(self.editor, self.topView))
        self.frontView.SetDropTarget(PandaTextDropTarget(self.editor, self.frontView))
        self.leftView.SetDropTarget(PandaTextDropTarget(self.editor, self.leftView))
        self.perspView.SetDropTarget(PandaTextDropTarget(self.editor, self.perspView))
        
        self.leftFrame.SetSashGravity(0.5)
        self.rightFrame.SetSashGravity(0.5)        
        self.baseFrame.SetSashGravity(1.0)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.mainFrame, 1, wx.EXPAND, 0)
        self.SetSizer(sizer); self.Layout()

        self.objectPaletteUI = ObjectPaletteUI(self.leftBarUpPane0, self.editor)
        self.protoPaletteUI = ProtoPaletteUI(self.leftBarUpPane1, self.editor)
        self.objectPropertyUI = ObjectPropertyUI(self.rightBarUpPane, self.editor)
        self.sceneGraphUI = SceneGraphUI(self.leftBarDownPane0, self.editor)
        self.layerEditorUI = LayerEditorUI(self.rightBarDownPane0, self.editor)

        self.showGridMenuItem.Check(True)

    def onViewChange(self, evt, viewIdx):
        for i in range(4):
            if viewIdx >=0 and\
               i != viewIdx:
                base.winList[i].setActive(0)
            else:
                base.winList[i].setActive(1)

        self.viewFrame.SetExpanded(viewIdx)

    def onRightDown(self, evt=None):
        """Invoked when the viewport is right-clicked."""
        if evt == None:
            mpos = wx.GetMouseState()
            mpos = self.ScreenToClient((mpos.x, mpos.y))
        else:
            mpos = evt.GetPosition()

        base.direct.fMouse3 = 0
        self.PopupMenu(self.contextMenu, mpos)

    def onKeyDownEvent(self, evt):
        if evt.GetKeyCode() == wx.WXK_ALT:
            base.direct.fAlt = 1
        elif evt.GetKeyCode() == wx.WXK_CONTROL:
            base.direct.fControl = 1
        elif evt.GetKeyCode() == wx.WXK_SHIFT:
            base.direct.fShift = 1
        elif evt.GetKeyCode() == wx.WXK_UP:
            messenger.send('arrow_up')
        elif evt.GetKeyCode() == wx.WXK_DOWN:
            messenger.send('arrow_down')
        elif evt.GetKeyCode() == wx.WXK_LEFT:
            messenger.send('arrow_left')
        elif evt.GetKeyCode() == wx.WXK_RIGHT:
            messenger.send('arrow_right')
        elif evt.GetKeyCode() == wx.WXK_PAGEUP:
            messenger.send('page_up')
        elif evt.GetKeyCode() == wx.WXK_PAGEDOWN:
            messenger.send('page_down')
        else:
            evt.Skip()

    def onKeyUpEvent(self, evt):
        if evt.GetKeyCode() == wx.WXK_ALT:
            base.direct.fAlt = 0
        elif evt.GetKeyCode() == wx.WXK_CONTROL:
            base.direct.fControl = 0
        elif evt.GetKeyCode() == wx.WXK_SHIFT:
            base.direct.fShift = 0
        elif evt.GetKeyCode() == wx.WXK_UP:
            messenger.send('arrow_up-up')
        elif evt.GetKeyCode() == wx.WXK_DOWN:
            messenger.send('arrow_down-up')
        elif evt.GetKeyCode() == wx.WXK_LEFT:
            messenger.send('arrow_left-up')
        elif evt.GetKeyCode() == wx.WXK_RIGHT:
            messenger.send('arrow_right-up')
        elif evt.GetKeyCode() == wx.WXK_PAGEUP:
            messenger.send('page_up-up')
        elif evt.GetKeyCode() == wx.WXK_PAGEDOWN:
            messenger.send('page_down-up')
        else:
            evt.Skip()
        
    def onKeyEvent(self, evt):
        input = ''
        if evt.GetKeyCode() in range(97, 123): # for keys from a to z
            if evt.GetModifiers() == 4: # when shift is pressed while caps lock is on
                input = 'shift-%s'%chr(evt.GetKeyCode())
            else:
                input = chr(evt.GetKeyCode())
        elif evt.GetKeyCode() in range(65, 91):
            if evt.GetModifiers() == 4: # when shift is pressed
                input = 'shift-%s'%chr(evt.GetKeyCode() + 32)
            else:
                input = chr(evt.GetKeyCode() + 32)
        elif evt.GetKeyCode() in range(1, 27): # for keys from a to z with control
            input = 'control-%s'%chr(evt.GetKeyCode()+96)
        elif evt.GetKeyCode() == wx.WXK_DELETE:
            input = 'delete'
        elif evt.GetKeyCode() == wx.WXK_ESCAPE:
            input = 'escape'
        else:
            if evt.GetModifiers() == 4:
                input = 'shift-%s'%chr(evt.GetKeyCode())
            elif evt.GetModifiers() == 2:
                input = 'control-%s'%chr(evt.GetKeyCode())
            elif evt.GetKeyCode() < 256:
                input = chr(evt.GetKeyCode())
        if input in base.direct.hotKeyMap.keys():
            keyDesc = base.direct.hotKeyMap[input]
            messenger.send(keyDesc[1])

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

    def reset(self):
        self.sceneGraphUI.reset()
        self.layerEditorUI.reset()

    def onNew(self, evt=None):
        self.editor.reset()

    def onOpen(self, evt=None):
        dialog = wx.FileDialog(None, "Choose a file", os.getcwd(), "", "*.py", wx.OPEN)
        if dialog.ShowModal() == wx.ID_OK:
            self.editor.load(dialog.GetPath())
            self.editor.setTitleWithFilename(dialog.GetPath())
        dialog.Destroy()

    def onSave(self, evt=None):
        if self.editor.currentFile is None or\
           not self.editor.currentFile.endswith('.py'):
            return self.onSaveAs(evt)
        else:
            self.editor.save()

    def onSaveAs(self, evt):
        dialog = wx.FileDialog(None, "Choose a file", os.getcwd(), "", "*.py", wx.SAVE)
        result = True
        if dialog.ShowModal() == wx.ID_OK:
            self.editor.saveAs(dialog.GetPath())
            self.editor.setTitleWithFilename(dialog.GetPath())
        else:
            result = False
        dialog.Destroy()
        return result

    def onDuplicate(self, evt):
        self.editor.objectMgr.duplicateSelected()

    def onMakeLive(self, evt):
        self.editor.objectMgr.makeSelectedLive()

    def toggleGrid(self, evt):
        if self.showGridMenuItem.IsChecked():
            for grid in [self.perspView.grid, self.topView.grid, self.frontView.grid, self.leftView.grid]:
                if grid.isHidden():
                    grid.show()
        else:
            for grid in [self.perspView.grid, self.topView.grid, self.frontView.grid, self.leftView.grid]:
                if not grid.isHidden():
                    grid.hide()
                
    def toggleGridSnap(self, evt):
        if self.gridSnapMenuItem.IsChecked():
            base.direct.manipulationControl.fGridSnap = 1
            for grid in [self.perspView.grid, self.topView.grid, self.frontView.grid, self.leftView.grid]:
                grid.fXyzSnap = 1

        else:
            base.direct.manipulationControl.fGridSnap = 0            
            for grid in [self.perspView.grid, self.topView.grid, self.frontView.grid, self.leftView.grid]:
                grid.fXyzSnap = 0
            
    def onGridSize(self, evt):
        gridSizeUI = GridSizeUI(self, -1, 'Change Grid Size', self.perspView.grid.gridSize, self.perspView.grid.gridSpacing)
        gridSizeUI.ShowModal()
        gridSizeUI.Destroy()
        
    def onShowPandaObjects(self, evt):
        self.sceneGraphUI.showPandaObjectChildren()

    def onDestroy(self, evt):
        self.editor.protoPalette.saveToFile()
        self.editor.saveSettings()
        self.editor.reset()

    def updateGrids(self, newSize, newSpacing):
        self.perspView.grid.gridSize = newSize
        self.perspView.grid.gridSpacing = newSpacing
        self.perspView.grid.updateGrid()

        self.topView.grid.gridSize = newSize
        self.topView.grid.gridSpacing = newSpacing
        self.topView.grid.updateGrid()

        self.frontView.grid.gridSize = newSize
        self.frontView.grid.gridSpacing = newSpacing
        self.frontView.grid.updateGrid()

        self.leftView.grid.gridSize = newSize
        self.leftView.grid.gridSpacing = newSpacing
        self.leftView.grid.updateGrid()        

    def onHotKeys(self, evt):
        hotKeyUI = HotKeyUI(self, -1, 'Hot Key List')
        hotKeyUI.ShowModal()
        hotKeyUI.Destroy()

    def buildContextMenu(self, nodePath):
        for menuItem in self.contextMenu.GetMenuItems():
            self.contextMenu.RemoveItem(menuItem)

class GridSizeUI(wx.Dialog):
    def __init__(self, parent, id, title, gridSize, gridSpacing):
        wx.Dialog.__init__(self, parent, id, title, size=(250, 240))

        self.parent = parent
        panel = wx.Panel(self, -1)
        vbox = wx.BoxSizer(wx.VERTICAL)

        wx.StaticBox(panel, -1, 'Grid Size', (5, 5), (235, 80))

        self.gridSizeSlider = WxSlider(panel, -1, float(gridSize), 10.0, 100000.0,
                           pos = (10, 25), size=(220, -1),
                           style=wx.SL_HORIZONTAL | wx.SL_LABELS, textSize=(80,20))
        self.gridSizeSlider.Enable()

        wx.StaticBox(panel, -1, 'Grid Space', (5, 90), (235, 80))

        self.gridSpacingSlider = WxSlider(panel, -1, float(gridSpacing), 0.01, 2000.0,
                           pos = (10, 115), size=(220, -1),
                           style=wx.SL_HORIZONTAL | wx.SL_LABELS)
        self.gridSpacingSlider.Enable()
        
        okButton = wx.Button(self, -1, 'Apply', size=(70, 20))
        okButton.Bind(wx.EVT_BUTTON, self.onApply)
        vbox.Add(panel)
        vbox.Add(okButton, 1, wx.ALIGN_CENTER | wx.TOP | wx.BOTTOM, 5)

        self.SetSizer(vbox)
        base.le.ui.bindKeyEvents(False)

    def onApply(self, evt):
        newSize = self.gridSizeSlider.GetValue()
        newSpacing = self.gridSpacingSlider.GetValue()
        self.parent.updateGrids(newSize, newSpacing)
        base.le.ui.bindKeyEvents(True)
        self.Destroy()

class ViewportMenu(wx.Menu):
    """Represents a menu that appears when right-clicking a viewport."""
    def __init__(self):
        wx.Menu.__init__(self)
  
    def addItem(self, name, parent = None, call = None, id = None):
        if id == None: id = wx.NewId()
        if parent == None: parent = self
        item = wx.MenuItem(parent, id, name)
        parent.AppendItem(item)
        if call != None:
            self.Bind(wx.EVT_MENU, call, item)

    def addMenu(self, name, parent = None, id = None):
        if id == None: id = wx.NewId()
        subMenu = wx.Menu()
        if parent == None: parent = self
        parent.AppendMenu(id, name, subMenu)
        return subMenu

