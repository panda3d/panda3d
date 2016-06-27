"""
Base class for Level Editor

You should write your own LevelEditor class inheriting this.
Refer LevelEditor.py for example.
"""

from direct.showbase.DirectObject import *
from direct.directtools.DirectUtil import *
from direct.gui.DirectGui import *

from .CurveEditor import *
from .FileMgr import *
from .ActionMgr import *
from .MayaConverter import *

class LevelEditorBase(DirectObject):
    """ Base Class for Panda3D LevelEditor """
    def __init__(self):
        #loadPrcFileData('startup', 'window-type none')
        self.currentFile = None
        self.fNeedToSave = False
        self.actionEvents = []
        #self.objectMgr = ObjectMgr(self)
        self.curveEditor = CurveEditor(self)
        self.fileMgr = FileMgr(self)
        self.actionMgr = ActionMgr()

        self.fMoveCamera = False

        self.NPParent = render

        # define your own config file in inherited class
        self.settingsFile = None

        # you can show/hide specific properties by using propertiesMask and this mode
        self.BASE_MODE = BitMask32.bit(0)
        self.CREATE_CURVE_MODE = BitMask32.bit(2)
        self.EDIT_CURVE_MODE = BitMask32.bit(3)
        self.ANIM_MODE = BitMask32.bit(4)
        self.GRAPH_EDITOR = False

        self.mode = self.BASE_MODE
        self.preMode = None

    def initialize(self):
        """ You should call this in your __init__ method of inherited LevelEditor class """
        # specifiy what obj can be 'selected' as objects
        base.direct.selected.addTag('OBJRoot')

        self.actionEvents.extend([
            # Node path events
            ('DIRECT-select', self.select),
            ('DIRECT-delete', self.handleDelete),
            ('DIRECT-preDeselectAll', self.deselectAll),
            ('DIRECT_deselectAll', self.deselectAllCB),
            ('preRemoveNodePath', self.removeNodePathHook),
            ('DIRECT_deselectedNodePath', self.deselectAllCB),
            ('DIRECT_selectedNodePath_fMulti_fTag_fLEPane', self.selectedNodePathHook),
            ('DIRECT_deselectAll', self.deselectAll),
            ('LE-Undo', self.actionMgr.undo),
            ('LE-Redo', self.actionMgr.redo),
            ('LE-Duplicate', self.objectMgr.duplicateSelected),
            ('DIRECT_manipulateObjectCleanup', self.cleanUpManipulating),
            ('LE-MakeLive', self.objectMgr.makeSelectedLive),
            ('LE-NewScene', self.ui.onNew),
            ('LE-SaveScene', self.ui.onSave),
            ('LE-OpenScene', self.ui.onOpen),
            ('LE-Quit', self.ui.quit),
            ('DIRECT-mouse1', self.handleMouse1),
            ('DIRECT-mouse1Up', self.handleMouse1Up),
            ('DIRECT-mouse2', self.handleMouse2),
            ('DIRECT-mouse2Up', self.handleMouse2Up),
            ('DIRECT-mouse3', self.handleMouse3),
            ('DIRECT-mouse3Up', self.handleMouse3Up),
            ('DIRECT-toggleWidgetVis', self.toggleWidget),
            ])

        # Add all the action events
        for event in self.actionEvents:
            if len(event) == 3:
                self.accept(event[0], event[1], event[2])
            else:
                self.accept(event[0], event[1])

        # editor state text display such as edit mode
        self.statusReadout = OnscreenText(
            pos = (-1.2, 0.9), bg=Vec4(1,1,1,1),
            scale = 0.05, align = TextNode.ALeft,
            mayChange = 1, font = TextNode.getDefaultFont())
        self.statusReadout.setText("")
        # Make sure readout is never lit or drawn in wireframe
        useDirectRenderStyle(self.statusReadout)
        self.statusReadout.reparentTo(hidden)
        self.statusLines = []
        taskMgr.doMethodLater(5, self.updateStatusReadoutTimeouts, 'updateStatus')

        self.loadSettings()
        self.reset()

    def setTitleWithFilename(self, filename=""):
        title = self.ui.appname
        if filename != "":
           filenameshort = os.path.basename(filename)
           title = title + " (%s)"%filenameshort
        self.ui.SetLabel(title)

    def removeNodePathHook(self, nodePath):
        if nodePath is None:
            return
        base.direct.deselect(nodePath)
        self.objectMgr.removeObjectByNodePath(nodePath)

        if base.direct.selected.last is not None and nodePath == base.direct.selected.last:
            # if base.direct.selected.last is refering to this
            # removed obj, clear the reference
            if (hasattr(__builtins__,'last')):
                __builtins__.last = None
            else:
                __builtins__['last'] = None
            base.direct.selected.last = None

    def toggleWidget(self):
        if self.objectMgr.currNodePath:
            obj = self.objectMgr.findObjectByNodePath(self.objectMgr.currNodePath)
            if obj and not obj[OG.OBJ_DEF].movable:
                return
        base.direct.toggleWidgetVis()

    def handleMouse1(self, modifiers):
        if base.direct.fAlt or modifiers == 4:
            self.fMoveCamera = True
            return
        if self.mode == self.CREATE_CURVE_MODE :
            self.curveEditor.createCurve()


    def handleMouse1Up(self):
        self.fMoveCamera = False


    def handleMouse2(self, modifiers):
        if base.direct.fAlt or modifiers == 4:
            self.fMoveCamera = True
            return

    def handleMouse2Up(self):
        self.fMoveCamera = False

    def handleMouse3(self, modifiers):
        if base.direct.fAlt or modifiers == 4:
            self.fMoveCamera = True
            return

        self.ui.onRightDown()

    def handleMouse3Up(self):
        self.fMoveCamera = False

    def handleDelete(self):
        oldSelectedNPs = base.direct.selected.getSelectedAsList()
        oldUIDs = []
        for oldNP in oldSelectedNPs:
            obj = self.objectMgr.findObjectByNodePath(oldNP)
            if obj:
                oldUIDs.append(obj[OG.OBJ_UID])

        action = ActionDeleteObj(self)
        self.actionMgr.push(action)
        action()

        for uid in oldUIDs:
            self.ui.sceneGraphUI.delete(uid)

##         reply = wx.MessageBox("Do you want to delete selected?", "Delete?",
##                               wx.YES_NO | wx.ICON_QUESTION)
##         if reply == wx.YES:
##             base.direct.removeAllSelected()
##         else:
##             # need to reset COA
##             dnp = base.direct.selected.last
##             # Update camera controls coa to this point
##             # Coa2Camera = Coa2Dnp * Dnp2Camera
##             mCoa2Camera = dnp.mCoa2Dnp * dnp.getMat(base.direct.camera)
##             row = mCoa2Camera.getRow(3)
##             coa = Vec3(row[0], row[1], row[2])
##             base.direct.cameraControl.updateCoa(coa)

    def cleanUpManipulating(self, selectedNPs):
        for np in selectedNPs:
            obj = self.objectMgr.findObjectByNodePath(np)
            if obj:
                action = ActionTransformObj(self, obj[OG.OBJ_UID], Mat4(np.getMat()))
                self.actionMgr.push(action)
                action()

    def select(self, nodePath, fMultiSelect=0, fSelectTag=1, fResetAncestry=1, fLEPane=0, fUndo=1):
        if fUndo:
            # Select tagged object if present
            if fSelectTag:
                for tag in base.direct.selected.tagList:
                    if nodePath.hasNetTag(tag):
                        nodePath = nodePath.findNetTag(tag)
                        break
            action = ActionSelectObj(self, nodePath, fMultiSelect)
            self.actionMgr.push(action)
            action()
        else:
            base.direct.selectCB(nodePath, fMultiSelect, fSelectTag, fResetAncestry, fLEPane, fUndo)

    def selectedNodePathHook(self, nodePath, fMultiSelect = 0, fSelectTag = 1, fLEPane = 0):
        # handle unpickable nodepath
        if nodePath.getName() in base.direct.iRay.unpickable:
            base.direct.deselect(nodePath)
            return

        if fMultiSelect == 0 and fLEPane == 0:
           oldSelectedNPs = base.direct.selected.getSelectedAsList()
           for oldNP in oldSelectedNPs:
              obj = self.objectMgr.findObjectByNodePath(oldNP)
              if obj:
                 self.ui.sceneGraphUI.deSelect(obj[OG.OBJ_UID])
        self.objectMgr.selectObject(nodePath, fLEPane)
        self.ui.buildContextMenu(nodePath)

        if self.mode == self.EDIT_CURVE_MODE:
            taskMgr.add(self.curveEditor.editCurve, "modify")
            self.curveEditor.accept("DIRECT-enter", self.curveEditor.onBaseMode)

    def deselectAll(self, np=None):
        if len(base.direct.selected.getSelectedAsList()) ==0:
            return
        action = ActionDeselectAll(self)
        self.actionMgr.push(action)
        action()

    def deselectAllCB(self, dnp=None):
        self.objectMgr.deselectAll()

    def reset(self):
        if self.fNeedToSave:
            reply = wx.MessageBox("Do you want to save current scene?", "Save?",
                               wx.YES_NO | wx.ICON_QUESTION)
            if reply == wx.YES:
                result = self.ui.onSave()
                if result == False:
                    return

        base.direct.deselectAll()
        base.direct.selected.last = None
        self.ui.reset()
        self.objectMgr.reset()
        self.animMgr.reset()
        self.actionMgr.reset()
        self.ui.perspView.camera.setPos(-19, -19, 19)
        self.ui.perspView.camera.lookAt(Point3(0, 0, 0))
        self.ui.leftView.camera.setPos(600, 0, 0)
        self.ui.frontView.camera.setPos(0, -600, 0)
        self.ui.topView.camera.setPos(0, 0, 600)
        self.resetOrthoCam(self.ui.topView)
        self.resetOrthoCam(self.ui.frontView)
        self.resetOrthoCam(self.ui.leftView)
        self.fNeedToSave = False
        self.setTitleWithFilename()

    def resetOrthoCam(self, view):
        base.direct.drList[base.camList.index(NodePath(view.camNode))].orthoFactor = 0.1
        x = view.ClientSize.GetWidth() * 0.1
        y = view.ClientSize.GetHeight() * 0.1
        view.camLens.setFilmSize(x, y)

    def save(self):
        self.ui.SetCursor(wx.StockCursor(wx.CURSOR_WAIT))
        if self.currentFile:
            self.fileMgr.saveToFile(self.currentFile)
        self.ui.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))

    def saveAs(self, fileName):
        self.ui.SetCursor(wx.StockCursor(wx.CURSOR_WAIT))
        self.fileMgr.saveToFile(fileName)
        self.currentFile = fileName
        self.ui.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))

    def load(self, fileName):
        self.ui.SetCursor(wx.StockCursor(wx.CURSOR_WAIT))
        self.reset()
        self.fileMgr.loadFromFile(fileName)
        self.currentFile = fileName
        self.ui.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))

    def saveSettings(self):
        if self.settingsFile is None:
            return

        try:
            f = open(self.settingsFile, 'w')
            f.write('gridSize\n%f\n'%self.ui.perspView.grid.gridSize)
            f.write('gridSpacing\n%f\n'%self.ui.perspView.grid.gridSpacing)
            f.write('hotKey\n%s\n'%base.direct.hotKeyMap)
            f.close()
        except:
            pass

    def loadSettings(self):
        if self.settingsFile is None:
            return

        self.ui.SetCursor(wx.StockCursor(wx.CURSOR_WAIT))
        try:
            f = open(self.settingsFile, 'r')
            configLines = f.readlines()
            f.close()

            gridSize = 100.0
            gridSpacing = 5.0
            for i in range(0, len(configLines)):
                line = configLines[i]
                i = i + 1
                if line.startswith('gridSize'):
                    gridSize = float(configLines[i])
                elif line.startswith('gridSpacing'):
                    gridSpacing = float(configLines[i])
                elif line.startswith('hotKey'):
                    customHotKeyMap = eval(configLines[i])
                    customHotKeyDict = {}
                    for hotKey in customHotKeyMap.keys():
                        desc = customHotKeyMap[hotKey]
                        customHotKeyDict[desc[1]] = hotKey

                    overriddenKeys = []
                    for key in base.direct.hotKeyMap.keys():
                        desc = base.direct.hotKeyMap[key]
                        if desc[1] in customHotKeyDict.keys():
                            overriddenKeys.append(key)

                    for key in overriddenKeys:
                        del base.direct.hotKeyMap[key]

                    base.direct.hotKeyMap.update(customHotKeyMap)

            self.ui.updateGrids(gridSize, gridSpacing)
            self.ui.updateMenu()
        except:
            pass
        self.ui.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))

    def convertMaya(self, modelname, callBack, obj=None, isAnim=False):
        if obj and isAnim:
            mayaConverter = MayaConverter(self.ui, self, modelname, callBack, obj, isAnim)
        else:
            reply = wx.MessageBox("Is it an animation file?", "Animation?",
                              wx.YES_NO | wx.ICON_QUESTION)
            if reply == wx.YES:
                mayaConverter = MayaConverter(self.ui, self, modelname, callBack, None, True)
            else:
                mayaConverter = MayaConverter(self.ui, self, modelname, callBack, None, False)
        mayaConverter.Show()

    def convertFromMaya(self, modelname, callBack):
        mayaConverter = MayaConverter(self.ui, self, modelname, callBack, None, False)
        mayaConverter.Show()

    def exportToMaya(self, mayaFileName):
        exportRootNP = render
        self.exportToMayaCB(mayaFileName, exportRootNP)

    def exportToMayaCB(self, mayaFileName, exportRootNP):
        bamFileName = mayaFileName + ".bam"

        if base.direct.selected.last:
            obj = self.objectMgr.findObjectByNodePath(base.direct.selected.last)
            if obj:
                exportRootNP = obj[OG.OBJ_NP]

        exportRootNP.writeBamFile(bamFileName)
        mayaConverter = MayaConverter(self.ui, self, mayaFileName, None, None, False, FROM_BAM_TO_MAYA)
        mayaConverter.Show()

    def updateStatusReadout(self, status, color=None):
        if status:
            # add new status line, first check to see if it already exists
            alreadyExists = False
            for currLine in self.statusLines:
                if (status == currLine[1]):
                    alreadyExists = True
                    break
            if (alreadyExists == False):
                time = globalClock.getRealTime() + 15
                self.statusLines.append([time,status,color])

        # update display of new status lines
        self.statusReadout.reparentTo(aspect2d)
        statusText = ""
        lastColor = None
        for currLine in self.statusLines:
            statusText += currLine[1] + '\n'
            lastColor = currLine[2]
        self.statusReadout.setText(statusText)
        if (lastColor):
            self.statusReadout.textNode.setCardColor(
                lastColor[0], lastColor[1], lastColor[2], lastColor[3])
            self.statusReadout.textNode.setCardAsMargin(0.1, 0.1, 0.1, 0.1)
        else:
            self.statusReadout.textNode.setCardColor(1,1,1,1)
            self.statusReadout.textNode.setCardAsMargin(0.1, 0.1, 0.1, 0.1)

    def updateStatusReadoutTimeouts(self,task=None):
        removalList = []
        for currLine in self.statusLines:
            if (globalClock.getRealTime() >= currLine[0]):
                removalList.append(currLine)
        for currRemoval in removalList:
            self.statusLines.remove(currRemoval)
        self.updateStatusReadout(None)
        # perform doMethodLater again after delay
        # This crashes when CTRL-C'ing, so this is a cheap hack.
        #return 2
        from direct.task import Task
        return Task.again

    def propMeetsReq(self, typeName, parentNP):
        if self.ui.parentToSelectedMenuItem.IsChecked():
           if base.direct.selected.last:
              parent = base.le.objectMgr.findObjectByNodePath(base.direct.selected.last)
              if parent:
                 parentNP[0] = parent[OG.OBJ_NP]
        else:
           parentNP[0] = None
        return True
