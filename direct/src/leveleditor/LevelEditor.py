from PandaObject import *
from PieMenu import *
from OnscreenText import *
from Tkinter import *
from DirectGeometry import *
from SceneGraphExplorer import *
from tkSimpleDialog import askstring
from tkFileDialog import *
from whrandom import *
import Pmw
import EntryScale
import VectorWidgets
import string


class LevelEditor(NodePath, PandaObject):
    """Class used to create a Toontown LevelEditor object"""
    def __init__(self,direct,parent = None):
        # Initialize superclass
        NodePath.__init__(self)
        # Become the new node path
        self.assign(hidden.attachNewNode( NamedNode('LevelEditor')))
        # Record handle to direct session
        self.direct = direct
	# Make sure direct is running
	self.direct.enable()
        # And only the appropriate handles are showing
        self.direct.widget.disableHandles(['x-ring', 'x-disc',
                                           'y-ring', 'y-disc',
                                           'z-post'])
	# CREATE LEVEL EDITOR DICTIONARIES
	# This dictionary stores information about new objects added
        # to the level
	self.levelDictionary = {}
	# This dictionary stores information about the various
        # pie menus in use
	self.pieMenuDictionary = {}
	# This dictionary stores info about current and possible
        # object attributes
	self.attributeDictionary = {}
	# This dictionary stores pleasing style combinations
	self.styleDictionary = {}
	# This dictionary stores all the different color palettes
	self.colorPaletteDictionary = {}
        # This dictionary stores pointers to the various maps
        self.mapDictionary = {}
        self.activeMap = None

	# DNAStorage instance for storing level DNA info
	self.dnaStore = DNAStorage()
	loadDNAFile(self.dnaStore, 'dna/storage.dna', CSDefault)
	# Top level DNA Data Object
	self.groupParentDNA = self.levelObjectsDNA = DNAData('LevelObjects')
	# Create top level node
	self.groupParent = self.levelObjects = self.attachNewNode(
            NamedNode('LevelObjects'))
	# Create a top level group
	self.createTopLevelGroup()

	self.selectedLevelObject = None
	self.targetDNAObject = None
	self.activeMenu = None

        self.dnaOutputDir = 'ToontownCentral'
        self.dnaOutputFile = 'toontown_working.dna'


        # Get a handle to the grid
	self.grid = self.direct.grid
        self.showGrid(0)

        self.levelMap = hidden.attachNewNode(NamedNode('level-map'))

	map = loader.loadModel('level_editor/toontown_central_layout')
	map.getBottomArc().setTransition(TransparencyTransition(1))
	map.setColor(Vec4(1,1,1,.4))
        self.mapDictionary['toontownCentral'] = map

	map = loader.loadModel('level_editor/donalds_dock_layout')
	map.getBottomArc().setTransition(TransparencyTransition(1))
	map.setColor(Vec4(1,1,1,.4))
        self.mapDictionary['donaldsDock'] = map

	map = loader.loadModel('level_editor/minnies_melody_land_layout')
	map.getBottomArc().setTransition(TransparencyTransition(1))
	map.setColor(Vec4(1,1,1,.4))
        self.mapDictionary['minniesMelodyLand'] = map

	map = loader.loadModel('level_editor/the_burrrgh_layout')
	map.getBottomArc().setTransition(TransparencyTransition(1))
	map.setColor(Vec4(1,1,1,.4))
        self.mapDictionary['theBurrrgh'] = map

	self.hitPt = Point3(0)
	self.offset = Point3(0)
	self.crankOrigin = Point3(0)
	self.crankDir = Vec3(0)

	self.hprSnap = 1
	self.snapAngle = 15.0
	self.lastAngle = 0.0

	# Create Level Editor Panel and pie menus
	# Initialize styles
	self.initializeStyleDictionary()
	# Initialize pie Menus (this depends on the style dictionary)
	self.initializePieMenus()

        self.panel = LevelEditorPanel(self, parent)

	base.cam.node().setNear(5.0)
	base.cam.node().setFar(10000)
	self.direct.camera.setPos(0,-10,10)

	# Default is to use the toontown central color palette
	self.editToontownCentral()

	self.enable()

    def initializeAttributeDictionary(self):
	# Retrieve lists of available attributes from DNAStorage object
	# Cornices
	attributeList = self.getCatalogCodesSuffix('cornice', '_ur')
        # Make the first one a null texture
        attributeList[:0] = [None]
	self.attributeDictionary['corniceTextures'] = attributeList
	self.attributeDictionary['corniceTexture'] = attributeList[1]
	self.attributeDictionary['corniceOrienation'] = '_ur'

	# Doors
	attributeList = self.getCatalogCodesSuffix('door', '_ur')
	self.attributeDictionary['doorTextures'] = attributeList
	self.attributeDictionary['doorTexture'] = attributeList[1]
	self.attributeDictionary['doorOrienation'] = '_ur'

	# FlatBuildings
	self.attributeDictionary['buildingHeight'] = 20.0

	# Props
	attributeList = self.getCatalogCodes('prop')
	self.attributeDictionary['propTypes'] = attributeList
	self.attributeDictionary['propType'] = attributeList[1]

	# Walls 
	attributeList = self.getCatalogCodesSuffix('wall', '_ur')
	self.attributeDictionary['wallTextures'] = attributeList
	self.attributeDictionary['wallTexture'] = attributeList[1]
	self.attributeDictionary['wallOrienation'] = '_ur'
	self.attributeDictionary['wallWidth'] = 15.0

	# Windows
	attributeList = self.getCatalogCodesSuffix('window', '_ur')
	self.attributeDictionary['windowTextures'] = attributeList
	self.attributeDictionary['windowTexture'] = attributeList[1]
	self.attributeDictionary['windowOrienation'] = '_ur'
	self.attributeDictionary['numWindows'] = 0

	# Streets
	attributeList = self.getCatalogCodes('street')
	self.attributeDictionary['streetTypes'] = attributeList
	self.attributeDictionary['streetType'] = attributeList[1]

    def getActiveColor(self):
	return self.attributeDictionary['activeColor']

    def setActiveColor(self, color):
	self.attributeDictionary['activeColor'] = color

    def getBuildingHeight(self):
	return self.attributeDictionary['buildingHeight']

    def setBuildingHeight(self, height):
	self.attributeDictionary['buildingHeight'] = height

    def getCorniceColor(self):
	return self.attributeDictionary['corniceColor']

    def getCorniceColors(self):
	return self.attributeDictionary['corniceColors']

    def getCorniceTexture(self):
	return self.attributeDictionary['corniceTexture']

    def setCorniceTexture(self, dnaString):
	self.attributeDictionary['corniceTexture'] = dnaString

    def getCorniceTextures(self):
	return self.attributeDictionary['corniceTextures']

    def getPropColor(self):
	return self.attributeDictionary['propColor']

    def getPropColors(self):
	return self.attributeDictionary['propColors']

    def getDnaStore(self):
	return self.dnaStore

    def getDoorColor(self):
	return self.attributeDictionary['doorColor']

    def getDoorColors(self):
	return self.attributeDictionary['doorColors']

    def getDoorTexture(self):
	return self.attributeDictionary['doorTexture']

    def setDoorTexture(self,dnaString):
	self.attributeDictionary['doorTexture'] = dnaString

    def getDoorTextures(self):
	return self.attributeDictionary['doorTextures']

    def getGrid(self):
	return self.grid

    def getGroupNum(self):
	return self.groupNum

    def setGroupNum(self,num):
	self.groupNum = num

    def setGroupParentToSelected(self):
        if self.direct.selected.last:
            self.setGroupParent(self.direct.selected.last)
        
    def setGroupParent(self,nodePath):
	parentDNA = self.getDNAGroup(nodePath)
        if parentDNA:
            self.groupParent = nodePath
            self.groupParentDNA = parentDNA

    def getLevelDictionary(self):
	return self.levelDictionary

    def getLevelMap(self):
	return self.levelMap
    
    def getLevelObjects(self):
	return self.levelObjects
    
    def getLevelObjectsDNA(self):
	return self.levelObjectsDNA
    
    def getNumWindows(self):
	return self.attributeDictionary.get('numWindows', 0)
    
    def setNumWindows(self, numWindows):
	self.attributeDictionary['numWindows'] = numWindows
        
    def printWallStyle(self):
        if self.selectedLevelObject:
            dnaObject = self.selectedLevelObject['DNA']
            objectClass = dnaObject.__class__.getClassType()
            if objectClass.eq(DNAFlatBuilding.getClassType()):
                self.printWallStyleFor(dnaObject)
                
    def printWallStyleFor(self, DNAFlatBuilding):
        wall = self.getLastWall(DNAFlatBuilding)
	cornice = self.getCornice(DNAFlatBuilding)
	self.printWallStyleWith(wall, cornice)

    def printWallStyleWith(self, wall, cornice):
        if wall:
            print 'self.addStyle(styleCount = styleCount + 1)'
            print 'wallTexture: ', self.getDNAString(wall)
            print 'wallColor: #(', wall.getColor(), ')'
            window = self.getWindow(wall,0)
            if window & (window.getWindowCount() > 0):
                print 'windowTexture: ', self.getDNAString(window)
                print 'windowColor: #(', window.getColor(), ')'
            else:
                print 'windowTexture: None'
                print 'windowColor: None'
            if cornice:
                print 'corniceTexture: ', self.getDNAString(cornice)
                print 'corniceColor: #(', cornice.getColor(), ')'
            else:
                print'corniceTexture: None'
                print'corniceColor: None'

    def printWallStyleForBldgWall(self, aDNAFlatBuilding, wallNum):
        if (aDNAFlatBuilding.getClassType().eq(DNAFlatBuilding.getClassType())):
            wall = self.getWall(aDNAFlatBuilding, wallNum)
            cornice = self.getCornice(aDNAFlatBuilding)
            self.printWallStyleWith(wall,cornice)

    def getPropType(self):
	return self.attributeDictionary['propType']

    def setPropType(self,dnaString):
	self.attributeDictionary['propType'] = dnaString

    def getPropTypes(self):
	return self.attributeDictionary['propTypes']

    def getStreetType(self):
	return self.attributeDictionary['streetType']

    def setStreetType(self,dnaString):
	self.attributeDictionary['streetType'] = dnaString

    def getStreetTypes(self):
	return self.attributeDictionary['streetTypes']

    def getSelectedLevelObject(self):
	return self.selectedLevelObject

    def getSnapAngle(self):
	return self.grid.getSnapAngle()

    def setSnapAngle(self, aFloat):
	self.grid.setSnapAngle(aFloat)

    def getWallColor(self):
	return self.attributeDictionary['wallColor']

    def setWallColor(self,aColor):
	self.attributeDictionary['wallColor'] = aColor

    def getWallColors(self):
	return self.attributeDictionary['wallColors']

    def getWallMenu(self):
	return wallMenu

    def getWallTexture(self):
	return self.attributeDictionary['wallTexture']

    def setWallTexture(self, texture):
        self.attributeDictionary['wallTexture'] = texture

    def getWallTextureDNA(self, dnaString):
	self.attributeDictionary['wallTexture'] = dnaString

    def getWallTextures(self):
	return self.attributeDictionary['wallTextures']

    def getWallWidth(self):
	return self.attributeDictionary['wallWidth']

    def setWallWidthVal(self,aFloat):
	self.attributeDictionary['wallWidth'] = aFloat

    def setWallWidthString(self, width):
        if width == 'fiveFt':
            self.setWallWidthVal(5.0)
        elif width == 'tenFt':
            self.setWallWidthVal(10.0)
        elif width == 'fifteenFt':
            self.setWallWidthVal(10.0)
        elif width == 'twentyFt':
            self.setWallWidthVal(10.0)
        elif width == 'twentyFiveFt':
            self.setWallWidthVal(10.0)

    def getWindowColor(self):
	return self.attributeDictionary['windowColor']

    def getWindowColors(self):
	return self.attributeDictionary['windowColors']

    def getWindowTexture(self):
	return self.attributeDictionary['windowTexture']

    def setWindowTexture(self,dnaString):
	self.attributeDictionary['windowTexture'] = dnaString

    def getWindowTextures(self):
	return self.attributeDictionary['windowTextures']

    def enable(self):
        self.reparentTo(render)
	self.show()
        self.accept('selectedNodePath', self.selectDNARoot)
	self.accept('preRemoveNodePath', self.preRemoveNodePath)
	self.accept('toggleMapViz', self.toggleMapViz)
	self.accept('reparentNodePath', self.reparentNodePath)
	self.accept('createNewLevelGroup', self.createNewLevelGroup)
	self.accept('setNodePathName', self.setNodePathName)
        self.accept('manipulateObjectCleanup', self.updateSelectedPose)
	self.accept('SGEFlashNodePath', self.flashNodePath)
        self.accept('SGESelectNodePath', self.selectNodePath)
        self.accept('SGEIsolateNodePath', self.isolateNodePath)
        self.accept('SGEToggle VizNodePath', self.toggleNodePathViz)
        self.accept('SGESet ParentNodePath', self.setGroupParent)
        self.accept('SGEAdd GroupNodePath', self.addGroupToSelected)
	self.accept('showAll', self.showAll)
	self.accept('p',self.plantSelectedNodePath)
	self.enableManipulation()

    def disable(self):
	self.direct.deselectAll()
        self.reparentTo(hidden)
	self.hide()
	self.grid.ignore('insert')
        self.ignore('selectedNodePath')
	self.ignore('preRemoveNodePath')
        self.ignore('toggleMapViz')
	self.ignore('reparentNodePath')
	self.ignore('createNewLevelGroup')
        self.ignore('setNodePathName')
        self.ignore('manipulateObjectCleanup')
        self.ignore('SGESelectNodePath')
        self.ignore('SGEIsolateNodePath')
        self.ignore('SGEToggle VizNodePath')
        self.ignore('SGESet ParentNodePath')
        self.ignore('SGEAdd GroupNodePath')
	self.ignore('showAll')
	self.ignore('p')
	self.disableManipulation()

    def destroy(self):
	self.disable()
	self.removeNode()

    def resetLevel(self):
	# Clear out all objects
	self.direct.deselectAll()
	children = self.levelObjects.getChildren()
        for i in range(children.getNumPaths()):
            path = children.getPath(i)
            path.reparentTo(hidden)
            path.remove()

	# Create fresh DNA Object
	self.levelObjectsDNA = DNAData('LevelObjects')

	# Create new levelDictionary
	self.levelDictionary = {}

	# Create root node
	self.createTopLevelGroup()

	self.grid.setPosHpr(0,0,0,0,0,0)

    def disableManipulation(self):
	# Disable handling of mouse events
	self.ignore('handleMouse3')
	self.ignore('handleMouse3Up')

    def editToontownCentral(self):
        self.editMode = 'toontownCentral'
	self.levelMap.setPos(0.0,0.0,0.0)
        self.showMap('toontownCentral')
	self.useToontownCentralColors()
	self.styleDictionary = (
            self.attributeDictionary['toontownCentralStyleDictionary'])
        self.pieMenuDictionary['styleMenu'] = (
            self.pieMenuDictionary['toontownCentralStyleMenu'])
	self.attributeDictionary['streetTexture'] = 'street_street_TT_tex'
	self.attributeDictionary['sidewalkTexture'] = 'street_sidewalk_TT_tex'
        self.dnaOutputDir = 'ToontownCentral'
        self.dnaOutputFile = 'toontown_central_working.dna'
        self.panel.editMenu.selectitem('Toontown Central')

    def editDonaldsDock(self):
        self.editMode = 'donaldsDock'
	self.levelMap.setPos(0.0,0.0,0.0)
        self.showMap('donaldsDock')
	self.useDonaldsDockColors()
	self.styleDictionary = (
            self.attributeDictionary['donaldsDockStyleDictionary'])
	self.pieMenuDictionary['styleMenu'] = (
            self.pieMenuDictionary['donaldsDockStyleMenu'])
	self.attributeDictionary['streetTexture'] = 'street_street_DD_tex'
	self.attributeDictionary['sidewalkTexture'] = (
            'street_sidewalk_DD_tex')
        self.dnaOutputDir = 'DonaldsDock'
        self.dnaOutputFile = 'donalds_dock_working.dna'
        self.panel.editMenu.selectitem('Donalds Dock')

    def editMinniesMelodyLand(self):
        self.editMode = 'minniesMelodyLand'
	self.levelMap.setPos(0.0,0.0,0.0)
        self.showMap('minniesMelodyLand')
	self.useMinniesMelodyLandColors()
	self.styleDictionary = (
            self.attributeDictionary['minniesMelodyLandStyleDictionary'])
	self.pieMenuDictionary['styleMenu'] = (
            self.pieMenuDictionary['minniesMelodyLandStyleMenu'])
	self.attributeDictionary['streetTexture'] = 'street_street_MM_tex'
	self.attributeDictionary['sidewalkTexture'] = (
            'street_sidewalk_MM_tex')
        self.dnaOutputDir = 'MinniesMelodyLand'
        self.dnaOutputFile = 'minnies_melody_land_working.dna'
        self.panel.editMenu.selectitem('Minnies Melody Land')

    def editTheBurrrgh(self):
        self.editMode = 'theBurrrgh'
	self.levelMap.setPos(0.0,0.0,0.0)
        self.showMap('theBurrrgh')
	self.useTheBurrrghColors()
	self.styleDictionary = (
            self.attributeDictionary['theBurrrghStyleDictionary'])
	self.pieMenuDictionary['styleMenu'] = (
            self.pieMenuDictionary['theBurrrghStyleMenu'])
	self.attributeDictionary['streetTexture'] = 'street_street_BR_tex'
	self.attributeDictionary['sidewalkTexture'] = (
            'street_sidewalk_BR_tex')
        self.dnaOutputDir = 'TheBurrrgh'
        self.dnaOutputFile = 'the_burrrgh_working.dna'
        self.panel.editMenu.selectitem('The Burrrgh')

    def showMap(self, mapName):
        if self.activeMap:
            self.activeMap.reparentTo(hidden)
        self.activeMap = self.mapDictionary[mapName]
        self.activeMap.reparentTo(self.levelMap)

    def enableManipulation(self):
	# Enable interactive placement of a nodePath
	# Turn off camera control
	base.disableMouse()

	# Handle mouse events for pie menus
	self.accept('handleMouse3',self.levelHandleMouse3)
	self.accept('handleMouse3Up',self.levelHandleMouse3Up)

    def useDriveMode(self):
        pos = base.camera.getPos()
        pos.setZ(4.0)
        hpr = base.camera.getHpr()
        hpr.set(hpr[0], 0.0, 0.0)
        t = base.camera.lerpPosHpr(pos, hpr, 1.0, blendType = 'easeInOut',
                                   task = 'manipulateCamera')
        # Note, if this dies an unatural death, this could screw things up
        t.uponDeath = self.switchToDriveMode

    def switchToDriveMode(self,state):
        self.direct.minimumConfiguration()
        self.disableManipulation()
        base.useDrive()
        # Make sure we're where we want to be
        pos = base.camera.getPos()
        pos.setZ(4.0)
        hpr = base.camera.getHpr()
        hpr.set(hpr[0], 0.0, 0.0)
        # Fine tune the drive mode
        base.mouseInterface.getBottomNode().setPos(pos)
        base.mouseInterface.getBottomNode().setHpr(hpr)
        base.mouseInterface.getBottomNode().setForwardSpeed(20.0)
        base.mouseInterface.getBottomNode().setReverseSpeed(20.0)

    def useDirectFly(self):
        base.disableMouse()
        self.enableManipulation()
        self.direct.enable()

    def useToontownCentralColors(self):
	self.attributeDictionary['wallColors'] = (
            self.colorPaletteDictionary['toontownCentralWallColors'])
	self.attributeDictionary['wallColor'] = (
            self.attributeDictionary['wallColors'][1])
	self.pieMenuDictionary['wallColorMenu'] = (
            self.pieMenuDictionary['toontownCentralWallColors'])

	self.attributeDictionary['windowColors'] = (
            self.colorPaletteDictionary['toontownCentralWindowColors'])
	self.attributeDictionary['windowColor'] = (
            self.attributeDictionary['windowColors'][1])
	self.pieMenuDictionary['windowColorMenu'] = (
            self.pieMenuDictionary['toontownCentralWindowColors'])

	self.attributeDictionary['doorColors'] = (
            self.colorPaletteDictionary['toontownCentralDoorColors'])
	self.attributeDictionary['doorColor'] = (
            self.attributeDictionary['doorColors'][1])
	self.pieMenuDictionary['doorColorMenu'] = (
            self.pieMenuDictionary['toontownCentralDoorColors'])

	self.attributeDictionary['corniceColors'] = (
            self.colorPaletteDictionary['toontownCentralCorniceColors'])
	self.attributeDictionary['corniceColor'] = (
            self.attributeDictionary['corniceColors'][1])
	self.pieMenuDictionary['corniceColorMenu'] = (
            self.pieMenuDictionary['toontownCentralCorniceColors'])

	self.attributeDictionary['propColors'] = (
            self.colorPaletteDictionary['toontownCentralPropColors'])
	self.attributeDictionary['propColor'] = (
            self.attributeDictionary['propColors'][1])
	self.pieMenuDictionary['propColorMenu'] = (
            self.pieMenuDictionary['toontownCentralPropColors'])

    def useDonaldsDockColors(self):
	self.attributeDictionary['wallColors'] = (
            self.colorPaletteDictionary['donaldsDockWallColors'])
	self.attributeDictionary['wallColor'] = (
            self.attributeDictionary['wallColors'][1])
	self.pieMenuDictionary['wallColorMenu'] = (
            self.pieMenuDictionary['donaldsDockWallColors'])

	self.attributeDictionary['windowColors'] = (
            self.colorPaletteDictionary['donaldsDockWindowColors'])
	self.attributeDictionary['windowColor'] = (
            self.attributeDictionary['windowColors'][1])
	self.pieMenuDictionary['windowColorMenu'] = (
            self.pieMenuDictionary['donaldsDockWindowColors'])

	self.attributeDictionary['doorColors'] = (
            self.colorPaletteDictionary['donaldsDockDoorColors'])
	self.attributeDictionary['doorColor'] = (
            self.attributeDictionary['doorColors'][1])
	self.pieMenuDictionary['doorColorMenu'] = (
            self.pieMenuDictionary['donaldsDockDoorColors'])

	self.attributeDictionary['corniceColors'] = (
            self.colorPaletteDictionary['donaldsDockCorniceColors'])
	self.attributeDictionary['corniceColor'] = (
            self.attributeDictionary['corniceColors'][1])
	self.pieMenuDictionary['corniceColorMenu'] = (
            self.pieMenuDictionary['donaldsDockCorniceColors'])

	self.attributeDictionary['propColors'] = (
            self.colorPaletteDictionary['donaldsDockPropColors'])
	self.attributeDictionary['propColor'] = (
            self.attributeDictionary['propColors'][1])
	self.pieMenuDictionary['propColorMenu'] = (
            self.pieMenuDictionary['donaldsDockPropColors'])

    def useTheBurrrghColors(self):
	self.attributeDictionary['wallColors'] = (
            self.colorPaletteDictionary['theBurrrghWallColors'])
	self.attributeDictionary['wallColor'] = (
            self.attributeDictionary['wallColors'][1])
	self.pieMenuDictionary['wallColorMenu'] = (
            self.pieMenuDictionary['theBurrrghWallColors'])

	self.attributeDictionary['windowColors'] = (
            self.colorPaletteDictionary['theBurrrghWindowColors'])
	self.attributeDictionary['windowColor'] = (
            self.attributeDictionary['windowColors'][1])
	self.pieMenuDictionary['windowColorMenu'] = (
            self.pieMenuDictionary['theBurrrghWindowColors'])

	self.attributeDictionary['doorColors'] = (
            self.colorPaletteDictionary['theBurrrghDoorColors'])
	self.attributeDictionary['doorColor'] = (
            self.attributeDictionary['doorColors'][1])
	self.pieMenuDictionary['doorColorMenu'] = (
            self.pieMenuDictionary['theBurrrghDoorColors'])

	self.attributeDictionary['corniceColors'] = (
            self.colorPaletteDictionary['theBurrrghCorniceColors'])
	self.attributeDictionary['corniceColor'] = (
            self.attributeDictionary['corniceColors'][1])
	self.pieMenuDictionary['corniceColorMenu'] = (
            self.pieMenuDictionary['theBurrrghCorniceColors'])

	self.attributeDictionary['propColors'] = (
            self.colorPaletteDictionary['theBurrrghPropColors'])
	self.attributeDictionary['propColor'] = (
            self.attributeDictionary['propColors'][1])
	self.pieMenuDictionary['propColorMenu'] = (
            self.pieMenuDictionary['theBurrrghPropColors'])

    def useMinniesMelodyLandColors(self):
	self.attributeDictionary['wallColors'] = (
            self.colorPaletteDictionary['minniesMelodyLandWallColors'])
	self.attributeDictionary['wallColor'] = (
            self.attributeDictionary['wallColors'][1])
	self.pieMenuDictionary['wallColorMenu'] = (
            self.pieMenuDictionary['minniesMelodyLandWallColors'])

	self.attributeDictionary['windowColors'] = (
            self.colorPaletteDictionary['minniesMelodyLandWindowColors'])
	self.attributeDictionary['windowColor'] = (
            self.attributeDictionary['windowColors'][1])
	self.pieMenuDictionary['windowColorMenu'] = (
            self.pieMenuDictionary['minniesMelodyLandWindowColors'])

	self.attributeDictionary['doorColors'] = (
            self.colorPaletteDictionary['minniesMelodyLandDoorColors'])
	self.attributeDictionary['doorColor'] = (
            self.attributeDictionary['doorColors'][1])
	self.pieMenuDictionary['doorColorMenu'] = (
            self.pieMenuDictionary['minniesMelodyLandDoorColors'])

	self.attributeDictionary['corniceColors'] = (
            self.colorPaletteDictionary['minniesMelodyLandCorniceColors'])
	self.attributeDictionary['corniceColor'] = (
            self.attributeDictionary['corniceColors'][1])
	self.pieMenuDictionary['corniceColorMenu'] = (
            self.pieMenuDictionary['minniesMelodyLandCorniceColors'])

	self.attributeDictionary['propColors'] = (
            self.colorPaletteDictionary['minniesMelodyLandPropColors'])
	self.attributeDictionary['propColor'] = (
            self.attributeDictionary['propColors'][1])
	self.pieMenuDictionary['propColorMenu'] = (
            self.pieMenuDictionary['minniesMelodyLandPropColors'])

    def addCollisionSphere(self):
	sphere = self.vizRegion.attachNewNode(NamedNode('vizSphere'))
	instance = self.vizSphere.instanceTo(sphere)
	instance.setScale(20.0 * math.sqrt(2))
        if (self.vizRegion.getNumChildren() > 1):
            sphere.setPos(vizRegion.getChild(vizRegion.getNumChildren() - 2),
                          40.0,0.0,0.0)
        else:
            sphere.setPos(0,0,0)
        self.selectNodePath(sphere)

    def addVizRegion(self):
        self.vizRegionCount = self.vizRegionCount + 1
        self.vizRegion = self.vizObjects.attachNewNode(
            NamedNode('vizRegion' + `self.vizRegionCount`))
	self.addCollisionSphere()
	# Start with region selected
	self.selectNodePath(vizRegion)

    def setHprSnap(self,flag):
        self.hprSnap = flag

    def isolateSelectedNodePath(self):
        if self.direct.selected.last:
            self.isolateNodePath(self.direct.selected.last)
    
    def isolateNodePath(self,aNodePath):
	# First show everything in level
	self.levelObjects.showAllDescendants()
	render.hideCollisionSolids()
	aNodePath.hideSiblings()

    def selectDNARoot(self, aNodePath):
        # If this isn't a root object see if one exists above it
        if (aNodePath.getName()[-8:] != '_DNARoot'):
            dnaRoot = self.getDNARoot(aNodePath)
            # Is this a DNA object?
            if dnaRoot:
                # Yes! Select root
                self.direct.select(dnaRoot)

    def getDNARoot(self, aNodePath):
        if not aNodePath.hasParent():
            return 0
        name = aNodePath.getName()
        if (name[-8:] == '_DNARoot'):
            return aNodePath
        else:
            return self.getDNARoot(aNodePath.getParent())

    def updateSelectedPose(self):
	# Move grid to line up with object
        for selectedNode in self.direct.selected:
            if self.levelDictionary.has_key(selectedNode.id()):
                # First snap to grid
                pos = selectedNode.getPos(self.grid)
                snapPos = self.grid.computeSnapPoint(pos)
                selectedNode.setPos(self.grid, snapPos[0], snapPos[1], 0)
                # Angle snap
                self.lastAngle = self.grid.computeSnapAngle(
                    selectedNode.getH(self.grid))
                selectedNode.setH(self.grid, self.lastAngle)
                # Update DNA
                self.updateDNAPosHpr(selectedNode)

                # If this node is the last selected reposition grid
                if selectedNode == self.direct.selected.last:
                    # Position grid for placing next object
                    self.autoPositionGrid()
                
    def levelHandleMouse3(self):
	# If nothing selected, just return
        if not self.direct.selected.last:
            return
	# Otherwise, find its dictionary entry
	self.selectedLevelObject = (
            self.getLevelObject(self.direct.selected.last))
	# If not None, determine interaction type
        if self.selectedLevelObject:
            selectedObjectDNA = self.selectedLevelObject['DNA']
            # Default target/menu
            target = None
            menuType = None
            # Interaction type depends on selected object's class
            objClass = selectedObjectDNA.__class__.getClassType()
            if objClass.eq(DNAFlatBuilding.getClassType()):
                # Where are we hitting the building?
                hitPt = self.getWallIntersectionPoint()
                if hitPt:
                    xPt = hitPt[0]
                    zPt = hitPt[2]
                    # Which wall are we pointing at (if any)
                    wallNum = self.getWallNum(selectedObjectDNA, zPt)
                    # How wide is the selected wall?
                    wallWidth = selectedObjectDNA.getWidth()
                    # Menu mode depends on where we are pointing
                    if (zPt > self.getWallHeights(selectedObjectDNA)[-1:][0]):
                        menuMode = 'cornice'
                    else:
                        if (xPt < 0.0):
                            menuMode = 'window'
                        elif (xPt > wallWidth):
                            menuMode = 'misc'
                        elif ((xPt >= 0.0) & (xPt <= wallWidth)):
                            menuMode = 'wall'
                    if menuMode == 'wall':
                        if wallNum != -1:
                            selectedWall = (
                                self.getWall(selectedObjectDNA, wallNum))
                            # Default is wall type menu
                            menuType = 'style'
                            target = selectedWall
                            # If shift, switch wall texture
                            if self.direct.fShift:
                                menuType = 'wall'
                            # If alt, switch to wall orientation menu
                            if self.direct.fAlt:
                                menuType = 'orientation'
                            # If control, switch to wall color menu
                            if self.direct.fControl:
                                menuType = 'wallColor'
                        else:
                            target = None
                    elif menuMode == 'window':
                        if wallNum != -1:
                            selectedWall = (
                                self.getWall(selectedObjectDNA, wallNum))
                            # Default is window type menu
                            menuType = 'window'
                            target = self.getWindow(selectedWall,0)
                            # If alt, switch to window orientation menu
                            if self.direct.fAlt:
                                menuType = 'upOrientation'
                            if self.direct.fShift:
                                menuType = 'numWindows'
                                target = selectedWall
                            if self.direct.fControl:
                                menuType = 'windowColor'
                    elif menuMode == 'cornice':
                        menuType = 'cornice'
                        target = self.getCornice(selectedObjectDNA)
                        if self.direct.fAlt:
                            menuType = 'upOrientation'
                        if self.direct.fControl:
                            menuType = 'corniceColor'
                    elif menuMode == 'misc':
                        menuType = 'wallWidth'
                        target =  selectedObjectDNA
                    else:
                        target = None
            elif objClass.eq(DNALandmarkBuilding.getClassType()):
                menuType = 'door'
                target = self.getDoor(selectedObjectDNA)
                if self.direct.fAlt:
                    menuType = 'upOrientation'
                if self.direct.fControl:
                    menuType = 'doorColor'
            elif objClass.eq(DNAProp.getClassType()):
                menuType = 'propType'
                target = selectedObjectDNA
                if self.direct.fControl:
                    menuType = 'propColor'
            elif objClass.eq(DNAStreet.getClassType()):
                menuType = 'streetType'
                target = selectedObjectDNA
            # Now spawn apropriate menu task
            if ((target != None) | (menuType == 'cornice')):
                self.spawnMenuTask(menuType, target)

    def levelHandleMouse3Up(self):
        if self.activeMenu:
            self.activeMenu.removePieMenuTask()

    def flashNodePath(self, aNodePath):
	taskMgr.removeTasksNamed('flashNodePath')
        t = Task.Task(self.flashNodePathTask)
        t.aNodePath = aNodePath
        t.initState = t.hidden = aNodePath.isHidden()
        t.flashCount = 0
        t.frameCount = 0
        t.uponDeath = self.preSelectDone
        taskMgr.spawnTaskNamed(t, 'flashNodePath')

    def flashNodePathTask(self, state):
        aNodePath = state.aNodePath
	initState = state.initState
        hidden = state.hidden
        flashCount = state.flashCount
        frameCount = state.frameCount
        if (flashCount < 4):
            if (frameCount % 3) == 0:
                if hidden:
                    aNodePath.show()
                else:
                    aNodePath.hide()
                state.hidden = not state.hidden
                state.flashCount = flashCount + 1
            state.frameCount = frameCount + 1
            return Task.cont
        else:
            return Task.done

    def preSelectDone(self,state):
        if state.initState:
            state.aNodePath.hide()
        else:
            state.aNodePath.show()

    def showAll(self):
	self.levelObjects.showAllDescendants()
	render.hideCollisionSolids()

    def showGrid(self,flag):
        if flag:
            self.grid.enable()
        else:
            self.grid.disable()
            
    def spawnMenuTask(self, menu, aDNAObject):
	# Record the starting window code and targetDNAObject
	# This will be used by pieMenu action selector
	self.targetDNAObject = aDNAObject
        # Update panel's color if appropriate
        if aDNAObject:
            targetClass = self.targetDNAObject.__class__.getClassType()
            if ((targetClass.eq(DNAWall.getClassType())) |
                (targetClass.eq(DNAWindows.getClassType())) |
                (targetClass.eq(DNACornice.getClassType())) |
                (targetClass.eq(DNAProp.getClassType()))
                ):
                self.panel.setCurrentColor(self.targetDNAObject.getColor())
        
	# What kind of menu is it?
        if menu == 'wall':
            self.activeMenu = self.pieMenuDictionary['wallMenu']
            state = self.getDNAString(aDNAObject)
        elif menu == 'window':
            self.activeMenu = self.pieMenuDictionary['windowMenu']
            state = self.getDNAString(aDNAObject)
        elif menu == 'orientation':
            self.activeMenu = self.pieMenuDictionary['orientationMenu']
            dnaString = self.getDNAString(aDNAObject)
            state = dnaString[-3:]
        elif menu == 'upOrientation':
            self.activeMenu = self.pieMenuDictionary['upOrientationMenu']
            if aDNAObject:
                dnaString = self.getDNAString(aDNAObject)
                state = dnaString[-3:]
            else:
                state = None
        elif menu == 'numWindows':
            self.activeMenu = self.pieMenuDictionary['numWindowsMenu']
            state = self.getWindow(aDNAObject, 0).getWindowCount()
        elif menu == 'cornice':
            self.activeMenu = self.pieMenuDictionary['corniceMenu']
            if aDNAObject:
                state = self.getDNAString(aDNAObject)
            else:
                state = None
        elif menu == 'door':
            self.activeMenu = self.pieMenuDictionary['doorMenu']
            state = self.getDNAString(aDNAObject)
        elif menu == 'wallWidth':
            self.activeMenu = self.pieMenuDictionary['wallWidthMenu']
            state = aDNAObject.getWidth()
        elif menu == 'wallColor':
            self.activeMenu = self.pieMenuDictionary['wallColorMenu']
            self.activeColors = self.getWallColors()
            state = aDNAObject.getColor()
        elif menu == 'windowColor':
            self.activeMenu = self.pieMenuDictionary['windowColorMenu']
            self.activeColors = self.getWindowColors()
            state = aDNAObject.getColor()
        elif menu == 'doorColor':
            self.activeMenu = self.pieMenuDictionary['doorColorMenu']
            self.activeColors = self.getDoorColors()
            state = aDNAObject.getColor()
        elif menu == 'corniceColor':
            self.activeMenu = self.pieMenuDictionary['corniceColorMenu']
            self.activeColors = self.getCorniceColors()
            state = aDNAObject.getColor()
        elif menu == 'propColor':
            self.activeMenu = self.pieMenuDictionary['propColorMenu']
            self.activeColors = self.getPropColors()
            state = aDNAObject.getColor()
        elif menu == 'propType':
            self.activeMenu = self.pieMenuDictionary['propTypesMenu']
            state = self.getDNAString(aDNAObject)
        elif menu == 'streetType':
            self.activeMenu = self.pieMenuDictionary['streetTypesMenu']
            state = self.getDNAString(aDNAObject)
        elif menu == 'style':
            self.activeMenu = self.pieMenuDictionary['styleMenu']
            state = 1

	# Store initial state in case you need to restore it on menu cancel
	self.activeMenu.setInitialState(state)
		
	# Spawn the menu task
	self.activeMenu.spawnPieMenuTask()

    def toggleMapViz(self, flag):
        if flag:
            self.levelMap.reparentTo(self)
        else:
            self.levelMap.reparentTo(hidden)

    def toggleNodePathViz(self, aNodePath):
        # First kill the flashing task
        taskMgr.removeTasksNamed('flashNodePath')
        if aNodePath.isHidden():
            aNodePath.show()
        else:
            aNodePath.hide()

    def setXyzSnap(self, flag):
        self.grid.setXyzSnap(flag)
        if flag:
            self.autoPositionGrid()

    def addStyle(self, dictionary, key, wallTexture, wallColor, windowTexture,
                 windowColor, corniceTexture, corniceColor):
        style = {}
        style['wallTexture'] = wallTexture
        color = VBase4(wallColor)
        style['wallColor'] = color
        style['windowTexture'] = windowTexture
        color = VBase4(windowColor)
        style['windowColor'] = color
        style['corniceTexture'] = corniceTexture
        color = None
        if corniceColor:
            color = VBase4(corniceColor)
        style['corniceColor'] = color
        dictionary[key] = style

    def createColorMenu(self, menuName, colorArray):
        # Create the new one
        numberNodes = []
	
	# Add in common grey scale colors 
	adjustedColorArray = (
            [VBase4(1,1,1,1)] +
            colorArray +
            [VBase4(0.75, 0.75, 0.75, 1.0 ),
             VBase4(0.5, 0.5, 0.5, 1.0),
             VBase4(0.25, 0.25, 0.25, 1.0)]
            )

        for i in range(len(adjustedColorArray)):
            node = OnscreenText('    ', 0.0, 0.0)
            numberNodes.append(node)
            node.setColor(adjustedColorArray[i])
	numItems = len(numberNodes)

	# Attach it to hidden for now
	newColorMenu = hidden.attachNewNode(NamedNode(menuName + 'Menu'))

	# Attach the color chips to the new menu and adjust sizes
	radius = 0.7
	angle = deg2Rad(360.0/float(numItems))
        for i in range (numItems):
            # Get the node
            node = numberNodes[i]
            node.setScale(node.getScale() * 2.0)
	
            # Reposition it
            node.setXY(radius * math.cos(i * angle),
                       (radius *
                        (self.direct.chan.width /
                         float(self.direct.chan.height)) *
                        math.sin(i * angle)))
            
            # Add it to the wallColorMenu
            node.reparentTo(newColorMenu)

	# Scale the whole shebang down by 0.5
	newColorMenu.setScale(0.5)

	# Store menu and colors in appropriate dictionarys
	self.pieMenuDictionary[menuName] = (
            PieMenu(self.direct, newColorMenu, self.updateColorIndex))
	self.colorPaletteDictionary[menuName] = adjustedColorArray

    def createColorMenusFromFile(self, prefix):
        dict = self.createColorDictionaryFromFile(
            'level_editor/' + prefix + 'Colors.txt')
        self.colorPaletteDictionary[prefix + 'Colors'] = dict
        self.createColorMenu(prefix + 'WallColors', dict['wallColors'])
        self.createColorMenu(prefix + 'WindowColors', dict['windowColors'])
        self.createColorMenu(prefix + 'DoorColors', dict['doorColors'])
        self.createColorMenu(prefix + 'CorniceColors', dict['corniceColors'])
        self.createColorMenu(prefix + 'PropColors', dict['propColors'])

    def createColorDictionaryFromFile(self, filename):
        print 'Loading Color Palettes from: ' + filename
        f = Filename(filename)
        f.resolveFilename(getModelPath())
        f = open(f.toOsSpecific(), 'r')
        rawData = f.readlines()
        f.close()
        dict = {}
        wallColors = []
        windowColors = []
        doorColors = []
        corniceColors = []
        propColors = []
        for line in rawData:
            l = string.strip(line)
            if l:
                if l[:4] == 'wall':
                    wallColors.append(eval(l[11:]))
                elif l[:4] == 'wind':
                    windowColors.append(eval(l[13:]))
                elif l[:4] == 'door':
                    doorColors.append(eval(l[11:]))
                elif l[:4] == 'corn':
                    corniceColors.append(eval(l[14:]))
                elif l[:4] == 'prop':
                    propColors.append(eval(l[11:]))
        dict['wallColors'] = wallColors
        dict['windowColors'] = windowColors
        dict['doorColors'] = doorColors
        dict['corniceColors'] = corniceColors
        dict['propColors'] = propColors
        return dict

    def saveColor(self):
        self.appendColorToColorPaletteFile(self.panel.colorEntry.get())

    def appendColorToColorPaletteFile(self, color):
        obj = self.targetDNAObject
        if obj:
            classType = obj.__class__.getClassType()
            if classType.eq(DNAWall.getClassType()):
                tag = 'wallColor:'
            elif classType.eq(DNAWindows.getClassType()):
                tag = 'windowColor:'
            elif classType.eq(DNADoor.getClassType()):
                tag = 'doorColor:'
            elif classType.eq(DNACornice.getClassType()):
                tag = 'corniceColor:'
            elif classType.eq(DNAProp.getClassType()):
                tag = 'propColor:'
            else:
                return
            # Valid type, add color to file
            filename = 'level_editor/' + self.editMode + 'Colors.txt'
            f = Filename(filename)
            f.resolveFilename(getModelPath())
            f = open(f.toOsSpecific(), 'a')
            f.write('%s Vec4(%.2f, %.2f, %.2f, 1.0)\n' %
                    (tag,
                     color[0]/255.0,
                     color[1]/255.0,
                     color[2]/255.0))
            f.close()
        
    def printColorDictionary(self, dict):
        for color in dict['wallColors']:
            print ('wallColor: Vec4(%.2f, %.2f, %.2f, 1.0)' %
                   (color[0], color[1], color[2]))
        for color in dict['windowColors']:
            print ('windowColor: Vec4(%.2f, %.2f, %.2f, 1.0)' %
                   (color[0], color[1], color[2]))
        for color in dict['doorColors']:
            print ('doorColor: Vec4(%.2f, %.2f, %.2f, 1.0)' %
                   (color[0], color[1], color[2]))
        for color in dict['corniceColors']:
            print ('corniceColor: Vec4(%.2f, %.2f, %.2f, 1.0)' %
                   (color[0], color[1], color[2]))
        for color in dict['propColors']:
            print ('propColor: Vec4(%.2f, %.2f, %.2f, 1.0)' %
                   (color[0], color[1], color[2]))

    def createColorMenus(self):
        self.createColorMenusFromFile('toontownCentral')
        self.createColorMenusFromFile('donaldsDock')
        self.createColorMenusFromFile('theBurrrgh')
        self.createColorMenusFromFile('minniesMelodyLand')
        # Use the toontown color set
        self.useToontownCentralColors()

    def recreateColorMenus(self):
        currentMode = self.editMode
        self.createColorMenus()
        # Reset to current mode
        if self.editMode == 'toontownCentral':
            self.useToontownCentralColors()
        elif self.editMode == 'donaldsDock':
            self.useDonaldsDockColors()
        elif self.editMode == 'minniesMelodyLand':
            self.useMinniesMelodyLandColors()
        elif self.editMode == 'theBurrrgh':
            self.useTheBurrrghColors()

    def createCorniceMenu(self):
	# Get the currently available window options	
	numItems = len(self.getCorniceTextures())

	newCorniceMenu = hidden.attachNewNode(NamedNode('corniceMenu'))

	# Attach an empty node for first item
	newCorniceMenu.attachNewNode(NamedNode('no cornice'))

	angle = deg2Rad(360.0/numItems)
	# Note: start at 2 to skip first item (none)
        for i in range(1, numItems):
            # Get the node
            node = self.dnaStore.findNode(self.getCorniceTextures()[i])
            
            # Add it to the window menu
            path = node.instanceTo(newCorniceMenu)

            # Place menu nodes in a circle, offset each in X and Z
            # by half node width/height (.5 * path scale)
            path.setPos(0.75 * math.cos(i * angle),
			0.0,
                        (0.75 *
                         (self.direct.chan.width /
                          float(self.direct.chan.height)) *
                         math.sin(i * angle)))
            path.setScale(0.5)

	# Scale the whole shebang down by 0.5
	newCorniceMenu.setScale(0.5)

	return newCorniceMenu

    def createDoorMenu(self):
	# Get the currently available door options	
	numItems = len(self.getDoorTextures())
		
	newDoorMenu = hidden.attachNewNode(NamedNode('doorMenu'))

	angle = deg2Rad(360.0/float(numItems))
        for i in range(numItems):
            # Get the node
            node = self.dnaStore.findNode(self.getDoorTextures()[i])

            # Add it to the door menu
            path = node.instanceTo(newDoorMenu)

            # Place menu nodes in a circle, offset each in X and Z
            # by half node width/height (.5 * path scale)
            path.setPos(0.75 * math.cos(i * angle) - 0.025,
			0.0,
			((0.75 *
                          (self.direct.chan.width/
                           float(self.direct.chan.height)) *
                          math.sin(i * angle)) - 0.025))
            path.setScale(0.05)

	# Scale the whole shebang down by 0.5
	newDoorMenu.setScale(0.5)

	return newDoorMenu

    def createNumWindowsMenu(self):
        
	numberNodes = []
        for i in range(5):
            node = OnscreenText(`i`, 0.0, 0.0)
            numberNodes.append(node)
	numItems = len(numberNodes)

	newNumWindowsMenu = hidden.attachNewNode(NamedNode('numWindowsMenu'))

	radius = 0.7
	angle = deg2Rad(360.0/numItems)
        for i in range(numItems):
            # Get the node
            node = numberNodes[i]
            node.setScale(node.getScale() * 4.0)
            
            # Reposition it
            node.setXY(radius * math.cos(i * angle),
                       (radius *
                        (self.direct.chan.width/
                         float(self.direct.chan.height)) *
                        math.sin(i * angle)))

            # Add it to the numWindowsMenu
            node.reparentTo(newNumWindowsMenu)

	# Scale the whole shebang down by 0.5
	newNumWindowsMenu.setScale(0.5)

	return newNumWindowsMenu

    def createOrientationMenu(self):
	newOrientationMenu = hidden.attachNewNode(NamedNode('orientationMenu'))
        
	radius = 0.5

	node = OnscreenText('UR',radius,radius)
	node.setScale(node.getScale() * 3.0)
	node.reparentTo(newOrientationMenu)

	node = OnscreenText('UL',-radius,radius)
	node.setScale(node.getScale() * 3.0)
	node.reparentTo(newOrientationMenu)

	node = OnscreenText('DL',-radius, -radius)
	node.setScale(node.getScale() * 3.0)
	node.reparentTo(newOrientationMenu)

	node = OnscreenText('DR',radius,-radius)
	node.setScale(node.getScale() * 3.0)
	node.reparentTo(newOrientationMenu)

	# Scale the whole shebang down by radius
	newOrientationMenu.setScale(radius)
        
	return newOrientationMenu

    def createPropTypesMenu(self):
	numItems = len(self.getPropTypes())

	propNodes = []
        for i in range (numItems):
            node = OnscreenText(self.getPropTypes()[i],0,0)
            propNodes.append(node)

	newPropTypeMenu = hidden.attachNewNode(NamedNode('propTypeMenu'))

	radius = 0.7
	angle = deg2Rad(360.0/numItems)
        for i in range (numItems):
            # Get the node
            node = propNodes[i]
            node.setScale(node.getScale())
	
            # Reposition it
            node.setXY(radius * math.cos(i * angle),
                       (radius *
                        (self.direct.chan.width/
                         float(self.direct.chan.height)) *
                        math.sin(i * angle)))

            # Add it to the propTypeMenu
            node.reparentTo(newPropTypeMenu)

	# Scale the whole shebang down by 0.5
	newPropTypeMenu.setScale(0.5)

	return newPropTypeMenu

    def createStreetTypesMenu(self):
	numItems = len(self.getStreetTypes())

	streetNodes = []
        for i in range (numItems):
            node = OnscreenText(self.getStreetTypes()[i],0,0)
            streetNodes.append(node)

	newStreetTypeMenu = hidden.attachNewNode(NamedNode('streetTypeMenu'))

	radius = 0.7
	angle = deg2Rad(360.0/numItems)
        for i in range (numItems):
            # Get the node
            node = streetNodes[i]
            node.setScale(node.getScale())
	
            # Reposition it
            node.setXY(radius * math.cos(i * angle),
                       (radius *
                        (self.direct.chan.width/
                         float(self.direct.chan.height)) *
                        math.sin(i * angle)))

            # Add it to the streetTypeMenu
            node.reparentTo(newStreetTypeMenu)

	# Scale the whole shebang down by 0.5
	newStreetTypeMenu.setScale(0.5)

	return newStreetTypeMenu

    def createStyleSample(self, style, num):
        # Create a wall
        wall = DNAWall('wall')
	wall.setCode(self.dnaStore.findCode(style['wallTexture']))
	wall.setColor(style['wallColor'])
        wall.setHeight(10.0)
        # Add its windows
	windows = DNAWindows('windows')
        windows.setWindowCount(2)
        windows.setCode(self.dnaStore.findCode(style['windowTexture']))
        windows.setColor(style['windowColor'])
        wall.add(windows)
        # And a cornice if necessary
        corniceTexture = style['corniceTexture']
        if corniceTexture:
            cornice = DNACornice('cornice')
            cornice.setCode(self.getDNACode(corniceTexture))
            cornice.setColor(style['corniceColor'])
            wall.add(cornice)
        # The final building
        bldg = DNAFlatBuilding('style' + `num`)
        bldg.add(wall)
        bldg.setWidth(12.0)
        return bldg.traverse(hidden, self.dnaStore)

    def createStyleMenuWith(self, dictionary):
	numItems = len(dictionary)

	newStyleMenu = hidden.attachNewNode(NamedNode('styleMenu'))

	radius = 0.7
	angle = deg2Rad(360.0/numItems)
        for i in range(numItems):
            # Get the node
            node = self.createStyleSample(dictionary[i], i)
            node.setScale(0.03)
	
            # Reposition it
            node.setPos(radius * math.cos(i * angle),
                        0.0,
                        (radius *
                         (self.direct.chan.width/
                          float(self.direct.chan.height)) *
                         math.sin(i * angle)))

            # Add it to the styleMenu
            node.reparentTo(newStyleMenu)

	# Scale the whole shebang down by 0.5
	newStyleMenu.setScale(0.5)

	return newStyleMenu

    def createUpOrientationMenu(self):
	newUpOrientationMenu = (
            hidden.attachNewNode(NamedNode('upOrientationMenu')))

	radius = 0.5

	node = OnscreenText('UR',radius, radius)
	node.setScale(node.getScale() * 3.0)
	node.reparentTo(newUpOrientationMenu)

	node = OnscreenText('UL',-radius, radius)
	node.setScale(node.getScale() * 3.0)
	node.reparentTo(newUpOrientationMenu)

	node = newUpOrientationMenu.attachNewNode(NamedNode('hiddenNode'))
	node.setScale(node.getScale() * 3.0)

	node = newUpOrientationMenu.attachNewNode(NamedNode('hiddenNode'))
	node.setScale(node.getScale() * 3.0)

	# Scale the whole shebang down by radius
	newUpOrientationMenu.setScale(radius)

	return newUpOrientationMenu

    def createWallMenu(self):
	numItems = len(self.getWallTextures())

	newWallMenu = hidden.attachNewNode(NamedNode('wallMenu'))

	angle = deg2Rad(360.0/numItems)
        for i in range(numItems):
            node = self.dnaStore.findNode(self.getWallTextures()[i])
            path = node.instanceTo(newWallMenu)
            # Place menu nodes in a circle, offset each in X and Z by
            # half node width/height (.5 * path scale)
            path.setPos(0.75 * math.cos(i * angle) - 0.15,
			0.0,
			(0.75 *
                         (self.direct.chan.width/
                          float(self.direct.chan.height)) *
                         math.sin(i * angle) - 0.15))
            path.setScale(0.25)

	# Scale the whole shebang down by 0.5
	newWallMenu.setScale(0.5)
	return newWallMenu

    def createWallWidthMenu(self):
	numberNodes = []
        self.wallWidths = [5, 10, 15, 15.607, 20, 20.706, 25]
        widthsAsText = ['5', '10', '15', '15.6', '20', '20.7', '25']
        for width in widthsAsText:
            node = OnscreenText(width,0,0)
            numberNodes.append(node)
        numItems = len(numberNodes)

	newWallWidthMenu = hidden.attachNewNode(NamedNode('wallWidthMenu'))

	radius = 0.7
	angle = deg2Rad(360.0/numItems)
        for i in range(numItems):
            # Get the node
            node = numberNodes[i]
            node.setScale(node.getScale()* 4.0)
	
            # Reposition it
            node.setXY(radius * math.cos(i * angle),
                       (radius *
                        (self.direct.chan.width/
                         float(self.direct.chan.height)) *
                        math.sin(i * angle)))

            # Add it to the wallWidthMenu
            node.reparentTo(newWallWidthMenu)

	# Scale the whole shebang down by 0.5
	newWallWidthMenu.setScale(0.5)

	return newWallWidthMenu

    def createWindowMenu(self):
	# Get the currently available window options	
	numItems = len(self.getWindowTextures())
	
	newWindowMenu = hidden.attachNewNode(NamedNode('windowMenu'))

	angle = deg2Rad(360.0/numItems)
        for i in range(numItems):
            # Get the node
            node = self.dnaStore.findNode(self.getWindowTextures()[i])

            # Add it to the window menu
            path = node.instanceTo(newWindowMenu)

            # Place menu nodes in a circle, offset each in X and Z by
            # half node width/height (.5 * path scale)
            path.setPos(0.75 * math.cos(i * angle) - 0.05,
			0.0,
			(0.75 *
                         (self.direct.chan.width/
                          float(self.direct.chan.height)) *
                         math.sin(i * angle) - 0.05))
            path.setScale(0.1)

	# Scale the whole shebang down by 0.5
	newWindowMenu.setScale(0.5)

	return newWindowMenu

    def createStyleDictionaryFromFile(self, filename):
        print 'Loading style from: ' + filename
        styleData = self.getStyleData(filename)
        styleDictionary = {}
        styleCount = 0
        while styleData:
            style, styleData = self.getStyleFromStyleData(styleData)
            styleDictionary[styleCount] = style
            styleCount = styleCount + 1
        return styleDictionary

    def getStyleData(self, filename):
        f = Filename(filename)
        f.resolveFilename(getModelPath())
        f = open(f.toOsSpecific(), 'r')
        rawData = f.readlines()
        f.close()
        styleData = []
        for line in rawData:
            l = string.strip(line)
            if l:
                styleData.append(l)
        return styleData

    def getStyleFromStyleData(self, styleData):
        style = {}
        # Wall
        style['wallTexture'] = string.strip(styleData[0][12:])
        style['wallColor'] = eval(styleData[1][10:])
        # Window Texture
        texture = string.strip(styleData[2][14:])
        if texture == 'None':
            style['windowTexture'] = None
        else:
            style['windowTexture'] = texture
        # Window Color
        color = string.strip(styleData[3][12:])
        if color == 'None':
            style['windowColor'] = None
        else:
            style['windowColor'] = eval(color,globals())
        # Cornice Texture
        texture = string.strip(styleData[4][15:])
        if texture == 'None':
            style['corniceTexture'] = None
        else:
            style['corniceTexture'] = texture
        # Cornice Color
        color = string.strip(styleData[5][13:])
        if color == 'None':
            style['corniceColor'] = None
        else:
            style['corniceColor'] = eval(color,globals())
        # Result
        return style, styleData[6:]

    def initializePieMenus(self):
	# Clear out any old menus just in case
        for key in self.pieMenuDictionary.keys():
            oldMenu = self.pieMenuDictionary[key]
            oldMenu.reparentTo(hidden)
            oldMenu.removeNode()

	# Get list of available attributes
	self.initializeAttributeDictionary()

	# Create pop-up pie menus
	self.pieMenuDictionary['wallMenu'] = (
            PieMenu(self.direct, self.createWallMenu(),
                    self.updateWallTextureNum))

	self.pieMenuDictionary['windowMenu'] = (
            PieMenu(self.direct, self.createWindowMenu(),
                    self.updateWindowTextureNum))

	menu = PieMenu(self.direct, self.createOrientationMenu(),
                       self.updateOrientationNum)
	# Clear angle offset on this menu
	menu.setItemOffset(0.0)
	self.pieMenuDictionary['orientationMenu'] = menu

	menu = PieMenu(self.direct, self.createUpOrientationMenu(),
                       self.updateOrientationNum)
        # Clear angle offset on this menu
	menu.setItemOffset(0.0)
	self.pieMenuDictionary['upOrientationMenu'] = menu

	self.pieMenuDictionary['numWindowsMenu'] = (
            PieMenu(self.direct, self.createNumWindowsMenu(),
                    self.updateNumWindows))
	self.pieMenuDictionary['corniceMenu'] = (
            PieMenu(self.direct,self.createCorniceMenu(),
                    self.updateCorniceTextureNum))
	self.pieMenuDictionary['doorMenu'] = (
            PieMenu(self.direct,self.createDoorMenu(),
                    self.updateDoorTextureNum))
	self.pieMenuDictionary['wallWidthMenu'] = (
            PieMenu(self.direct,self.createWallWidthMenu(),
                    self.updateWallWidthSF))
	self.pieMenuDictionary['propTypesMenu'] = (
            PieMenu(self.direct,self.createPropTypesMenu(),
                    self.updatePropNum))
	self.pieMenuDictionary['streetTypesMenu'] = (
            PieMenu(self.direct,self.createStreetTypesMenu(),
                    self.updateStreetNum))
        # Create several different style menus
        self.createStyleMenus()
	# Create several differnt color palette menus
	self.createColorMenus()

    def createStyleMenus(self):
	self.pieMenuDictionary['toontownCentralStyleMenu'] = (
            PieMenu(self.direct,self.createStyleMenuWith(
            self.attributeDictionary['toontownCentralStyleDictionary']),
                    self.updateWallStyleNum))
	self.pieMenuDictionary['donaldsDockStyleMenu'] = (
            PieMenu(self.direct,self.createStyleMenuWith(
            self.attributeDictionary['donaldsDockStyleDictionary']),
                    self.updateWallStyleNum))
	self.pieMenuDictionary['theBurrrghStyleMenu'] = (
            PieMenu(self.direct,self.createStyleMenuWith(
            self.attributeDictionary['theBurrrghStyleDictionary']),
                    self.updateWallStyleNum))
	self.pieMenuDictionary['minniesMelodyLandStyleMenu'] = (
            PieMenu(self.direct,self.createStyleMenuWith(
            self.attributeDictionary['minniesMelodyLandStyleDictionary']),
                    self.updateWallStyleNum))
	self.pieMenuDictionary['styleMenu'] = (
            self.pieMenuDictionary['toontownCentralStyleMenu'])
        
    def recreateStyleMenus(self):
        editMode = self.editMode
        self.initializeStyleDictionary()
        self.createStyleMenus()
	self.styleDictionary = (
            self.attributeDictionary[self.editMode + 'StyleDictionary'])

    def initializeStyleDictionary(self):
        # Create a dictionary for toontownCentral
	dictionary = self.createStyleDictionaryFromFile(
            'level_editor/toontownCentralStyles.txt')
	# Store this dictionary in the self.attributeDictionary
	self.attributeDictionary['toontownCentralStyleDictionary'] = dictionary
        
        # Create a dictionary for donaldsDock
	dictionary = self.createStyleDictionaryFromFile(
            'level_editor/donaldsDockStyles.txt')
	# Store this dictionary in the self.attributeDictionary
	self.attributeDictionary['donaldsDockStyleDictionary'] = dictionary
        
        # Create a dictionary for theBurrrgh
	dictionary = self.createStyleDictionaryFromFile(
            'level_editor/theBurrrghStyles.txt')
	# Store this dictionary in the self.attributeDictionary
	self.attributeDictionary['theBurrrghStyleDictionary'] = dictionary
        
        # Create a dictionary for minniesMelodyLand
	dictionary = self.createStyleDictionaryFromFile(
            'level_editor/minniesMelodyLandStyles.txt')
	# Store this dictionary in the self.attributeDictionary
	self.attributeDictionary['minniesMelodyLandStyleDictionary'] = (
            dictionary)
        
        # Record active style dictionary
	self.styleDictionary = (
            self.attributeDictionary['toontownCentralStyleDictionary'])

    def addDNAGroup(self,dnaGroup):
	# Add hook to allow placement of a new dna Group of this type
        # by simply hitting the space bar
	# First clear out old hooks just to be safe
	self.ignore('insert')
	# Now add new hook
	self.accept('insert', self.initNewDNAGroupWithParent,
                    [dnaGroup, self.groupParent])

	# Now add the first copy of this Group
	self.initDNAGroupWithParent(dnaGroup,self.groupParent)

    def addDNAGroupType(self, dnaGroup, type):
	# Add hook to allow placement of a new dna Group of this type
        # by simply hitting the space bar
	# First clear out old hooks just to be safe
	self.ignore('insert')
	# Now add new hook
	self.accept('insert',
                    self.addFlatBuilding,
                    [type])
	# First clear out old hooks just to be safe
	self.ignore('space')
        self.accept('space',
                    initNewDNAGroupWithParentType,
                    [dnaGroup, self.groupParent, type])

	# Now add the first copy of this Group
	self.initDNAGroupWithParentType(dnaGroup, self.groupParent, type)

    def addDNAGroupTypeMethod(self, dnaGroup, type, method):
	# Add hook to allow placement of a new dna Group of this type
        # by simply hitting the space bar
	# First clear out old hooks just to be safe
	self.ignore('insert')

	# Now add new hooks
	# Insert key generates random version
	self.accept('insert', method, [type])

	# Space bar creates a copy
	self.ignore('space')
	self.accept('space', self.initNewDNAGroupWithParentType,
                    [dnaGroup, self.groupParent, type])

	# Now add the first copy of this Group
	self.initDNAGroupWithParentType(dnaGroup, self.groupParent, type)

    def addFlatBuilding(self, buildingType):
	# Create new building
	newDNAFlatBuilding = DNAFlatBuilding(buildingType + '_DNARoot')
	# Select walls
        if buildingType == 'random20':
            selectedType = self.selectBuildingType('twenty')
        elif buildingType == 'random30':
            selectedType = self.selectBuildingType('thirty')
        else:
            selectedType = buildingType
        if selectedType == 'toonTenTen':
            self.setBuildingHeight(20.0)
            newDNAFlatBuilding.add(self.createWall(10.0))
            newDNAFlatBuilding.add(self.createWall(10.0))
        elif selectedType == 'toonTwenty':
            self.setBuildingHeight(20.0)
            newDNAFlatBuilding.add(self.createWall(20.0))
        elif selectedType == 'toonTenTwenty':
            self.setBuildingHeight(30.0)
            newDNAFlatBuilding.add(self.createWall(10.0))
            newDNAFlatBuilding.add(self.createWall(20.0))
        elif selectedType == 'toonTwentyTen':
            self.setBuildingHeight(30.0)
            newDNAFlatBuilding.add(self.createWall(20.0))
            newDNAFlatBuilding.add(self.createWall(10.0))
        elif selectedType == 'toonTenTenTen':
            self.setBuildingHeight(30.0)
            newDNAFlatBuilding.add(self.createWall(10.0))
            newDNAFlatBuilding.add(self.createWall(10.0))
            newDNAFlatBuilding.add(self.createWall(10.0))
        elif selectedType == 'toonTenTwenty':
            newDNAFlatBuilding.add(self.createWall(10.0))
            newDNAFlatBuilding.add(self.createWall(20.0))
        elif selectedType == 'toonTwentyTen':
            newDNAFlatBuilding.add(self.createWall(20.0))
            newDNAFlatBuilding.add(self.createWall(10.0))
        elif selectedType == 'toonThirty':
            newDNAFlatBuilding.add(self.createWall(30.0))
	# Pick a style for this building
	self.setRandomBuildingStyle(newDNAFlatBuilding)
	# Initialize its position and hpr
	newDNAFlatBuilding.setPos(VBase3(0))
	newDNAFlatBuilding.setHpr(VBase3(0))
	# Now place new building in the world
	self.addDNAGroupTypeMethod(newDNAFlatBuilding,buildingType,
                                   self.addFlatBuilding)

    def addLandmark(self, landmarkType):
	newDNALandmarkBuilding = DNALandmarkBuilding(landmarkType + '_DNARoot')
	newDNALandmarkBuilding.setCode(self.getDNACode(landmarkType))
	newDNALandmarkBuilding.setPos(VBase3(0))
	newDNALandmarkBuilding.setHpr(VBase3(0))
	newDNADoor = self.createDoor(self.getDoorTexture())
	newDNALandmarkBuilding.add(newDNADoor)
	# Now place new building in the world
	self.addDNAGroup(newDNALandmarkBuilding)

    def addObject(self, aNodePath, dnaGroup):
	# Add specified node path to the scene dictionary
	objectDictionary = {}
	objectDictionary['nodePath'] = aNodePath 
	objectDictionary['DNA'] = dnaGroup
	self.levelDictionary[aNodePath.id()] = objectDictionary
        return objectDictionary

    def addProp(self, newPropType):
	newDNAProp = DNAProp(newPropType + '_DNARoot')
	newDNAProp.setCode(self.getDNACode(newPropType))
	newDNAProp.setPos(VBase3(0))
	newDNAProp.setHpr(VBase3(0))
	# Now place new building in the world
	self.addDNAGroup(newDNAProp)
	self.setPropType(newPropType)

    def addStreetModule(self, streetType):
	newDNAStreet = DNAStreet(streetType + '_DNARoot')
	newDNAStreet.setCode(self.getDNACode(streetType))
	newDNAStreet.setPos(VBase3(0))
	newDNAStreet.setHpr(VBase3(0))
	newDNAStreet.setStreetTexture(
            self.getDNACode(self.attributeDictionary['streetTexture']))
        newDNAStreet.setSidewalkTexture(
            self.getDNACode(self.attributeDictionary['sidewalkTexture']))
	# Now place new building in the world
	self.addDNAGroup(newDNAStreet)

    def addWall(self, dnaString, height):
	# Create the DNA for this wall
	newDNAWall = self.createWallWithDNA(dnaString, height)
	# Pick a default window
	newDNAWindows = DNAWindows()
	self.setWindowTexture(self.getRandomWindowTexture())
	newDNAWindows.setCode(self.getDNACode(self.getWindowTexture()))
	newDNAWindows.setWindowCount(self.getRandomNumWindows(height))
        colors = self.getWallColors()
	newDNAWindows.setColor(colors[randint(0,len(colors) - 1)])
	newDNAWall.add(newDNAWindows)
	return newDNAWall

    def createCornice(self,dnaString):
	newDNACornice = DNACornice()
	newDNACornice.setCode(self.getDNACode(dnaString))
        colors = self.getCorniceColors()
	newDNACornice.setColor(colors[randint(0,len(colors) - 1)])
	return newDNACornice

    def createDoor(self, dnaString):
	newDNADoor = DNADoor()
	newDNADoor.setCode(self.getDNACode(dnaString))
        colors = self.getDoorColors()
	newDNADoor.setColor(colors[randint(0,len(colors) - 1)])
	return newDNADoor

    def addGroupToSelected(self, aNodePath):
        self.setGroupParent(aNodePath)
        self.createNewLevelGroup()

    def createNewLevelGroup(self):
	newGroupParentDNA = DNAGroup('group=' + `self.groupNum`)
	# Add it to the level objects
	self.groupParentDNA.add(newGroupParentDNA)

	# Make the new one be the current one
	self.groupParentDNA = newGroupParentDNA

	newGroupParent = self.groupParentDNA.traverse(
            self.groupParent, self.dnaStore)

	self.groupParent = newGroupParent

	self.groupNum = self.groupNum + 1

	# Add it to the level dictionary
	self.addObject(self.groupParent, self.groupParentDNA)

    def createTopLevelGroup(self):
	# Create a new top level group
	self.groupParentDNA = DNAGroup('rootNode')
	self.groupNum = 0

	# Add it to the level objects
	self.levelObjectsDNA.add(self.groupParentDNA)
	self.groupParent = self.groupParentDNA.traverse(
            self.levelObjects, self.dnaStore)

	# Add it to the level dictionary
	self.addObject(self.groupParent, self.groupParentDNA)

    def createWall(self, height):
	return self.createWallWithDNA(self.getWallTexture(), height)

    def createWallWithDNA(self, dnaString, height):
	# Create a new DNAWall using default attributes
	# Create the DNA for this wall
	newDNAWall = DNAWall()
	newDNAWall.setCode(self.getDNACode(dnaString))
	newDNAWall.setHeight(height)
	newDNAWall.setColor(self.getWallColor())
	
	# Pick a default window
	newDNAWindows = DNAWindows()
	newDNAWindows.setCode(self.getDNACode(self.getWindowTexture()))
	newDNAWindows.setWindowCount(1)
	newDNAWindows.setColor(self.getWindowColor())
	newDNAWall.add(newDNAWindows)
	
	return newDNAWall

    def createWindows(self, numWindows):
	newDNAWindows = DNAWindows()
	newDNAWindows.setCode(self.getDNACode(self.getWindowTexture()))
	newDNAWindows.setWindowCount(numWindows)
	newDNAWindows.setColor(self.getWindowColor())
	return newDNAWindows

    def getCatalogCode(self, category, i):
        return self.dnaStore.getCatalogCode(category, i)
    
    def getCatalogCodes(self, category):
	numCodes = self.dnaStore.getNumCatalogCodes(category)
	codes = []
        for i in range(numCodes):
            codes.append(self.dnaStore.getCatalogCode(category, i))
	return codes

    def getCatalogCodesSuffix(self, category, suffix):
	codes = self.getCatalogCodes(category)
	orientedCodes = []
        for code in codes:
            if code[-3:] == suffix:
                orientedCodes.append(code)
	return orientedCodes

    def getCornice(self, aDNAFlatBuilding):
	lastWall = self.getLastWall(aDNAFlatBuilding)
        if lastWall:
            for i in range(lastWall.getNumChildren()):
                child = lastWall.at(i)
                if child.__class__.getClassType().eq(DNACornice.getClassType()):
                    return child
        # Not found
	return None

    def getDNACode(self, dnaString):
	dnaCode = self.dnaStore.findCode(dnaString)
        if dnaCode == 0:
            print 'getDNACode Error!'
	return dnaCode

    def getDNAGroup(self, aNodePath):
	dict = self.getLevelObject(aNodePath)
        if dict:
            return dict['DNA']
        else:
            return None

    def getDNAString(self, aDNAObject):
	return (self.dnaStore.findStringFromCode(aDNAObject.getCode()))

    def getDoor(self, aDNAGroup):
        for i in range(aDNAGroup.getNumChildren()):
            child = aDNAGroup.at(i)
            if child.__class__.getClassType().eq(DNADoor.getClassType()):
                return child
        # Not found
	return None

    def getLastWall(self, aDNAFlatBuilding):
	lastWall = None
        for i in range(aDNAFlatBuilding.getNumChildren()):
            child = aDNAFlatBuilding.at(i)
            if child.__class__.getClassType().eq(DNAWall.getClassType()):
                lastWall = child
        return lastWall

    def getLevelObject(self, aNodePath):
	# Given a node path, find the corresponding level object
        # in the levelDictionary, if none exists, return 0
	return self.levelDictionary.get(aNodePath.id(), None)

    def getRandomCorniceTexture(self):
	chance = 100 * random()
        if (chance < 20):
            textures = self.getCorniceTextures()
            len = len(textures)
            index = randint(0,len)
            return textures[index]
        else:
            return None

    def rounded(self,val):
        return int(round(val))
    
    def getRandomNumWindows(self, height):
	h = self.rounded(height)
        if h == 10:
            # Only return 0 25% of the time
            if self.rounded(self.getWallWidth()) == 5:
                return randint(1,3)
            else:
                return randint(0,3)
        elif h == 20:
            if self.rounded(self.getWallWidth()) == 5:
                return randint(1,3)
            else:
                return randint(0,4)
        elif h == 30:
            if self.rounded(self.getWallWidth()) == 5:
                return randint(1,3)
            else:
                return randint(0,4)


    def getRandomDictionaryEntry(self,dict):
        numKeys = len(dict)
        if numKeys > 0:
            keys = dict.keys()
            key = keys[randint(0,numKeys - 1)]
            return dict[key]
        else:
            return None
        
    def getRandomStyle(self):
	return self.getRandomDictionaryEntry(self.styleDictionary)

    def getRandomWallTexture(self):
	return self.getWallTextures()[
            randint(0, len(self.getWallTextures()) - 1)]

    def getRandomWallWidth(self):
	chance = randint(0,100)
        if chance <= 15:
            return 5.0
        elif (chance > 15) & (chance <= 30):
            return 10.0
        elif (chance > 30) & (chance <= 65):
            return 15.0
        elif (chance > 65) & (chance <= 85):
            return 20.0
        elif (chance > 85):
            return 25.0

    def getRandomWindowTexture(self):
        wt = self.getWindowTextures()
	return wt[randint(9, len(wt) -1 )]

    def getWall(self, aDNAGroup, wallNum):
	wallCount = 0
        for i in range(aDNAGroup.getNumChildren()):
            child = aDNAGroup.at(i)
            if child.__class__.getClassType().eq(DNAWall.getClassType()):
                if wallCount == wallNum:
                    return child
                wallCount = wallCount + 1
        # Not found
	return None

    def getWallHeights(self, aDNAFlatBuilding):
	heightList = []
	heightTotal = 0.0

	# Compute wall heights
        for i in range(aDNAFlatBuilding.getNumChildren()):
            child = aDNAFlatBuilding.at(i)
            if child.__class__.getClassType().eq(DNAWall.getClassType()):
                heightTotal = heightTotal + child.getHeight()
                heightList.append(heightTotal)
	return heightList

    def getWallNum(self, aDNAFlatBuilding, zPt):
        if zPt < 0:
            return -1
	heightList = self.getWallHeights(aDNAFlatBuilding)
	wallNum = 0
        for height in heightList:
            if zPt < height:
                return wallNum
            wallNum = wallNum + 1
	return -1

    def getWindow(self, aDNAGroup, windowNum):
	windowCount = 0
        for i in range(aDNAGroup.getNumChildren()):
            child = aDNAGroup.at(i)
            if child.__class__.getClassType().eq(DNAWindows.getClassType()):
                if windowCount == windowNum:
                    return child
                windowCount = windowCount + 1
	# Not found
	return None

    def initDNAGroupWithParent(self, dnaGroup, parent):
	# Create the geometry
	# If it is a flat building, update building DNA to current wall width
        if (dnaGroup.__class__.getClassType().eq(DNAFlatBuilding.getClassType())):
            dnaGroup.setWidth(self.getWallWidth())
	newNodePath = dnaGroup.traverse(parent,self.dnaStore)
        #newNodePath.node().setName(newNodePath.node().getName() + '_DNARoot')
        
	# Add it to the level dictionary
	self.addObject(newNodePath, dnaGroup)
	# Add it to the top level DNA Group
	self.groupParentDNA.add(dnaGroup)

	# Place the new node path at the current grid origin
	newNodePath.setPos(self.grid,0,0,0)
	# Initialize angle to match last object
	newNodePath.setH(self.grid, self.lastAngle)

	# Select the instance
	self.selectNodePath(newNodePath)

	# Now move the grid to get ready for the next group
	self.autoPositionGrid()

	# Update dictionary
	dnaGroup.setPos(newNodePath.getPos())
	dnaGroup.setHpr(newNodePath.getHpr())

    def initDNAGroupWithParentType(self, dnaGroup, parent, type):
	# Create the geometry
	# If it is a flat building, update building DNA to current wall width
        if dnaGroup.__class__.getClassType().eq(DNAFlatBuilding.getClassType()):
            dnaGroup.setWidth(self.getWallWidth())
	newNodePath = dnaGroup.traverse(parent, self.dnaStore)
        #newNodePath.node().setName(newNodePath.node().getName() + '_DNARoot')        
	# Add it to the level dictionary
	self.addObject(newNodePath, dnaGroup)
        
	# Add it to the top level DNA Group
	self.groupParentDNA.add(dnaGroup)

	# Place the new node path at the current grid origin
	newNodePath.setPos(self.grid,0,0,0)
	# Initialize angle to match last object
	newNodePath.setH(self.grid, self.lastAngle)

	# Select the instance
	self.selectNodePath(newNodePath)

	# Now move the grid to get ready for the next group
	self.autoPositionGrid()

	# Update dictionary
	dnaGroup.setPos(newNodePath.getPos())
	dnaGroup.setHpr(newNodePath.getHpr())

    def initNewDNAGroupWithParent(self, dnaGroup, rootNode):
	# Reflect currently selected prop type
        groupClass = dnaGroup.__class__.getClassType()
        if groupClass.eq(DNAProp.getClassType()):
            self.updatePropDNA(dnaGroup,self.getPropType())
        elif groupClass.eq(DNAStreet.getClassType()):
            self.updateStreetDNA(dnaGroup,self.getStreetType())
            
        # Create a new copy of dnaGroup's class
        # Extract group's class using __class__
        # Call that class's constructor passing in dnaGroup to make a copy
        self.initDNAGroupWithParent(dnaGroup.__class__(dnaGroup), self.groupParent)
        # Initialize
        if dnaGroup.__class__.getClassType().eq(DNAProp.getClassType()):
            objectType = self.getDNAString(dnaGroup)
            if objectType != 'prop_sphere':
                # Update props placement to reflect current mouse position
                # Where is the mouse relative to the grid?
                hitPt = self.getGridIntersectionPoint()
                self.direct.selected.last.setPos(self.grid, self.hitPt)
                dnaGroup.setPos(self.direct.selected.last.getPos())

    def initNewDNAGroupWithParentType(self, dnaGroup, rootNode, type):
        # Create a new dna Group of the same type a dnaGroup
	newDNAGroup = dnaGroup.__class__(dnaGroup)
        if dnaGroup.__class__.getClassType().eq(DNAProp.getClassType()):
            self.updatePropDNA(newDNAGroup,self.getPropType())

        self.initDNAGroupWithParentType(newDNAGroup, self.groupParent, type)

    def loadSpecifiedDNAFile(self):
        f = Filename('/alpha/DIRECT/LevelEditor/DNAFiles')
        path = os.path.join(f.toOsSpecific(), self.dnaOutputDir)
        #f = Filename('dna')
        #f.resolveFilename(getModelPath())
        #path = f.toOsSpecific()
        if not os.path.isdir(path):
            print 'LevelEditor Warning: Invalid default DNA directory!'
            print 'Using: C:\\'
            path = 'C:\\'
        dnaFilename = askopenfilename(
            defaultextension = '.dna',
            filetypes = (('DNA Files', '*.dna'),('All files', '*')),
            initialdir = path,
            title = 'Load DNA File',
            parent = self.panel.component('hull'))
        if dnaFilename:
            self.loadDNAFromFile(dnaFilename)

    def saveToSpecifiedDNAFile(self):
        f = Filename('/alpha/DIRECT/LevelEditor/DNAFiles')
        path = os.path.join(f.toOsSpecific(), self.dnaOutputDir)
        if not os.path.isdir(path):
            print 'LevelEditor Warning: Invalid DNA save directory!'
            print 'Using: C:\\'
            path = 'C:\\'
        dnaFilename = asksaveasfilename(
            defaultextension = '.dna',
            filetypes = (('DNA Files', '*.dna'),('All files', '*')),
            initialdir = path,
            title = 'Save DNA File as',
            parent = self.panel.component('hull'))
        if dnaFilename:
            self.outputDNA(dnaFilename)

    def loadDNAFromFile(self, filename):
	# Out with the old, in with the new
	self.resetLevel()
	# Get rid of default group and root node
	self.preRemoveNodePath(self.groupParent)
	self.removeNodePath(self.groupParent)
	# Clear self.dnaStore
	self.dnaStore.resetDNAGroups()
        # Reset DNA VIS Groups
        self.dnaStore.resetDNAVisGroups()
	# Now load in new file
	self.groupParent = loadDNAFile(self.dnaStore, filename, CSDefault)
 	# Make sure the topmost level object gets put under level objects dna
 	self.groupParentDNA = self.dnaStore.findDNAGroup(
            self.groupParent.getBottomArc())
 	self.levelObjectsDNA.add(self.groupParentDNA)
	# No level objects node found, just add the whole thing
	self.groupParent.reparentTo(self.levelObjects)
        if 0:
            newLevelObjects = nodePath.find('**/LevelObjects')
            if newLevelObjects.isEmpty():
                # No level objects node found, just add the whole thing
                nodePath.reparentTo(self.levelObjects)
            else:
                # There is a LevelObjects node, add its children
                # Before reparenting children, try to set groupNum to
                # something reasonable
                self.groupNum = newLevelObjects.getNumChildren()
                # Go ahead and get rid of the default parent
                # (since there is probably one in the dnafile
                self.preRemoveNodePath(self.groupParent)
                self.removeNodePath(self.groupParent)
                # Now add the children from the DNA File
                children = newLevelObjects.getChildren()
                for i in range(children.getNumPaths()):
                    children.getPath(i).reparentTo(self.levelObjects)
                # Now create a new top level group with next group number
                self.createTopLevelGroup()
        # Add these objects to the levelDictionary
	numPaths = self.dnaStore.getNumNodeRelations()
        for pathNum in range(numPaths):
            relation = self.dnaStore.getNodeRelationAt(pathNum)
            if relation:
                if (relation.getChild() and relation.getParent()):
                    newNodePath = NodePath(relation)
                    group = self.dnaStore.findDNAGroup(relation)
                    if newNodePath.isSingleton():
                        print 'Singleton!!'
                    else: 
                        self.addObject(newNodePath, group)
            else:
                print'blah'
	self.createNewLevelGroup()

    def outputDNADefaultFile(self):
        f = Filename('/alpha/DIRECT/LevelEditor/DNAFiles')
        file = os.path.join(f.toOsSpecific(), self.dnaOutputDir,
                            self.dnaOutputFile)
        self.outputDNA(file)
        
    def outputDNA(self,filename):
	print 'Saving DNA to: ', filename
	self.levelObjectsDNA.writeDna(Filename(filename),
                                      Notify.out(),self.dnaStore)

    def preRemoveNodePath(self, aNodePath):
	# Remove nodePath's DNA from its parent
	dnaGroup = self.getDNAGroup(aNodePath)
        if dnaGroup:
            parentDNAGroup = self.getDNAGroup((aNodePath.getParent()))
            if parentDNAGroup:
                parentDNAGroup.remove(dnaGroup)

    def printVizRegions(self):
        if self.direct.selected.last:
            self.printVizRegionsOf(self.direct.selected.last)

    def printVizRegionsOf(self, aNodePath):
	# Print out commands to create viz regions
	print 'To Be Supplied'

    def removeCornices(self, aDNAGroup):
        while self.removeDNAObjectOfClass(
            DNACornice,self.getLastWall(aDNAGroup)):
            pass

    def removeDNAObjectOfClass(self, objectClass, aDNAGroup):
	# Remove the first object of that type you come across
        for i in range(aDNAGroup.getNumChildren()):
            child = aDNAGroup.at(i)
            if child.__class__.getClassType().eq(objectClass.getClassType()):
                aDNAGroup.remove(child)
                return 1
	# None found
	return 0

    def removeNodePath(self, aNodePath):
	# Remove specified node path from the scene dictionary
	nodePathHandle = aNodePath.id()
	del(self.levelDictionary[nodePathHandle])
	# Get rid of its visible representation
	aNodePath.removeNode()

    def removeWindows(self, aDNAGroup):
        while self.removeDNAObjectOfClass(DNAWindows, aDNAGroup):
            pass

    def reparentNodePath(self, aNodePath):
	selectedDNAObject = self.getDNAGroup(aNodePath)
        if not selectedDNAObject:
            return 0
	parent = aNodePath.getParent()
        if not parent:
            return 0
	parentDNAObject = self.getDNAGroup(parent)
        if not parentDNAObject:
            return 0
        if self.groupParent & self.groupParentDNA:
            # Everybody seems happy, move it
            aNodePath.reparentTo(self.groupParent)
            parentDNAObject.remove(selectedDNAObject)
            self.groupParentDNA.add(selectedDNAObject)
            return 1

    def reparentSelectedNodePath(self):
	selectedNodePath = self.direct.selected.last
        if not selectedNodePath:
            return 0
	return self.reparentNodePath(selectedNodePath)

    def replaceLevelObject(self, levelObject):
	# Get the DNA for this object
	dnaGroup = levelObject['DNA']

	# Get to current node path and its parent
	oldNodePath = levelObject['nodePath']
	parent = oldNodePath.getParent()

	# Traverse the dna to create the new node path
	newNodePath = dnaGroup.traverse(parent, self.dnaStore)
        #newNodePath.node().setName(newNodePath.node().getName() + '_DNARoot')
	self.selectNodePath(newNodePath)

	# Add it to the levelObjects dictionary
	self.selectedLevelObject = self.addObject(newNodePath, dnaGroup)

	# Now get rid of the old level object
	self.removeNodePath(oldNodePath)

    def replaceLevelObjectNodePath(self, levelObject):
	# Get and reuse the DNA for this object
	dnaGroup = levelObject['DNA']

	# Get to current node path and its parent
	oldNodePath = levelObject['nodePath']
	parent = oldNodePath.getParent()

	# Traverse the dna to create the new node path
	newNodePath = dnaGroup.traverse(parent, self.dnaStore)
        #newNodePath.node().setName(newNodePath.node().getName() + '_DNARoot')
	self.selectNodePath(newNodePath)

	# Add it to the levelObjects dictionary
	self.selectedLevelObject = self.addObject(newNodePath, dnaGroup)

	# Now get rid of the old level object
	self.removeNodePath(oldNodePath)

    def selectBuildingType(self, heightType):
	chance = randint(0,100)
        if heightType == 'twenty':
            if chance <= 65:
                return 'toonTenTen'
            else:
                return 'toonTwenty'
        else:
            if chance <= 40:
                return 'toonTenTwenty'
            elif (chance > 40) & (chance <= 70):
                return 'toonTwentyTen'
            elif (chance > 70) & (chance <= 90):
                return 'toonTenTenTen'
            else:
		return 'toonThirty'

    def setGroupName(self, aNodePath, aName):
	aDNAGroup = self.getDNAGroup(aNodePath)
        if aDNAGroup:
            aNodePath.setName(aName)
            aDNAGroup.setName(aName)

    def setNodePathName(self, aNodePath, aName):
	levelObject = self.getLevelObject(aNodePath)
        if levelObject:
            levelObjectDNA = levelObject['DNA']
            aNodePath.setName(aName)
            levelObjectDNA.setName(aName)
            self.replaceLevelObject(levelObject)

    def setRandomBuildingStyle(self, aDNAFlatBuilding):
	self.setWallWidthVal(self.getRandomWallWidth())
	aDNAFlatBuilding.setWidth(self.getWallWidth())
	style = self.getRandomStyle()
        for i in range(aDNAFlatBuilding.getNumChildren()):
            # Set style of each child
            child = aDNAFlatBuilding.at(i)
            if child.__class__.getClassType().eq(DNAWall.getClassType()):
                if randint(0,100) < 40:
                    style = self.getRandomStyle()
                self.setWallStyle(child, style)

        # Using the style of the last wall:
        if not style['corniceTexture']:
            self.removeCornices(aDNAFlatBuilding)
        else:
            aDNACornice = self.getCornice(aDNAFlatBuilding)
            if not aDNACornice:
                aDNACornice = DNACornice()
                aDNACornice.setCode(
                    self.dnaStore.findCode(style['corniceTexture']))
                aDNACornice.setColor(style['corniceColor'])
                lastWall = self.getLastWall(aDNAFlatBuilding)
                lastWall.add(aDNACornice)

    def setRandomNumWindows(self, aDNAWall, numWindows):
	window = self.getWindow(aDNAWall, 0)
	window.setWindowCount(numWindows)

    def setWallStyle(self, aDNAWall, style):
	aDNAWall.setCode(self.dnaStore.findCode(style['wallTexture']))
	aDNAWall.setColor(style['wallColor'])
	aDNAWindows = self.getWindow(aDNAWall, 0)
        # If the wall has windows:
        if aDNAWindows:
            aDNAWindows.setWindowCount(
                self.getRandomNumWindows(aDNAWall.getHeight()))
            aDNAWindows.setCode(self.dnaStore.findCode(style['windowTexture']))
            aDNAWindows.setColor(style['windowColor'])

    def setWallStyleNum(self, aDNAWall, styleNum):
        # What about cornices
	self.setWallStyle(aDNAWall, self.styleDictionary[styleNum])

    def updateColorIndex(self, colorIndex):
        if colorIndex < 0:
            self.updateObjColor(
                self.targetDNAObject,self.activeMenu.getInitialState())
        else:
            self.updateObjColor(
                self.targetDNAObject,self.activeColors[colorIndex])

    def updateObjColor(self, aDNAObject, color):
	aDNAObject.setColor(color)
	# Replace object in levelObjects dictionary and scene graph
	self.replaceLevelObjectNodePath(self.selectedLevelObject)
	self.setActiveColor(color)

    def updateCorniceTextureNum(self, corniceNumber):
	self.updateObjCorniceTexture(self.targetDNAObject, corniceNumber)

    def updateObjCorniceTexture(self, aDNACornice, corniceTextureNumber):
	# Which wall texture was picked by the user?
	if (corniceTextureNumber < 0):
            dnaString = self.activeMenu.getInitialState()
        else:
            dnaString = self.getCorniceTextures()[corniceTextureNumber]

	# Now update the texture on the cornice with that texture
	self.updateCorniceTextureDNA(aDNACornice, dnaString)

    def updateCorniceTextureDNA(self, aDNACornice, dnaString):
	# Get the currently selected DNA Group
	aDNAGroup = self.selectedLevelObject['DNA']

        if dnaString == None:
            # Remove any existing cornices
            self.removeCornices(aDNAGroup)
            # Clear out target DNA Object
            self.targetDNAObject = None
        else:
            # Change the cornice type
            if aDNACornice:
                # Change existing one
                aDNACornice.setCode(self.dnaStore.findCode(dnaString))
            else:
                lastWall = self.getLastWall(aDNAGroup)
                if lastWall:
                    # No cornice exists, add a new one
                    newDNACornice = self.createCornice(dnaString)
                    lastWall.add(newDNACornice)
                    # Update target DNA Object
                    self.targetDNAObject = newDNACornice

	# Replace object in levelObjects dictionary and scene graph
	self.replaceLevelObjectNodePath(self.selectedLevelObject)
	self.setCorniceTexture(dnaString)

    def updateDNAPosHpr(self, aNodePath):
        if aNodePath:
            dnaGroup = self.getDNAGroup(aNodePath)
            if dnaGroup:
                groupClass = dnaGroup.__class__.getClassType()
                if not(groupClass.eq(DNAGroup.getClassType())):
                    dnaGroup.setPos(aNodePath.getPos())
                    dnaGroup.setHpr(aNodePath.getHpr())
                    if (groupClass.eq(DNAProp.getClassType())):
                        dnaGroup.setScale(aNodePath.getScale())

    def updateDoorTextureNum(self, doorTextureNumber):
	self.updateObjDoorTexture(self.targetDNAObject, doorTextureNumber)

    def updateObjDoorTexture(self, aDNADoor, doorTextureNumber):
	# Which door texture was picked by the user?
        if doorTextureNumber < 0:
            dnaString = self.activeMenu.getInitialState()
        else:
            dnaString = self.getDoorTextures()[doorTextureNumber]

	# Now update the texture on the door with that texture
	self.updateDoorTextureDNA(aDNADoor, dnaString)

    def updateDoorTextureDNA(self, aDNADoor ,dnaString):
	aDNADoor.setCode(self.dnaStore.findCode(dnaString))
	# Replace object in levelObjects dictionary and scene graph
	self.replaceLevelObjectNodePath(self.selectedLevelObject)
	self.setDoorTexture(dnaString)

    def updateNumWindows(self, numWindows):
        if numWindows < 0:
            self.updateObjNumWindows(self.targetDNAObject,
                                     self.activeMenu.getInitialState())
        else:
            self.updateObjNumWindows(self.targetDNAObject,numWindows)

    def updateObjNumWindows(self, aDNAWall, numWindows):
	# remove any existing windows
	self.removeWindows(aDNAWall)

	# Add the newly specified number of windows
	newDNAWindows = self.createWindows(numWindows)
	aDNAWall.add(newDNAWindows)

	# Replace object in levelObjects dictionary and scene graph
	self.replaceLevelObjectNodePath(self.selectedLevelObject)

	self.setNumWindows(numWindows)

    def updateOrientationNum(self, orientationNumber):
	remappedOrientationNumber = orientationNumber

	# Remap lower menu values for Door's and Cornices'
        # (only have upper orientations)
        targetClass = self.targetDNAObject.__class__.getClassType()
	if (targetClass.eq(DNADoor.getClassType()) |
            targetClass.eq(DNAWindows.getClassType()) |
            targetClass.eq(DNACornice.getClassType())):
            if (orientationNumber == 2):
                remappedOrientationNumber = 1
            elif (orientationNumber == 3):
                remappedOrientationNumber = 0
	self.updateOrientation(self.targetDNAObject, remappedOrientationNumber)

    def updateOrientation(self, aDNAObject, orientationNumber):
        if self.activeMenu.getInitialState() == None:
            return None
        currString = self.getDNAString(aDNAObject)
        # Strip off current suffix
        newString = currString[:-3]
        if orientationNumber == 0:
            dnaString = newString + '_ur'
        elif orientationNumber == 1:
            dnaString = newString + '_ul'
        elif orientationNumber == 2:
            dnaString = newString + '_dl'
        elif orientationNumber == 3:
            dnaString = newString + '_dr'
        else:
            dnaString = self.activeMenu.getInitialState()
        if dnaString != currString:
            objClass = aDNAObject.__class__.getClassType()
            if objClass.eq(DNAWall.getClassType()):
                self.updateWallTextureDNA(aDNAObject, dnaString)
            elif objClass.eq(DNACornice.getClassType()):
                self.updateCorniceTextureDNA(aDNAObject, dnaString)
            elif objClass.eq(DNADoor.getClassType()):
                self.updateDoorTextureDNA(aDNAObject, dnaString)
            elif objClass.eq(DNAWindows.getClassType()):
                self.updateWindowTextureDNA(aDNAObject, dnaString)

    def updatePropNum(self,propNumber):
	self.updatePropType(self.targetDNAObject, propNumber)

    def updatePropType(self, aDNAProp, propNumber):
	# Which propType was picked by the user?
        if (propNumber < 0):
            dnaString = self.activeMenu.getInitialState()
        else:
            dnaString = self.getPropTypes()[propNumber]

	# Now update the texture on the wall with that texture
	self.updatePropDNA(aDNAProp,dnaString)

    def updatePropDNA(self, aDNAProp, dnaString):
	aDNAProp.setCode(self.dnaStore.findCode(dnaString))
        aDNAProp.setName(dnaString + '_DNARoot')
	# Replace object in levelObjects dictionary and scene graph
	self.replaceLevelObjectNodePath(self.selectedLevelObject)
	self.setPropType(dnaString)

    def updateStreetNum(self,streetNumber):
	self.updateStreetType(self.targetDNAObject, streetNumber)

    def updateStreetType(self, aDNAStreet, streetNumber):
	# Which streetType was picked by the user?
        if (streetNumber < 0):
            dnaString = self.activeMenu.getInitialState()
        else:
            dnaString = self.getStreetTypes()[streetNumber]

	# Now update the texture on the wall with that texture
	self.updateStreetDNA(aDNAStreet,dnaString)

    def updateStreetDNA(self, aDNAStreet, dnaString):
	aDNAStreet.setCode(self.dnaStore.findCode(dnaString))
        aDNAStreet.setName(dnaString + '_DNARoot')
        aDNAStreet.setStreetTexture(
            self.getDNACode(self.attributeDictionary['streetTexture']))
        if (string.find(dnaString, 'keyboard') >= 0):
            aDNAStreet.setSidewalkTexture(
                self.getDNACode('street_sidewalk_MM_keyboard_tex'))
        else:
            aDNAStreet.setSidewalkTexture(
                self.getDNACode(self.attributeDictionary['sidewalkTexture']))
	# Replace object in levelObjects dictionary and scene graph
	self.replaceLevelObjectNodePath(self.selectedLevelObject)
	self.setStreetType(dnaString)

    def updateRandomNumWindows(self, aDNAFlatBuilding):
        for i in range(aDNAFlatBuilding.getNumChildren()):
            child = aDNAFlatBuilding.at(i)
            if child.__class__.getClassType().eq(DNAWall.getClassType()):
                self.setRandomNumWindows(
                    child,
                    self.getRandomNumWindows(child.getHeight()))

    def updateWallColor(self, aDNAWall, color):
	aDNAWall.setColor(color)
	# Replace object in levelObjects dictionary and scene graph
	self.replaceLevelObject(self.selectedLevelObject)

    def updateWallOrientationNum(self, wallOrientationNumber):
	self.updateWallOrientation(self.targetDNAObject, wallOrientationNumber)

    def updateWallOrientation(self, aDNAWall, wallOrientationNumber):
	currString = self.getDNAString(aDNAWall)

	# Strip off current suffix
        newString = currString[:-3]
        if wallOrientationNumber == 0:
            dnaString = newString + '_ur'
        elif wallOrientationNumber == 1:
            dnaString = newString + '_ul'
        elif wallOrientationNumber == 2:
            dnaString = newString + '_dr'
        elif wallOrientationNumber == 3:
            dnaString = newString + '_dl'
        else:
            dnaString = self.activeMenu.getInitialState()

        if dnaString != currString:
            self.updateWallTextureDNA(aDNAWall, dnaString)

    def updateWallStyleNum(self, styleNum):
        if styleNum < 0:
            self.setWallStyleNum(self.targetDNAObject, 0)
        else:
            self.setWallStyleNum(self.targetDNAObject, styleNum)
	self.replaceLevelObjectNodePath(self.selectedLevelObject)

    def updateWallTextureNum(self, wallTextureNumber):
	self.updateObjWallTextureNum(self.targetDNAObject, wallTextureNumber)

    def updateObjWallTextureNum(self, aDNAWall, wallTextureNumber):
	# Which wall texture was picked by the user?
        if wallTextureNumber < 0:
            dnaString = self.activeMenu.getInitialState()
        else:
            dnaString = self.getWallTextures()[wallTextureNumber]

	# Now update the texture on the wall with that texture
	self.updateWallTextureDNA(aDNAWall, dnaString)

    def updateWallTextureDNA(self, aDNAWall, dnaString):
	aDNAWall.setCode(self.dnaStore.findCode(dnaString))
	# Replace object in levelObjects dictionary and scene graph
	self.replaceLevelObjectNodePath(self.selectedLevelObject)
	self.setWallTexture(dnaString)

    def updateWallWidthSF(self, scaleFactor):
        if (scaleFactor < 0):
            self.updateWallWidth(self.targetDNAObject,
                                 (self.activeMenu.getInitialState()))
        else:
            self.updateWallWidth(self.targetDNAObject,
                                 self.wallWidths[scaleFactor])

    def updateSelectedWallWidth(self, width):
        if self.targetDNAObject:
            if self.targetDNAObject.__class__.getClassType().eq(
                DNAFlatBuilding.getClassType()):
                self.updateWallWidth(self.targetDNAObject,width)

    def updateWallWidth(self, aDNAWall, width):
	aDNAWall.setWidth(width)
	# Replace object in levelObjects dictionary and scene graph
	self.replaceLevelObjectNodePath(self.selectedLevelObject)
	self.setWallWidthVal(width)
	self.autoPositionGrid()

    def updateWindowTextureNum(self, windowTextureNumber):
	self.updateObjWindowTexture(self.targetDNAObject,
                                    windowTextureNumber)

    def updateObjWindowTexture(self, aDNAWindow, windowTextureNumber):
	# Which window texture was picked by the user?
        if windowTextureNumber < 0:
            dnaString = self.activeMenu.getInitialState()
        else:
            dnaString = self.getWindowTextures()[windowTextureNumber]

	# Now update the texture on the window with that texture
	self.updateWindowTextureDNA(aDNAWindow, dnaString)

    def updateWindowTextureDNA(self, aDNAWindow, dnaString):
	aDNAWindow.setCode(self.dnaStore.findCode(dnaString))
	# Replace object in levelObjects dictionary and scene graph
	self.replaceLevelObjectNodePath(self.selectedLevelObject)
	self.setWindowTexture(dnaString)

    def roundTo(self, value, divisor):
        return round(value/float(divisor)) * divisor

    def autoPositionGrid(self):
	# Move grid to prepare for placement of next object
	selectedNode = self.direct.selected.last
        if selectedNode:
            dnaGroup = self.getDNAGroup(selectedNode)
            groupClass = dnaGroup.__class__.getClassType()
            deltaPos = Point3(20,0,0)
            deltaHpr = VBase3(0)
            if groupClass.eq(DNAFlatBuilding.getClassType()):
                deltaPos.setX(self.getWallWidth())
            elif groupClass.eq(DNAStreet.getClassType()):
                objectType = self.getDNAString(dnaGroup)
                if objectType == 'street_5x20':
                    deltaPos.setX(5.0)
                elif objectType == 'street_10x20':
                    deltaPos.setX(10.0)
                elif objectType == 'street_20x20':
                    deltaPos.setX(20.0)
                elif objectType == 'street_40x20':
                    deltaPos.setX(40.0)
                elif objectType == 'street_80x20':
                    deltaPos.setX(80.0)
                elif objectType == 'street_5x40':
                    deltaPos.setX(5.0)
                elif objectType == 'street_10x40':
                    deltaPos.setX(10.0)
                elif objectType == 'street_20x40':
                    deltaPos.setX(20.0)
                elif objectType == 'street_30x40':
                    deltaPos.setX(30.0)
                elif objectType == 'street_40x40':
                    deltaPos.setX(40.0)
                elif objectType == 'street_80x40':
                    deltaPos.setX(80.0)
                elif objectType == 'street_angle_30':
                    deltaPos.setX(0.0)
                elif objectType == 'street_angle_45':
                    deltaPos.setX(0.0)
                elif objectType == 'street_angle_60':
                    deltaPos.setX(0.0)
                elif objectType == 'street_inner_corner':
                    deltaPos.setX(20.0)
                elif objectType == 'street_outer_corner':
                    deltaPos.setX(20.0)
                elif objectType == 'street_full_corner':
                    deltaPos.setX(40.0)
                elif objectType == 'street_t_intersection':
                    deltaPos.setX(40.0)
                elif objectType == 'street_y_intersection':
                    deltaPos.setX(30.0)
                elif objectType == 'street_street_20x20':
                    deltaPos.setX(20.0)
                elif objectType == 'street_street_40x40':
                    deltaPos.setX(40.0)
                elif objectType == 'street_sidewalk_20x20':
                    deltaPos.setX(20.0)
                elif objectType == 'street_sidewalk_40x40':
                    deltaPos.setX(40.0)
                elif objectType == 'street_divided_transition':
                    deltaPos.setX(40.0)
                elif objectType == 'street_divided_40x70':
                    deltaPos.setX(40.0)
                elif objectType == 'street_stairs_40x10x5':
                    deltaPos.setX(40.0)
                elif objectType == 'street_4way_intersection':
                    deltaPos.setX(40.0)
                elif objectType == 'street_incline_40x40x5':
                    deltaPos.setX(40.0)
                elif objectType == 'street_courtyard_70':
                    deltaPos.setX(0.0)
                elif objectType == 'street_courtyard_70_exit':
                    deltaPos.setX(0.0)
                elif objectType == 'street_courtyard_90':
                    deltaPos.setX(0.0)
                elif objectType == 'street_courtyard_90_exit':
                    deltaPos.setX(0.0)
                elif objectType == 'street_50_transition':
                    deltaPos.setX(10.0)
                elif objectType == 'street_20x50':
                    deltaPos.setX(20.0)
                elif objectType == 'street_40x50':
                    deltaPos.setX(40.0)
            elif groupClass.eq(DNALandmarkBuilding.getClassType()):
                objectType = self.getDNAString(dnaGroup)
                if objectType[:-1] == 'toon_landmark_building_0':
                    deltaPos.setX(25.0)
                elif objectType[:-1] == 'toon_landmark_building_2':
                    deltaPos.setX(15.0)
                elif objectType[:-1] == 'toon_landmark_building_3':
                    deltaPos.setX(20.0)
            elif groupClass.eq(DNAProp.getClassType()):
                objectType = self.getDNAString(dnaGroup)
                if objectType == 'prop_sphere':
                    deltaPos.setX(40.0)

            # Position grid for placing next object
            # Eventually we need to setHpr too
            taskMgr.removeTasksNamed('autoPositionGrid')
            t = self.grid.lerpPos(deltaPos, 0.25,
                                  other = selectedNode,
                                  blendType = 'easeInOut',
                                  task = 'autoPositionGrid')
            t.deltaPos = deltaPos
            t.selectedNode = selectedNode
            t.uponDeath = self.autoPositionCleanup
                                  
	# Also move the camera
	taskMgr.removeTasksNamed('autoMoveDelay')
	handlesToCam = self.direct.widget.getPos(self.direct.camera)
	handlesToCam = handlesToCam * ( self.direct.chan.near/handlesToCam[1])
	if ((abs(handlesToCam[0]) > (self.direct.chan.nearWidth * 0.4)) |
            (abs(handlesToCam[2]) > (self.direct.chan.nearHeight * 0.4))):
            taskMgr.removeTasksNamed('manipulateCamera')
            self.direct.cameraControl.centerCamIn(self.direct.chan, 0.5)

    def autoPositionCleanup(self,state):
        self.grid.setPos(state.selectedNode, state.deltaPos)
        if self.grid.getXyzSnap():
            # Tighten up grid position
            pos = self.grid.getPos()
            roundVal = self.roundTo(self.grid.getGridSpacing(), 1)
            x = self.roundTo(pos[0], roundVal)
            y = self.roundTo(pos[1], roundVal)
            z = self.roundTo(pos[2], roundVal)
            self.grid.setPos(x,y,z)

    def plantSelectedNodePath(self):
	# Move grid to prepare for placement of next object
	selectedNode = self.direct.selected.last
        if selectedNode:
            # Where is the mouse relative to the grid?
            hitPt = self.getGridIntersectionPoint()
            selectedNode.setPos(self.grid, self.hitPt)
            dnaGroup = self.getDNAGroup(selectedNode)
            if dnaGroup:
                # Update props placement to reflect current mouse position
                dnaGroup.setPos(self.direct.selected.last.getPos())

    def selectNodePath(self, aNodePath):
        # Select new node path
	self.direct.select(aNodePath)
	# Update selectedLevelObject
	self.selectedLevelObject = self.getLevelObject(aNodePath)

    def getWallIntersectionPoint(self):
	"""
        Return point of intersection between building's wall and line from cam
        through mouse. Return false, if nothing selected
        """
	selectedNode = self.direct.selected.last
        if not selectedNode:
            return 0
        # Find mouse point on near plane
        chan = self.direct.chan
    	mouseX = chan.mouseX
  	mouseY = chan.mouseY
   	nearX = math.tan(deg2Rad(chan.fovH)/2.0) * mouseX * chan.near
   	nearZ = math.tan(deg2Rad(chan.fovV)/2.0) * mouseY * chan.near
        # Initialize points
   	mCam2Wall = chan.camera.getMat(selectedNode)
	mouseOrigin = Point3(0)
   	mouseOrigin.assign(mCam2Wall.getRow3(3))
	mouseDir = Vec3(0)
   	mouseDir.set(nearX, chan.near, nearZ)
   	mouseDir.assign(mCam2Wall.xformVec(mouseDir))
        # Calc intersection point
        return planeIntersect(mouseOrigin, mouseDir, ZERO_POINT, NEG_Y_AXIS)

    def getGridIntersectionPoint(self):
	"""
        Return point of intersection between ground plane and line from cam
        through mouse. Return false, if nothing selected
        """
        # Find mouse point on near plane
        chan = self.direct.chan
    	mouseX = chan.mouseX
  	mouseY = chan.mouseY
   	nearX = math.tan(deg2Rad(chan.fovH)/2.0) * mouseX * chan.near
   	nearZ = math.tan(deg2Rad(chan.fovV)/2.0) * mouseY * chan.near
        # Initialize points
   	mCam2Grid = chan.camera.getMat(self.direct.grid)
	mouseOrigin = Point3(0)
   	mouseOrigin.assign(mCam2Grid.getRow3(3))
	mouseDir = Vec3(0)
   	mouseDir.set(nearX, chan.near, nearZ)
   	mouseDir.assign(mCam2Grid.xformVec(mouseDir))
        # Calc intersection point
        return planeIntersect(mouseOrigin, mouseDir, ZERO_POINT, Z_AXIS)

class LevelEditorPanel(Pmw.MegaToplevel):
    def __init__(self, levelEditor, parent = None, **kw):

        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',       'Toontown Level Editor',       None),
            )
        self.defineoptions(kw, optiondefs)

        Pmw.MegaToplevel.__init__(self, parent, title = self['title'])

        self.levelEditor = levelEditor
        self.direct = levelEditor.direct
        # Handle to the toplevels hull
        hull = self.component('hull')

        balloon = self.balloon = Pmw.Balloon(hull)
        # Start with balloon help disabled
        self.balloon.configure(state = 'none')
        
        menuFrame = Frame(hull, relief = GROOVE, bd = 2)
        menuFrame.pack(fill = X, expand = 1)

        menuBar = Pmw.MenuBar(menuFrame, hotkeys = 1, balloon = balloon)
        menuBar.pack(side = LEFT, expand = 1, fill = X)
        menuBar.addmenu('Level Editor', 'Level Editor Operations')
        menuBar.addmenuitem('Level Editor', 'command',
                            'Load DNA from specified file',
                            label = 'Load DNA...',
                            command = self.levelEditor.loadSpecifiedDNAFile)
        menuBar.addmenuitem('Level Editor', 'command',
                            'Save DNA data to specified file',
                            label = 'Save DNA As...',
                            command = self.levelEditor.saveToSpecifiedDNAFile)
        menuBar.addmenuitem('Level Editor', 'command',
                            'Save DNA File',
                            label = 'Save DNA',
                            command = self.levelEditor.outputDNADefaultFile)
        menuBar.addmenuitem('Level Editor', 'command',
                            'Reload Color Palettes',
                            label = 'Reload Colors',
                            command = self.levelEditor.recreateColorMenus)
        menuBar.addmenuitem('Level Editor', 'command',
                            'Reload Style Palettes',
                            label = 'Reload Styles',
                            command = self.levelEditor.recreateStyleMenus)
        menuBar.addmenuitem('Level Editor', 'command',
                            'Exit Level Editor Panel',
                            label = 'Exit',
                            command = self.destroy)

        menuBar.addmenu('Help', 'Level Editor Help Operations')
        self.toggleBalloonVar = IntVar()
        self.toggleBalloonVar.set(0)
        menuBar.addmenuitem('Help', 'checkbutton',
                            'Toggle balloon help',
                            label = 'Balloon Help',
                            variable = self.toggleBalloonVar,
                            command = self.toggleBalloon)

        self.editMenu = Pmw.ComboBox(
            menuFrame, labelpos = W,
            label_text = 'Edit Mode:', entry_width = 12,
            selectioncommand = self.chooseNeighborhood, history = 0,
            scrolledlist_items = ['Toontown Central', 'Donalds Dock',
                                  'The Burrrgh', 'Minnies Melody Land'])
        self.editMenu.selectitem('Toontown Central')
        self.editMenu.pack(side = 'left', expand = 0)
                                     

        # Create the notebook pages
        notebook = Pmw.NoteBook(hull)
        notebook.pack(fill = BOTH, expand = 1)
        streetsPage = notebook.add('Streets')
        toonBuildingsPage = notebook.add('Toon Bldgs')
        landmarkBuildingsPage = notebook.add('Landmark Bldgs')
        # suitBuildingsPage = notebook.add('Suit Buildings')
        propsPage = notebook.add('Props')
        sceneGraphPage = notebook.add('SceneGraph')

        self.addStreetButton = Button(
            streetsPage,
            text = 'ADD STREET',
            command = self.addStreetModule)
        self.addStreetButton.pack(fill = 'x')
        self.streetSelector = Pmw.ComboBox(
            streetsPage,
            dropdown = 0,
            listheight = 200,
            labelpos = W,
            label_text = 'Street type:',
            label_width = 12,
            label_anchor = W,
            entry_width = 30,
            selectioncommand = self.setStreetModuleType,
            scrolledlist_items = map(lambda s: s[7:],
                                     levelEditor.getCatalogCodes(
            'street'))
            )
        self.streetModuleType = levelEditor.getCatalogCode('street',0)
        self.streetSelector.selectitem(self.streetModuleType[7:])
        self.streetSelector.pack(expand = 1, fill = 'both')

        self.addToonBuildingButton = Button(
            toonBuildingsPage,
            text = 'ADD TOON BUILDING',
            command = self.addFlatBuilding)
        self.addToonBuildingButton.pack(fill = 'x')
        self.toonBuildingSelector = Pmw.ComboBox(
            toonBuildingsPage,
            dropdown = 0,
            listheight = 200,
            labelpos = W,
            label_width = 12,
            label_anchor = W,
            label_text = 'Toon bldg type:',
            entry_width = 30,
            selectioncommand = self.setFlatBuildingType,
            scrolledlist_items = ('random20', 'random30',
                                  'toonTenTen', 'toonTwenty',
                                  'toonTenTwenty', 'toonTwentyTen',
                                  'toonTenTenTen', 'toonThirty')
            )
        self.toonBuildingType = 'random20'
        self.toonBuildingSelector.selectitem(self.toonBuildingType)
        self.toonBuildingSelector.pack(expand = 1, fill = 'both')
        
        self.toonBuildingWidthScale = EntryScale.EntryScale(
            toonBuildingsPage, min = 1.0, max = 30.0,
            resolution = 0.01, text = 'Wall Width',
            command = self.updateSelectedWallWidth)
        self.toonBuildingWidthScale.pack(fill = 'x')
        
        self.addLandmarkBuildingButton = Button(
            landmarkBuildingsPage,
            text = 'ADD LANDMARK BUILDING',
            command = self.addLandmark)
        self.addLandmarkBuildingButton.pack(fill = 'x')
        self.landmarkBuildingSelector = Pmw.ComboBox(
            landmarkBuildingsPage,
            dropdown = 0,
            listheight = 200,
            labelpos = W,
            label_width = 12,
            label_anchor = W,
            label_text = 'Landmark Building type:',
            entry_width = 30,
            selectioncommand = self.setLandmarkType,
            scrolledlist_items = map(lambda s: s[14:],
                                     levelEditor.getCatalogCodes(
            'toon_landmark'))
            )
        self.landmarkType = levelEditor.getCatalogCode(
            'toon_landmark',0)
        self.landmarkBuildingSelector.selectitem(
            levelEditor.getCatalogCode('toon_landmark',0)[14:])
        self.landmarkBuildingSelector.pack(expand = 1, fill = 'both')

        self.addPropsButton = Button(
            propsPage,
            text = 'ADD PROP',
            command = self.addProp)
        self.addPropsButton.pack(fill = 'x')
        self.propSelector = Pmw.ComboBox(
            propsPage,
            dropdown = 0,
            listheight = 200,
            labelpos = W,
            label_width = 12,
            label_anchor = W,
            label_text = 'Prop type:',
            entry_width = 30,
            selectioncommand = self.setPropType,
            scrolledlist_items = map(lambda s: s[5:],
                                     levelEditor.getCatalogCodes('prop'))
            )
        self.propType = levelEditor.getCatalogCode('prop',0)
        self.propSelector.selectitem(
            levelEditor.getCatalogCode('prop',0)[5:])
        self.propSelector.pack(expand = 1, fill = 'both')
        # Compact down notebook
        notebook.setnaturalsize()

        self.colorEntry = VectorWidgets.ColorEntry(
            hull, text = 'Select Color',
            command = self.updateSelectedObjColor)
        self.colorEntry.menu.add_command(
            label = 'Save Color', command = self.levelEditor.saveColor)
        self.colorEntry.pack(fill = 'x')

        buttonFrame = Frame(hull)
        self.fMapViz = IntVar()
        self.fMapViz.set(0)
        self.mapSnapButton = Checkbutton(buttonFrame,
                                      text = 'Map Viz',
                                      width = 6,
                                      variable = self.fMapViz,
                                      command = self.toggleMapViz)
        self.mapSnapButton.pack(side = 'left', expand = 1, fill = 'x')

        self.fXyzSnap = IntVar()
        self.fXyzSnap.set(1)
        self.xyzSnapButton = Checkbutton(buttonFrame,
                                      text = 'XyzSnap',
                                      width = 6,
                                      variable = self.fXyzSnap,
                                      command = self.toggleXyzSnap)
        self.xyzSnapButton.pack(side = 'left', expand = 1, fill = 'x')

        self.fHprSnap = IntVar()
        self.fHprSnap.set(1)
        self.hprSnapButton = Checkbutton(buttonFrame,
                                      text = 'HprSnap',
                                      width = 6,
                                      variable = self.fHprSnap,
                                      command = self.toggleHprSnap)
        self.hprSnapButton.pack(side = 'left', expand = 1, fill = 'x')

        self.fGrid = IntVar()
        self.fGrid.set(0)
        self.gridButton = Checkbutton(buttonFrame,
                                      text = 'Show Grid',
                                      width = 6,
                                      variable = self.fGrid,
                                      command = self.toggleGrid)
        self.gridButton.pack(side = 'left', expand = 1, fill = 'x')

        buttonFrame.pack(expand = 1, fill = 'x')

        buttonFrame2 = Frame(hull)
        self.groupButton = Button(
            buttonFrame2,
            text = 'New group',
            command = self.levelEditor.createNewLevelGroup)
        self.groupButton.pack(side = 'left', expand = 1, fill = 'x')
        
        self.saveButton = Button(
            buttonFrame2,
            text = 'Set Parent',
            command = self.levelEditor.setGroupParentToSelected)
        self.saveButton.pack(side = 'left', expand = 1, fill = 'x')

        self.isolateButton = Button(
            buttonFrame2,
            text = 'Isolate Selected',
            command = self.levelEditor.isolateSelectedNodePath)
        self.isolateButton.pack(side = 'left', expand = 1, fill = 'x')

        self.showAllButton = Button(
            buttonFrame2,
            text = 'Show All',
            command = self.levelEditor.showAll)
        self.showAllButton.pack(side = 'left', expand = 1, fill = 'x')

        buttonFrame2.pack(fill = 'x')

        buttonFrame3 = Frame(hull)
        self.driveMode = IntVar()
        self.driveMode.set(1)
        self.driveModeButton = Radiobutton(
            buttonFrame3,
            text = 'Drive Mode',
            value = 0,
            variable = self.driveMode,
            command = self.levelEditor.useDriveMode)
        self.driveModeButton.pack(side = 'left', expand = 1, fill = 'x')
        self.directModeButton = Radiobutton(
            buttonFrame3,
            text = 'DIRECT Fly',
            value = 1,
            variable = self.driveMode,
            command = self.levelEditor.useDirectFly)
        self.directModeButton.pack(side = 'left', expand = 1, fill = 'x')
        buttonFrame3.pack(fill = 'x')

        self.sceneGraphExplorer = SceneGraphExplorer(
            parent = sceneGraphPage,
            root = self.levelEditor.getLevelObjects(),
            menuItems = ['Select', 'Isolate', 'Flash',
                         'Toggle Viz', 'Set Parent', 'Add Group'])
        self.sceneGraphExplorer.pack(expand = 1, fill = 'both')
        
    def toggleGrid(self):
        if self.fGrid.get():
            self.direct.grid.enable()
        else:
            self.direct.grid.disable()

    def toggleXyzSnap(self):
        self.direct.grid.setXyzSnap(self.fXyzSnap.get())

    def toggleHprSnap(self):
        self.direct.grid.setHprSnap(self.fXyzSnap.get())
        
    def toggleMapViz(self):
        self.levelEditor.toggleMapViz(self.fMapViz.get())
        
    def setStreetModuleType(self,name):
        self.streetModuleType = 'street_' + name

    def addStreetModule(self):
        self.levelEditor.addStreetModule(self.streetModuleType)

    def setFlatBuildingType(self,name):
        self.toonBuildingType = name
        
    def addFlatBuilding(self):
        self.levelEditor.addFlatBuilding(self.toonBuildingType)

    def setLandmarkType(self,name):
        self.landmarkType = 'toon_landmark_' + name

    def addLandmark(self):
        self.levelEditor.addLandmark(self.landmarkType)

    def setPropType(self,name):
        self.propType = 'prop_' + name
        
    def addProp(self):
        self.levelEditor.addProp(self.propType)

    def chooseNeighborhood(self, neighborhood):
        if neighborhood == "Toontown Central":
            self.levelEditor.editToontownCentral()
        elif neighborhood == "Donalds Dock":
            self.levelEditor.editDonaldsDock()
        elif neighborhood == "The Burrrgh":
            self.levelEditor.editTheBurrrgh()
        elif neighborhood == "Minnies Melody Land":
            self.levelEditor.editMinniesMelodyLand()

    def updateSelectedWallWidth(self, strVal):
        self.levelEditor.updateSelectedWallWidth(string.atof(strVal))

    def setCurrentColor(self, colorVec):
        self.colorEntry.set([int(colorVec[0] * 255.0),
                             int(colorVec[1] * 255.0),
                             int(colorVec[2] * 255.0),
                             255])
        self.colorEntry['resetValue'] = (
            [int(colorVec[0] * 255.0),
             int(colorVec[1] * 255.0),
             int(colorVec[2] * 255.0),
             255])

    def updateSelectedObjColor(self, color):
        obj = self.levelEditor.targetDNAObject
        if obj:
            objClass = obj.__class__.getClassType()
            if ((objClass.eq(DNAWall.getClassType())) |
                (objClass.eq(DNAWindows.getClassType())) |
                (objClass.eq(DNADoor.getClassType())) |
                (objClass.eq(DNACornice.getClassType())) |
                (objClass.eq(DNAProp.getClassType()))
                ):
                self.levelEditor.updateObjColor(
                    self.levelEditor.targetDNAObject,
                    VBase4((color[0]/255.0),
                           (color[1]/255.0),
                           (color[2]/255.0),
                           1.0))

    def toggleBalloon(self):
        if self.toggleBalloonVar.get():
            self.balloon.configure(state = 'balloon')
        else:
            self.balloon.configure(state = 'none')


"""

!Level methodsFor: 'object operations' stamp: 'panda 00/00/0000 00:00'!
keyboardRotateNodePath: aNodePath key: arrowDirection 
	| pos hpr scale |
	pos = aNodePath getPos.
	scale = aNodePath getScale.
	self.lastAngle = self.lastAngle + (arrowDirection caseOf: {	[#left]->[ 90.0 ] .
									[#right]->[ -90.0 ] .
									[#up]->[ 90.0 ] .
									[#down]->[ -90.0 ] }).
	(self.lastAngle < -180.0) ifTrue: [ self.lastAngle = self.lastAngle + 360.0 ].
	(self.lastAngle > 180.0) ifTrue: [ self.lastAngle = self.lastAngle - 360.0 ].
	hpr = VBase3 new: self.lastAngle y: 0.0 z: 0.0.

	aNodePath setPosHprScale: pos hpr: hpr scale: scale.

	# Refresh DNA
	self.updateDNAPosHpr: aNodePath.
	# Position grid for placing next object
	self.autoPositionGrid.

! !

!Level methodsFor: 'object operations' stamp: 'panda 00/00/0000 00:00'!
keyboardTranslateNodePath: aNodePath key: arrowDirection 
	| pos deltaMove gridToCamera xAxis zAxis camXAxis xxDot xzDot |

	gridToCamera = self.grid getMat: chanCenter camera.
	xAxis = Vec3 right.
	zAxis = Vec3 up.
	camXAxis = gridToCamera xformVec: xAxis.
	xxDot = camXAxis dot: xAxis.
	xzDot = camXAxis dot: zAxis.
	
	# get current object position
	pos = aNodePath getPos: self.grid.

	# what is the current grid spacing?
	deltaMove = self.grid gridSpacing.

	# Add or subtract the specified delta
	(xxDot abs > xzDot abs) ifTrue: [
		(xxDot < 0.0) ifTrue: [ deltaMove = deltaMove negated. ].
		arrowDirection caseOf: {	[#left]->[ pos setX: ((pos at: 0) - deltaMove) ] .
								[#right]->[ pos setX: ((pos at: 0) + deltaMove) ] .
								[#up]->[ pos setY: ((pos at: 1) + deltaMove) ] .
								[#down]->[ pos setY: ((pos at: 1) - deltaMove) ] } 
		]
	ifFalse: [
		(xzDot < 0.0) ifTrue: [ deltaMove = deltaMove negated. ].
		arrowDirection caseOf: {	[#left]->[ pos setY: ((pos at: 1) + deltaMove) ] .
								[#right]->[ pos setY: ((pos at: 1) - deltaMove) ] .
								[#up]->[ pos setX: ((pos at: 0) + deltaMove) ] .
								[#down]->[ pos setX: ((pos at: 0) - deltaMove) ] } 
		].

	# Move it
	aNodePath setPos: self.grid pos: pos.
	
	# Refresh DNA
	self.updateDNAPosHpr: aNodePath.
	# Position grid for placing next object
	self.autoPositionGrid.

! !

!Level methodsFor: 'object operations' stamp: 'panda 00/00/0000 00:00'!
keyboardXformNodePath: arrowDirection 
	(direct fControl) ifTrue: [
		direct selectedNodePaths 
			valuesDo: [ :np | self.keyboardRotateNodePath: np key: arrowDirection ]. 
		]
	ifFalse: [
		direct selectedNodePaths
			valuesDo: [ :np | self.keyboardTranslateNodePath: np key: arrowDirection ].
		].
! !

!Level methodsFor: 'object operations' stamp: 'panda 00/00/0000 00:00'!
mouseCrank: aNodePath
	# Rotate object about its origin, based upon current mouse position
	# Positive X axis of object is rotated to line up with current Crank Dir
	(or closest snapAngle degree angle if HPR Snap is on
	(self.grid getMouseIntersectionPoint: self.hitPt	) 
		ifTrue: [self.crankDir = self.hitPt - self.crankOrigin.
	              ((self.crankDir length) > 1.0)
					ifTrue: [ | deltaAngle newH |
							 deltaAngle = (self.getCrankAngle - startAngle).
							 newH = startH + deltaAngle.
						 	 self.hprSnap ifTrue: [ newH = newH roundTo: self.snapAngle. ].
							 aNodePath setH: newH.
							 # record angle (used for placing new instance)
 							 self.lastAngle = newH.
							].
				].
! !


    def acceptArrowKeys(self):
	# Accept arrow key events for swinging piece around
	self.accept('left', self.keyboardXformNodePath, ['left'])
	self.accept('right', self.keyboardXformNodePath, ['right'])
	self.accept('up', self.keyboardXformNodePath, ['up'])
	self.accept('down', self.keyboardXformNodePath, ['down'])

    def keyboardXformNodePath(self,x):
        pass

    def ignoreArrowKeys(self):
	# Accept arrow key events for swinging piece around
	self.ignore('left')
	self.ignore('right')
	self.ignore('up')
	self.ignore('down')


"""
