"""
Base class for Level Editor

You should write your own LevelEditor class inheriting this.
Refer LevelEditor.py for example.
"""

from pandac.PandaModules import *
from direct.showbase.ShowBase import *
from direct.showbase.DirectObject import *
from direct.directtools.DirectGlobals import *

base = ShowBase(False)

from ObjectMgr import *
from FileMgr import *
from ActionMgr import *
from MayaConverter import *

class LevelEditorBase(DirectObject):
    """ Base Class for Panda3D LevelEditor """ 
    def __init__(self):
        #loadPrcFileData('startup', 'window-type none')
        self.currentFile = None
        self.actionEvents = []
        self.objectMgr = ObjectMgr(self)
        self.fileMgr = FileMgr(self)
        self.actionMgr = ActionMgr()

        # define your own config file in inherited class
        self.settingsFile = None
        
    def initialize(self):
        """ You should call this in your __init__ method of inherited LevelEditor class """
        fTk = 0
        fWx = 0
        base.startDirect(fWantTk = fTk, fWantWx = fWx)

        base.closeWindow(base.win)
        base.win = base.winList[3]

        base.direct.disableMouseEvents()
        newMouseEvents = map(lambda x: "_le_per_%s"%x, base.direct.mouseEvents) +\
                         map(lambda x: "_le_fro_%s"%x, base.direct.mouseEvents) +\
                         map(lambda x: "_le_lef_%s"%x, base.direct.mouseEvents) +\
                         map(lambda x: "_le_top_%s"%x, base.direct.mouseEvents)
        base.direct.mouseEvents = newMouseEvents
        base.direct.enableMouseEvents()

        base.direct.disableKeyEvents()
        keyEvents = map(lambda x: "_le_per_%s"%x, base.direct.keyEvents) +\
                         map(lambda x: "_le_fro_%s"%x, base.direct.keyEvents) +\
                         map(lambda x: "_le_lef_%s"%x, base.direct.keyEvents) +\
                         map(lambda x: "_le_top_%s"%x, base.direct.keyEvents)
        base.direct.keyEvents = keyEvents
        base.direct.enableKeyEvents()

        base.direct.disableModifierEvents()
        modifierEvents = map(lambda x: "_le_per_%s"%x, base.direct.modifierEvents) +\
                         map(lambda x: "_le_fro_%s"%x, base.direct.modifierEvents) +\
                         map(lambda x: "_le_lef_%s"%x, base.direct.modifierEvents) +\
                         map(lambda x: "_le_top_%s"%x, base.direct.modifierEvents)
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

        for widget in base.direct.manipulationControl.widgetList:
            widget.setBin('gui-popup', 0)
            widget.setDepthTest(0)

        # [gjeon] to intercept messages here
        base.direct.ignore('DIRECT-delete')
        base.direct.ignore('DIRECT-select')
        base.direct.ignore('DIRECT-preDeselectAll')
        base.direct.fIgnoreDirectOnlyKeyMap = 1
        
        # [gjeon] do not use the old way of finding current DR
        base.direct.drList.tryToGetCurrentDr = False

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
            ])

        # Add all the action events
        for event in self.actionEvents:
            if len(event) == 3:
                self.accept(event[0], event[1], event[2])
            else:
                self.accept(event[0], event[1])        

        self.loadSettings()
        
    def removeNodePathHook(self, nodePath):
        if nodePath is None:
            return
        base.direct.deselect(nodePath)
        self.objectMgr.removeObjectByNodePath(nodePath)

        if (base.direct.selected.last != None and nodePath.compareTo(base.direct.selected.last)==0):
            # if base.direct.selected.last is refering to this
            # removed obj, clear the reference
            if (hasattr(__builtins__,'last')):
                __builtins__.last = None
            else:
                __builtins__['last'] = None
            base.direct.selected.last = None

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

    def deselectAll(self, np=None):
        if len(base.direct.selected.getSelectedAsList()) ==0:
            return
        action = ActionDeselectAll(self)
        self.actionMgr.push(action)
        action()

    def deselectAllCB(self, dnp=None):
        self.objectMgr.deselectAll()

    def reset(self):
        base.direct.deselectAll()
        self.objectMgr.reset()
        self.actionMgr.reset()
        self.ui.perspView.camera.setPos(-19, -19, 19)
        self.ui.leftView.camera.setPos(600, 0, 0)
        self.ui.frontView.camera.setPos(0, -600, 0)
        self.ui.topView.camera.setPos(0, 0, 600)
        self.resetOrthoCam(self.ui.topView)
        self.resetOrthoCam(self.ui.frontView)
        self.resetOrthoCam(self.ui.leftView)
        
    def resetOrthoCam(self, view):
        base.direct.drList[base.camList.index(NodePath(view.camNode))].orthoFactor = 0.1
        x = view.ClientSize.GetWidth() * 0.1
        y = view.ClientSize.GetHeight() * 0.1
        view.camLens.setFilmSize(x, y)
        
    def save(self):
        if self.currentFile:
            self.fileMgr.saveToFile(self.currentFile)

    def saveAs(self, fileName):
        self.fileMgr.saveToFile(fileName)

    def load(self, fileName):
        self.reset()
        self.fileMgr.loadFromFile(fileName)
        self.currentFile = fileName

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
                    base.direct.hotKeyMap.update(eval(configLines[i]))

            self.ui.updateGrids(gridSize, gridSpacing)
        except:
            pass


    def convertMaya(self, modelname, obj=None, isAnim=False):
        if obj and isAnim:
            mayaConverter = MayaConverter(self.ui, self, modelname, obj, isAnim)
        else:
            reply = wx.MessageBox("Is it an animation file?", "Animation?",
                              wx.YES_NO | wx.ICON_QUESTION)
            if reply == wx.YES:
                mayaConverter = MayaConverter(self.ui, self, modelname, None, True)
            else:        
                mayaConverter = MayaConverter(self.ui, self, modelname, None, False)
        mayaConverter.Show()
