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

class LevelEditorBase(DirectObject):
    """ Base Class for Panda3D LevelEditor """ 
    def __init__(self):
        #loadPrcFileData('startup', 'window-type none')
        self.currentFile = None
        self.actionEvents = []
        self.objectMgr = ObjectMgr(self)
        self.fileMgr = FileMgr(self)

        # define your own config file in inherited class
        self.settingsFile = None
        
    def initialize(self):
        """ You should call this in your __init__ method of inherited LevelEditor class """
        fTk = base.config.GetBool('want-tk', 0)
        fWx = 0
        base.startDirect(fWantTk = fTk, fWantWx = fWx)
        base.closeWindow(base.win)
        base.win = base.winList[3]

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
        
        # [gjeon] to handle delete here first
        base.direct.ignore('DIRECT-delete')
        
        # [gjeon] do not use the old way of finding current DR
        base.direct.drList.tryToGetCurrentDr = False

        # specifiy what obj can be 'selected' as objects
        base.direct.selected.addTag('OBJRoot')

        self.actionEvents.extend([
            # Node path events
            ('DIRECT-delete', self.handleDelete),
            ('preRemoveNodePath', self.removeNodePathHook),
            ('DIRECT_selectedNodePath_fMulti_fTag', self.selectedNodePathHook),
            ('DIRECT_deselectAll', self.deselectAll),
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
        reply = wx.MessageBox("Do you want to delete selected?", "Delete?",
                              wx.YES_NO | wx.ICON_QUESTION)
        if reply == wx.YES:
            base.direct.removeAllSelected()
            self.objectMgr.deselectAll()
        else:
            # need to reset COA
            dnp = base.direct.selected.last
            # Update camera controls coa to this point
            # Coa2Camera = Coa2Dnp * Dnp2Camera
            mCoa2Camera = dnp.mCoa2Dnp * dnp.getMat(base.direct.camera)
            row = mCoa2Camera.getRow(3)
            coa = Vec3(row[0], row[1], row[2])
            base.direct.cameraControl.updateCoa(coa)

    def selectedNodePathHook(self, nodePath, fMultiSelect = 0, fSelectTag = 1):
        # handle unpickable nodepath
        if nodePath.getName() in base.direct.iRay.unpickable:
            base.direct.deselect(nodePath)
            return

        self.objectMgr.selectObject(nodePath)

    def deselectAll(self):
        self.objectMgr.deselectAll()

    def reset(self):
        base.direct.deselectAll()
        self.objectMgr.reset()

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
            f.write('gridSize %f\n'%self.ui.perspView.grid.gridSize)
            f.write('gridSpacing %f\n'%self.ui.perspView.grid.gridSpacing)
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
            for line in configLines:
                if line.startswith('gridSize'):
                    gridSize = float(line.split()[1])
                elif line.startswith('gridSpacing'):
                    gridSpacing = float(line.split()[1])

            self.ui.updateGrids(gridSize, gridSpacing)
        except:
            pass

