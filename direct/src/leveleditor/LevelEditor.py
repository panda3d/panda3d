from PandaObject import *
from PieMenu import *
from OnscreenText import *
from whrandom import *


class LevelEditor(NodePath, PandaObject):
    def __init__(self,direct):
        # Initialize superclass
        NodePath.__init__(self)
        self.assign(hidden.attachNewNode( NamedNode('LevelEditor')))

        # Record handle to direct session
        self.direct = direct
	# Make sure direct is running
	self.direct.enable()

	# Create level editor dictionaries
	# This dictionary stores information about new objects added
        # to the level
	self.levelDictionary = {}
	# This dictionary stores information about module hooks,
        # grouped by level
	self.hooksDictionary = {}
	# This dictionary stores information about clickBoxLists,
        # grouped by level
	self.clickBoxDictionary = {}
	# This dictionary stores information about the various
        # pie menus in use
	self.pieMenuDictionary = {}
	# This dictionary stores all the different color palettes
	self.colorPaletteDictionary = {}
	# This dictionary stores info about current and possible
        # object attributes
	self.attributeDictionary = {}
	# This dictionary stores pleasing style combinations
	self.styleDictionary = {}

	# DNAStorage instance for storing level DNA info
	self.dnaStore = DNAStorage()
	self.dnaStore.loadDNAFile('dna/storage.dna',
                                  getDefaultCoordinateSystem())

	# Top level DNA Data Object
	self.levelObjectsDNA = DNAData('LevelObjects')

	# Create top level node
	self.levelObjects = self.attachNewNode(NamedNode('LevelObjects'))

	# Create a top level group
	self.createTopLevelGroup()

	# Set used to iterate over module categories
	self.categorySet = []

	self.selectedLevelObject = None
	self.targetDNAObject = None
	self.activeMenu = None

	# Plane for intersection tests with wall
	self.wallIntersectionPlane = Plane(Vec3.up(), Point3(0))

        # Get a handle to the grid
	self.grid = self.direct.grid
        self.showGrid(0)

	#self.levelMap = loader.loadModel('LevelEditor/level-map.egg')
        self.levelMap = hidden.attachNewNode(NamedNode('level-map'))
	self.levelMap.reparentTo(self)
	self.levelMap.getBottomArc().setTransition(TransparencyTransition(1))
	self.levelMap.setColor(Vec4(1,1,1,.4))
	self.levelMap.hide()

	self.hitPt = Point3(0)
	self.offset = Point3(0)
	self.crankOrigin = Point3(0)
	self.crankDir = Vec3(0)

	self.hprSnap = 1
	self.snapAngle = 90.0
	self.lastAngle = 0.0

	# Create buttons and pie menus
        # MRM
	#self.initializeLevelEditorButtons()
	# Initialize styles
	self.initializeStyleDictionary()
	# Initialize pie Menus (this depends on the style dictionary)
	self.initializePieMenus()

	base.cam.node().setNear(5.0)
	base.cam.node().setFar(10000)
	self.direct.camera.setPos(0,0,10)

	# Default is to use the toontown central color palette
	self.editToontownCentral()

	self.enable()

    def initializeAttributeDictionary(self):

	# Retrieve lists of available attributes from DNAStorage object
	# Cornices
	attributeList = self.getCatalogCodesSuffix('cornice', '_ur')
        # MRM: What does this do?
	# attributeList addFirst: 'none'
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

    def getActiveColor(self):
	return self.attributeDictionary['activeColor']

    def setActiveColor(self, color):
	self.attributeDictionary['activeColor'] = color

    def getBuildingHeight(self):
	return self.attributeDictionary['buildingHeight']

    def setBuildingHeight(self, height):
	self.attributeDictionary['buildingHeight'] = height

    def getCategorySet(self):
	return self.categorySet

    def getClickBoxDictionary(self):
	return self.clickBoxDictionary

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

    def setGroupParent(self,nodePath):
	parentDNA = self.getDNAGroup(nodePath)
        if parentDNA:
            self.groupParent = nodePath
            self.groupParentDNA = parentDNA

    def getHooksDictionary(self):
	return self.hooksDictionary

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
            if dnaObject.getClassType().eq(DNAFlatBuilding.getClassType()):
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
            if window & (window.getCount() > 0):
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

    def printWallStyleForBldgWall(self, DNAFlatBuilding,wallNum):
        if (DNAFlatBuilding.getClassType().eq(DNAFlatBuilding.getClassType())):
            wall = self.getWall(DNAFlatBuilding, wallNum)
            cornice = self.getCornice(DNAFlatBuilding)
            self.printWallStyleWith(wall,cornice)

    def getPropType(self):
	return self.attributeDictionary['propType']

    def setPropType(self,dnaString):
	self.attributeDictionary['propType'] = dnaString

    def getPropTypes(self):
	return self.attributeDictionary['propTypes']

    def getSelectedLevelObject(self):
	return self.selectedLevelObject

    def getSnapAngle(self):
	return self.snapAngle

    def setSnapAngle(self, aFloat):
	self.snapAngle = aFloat

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

    def acceptArrowKeys(self):
	# Accept arrow key events for swinging piece around
	self.accept('left', self.keyboardXformNodePath, ['left'])
	self.accept('right', self.keyboardXformNodePath, ['right'])
	self.accept('up', self.keyboardXformNodePath, ['up'])
	self.accept('down', self.keyboardXformNodePath, ['down'])

    def keyboardXformNodePath(self,x):
        pass

    def activateLandmarkButtons(self):
	# Switch menus to reveal street menu
	self.mainMenuDisable()
	self.categoryEnable('toon_landmark')

    def activatePropButtons(self):
	# Switch menus to reveal street menu
	self.mainMenuDisable()
	self.categoryEnable('prop')

    def activateStreetModuleButtons(self):
	# Switch menus to reveal street menu
	self.mainMenuDisable()
	self.categoryEnable('street')

    def activateVizObjectsButtons(self):
	# Switch menus to reveal viz region menu
	#self.mainMenuDisable()
	#self.clickBoxDictionary['vizRegionButtons'].enable()
	self.accept('addVizRegion', self.addVizRegion)
	self.accept('addCollisionSphere', self.addCollisionSphere)
	self.grid.setGridSpacing(10.0)

    def activateWallModuleButtons(self):
	# Switch menus to reveal street menu
	self.mainMenuDisable()
	self.categoryEnable('wall')

    def addHook(self, hook, function):
	self.accept(hook, function, [hook])

    def allMenuDisable(self):
	self.mainMenuDisable()
	self.gridMenuDisable()
	self.subMenuDisable()
	self.dnaMenuDisable()

    def categoryDisable(self, categoryName):
        clickBoxList = self.clickBoxDictionary.set(categoryName,None)
        if clickBoxList:
            clickBoxList.disable()

        hooks = self.hooksDictionary.get(categoryName, None)
        if hooks:
            for hook in hooks:
                self.ignore(hook)

	# Do any category specific disabilizaton here
        if categoryName == 'wall':
            clickBoxList = self.clickBoxDictionary.get('wallWidths',None)
            if clickBoxList:
                clickBoxList.disable()
                hooks = self.hooksDictionary.get('wallWidths', None)
                if hooks:
                    for hook in hooks:
                        self.ignore(hook)

	# Clear out space and insert hooks
	self.ignore('space')
	self.ignore('insert')

    def categoryEnable(self,categoryName):
	# First activate this category's main buttons
        """
        clickBoxList = self.clickBoxDictionary.get(categoryName,None)
        if clickBoxList:
            clickBoxList.enable()
        """
	# Now activate hooks and any supplemental actions
        if categoryName == 'street':
            # activate street module hooks
            hooks = self.hooksDictionary.get(categoryName,None)
            if hooks:
                for hook in hooks:
                    self.addHook(hook,self.addStreetModule)
        elif categoryName == 'wall':
            # Activate wall module hooks	
            hooks = self.hooksDictionary.get(categoryName,None)
            if hooks:
                for hook in hooks:
                    self.addHook(hook, self.addFlatBuilding)
            """
            # Also activate wall width buttons and hooks
            clickBoxList = self.clickBoxDictionary.get('wallWidths', None)
            if clickBoxList:
                clickBoxList.enable()
            """
            hooks = self.hooksDictionary.get('wallWidths',None)
            if hooks:
                for hook in hooks:
                    self.addHook(hook,self.wallWidthSym)
        elif categoryName == 'toon_landmark':
            # activate landmark hooks
            hooks = self.hooksDictionary.get(categoryName,None)
            if hooks:
                for hook in hooks:
                    self.addHook(hook,self.addLandmark)
        elif categoryName == 'prop':
            # activate prop hooks
            hooks = self.hooksDictionary.get(categoryName,None)
            if hooks:
                for hook in hooks:
                    self.addHook(hook,self.addProp)

    def destroy(self):
	self.disable()
	self.removeNode()

    def disable(self):
	self.direct.deselectAll()
	self.allMenuDisable()
	self.hide()
	self.grid.ignore('insert')
	self.ignore('mainMenuEnable')
	self.ignore('preRemoveNodePath')
	self.ignore('preSelectNodePath')
	self.ignore('setGroupParent')
	self.ignore('isolateNodePath')
	self.ignore('reparentNodePath')
	self.ignore('createNewLevelGroup')
	self.ignore('showAll')
	self.ignore('setNodePathName')
	self.ignore('p')
	self.disableManipulation()

    def disableManipulation(self):
	# Ignore arrow keys
	self.ignoreArrowKeys()

	# Disable handling of mouse events
	self.ignore('handleMouse1')
	self.ignore('handleMouse1Up')
	self.ignore('handleMouse3')
	self.ignore('handleMouse3Up')

    def dnaMenuDisable(self):
	# Disable DNA menu
	self.clickBoxDictionary['groupButton'].disable()
	self.ignore('createNewLevelGroup')
	self.clickBoxDictionary['saveButton'].disable()
	self.ignore('outputDNA:')
	self.clickBoxDictionary['mapButton'].disable()
	self.ignore('toggleMapViz')

    def dnaMenuEnable(self):
	# Enable DNA menu
	self.clickBoxDictionary['groupButton'].enable()
	self.accept('createNewLevelGroup', self.createNewLevelGroup)
	self.clickBoxDictionary['saveButton'].enable()
	self.accept('outputDNA', self.outputDNA)
	self.clickBoxDictionary['mapButton'].enable()
	self.accept('toggleMapViz', self.toggleMapViz)

    def editDonaldsDock(self):
	self.levelMap.setPos(-900.0,-300.0,0.0)
	self.useDonaldsDockColors()
	self.styleDictionary = (
            self.attributeDictionary['donaldsDockStyleDictionary'])
	self.pieMenuDictionary['styleMenu'] = (
            self.pieMenuDictionary['donaldsDockStyleMenu'])
	self.attributeDictionary['streetTexture'] = 'street_street_dock_tex'
	self.attributeDictionary['sidewalkTexture'] = (
            'street_sidewalk_dock_tex')

    def editToontownCentral(self):
	self.levelMap.setPos(0.0,0.0,0.0)
	self.useToontownCentralColors()
	self.styleDictionary = (
            self.attributeDictionary['toontownCentralStyleDictionary'])
        self.pieMenuDictionary['styleMenu'] = (
            self.pieMenuDictionary['toontownCentralStyleMenu'])
	self.attributeDictionary['streetTexture'] = 'street_street_tex'
	self.attributeDictionary['sidewalkTexture'] = 'street_sidewalk_tex'

    def enable(self):
	#self.allMenuDisable()
	#self.mainMenuEnable()
	#self.gridMenuEnable()
	#self.dnaMenuEnable()
	self.show()
	self.accept('mainMenuEnable', self.mainMenuEnable)
	self.accept('preRemoveNodePath', self.preRemoveNodePath)
	self.accept('preSelectNodePath', self.preSelectNodePath)
	self.accept('toggleMapViz', self.toggleMapViz)
	self.accept('setGroupParent', self.setGroupParent)
	self.accept('isolateNodePath', self.isolateNodePath)
	self.accept('reparentNodePath', self.reparentNodePath)
	self.accept('createNewLevelGroup', self.createNewLevelGroup)
	self.accept('setNodePathName', self.setNodePathName)
	self.accept('showAll', self.showAll)
	self.accept('p',self.plantSelectedNodePath)
	self.enableManipulation()

    def enableManipulation(self):
	# Enable interactive placement of a nodePath
	# Turn off camera control
	base.disableMouse()

	# Update arrow key events for swinging selected around
        # using the keyboard
	self.acceptArrowKeys()

	# Handle mouse events for moving object around
	self.accept('handleMouse1',self.levelHandleMouse1)
	self.accept('handleMouse1Up',self.levelHandleMouse1Up)
	self.accept('handleMouse3',self.levelHandleMouse3)
	self.accept('handleMouse3Up',self.levelHandleMouse3Up)

    def gridMenuDisable(self):
	self.clickBoxDictionary['gridMenuButtons'].disable()
	self.ignore('showGrid')
	self.ignore('xyzSnap')
	self.ignore('hprSnap')

    def gridMenuEnable(self):
	# Enable grid menu
	self.clickBoxDictionary['gridMenuButtons'].enable()
	self.accept('showGrid', self.showGrid)
	self.accept('xyzSnap', self.xyzSnap)
	self.accept('hprSnap', self.hprSnap)

    def ignoreArrowKeys(self):
	# Accept arrow key events for swinging piece around
	self.ignore('left')
	self.ignore('right')
	self.ignore('up')
	self.ignore('down')

    def mainMenuDisable(self):
	self.clickBoxDictionary['mainMenuButtons'].disable()
	self.ignore('activateWallModuleButtons')
	self.ignore('activateStreetModuleButtons')
	self.ignore('activateLandmarkButtons')
	self.ignore('activatePropButtons')

    def mainMenuEnable(self):
	# Make sure all submenus are hidden
	self.subMenuDisable()
	# Now enable main menu
	self.clickBoxDictionary['mainMenuButtons'].enable
	self.accept('activateWallModuleButtons', self.activateWallModuleButtons)
	self.accept('activateStreetModuleButtons', self.activateStreetModuleButtons)
	self.accept('activateLandmarkButtons', self.activateLandmarkButtons)
	self.accept('activatePropButtons', self.activatePropButtons)

    def subMenuDisable(self):
        for category in self.categorySet:
            self.categoryDisable(category)

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

    def vizMenuDisable(self):
	self.clickBoxDictionary['vizRegionButtons'].disable()
	self.ignore('addVizRegion')
	self.ignore('addCollisionSphere')
	self.grid.setGridSpacing(5.0)

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

    def isolateNodePath(self,aNodePath):
	# First show everything in level
	self.levelObjects.showAllDescendants()
	render.hideCollisionSolids()
	aNodePath.hideSiblings()
        
    def levelHandleMouse1(self):
        selectedNodePath = self.direct.selected.last
        if selectedNodePath:
            self.followMouseStart(selectedNodePath)
            
    def levelHandleMouse1Up(self):
	self.followMouseStop()

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

            # Interaction type depends on selected object's class
            objClass = selectedObjectDNA.getClassType()
            if objClass.eq(DNAFlatBuilding.getClassType()):
                # Where are we hitting the building?
                hitPt = Point3(0)
                if self.getWallIntersectionPoint(hitPt):
                    xPt = hitPt[0]
                    zPt = hitPt[2]
                    # Which wall are we pointing at (if any)
                    wallNum = self.getWallNum(selectedObjectDNA, zPt)
                    # How wide is the selected wall?
                    wallWidth = selectedObjectDNA.getWidth()
                    # Menu mode depends on where we are pointing
                    if (zPt > self.getWallHeights(selectedObjectDNA)[-1:]):
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
                            menuType = 'wall'
                            target = selectedWall
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
                        if ((wallNum != -1) & (self.direct.fAlt)):
                            menuType = 'style'
                            target = self.getWall(selectedObjectDNA,wallNum)
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
            # Now spawn apropriate menu task
            if (target | (menuType == 'cornice')):
                self.spawnMenuTask(menuType, target)

    def levelHandleMouse3Up(self):
        if self.activeMenu:
            self.activeMenu.removePieMenuTask()

    def preSelectNodePath(self, aNodePath):
	taskMgr.removeTasksNamed('preselectNodePath')
        t = Task.Task(self.preSelectNodePathTask)
        t.aNodePath = aNodePath
        t.initState = t.hidden = aNodePath.isHidden()
        t.count = 0
        t.uponDeath = self.preSelectDone
        #taskMgr.spawnTaskNamed(t, 'preselectNodePath')

    def preSelectNodePathTask(self, state):
        aNodePath = state.aNodePath
	initState = state.initState
        hidden = state.hidden
        count = t.count
        if (t.count < 4):
            if hidden:
                aNodePath.show()
            else:
                aNodePath.hide()
                t.count = count + 1
            t.hidden = not t.hidden
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
            self.grid.show()
        else:
            self.grid.hide()
            
    def spawnMenuTask(self, menu, aDNAObject):
	# Record the starting window code and targetDNAObject
	# This will be used by pieMenu action selector
	self.targetDNAObject = aDNAObject
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
            state = self.getWindow(aDNAObject, 0).getCount()
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
            self.activeMenu = self.pieMenuDictionary['wallColorMenu']
            self.activeColors = self.getWallColors()
            state = aDNAObject.getColor()
        elif menu == 'propType':
            self.activeMenu = self.pieMenuDictionary['propTypesMenu']
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
            self.levelMap.show()
        else:
            self.levelMap.hide()

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
        dictionary['key'] = style

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
                        (self.direct.chan.width / self.direct.chan.height) *
                        math.sin(i * angle)))
            
            # Add it to the wallColorMenu
            node.reparentTo(newColorMenu)

	# Scale the whole shebang down by 0.5
	newColorMenu.setScale(0.5)

	# Store menu and colors in appropriate dictionarys
	self.pieMenuDictionary[menuName] = (
            PieMenu(self.direct, newColorMenu, self.updateColorIndex))
	self.colorPaletteDictionary[menuName] = adjustedColorArray

    def createColorMenus(self):
        self.createColorMenu('toontownCentralWallColors', [
            VBase4((193.0/255.0), (187.0/255.0), (163.0/255.0), 1.0),
            VBase4((208.0/255.0), (232.0/255.0), (113.0/255.0), 1.0),
            VBase4((230.0/255.0), (144.0/255.0), (86.0/255.0), 1.0),
            VBase4((232.0/255.0), (137.0/255.0), (112.0/255.0), 1.0),
            VBase4((232.0/255.0), (160.0/255.0), (113.0/255.0), 1.0),
            VBase4((240.0/255.0), (90.0/255.0), (90.0/255.0), 1.0),
            VBase4((254.0/255.0), (176.0/255.0), (124.0/255.0), 1.0),
            VBase4((255.0/255.0), (106.0/255.0), (69.0/255.0), 1.0),
            VBase4((255.0/255.0), (180.0/255.0), (69.0/255.0), 1.0),
            VBase4((255.0/255.0), (213.0/255.0), (151.0/255.0), 1.0),
            VBase4((255.0/255.0), (225.0/255.0), (205.0/255.0), 1.0),
            VBase4((255.0/255.0), (234.0/255.0), (151.0/255.0), 1.0),
            VBase4((255.0/255.0), (255.0/255.0), (151.0/255.0), 1.0),
            VBase4((55.0/255.0), (244.0/255.0), (70.0/255.0), 1.0),
            VBase4((27.0/255.0), (203.0/255.0), (56.0/255.0), 1.0),
            VBase4((53.0/255.0), (185.0/255.0), (78.0/255.0), 1.0)
            ])

        self.createColorMenu('toontownCentralWindowColors', [
            VBase4((101.0/255.0), (156.0/255.0), (170.0/255.0), 1.0),
            VBase4((150.0/255.0), (254.0/255.0), (124.0/255.0), 1.0),
            VBase4((232.0/255.0), (160.0/255.0), (113.0/255.0), 1.0),
            VBase4((255.0/255.0), (128.0/255.0), (96.0/255.0), 1.0),
            VBase4((255.0/255.0), (151.0/255.0), (151.0/255.0), 1.0),
            VBase4((255.0/255.0), (160.0/255.0), (96.0/255.0), 1.0),
            VBase4((255.0/255.0), (234.0/255.0), (151.0/255.0), 1.0),
            VBase4((33.0/255.0), (200.0/255.0), (133.0/255.0), 1.0),
            VBase4((69.0/255.0), (255.0/255.0), (106.0/255.0), 1.0),
            VBase4((82.0/255.0), (170.0/255.0), (152.0/255.0), 1.0)
            ])
        
        self.createColorMenu('toontownCentralDoorColors', [
            VBase4((190.0/255.0), (202.0/255.0), (141.0/255.0), 1.0),
            VBase4((206.0/255.0), (155.0/255.0), (122.0/255.0), 1.0),
            VBase4((255.0/255.0), (151.0/255.0), (151.0/255.0), 1.0),
            VBase4((255.0/255.0), (160.0/255.0), (96.0/255.0), 1.0),
            VBase4((255.0/255.0), (223.0/255.0), (96.0/255.0), 1.0)
            ])
        
        self.createColorMenu('toontownCentralCorniceColors', [
            VBase4((113.0/255.0), (232.0/255.0), (160.0/255.0), 1.0),
            VBase4((240.0/255.0), (182.0/255.0), (168.0/255.0), 1.0),
            VBase4((255.0/255.0), (191.0/255.0), (96.0/255.0), 1.0),
            VBase4((255.0/255.0), (255.0/255.0), (151.0/255.0), 1.0),
            VBase4((53.0/255.0), (185.0/255.0), (78.0/255.0), 1.0),
            VBase4((201.0/255.0), (91.0/255.0), (48.0/255.0), 1.0),
            VBase4((193.0/255.0), (187.0/255.0), (163.0/255.0), 1.0),
            VBase4((208.0/255.0), (232.0/255.0), (113.0/255.0), 1.0),
            VBase4((230.0/255.0), (144.0/255.0), (86.0/255.0), 1.0),
            VBase4((232.0/255.0), (137.0/255.0), (112.0/255.0), 1.0),
            VBase4((232.0/255.0), (160.0/255.0), (113.0/255.0), 1.0),
            VBase4((240.0/255.0), (90.0/255.0), (90.0/255.0), 1.0),
            VBase4((254.0/255.0), (176.0/255.0), (124.0/255.0), 1.0),
            VBase4((255.0/255.0), (106.0/255.0), (69.0/255.0), 1.0),
            VBase4((255.0/255.0), (180.0/255.0), (69.0/255.0), 1.0),
            VBase4((255.0/255.0), (213.0/255.0), (151.0/255.0), 1.0),
            VBase4((255.0/255.0), (225.0/255.0), (205.0/255.0), 1.0),
            VBase4((255.0/255.0), (234.0/255.0), (151.0/255.0), 1.0),
            VBase4((255.0/255.0), (255.0/255.0), (151.0/255.0), 1.0),
            VBase4((55.0/255.0), (244.0/255.0), (70.0/255.0), 1.0),
            VBase4((27.0/255.0), (203.0/255.0), (56.0/255.0), 1.0),
            VBase4((53.0/255.0), (185.0/255.0), (78.0/255.0), 1.0)
            ])

        self.createColorMenu('donaldsDockWallColors', [
           VBase4((106.0/255.0), (63.0/255.0), (63.0/255.0), 1.0),
           VBase4((160.0/255.0), (120.0/255.0), (60.0/255.0), 1.0),
           VBase4((162.0/255.0), (61.0/255.0), (81.0/255.0), 1.0),
           VBase4((182.0/255.0), (126.0/255.0), (88.0/255.0), 1.0),
           VBase4((192.0/255.0), (115.0/255.0), (114.0/255.0), 1.0),
           VBase4((206.0/255.0), (122.0/255.0), (122.0/255.0), 1.0),
           VBase4((222.0/255.0), (176.0/255.0), (108.0/255.0), 1.0),
           VBase4((236.0/255.0), (39.0/255.0), (39.0/255.0), 1.0),
           VBase4((43.0/255.0), (116.0/255.0), (58.0/255.0), 1.0),
           VBase4((92.0/255.0), (116.0/255.0), (56.0/255.0), 1.0)
           ])

        self.createColorMenu('donaldsDockWindowColors', [
           VBase4((0.0/255.0), (156.0/255.0), (93.0/255.0), 1.0),
           VBase4((164.0/255.0), (238.0/255.0), (116.0/255.0), 1.0),
           VBase4((202.0/255.0), (120.0/255.0), (120.0/255.0), 1.0),
           VBase4((210.0/255.0), (210.0/255.0), (102.0/255.0), 1.0),
           VBase4((224.0/255.0), (109.0/255.0), (109.0/255.0), 1.0),
           VBase4((232.0/255.0), (87.0/255.0), (116.0/255.0), 1.0),
           VBase4((255.0/255.0), (128.0/255.0), (96.0/255.0), 1.0)
           ])

        self.createColorMenu('donaldsDockDoorColors', [
           VBase4((115.0/255.0), (136.0/255.0), (115.0/255.0), 1.0),
           VBase4((132.0/255.0), (156.0/255.0), (132.0/255.0), 1.0),
           VBase4((152.0/255.0), (172.0/255.0), (138.0/255.0), 1.0),
           VBase4((202.0/255.0), (120.0/255.0), (120.0/255.0), 1.0),
           VBase4((220.0/255.0), (123.0/255.0), (59.0/255.0), 1.0),
           VBase4((224.0/255.0), (109.0/255.0), (109.0/255.0), 1.0),
           VBase4((232.0/255.0), (87.0/255.0), (87.0/255.0), 1.0),
           VBase4((255.0/255.0), (151.0/255.0), (151.0/255.0), 1.0)
           ])

        self.createColorMenu('donaldsDockCorniceColors', [
           VBase4((146.0/255.0), (98.0/255.0), (86.0/255.0), 1.0),
           VBase4((151.0/255.0), (255.0/255.0), (234.0/255.0), 1.0),
           VBase4((192.0/255.0), (114.0/255.0), (114.0/255.0), 1.0),
           VBase4((194.0/255.0), (145.0/255.0), (73.0/255.0), 1.0),
           VBase4((240.0/255.0), (182.0/255.0), (168.0/255.0), 1.0),
           VBase4((193.0/255.0), (187.0/255.0), (163.0/255.0), 1.0),
           VBase4((208.0/255.0), (232.0/255.0), (113.0/255.0), 1.0),
           VBase4((230.0/255.0), (144.0/255.0), (86.0/255.0), 1.0),
           VBase4((232.0/255.0), (137.0/255.0), (112.0/255.0), 1.0),
           VBase4((232.0/255.0), (160.0/255.0), (113.0/255.0), 1.0),
           VBase4((240.0/255.0), (90.0/255.0), (90.0/255.0), 1.0),
           VBase4((254.0/255.0), (176.0/255.0), (124.0/255.0), 1.0),
           VBase4((255.0/255.0), (106.0/255.0), (69.0/255.0), 1.0),
           VBase4((255.0/255.0), (180.0/255.0), (69.0/255.0), 1.0),
           VBase4((255.0/255.0), (213.0/255.0), (151.0/255.0), 1.0),
           VBase4((255.0/255.0), (225.0/255.0), (205.0/255.0), 1.0),
           VBase4((255.0/255.0), (234.0/255.0), (151.0/255.0), 1.0),
           VBase4((255.0/255.0), (255.0/255.0), (151.0/255.0), 1.0),
           VBase4((55.0/255.0), (244.0/255.0), (70.0/255.0), 1.0),
           VBase4((27.0/255.0), (203.0/255.0), (56.0/255.0), 1.0),
           VBase4((53.0/255.0), (185.0/255.0), (78.0/255.0), 1.0)
           ])
        # Use the toontown color set
        self.useToontownCentralColors()

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
                         (self.direct.chan.width/self.direct.chan.height) *
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
                           self.direct.chan.height) *
                          math.sin(i * angle)) - 0.025))
            path.setScale(0.05)

	# Scale the whole shebang down by 0.5
	newDoorMenu.setScale(0.5)

	return newDoorMenu

    def createNumWindowsMenu(self):
        
	numberNodes = []
        for i in range(4):
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
                        (self.direct.chan.width/self.direct.chan.height) *
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
                        (self.direct.chan.width/self.direct.chan.height) *
                        math.sin(i * angle)))

            # Add it to the propTypeMenu
            node.reparentTo(newPropTypeMenu)

	# Scale the whole shebang down by 0.5
	newPropTypeMenu.setScale(0.5)

	return newPropTypeMenu

    def createStyleMenu(self):
	numberNodes = []
        for i in range(13):
            node = OnscreenText(`i`,0,0)
            numberNodes.append(node)
	numItems = len(numberNodes)

	newStyleMenu = hidden.attachNewNode(NamedNode('styleMenu'))

	radius = 0.7
	angle = deg2Rad(360.0/numItems)
        for i in range(numItems):
            # Get the node
            node = numberNodes[i]
            node.setScale(node.getScale()* 4.0)
                          
            # Reposition it
            node.setXY(radius * math.cos(i * angle),
                       (radius *
                        (self.direct.chan.width/self.direct.chan.height) *
                        math.sin(i * angle)))

            # Add it to the styleMenu
            node.reparentTo(newStyleMenu)

	# Scale the whole shebang down by 0.5
	newStyleMenu.setScale(0.5)

	return newStyleMenu

    def createStyleMenuWith(self, dictionary):
	numberNodes = []
	numItems = len(dictionary)
        for i in range(numItems):
            node = OnscreenText(`i`,0,0)
            numberNodes.append(node)

	newStyleMenu = hidden.attachNewNode(NamedNode('styleMenu'))

	radius = 0.7
	angle = deg2Rad(360.0/numItems)
        for i in range(numItems):
            # Get the node
            node = numberNodes[i]
            node.setScale(node.getScale()* 3.0)
	
            # Reposition it
            node.setXY(radius * math.cos(i * angle),
                       (radius *
                        (self.direct.chan.width/self.direct.chan.height) *
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
                         (self.direct.chan.width/self.direct.chan.height) *
                         math.sin(i * angle) - 0.15))
            path.setScale(0.3)

	# Scale the whole shebang down by 0.5
	newWallMenu.setScale(0.5)
	return newWallMenu

    def createWallWidthMenu(self):
	numberNodes = []
        for i in range(5):
            node = OnscreenText(`(i * 5)`,0,0)
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
                        (self.direct.chan.width/self.direct.chan.height) *
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
                         (self.direct.chan.width/self.direct.chan.height) *
                         math.sin(i * angle) - 0.05))
            path.setScale(0.1)

	# Scale the whole shebang down by 0.5
	newWindowMenu.setScale(0.5)

	return newWindowMenu

    def initializeDonaldsDockStyleDictionary(self):
	dictionary = {}
	styleCount = 0

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall_bricks_ur',
                      Vec4(0.627, 0.470,0.235,1.0),
                      'window_sm_shuttered_ur',
                      Vec4(1.0, 0.501, 0.376, 1.0),
                      'cornice_stone_ur',
                      Vec4(0.760, 0.568, 0.286, 1.0))

        styleCount = styleCount + 1
        self.addStyle(dictionary, styleCount,
                      'wall_md_board_ur',
                      Vec4(0.713,0.494,0.345,1.0 ),
                      'window_porthole_ur',
                      Vec4(1.0, 0.501, 0.376, 1.0),
                      'cornice_shingles_ur',
                      Vec4(0.572, 0.384, 0.337, 1.0))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall_md_board_ur',
                      Vec4(0.925,0.152,0.152,1.0 ),
                      'window_sm_round_ur',
                      Vec4(0.643, 0.933, 0.454, 1.0),
                      None,
                      None)

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall_md_board_ur',
                      Vec4(0.925, 0.152, 0.152, 1.0 ),
                      'window_porthole_ur',
                      Vec4(0.0, 0.611, 0.364, 1.0),
                      'cornice_shingles_ur',
                      Vec4(0.760, 0.568, 0.286, 1.0))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall_md_board_ur',
                      Vec4(0.713,0.494,0.345,1.0),
                      'window_sm_square_ur',
                      Vec4(0.823, 0.823, 0.400, 1.0 ),
                      None,
                      None)

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall_lg_rock_ur',
                      Vec4(0.752, 0.450, 0.447, 1.0),
                      'window_sm_round_ur',
                      Vec4(0.823, 0.823, 0.400, 1.0),
                      'cornice_brick_ur',
                      Vec4(0.760, 0.568, 0.286, 1.0))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall_md_red_ur',
                      Vec4(0.415, 0.933, 0.454, 1.0 ),
                      'cornice_marble_ur',
                      Vec4(0.643, 0.933, 0.454, 1.0),
                      'cornice_marble_ur',
                      Vec4(0.643, 0.933, 0.454, 1.0))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall_sm_wood_ur',
                      Vec4(0.713, 0.494, 0.345, 1.0 ),
                      'window_sm_shuttered_ur',
                      Vec4(0.878,  0.427,  0.427,  1.0 ),
                      'cornice_shingles_ur',
                      Vec4(0.760,  0.568,  0.286,  1.0 ))

        styleCount = styleCount + 1
        self.addStyle(dictionary, styleCount,
                      'wall_bricks_ur',
                      Vec4(0.168, 0.454, 0.227, 1.0 ),
                      'window_sm_round_ur',
                      Vec4(0.643,  0.933,  0.454,  1.0),
                      'cornice_horizontal_ur',
                      Vec4(0.752,  0.447,  0.447,  1.0 ))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall_sm_wood_ur',
                      Vec4(0.713, 0.494, 0.345, 1.0 ),
                      'window_porthole_ur',
                      Vec4(1.0, 0.501,  0.376,  1.0 ),
                      'cornice_shingles_ur',
                      Vec4(0.75, 0.75, 0.75, 1.0 ))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall_md_red_ur',
                      Vec4(0.415, 0.247, 0.247, 1.0 ),
                      'window_sm_round_ur',
                      Vec4(0.643,  0.933,  0.454,  1.0 ),
                      'cornice_dental_ur',
                      Vec4(0.572,  0.384,  0.337,  1.0 ))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount ,
                      'wall_md_board_ur',
                      Vec4(0.925, 0.152, 0.152, 1.0 ),
                      'window_porthole_ur',
                      Vec4(0.0, 0.611,  0.364,  1.0 ),
                      'cornice_shingles_ur',
                      Vec4(0.941,  0.713,  0.658,  1.0 ))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall_lg_brick_dr',
                      Vec4(0.168, 0.454, 0.227, 1.0 ),
                      'window_md_curtains_ur',
                      Vec4(0.75, 0.75, 0.75, 1.0 ),
                      'cornice_dental_ur',
                      Vec4(0.5, 0.5, 0.5, 1.0 ))

	# Store this dictionary in the self.attributeDictionary
	self.attributeDictionary['donaldsDockStyleDictionary'] = dictionary

    def initializeLandmarkButtons(self):
	# Initialize Hooks and Buttons for the wall type buttons
	landmarkTypes = self.getCatalogCodes('toon_landmark')
        methodArray = []
        for landmark in landmarkTypes:
            methodArray.append(
                [landmark[15:len(landmark)], landmark])

	hooksSet = self.hooksDictionary.get('toon_landmark',[])
        for pair in methodArray:
            hooksSet.append(pair[1])
	self.hooksDictionary['toon_landmark'] = hooksSet

	# Create wall module buttons
	# First get rid of any existing buttons
	buttons = self.clickBoxDictionary.get('toon_landmark', None)
        if buttons:
            buttons.disable()
	buttons = ClickBoxList(methodArray, 0.95, 0.90)
	buttons.addButtonWithText('back',self.mainMenuEnable)
	buttons.alignRight()
	buttons.setScale(0.06)
	buttons.setColor(0.6, 0.6, 0.6, 0.8)
	self.clickBoxDictionary['landmarkSym'] = buttons

	self.categorySet.append(landmarkSym)

    def initializeLevelEditorButtons(self):
	newClickBoxObject = ToggleBoxList(
            [('Show Grid', self.showGrid, 0)
             ('XYZ Snap', self.xyzSnap, 1),
             ('HPR Snap', self.hprSnap, 1)],
            -0.95, 0.90)
	newClickBoxObject.alignLeft()
	newClickBoxObject.setScale(0.06)
	newClickBoxObject.makeAllWideAsWidest()
	newClickBoxObject.enable()
	self.clickBoxDictionary['gridMenuButtons'] = newClickBoxObject

	newClickBoxObject = ClickBoxList(
            [('Street modules', self.activateStreetModuleButtons),
             ('Toon Walls', self.activateWallModuleButtons),
             ('Landmark bldgs', self.activateLandmarkButtons),
             ('Props', self.activatePropButtons)],
            0.95, 0.90)
	newClickBoxObject.setColor(0.5, 0.5, 0.5, 0.5)
	newClickBoxObject.setScale(0.06)
	newClickBoxObject.alignRight()
	self.clickBoxDictionary['mainMenuButtons'] = newClickBoxObject

	newClickBoxObject = ClickBox(
            'New Group', 0.3, 0.3, -0.95, -0.9,
            self.createNewLevelGroup, 0)
	newClickBoxObject.nodePath().node().setCardColor(Point4(1,1,1,.5))
	newClickBoxObject.setScale(0.05)
	self.clickBoxDictionary['groupButton'] = newClickBoxObject

	newClickBoxObject = ClickBox(
            'Save DNA', 0.3, 0.3, -0.7, -0.9,
            self.outputDNA, ['toontown.dna'],0)
	newClickBoxObject.nodePath().node().setCardColor(Point4(1,1,1,.5))
	newClickBoxObject.setScale(0.05)
	self.clickBoxDictionary['saveButton'] = newClickBoxObject

	newClickBoxObject = ToggleBox(
            'Level Map', 0.3, 0.3, -0.47, -0.9,
            self.toggleMapViz, [], 0)
	newClickBoxObject.nodePath().node().setCardColor(Point4(1,1,1,.5))
	newClickBoxObject.setButtonState(0)
	newClickBoxObject.setScale(0.05)
	self.clickBoxDictionary['mapButton'] = newClickBoxObject

	self.dnaMenuEnable()

	# Initialize module Dictionary with pointers to module
                                   # node paths and create module buttons
	self.initializeStreetButtons()
	self.initializeWallButtons()
	self.initializeLandmarkButtons()
	self.initializePropButtons()

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
            PieMenu(self.direct, self.createWallMenu(), self.updateWallTextureNum))

	self.pieMenuDictionary['windowMenu'] = (
            PieMenu(self.direct, self.createWindowMenu(), self.updateWindowTextureNum))

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
	self.pieMenuDictionary['toontownCentralStyleMenu'] = (
            PieMenu(self.direct,self.createStyleMenuWith(
            self.attributeDictionary['toontownCentralStyleDictionary']),
                    self.updateWallStyleNum))
	self.pieMenuDictionary['donaldsDockStyleMenu'] = (
            PieMenu(self.direct,self.createStyleMenuWith(
            self.attributeDictionary['donaldsDockStyleDictionary']),
                    self.updateWallStyleNum))
	self.pieMenuDictionary['styleMenu'] = (
            self.pieMenuDictionary['toontownCentralStyleMenu'])
	# Create several differnt color palette menus
	self.createColorMenus()

    def initializeStyleDictionary(self):
	self.initializeToontownCentralStyleDictionary()
	self.initializeDonaldsDockStyleDictionary()
	self.styleDictionary = (
            self.attributeDictionary['toontownCentralStyleDictionary'])

    def initializeToontownCentralStyleDictionary(self):

	dictionary = {}
	styleCount = 0

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=md=pillars_ur',
                      Vec4(1.0, 0.917, 0.592, 1.0 ),
                      'window=sm=pointed_ur',
                      Vec4(0.396,  0.611,  0.666,  1.0 ),
                      'cornice=stone_ur',
                      Vec4(1.0, 1.0, 0.592,  1.0 ))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=lg=brick_ur',
                      Vec4(1.0, 0.705, 0.270, 1.0 ),
                      'window=sm=round_ur',
                      Vec4(0.588,  0.996,  0.486,  1.0 ),
                      None,
                      None)

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=lg=brick_ur',
                      Vec4(1.0, 0.415, 0.270, 1.0 ),
                      'window=porthole_ur',
                      Vec4(0.129, 0.784, 0.521, 1.0 ),
                      None,
                      None)

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=sm=cement_ur',
                      Vec4(1.0, 0.882, 0.803, 1.0 ),
                      'window=sm=round_ur',
                      Vec4(0.270, 1.0, 0.415, 1.0 ),
                      None,
                      None)

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=md=dental_ur',
                      Vec4(1.0, 0.917, 0.592, 1.0 ),
                      'window=md=curved_ur',
                      Vec4(1.0, 0.627, 0.376, 1.0 ),
                      None,
                      None)

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=lg=brick_ur',
                      Vec4(0.901, 0.564, 0.337, 1.0 ),
                      'window=sm=round_ur',
                      Vec4(0.588, 0.996, 0.486, 1.0 ),
                      None,
                      None)

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=md=pillars_ur',
                      Vec4(0.996, 0.690, 0.486, 1.0 ),
                      'window=porthole_ur',
                      Vec4(0.909, 0.627, 0.443, 1.0 ),
                      None,
                      None)

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=sm=brick_ur',
                      Vec4(1.0, 0.835, 0.592, 1.0 ),
                      'window=sm=pointed_ur',
                      Vec4(1.0, 0.627, 0.376, 1.0 ),
                      None,
                      None)

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=md=yellow_ur',
                      Vec4(0.901, 0.564, 0.337, 1.0 ),
                      'window=sm=pointed_ur',
                      Vec4(0.321, 0.666, 0.596, 1.0 ),
                      'cornice=stone_ur',
                      Vec4(0.941, 0.713, 0.658, 1.0 ))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=md=pillars_ur',
                      Vec4(0.815, 0.909, 0.443, 1.0 ),
                      'window=sm=round_ur',
                      Vec4(0.588,  0.996,  0.486,  1.0 ),
                      'cornice=stone_ur',
                      Vec4(1.0, 1.0, 0.592,  1.0))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=md=dental_ur',
                      Vec4(0.909, 0.627, 0.443, 1.0 ),
                      'window=sm=round_ur',
                      Vec4(0.588,  0.996,  0.486,  1.0 ),
                      None,
                      None)



        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=md=pillars_ur',
                      Vec4(1.0, 0.882, 0.803, 1.0 ),
                      'window=sm=curved_ur',
                      Vec4(1.0, 0.917, 0.592, 1.0 ),
                      None,
                      None)

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=md=pillars_ur',
                      Vec4(1.0, 0.882, 0.803, 1.0 ),
                      'window=porthole_ur',
                      Vec4(1.0, 0.917, 0.592, 1.0 ),
                      None,
                      None)

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=md=pillars_ur',
                      Vec4(1.0, 0.882, 0.803, 1.0 ),
                      'window=sm=round_ur',
                      Vec4(0.588, 0.996, 0.486, 1.0 ),
                      'cornice=stone_ur',
                      Vec4(1.0, 1.0, 1.0, 1.0 ))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=bricks_ur',
                      Vec4(1.0,0.835, 0.592, 1.0 ),
                      'window=sm=shuttered_ur',
                      Vec4(1.0, 1.0, 1.0, 1.0 ),
                      'cornice=marble_ur',
                      Vec4(1.0, 1.0, 0.592,  1.0 ))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=md=yellow_ur',
                      Vec4(1.0, 0.835, 0.592, 1.0 ),
                      'window=sm=shuttered_ur',
                      Vec4(1.0, 0.917,  0.592,  1.0 ),
                      'cornice=dental_ur',
                      Vec4(1.0, 1.0, 0.592,  1.0 ))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=md=yellow_ur',
                      Vec4(1.0, 0.835, 0.592, 1.0 ),
                      'window=sm=round_ur',
                      Vec4(1.0, 0.627,  0.376,  1.0 ),
                      'cornice=dental_ur',
                      Vec4(1.0, 0.749,  0.376,  1.0 ))

        styleCount = styleCount + 1
	self.addStyle(dictionary, styleCount,
                      'wall=md=yellow_ur',
                      Vec4(1.0,0.917, 0.592, 1.0 ),
                      'window=sm=pointed_ur',
                      Vec4(1.0, 1.0, 1.0, 1.0 ),
                      'cornice=marble_ur',
                      Vec4(0.443,  0.909,  0.627,  1.0 ))

        styleCount = styleCount + 1
	self.addStyle(self.styleDictionary, styleCount,
                      'wall=md=yellow_ur',
                      Vec4(1.0, 0.917, 0.592, 1.0 ),
                      'window=sm=curved_ur',
                      Vec4(1.0, 0.627,  0.376,  1.0 ),
                      None,
                      None)

	self.attributeDictionary['toontownCentralStyleDictionary'] = dictionary

    #XXXX

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
	newDNAFlatBuilding = DNAFlatBuilding(buildingType)

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
	newDNALandmarkBuilding = DNALandmarkBuilding(landmarkType)
	newDNALandmarkBuilding.setCode(self.getDNACode(landmarkType))
	newDNALandmarkBuilding.setPos(VBase3(0))
	newDNALandmarkBuilding.setHpr(VBase3(self.lastAngle,0.0,0.0))
	newDNADoor = self.createDoor(self.getDoorTexture())
	newDNALandmarkBuilding.add(newDNADoor)
	# Now place new building in the world
	self.addDNAGroup(newDNALandmarkBuilding)

    def addObject(self, aNodePath, dnaGroup):
	# Add specified node path to the scene dictionary
	objectDictionary = {}
	objectDictionary['nodePath'] = aNodePath 
	objectDictionary['DNA'] = dnaGroup
	self.levelDictionary[aNodePath.node().this] = objectDictionary
        return objectDictionary

    def addProp(self, newPropType):
	newDNAProp = DNAProp(newPropType)
	newDNAProp.setCode(self.getDNACode(newPropType))
	newDNAProp.setPos(VBase3(0))
	newDNAProp.setHpr(VBase3(self.lastAngle,0.0,0.0))
	# Now place new building in the world
	self.addDNAGroup(newDNAProp)
	self.setPropType(newPropType)

    def addStreetModule(self, streetType):
	newDNAStreet = DNAStreet(streetType)
	newDNAStreet.setCode(self.getDNACode(streetType))
	newDNAStreet.setPos(VBase3(0))
	newDNAStreet.setHpr(VBase3(self.lastAngle, 0.0, 0.0))
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
	newDNAWindows.setCount(self.getRandomNumWindows(height))
        # MRM Need to randomize
	newDNAWindows.setColor(self.getWallColors())
	newDNAWall.add(newDNAWindows)
	return newDNAWall

    def createCornice(self,dnaString):
	newDNACornice = DNACornice()
	newDNACornice.setCode(self.getDNACode(dnaString))
        # MRM Need to randomize
	newDNACornice.setColor(self.getCorniceColors())
	return newDNACornice

    def createDoor(self, dnaString):
	newDNADoor = DNADoor()
	newDNADoor.setCode(self.getDNACode(dnaString))
        # MRM Need to randomize
        colors = self.getDoorColors()
	newDNADoor.setColor(colors[randint(0,len(colors) - 1)])
	return newDNADoor

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
	newDNAWindows.setCount(1)
	newDNAWindows.setColor(self.getWindowColor())
	newDNAWall.add(newDNAWindows)
	
	return newDNAWall

    def createWindows(self, numWindows):
	newDNAWindows = DNAWindows()
	newDNAWindows.setCode(self.getDNACode(self.getWindowTexture()))
	newDNAWindows.setCount(numWindows)
	newDNAWindows.setColor(self.getWindowColor())
	return newDNAWindows

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
                if child.getClassType().eq(DNACornice.getClassType()):
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
            # MRM CLASS INFO?
            if child.getClassType().eq(DNADoor.getClassType()):
                return child
        # Not found
	return None

    def getLastWall(self, aDNAFlatBuilding):
	lastWall = None
        for i in range(aDNAFlatBuilding.getNumChildren()):
            child = aDNAFlatBuilding.at(i)
            if child.getClassType().eq(DNAWall.getClassType()):
                lastWall = child
        return lastWall

    def getLevelObject(self, aNodePath):
	# Given a node path, find the corresponding level object
        # in the levelDictionary, if none exists, return 0
	return self.levelDictionary.get(aNodePath.node().this, None)

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
	h = rounded(height)
        if h == 10:
            # Only return 0 25% of the time
            if rounded(self.getWallWidth()) == 5:
                return randint(1,3)
            else:
                return randint(0,3)
        elif h == 20:
            if rounded(self.getWallWidth()) == 5:
                return randint(1,3)
            else:
                return randint(0,4)
        elif h == 30:
            if rounded(self.getWallWidth()) == 5:
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
            if child.getClassType().eq(DNAWall.getClassType()):
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
            if child.getClassType().eq(DNAWall.getClassType()):
                heightTotal = heightTotal + child.getHeight()
                heightList.add(heightTotal)
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
            if (child.getClassType().eq(DNAWindow.getClassType()) |
                child.getClassType().eq(DNAWindows.getClassType())):
                if windowCount == windowNum:
                    return child
                windowCount = windowCount + 1
	# Not found
	return None

    def initDNAGroupWithParent(self, dnaGroup, parent):
	# Create the geometry
	# If it is a flat building, update building DNA to current wall width
        if (dnaGroup.getClassType().eq(DNAFlatBuilding.getClassType())):
            dnaGroup.setWidth(self.getWallWidth())
	newNodePath = dnaGroup.traverse(parent,self.dnaStore)
	# Add it to the level dictionary
	self.addObject(newNodePath, dnaGroup)
	# Add it to the top level DNA Group
	self.groupParentDNA.add(dnaGroup)

	# Place the new node path at the current grid origin
	newNodePath.setPos(self.grid,0,0,0)
	# Initialize angle to match last object
	newNodePath.setH(self.lastAngle)

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
        if dnaGroup.getClassType().eq(DNAFlatBuilding.getClassType()):
            dnaGroup.setWidth(self.getWallWidth())
	newNodePath = dnaGroup.traverse(parent, self.dnaStore)
	# Add it to the level dictionary
	self.addObject(newNodePath, dnaGroup)
	# Add it to the top level DNA Group
	self.groupParentDNA.add(dnaGroup)

	# Place the new node path at the current grid origin
	newNodePath.setPos(self.grid,0,0,0)
	# Initialize angle to match last object
	newNodePath.setH(self.lastAngle)

	# Select the instance
	self.selectNodePath(newNodePath)

	# Now move the grid to get ready for the next group
	self.autoPositionGrid()

	# Update dictionary
	dnaGroup.setPos(newNodePath.getPos())
	dnaGroup.setHpr(newNodePath.getHpr())

    def initNewDNAGroupWithParent(self, dnaGroup, rootNode):
	# Reflect currently selected prop type
        if dnaGroup.getClassType().eq(DNAProp.getClassType()):
            self.updatePropType(dnaGroup,self.getPropType())

        # Create a new copy of dnaGroup's class
        # Extract group's class using __class__
        # Call that class's constructor passing in dnaGroup to make a copy
        self.initDNAGroupWithParent(dnaGroup.__class__(dnaGroup), self.groupParent)
        # Initialize
        if dnaGroup.getClassType().eq(DNAProp.getClassType()):
            objectType = self.getDNAString(dnaGroup)
            if objectType != 'prop_sphere':
                # Update props placement to reflect current mouse position
                # Where is the mouse relative to the grid?
                self.grid.getMouseIntersectionPoint(self.hitPt, 0)
                self.direct.selected.last.setPos(self.grid, self.hitPt)
                dnaGroup.setPos(self.direct.selected.last.getPos())

    def initNewDNAGroupWithParentType(self, dnaGroup, rootNode, type):
        # Create a new dna Group of the same type a dnaGroup
	newDNAGroup = dnaGroup.__class__(dnaGroup)
        if dnaGroup.getClassType().eq(DNAProp.getClassType()):
            self.updatePropType(newDNAGroup,self.getPropType())

        self.initDNAGroupWithParentType(newDNAGroup, self.groupParent, type)

    def loadDNAFile(self, filename):
	# Out with the old, in with the new
	self.resetLevel()
	# Get rid of default group and root node
	self.preRemoveNodePath(self.groupParent)
	self.removeNodePath(self.groupParent)

	# Clear self.dnaStore
	self.dnaStore.resetDNAGroups()

	# Now load in new file
	self.groupParent = self.dnaStore.loadDNAFile(filename)

 	# Make sure the topmost level object gets put under level objects dna
 	self.groupParentDNA = self.dnaStore.findDNAGroup(self.groupParent.getBottomArc())
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
                # Before reparenting children, try to set groupNum to something reasonable
                self.groupNum = newLevelObjects.getNumChildren()
                # Go ahead and get rid of the default parent (since there is probably one in the dnafile
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
                newNodePath = NodePath(relation)
                group = self.dnaStore.findDNAGroup(relation)
                if newNodePath.isSingleton():
                    print 'Singleton!!'
                else: 
                    self.addObject(newNodePath, group)
            else:
                print'blah'

	self.createNewLevelGroup()

    def outputDNA(self,filename):
	print 'Saving DNA to: ', filename
	self.levelObjectsDNA.writeDna(Filename(filename),Notify.out(),self.dnaStore)

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
        while self.removeDNAObjectOfClass(DNACornice,self.getLastWall(aDNAGroup)):
            pass

    def removeDNAObjectFrom(self, aDNAGroup, objectClass):
	# Remove the first object of that type you come across
        for i in range(aDNAGroup.getNumChildren()):
            child = aDNAGroup.at(i)
            if child.getClassType().eq(objectClass):
                aDNAGroup.remove(child)
                return 1
	# None found
	return 0

    def removeDNAObjectOfClass(self, objectClass, aDNAGroup):
	# Remove the first object of that type you come across
        for i in range(aDNAGroup.getNumChildren()):
            child = aDNAGroup.at(i)
            if child.getClassType().eq(objectClass):
                aDNAGroup.remove(child)
                return 1
	# None found
	return 0

    def removeLevelObject(self, aNodePath):
	# Remove specified node path from the scene dictionary
	nodePathHandle = aNodePath.this
	del(self.levelDictionary[nodePathHandle])
	# Get rid of its visible representation
	aNodePath.removeNode()

    def removeNodePath(self, aNodePath):
	# Remove specified node path from the scene dictionary
	nodePathHandle = aNodePath.this
	del(self.levelDictionary[nodePathHandle])
	# Get rid of its visible representation
	aNodePath.removeNode()

    def removeWindows(self, aDNAGroup):
        while self.removeDNAObjectOfClass(DNAWindow, aDNAGroup):
            pass
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
            child = aDNAFlatBuilding.at(i)
            if child.getClassType().eq(DNAWall):
                self.setWallStyle(child, style)
                if randint(0,100) < 40:
                    style = self.getRandomStyle()

	# Randomly add cornice
        if randint(0,100) < 40:
            self.removeCornices(aDNAFlatBuilding)
        else:
            if not style['corniceTexture']:
                self.removeCornices(aDNAFlatBuilding)
            else:
                aDNACornice = self.getCornice(aDNAFlatBuilding)
                if not aDNACornice:
                    aDNACornice = DNACornice()
                    aDNACornice.setCode(self.dnaStore.findCode(style['corniceTexture']))
                    aDNACornice.setColor(style['corniceColor'])
                    lastWall = self.getLastWall(aDNAFlatBuilding)
                    lastWall.add(aDNACornice)

    def setRandomNumWindows(self, aDNAWall, numWindows):
	window = self.getWindow(aDNAWall, 0)
	window.setCount(numWindows)

    def setWallStyle(self, aDNAWall, style):
	aDNAWall.setCode(self.dnaStore,style['wallTexture'])
	aDNAWall.setColor(style['wallColor'])
	aDNAWindows = self.getWindow(aDNAWall, 0)
	aDNAWindows.setCount(self.getRandomNumWindows(aDNAWall.getHeight()))
	aDNAWindows.setCode(self.dnaStore.findCode(style['windowTexture']))
	aDNAWindows.setColor(style['windowColor'])

    def setWallStyle(self, aDNAWall, styleNum):
	self.setWallStyle(aDNAWall, self.styleDictionary[styleNum])

    def updateColorIndex(self, colorIndex):
        if colorIndex < 0:
            self.updateObjColor(self.targetDNAObject,self.activeMenu.getInitialState())
        else:
            self.updateObjColor(self.targetDNAObject,self.activeColors[colorIndex])

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
	self.corniceTexture(dnaString)

    def updateDNAPosHpr(self, aNodePath):
        if aNodePath:
            dnaGroup = self.getDNAGroup(aNodePath)
            if dnaGroup:
                if not(dnaGroup.getClassType().eq(DNAGroup.getClassType())):
                    dnaGroup.setPos(aNodePath.getPos())
                    dnaGroup.setHpr(aNodePath.getHpr())

    def updateDoorTextureNum(self, doorTextureNumber):
	self.updateObjDoorTexture(self.targetDNAObject, doorTextureNumber)

    def updateObjDoorTexture(self, aDNADoor, doorTextureNumber):
	# Which door texture was picked by the user?
        if doorTextureNumber < 0:
            dnaString = self.activeMenu.getInitialState()
        else:
            dnaString = self.doorTextures()[doorTextureNumber]

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
	if ((self.targetDNAObject.getClassType().eq(DNADoor.getClassType())) |
            (self.targetDNAObject.getClassType().eq(DNACornice.getClassType()))):
            if (orientationNumber == 2):
                remappedOrientationNumber = 0
            elif (orientationNumber == 3):
                remappedOrientationNumber = 1

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
            dnaString = newString + '_dr'
        elif orientationNumber == 3:
            dnaString = newString + '_dl'
        else:
            self.activeMenu.getInitialState()

        if newString != currString:
            objClass = aDNAObject.getClassType()
            if objClass == DNAWall.getClassType():
                self.updateWallTextureDNA(aDNAObject, dnaString)
            elif objClass == DNACornice.getClassType():
                self.updateCorniceTextureDNA(aDNAObject, dnaString)
            elif objClass == DNADoor.getClassType():
                self.updateDoorTextureDNA(aDNAObject, dnaString)
            elif objClass == DNAWindow.getClassType():
                self.updateWindowTextureDNA(aDNAObject, dnaString)
            elif objClass == DNAWindows.getClassType():
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
	# Replace object in levelObjects dictionary and scene graph
	self.replaceLevelObjectNodePath(self.selectedLevelObject)
	self.setPropType(dnaString)

    def updateRandomNumWindows(self, aDNAFlatBuilding):
        for i in range(aDNAFlatBuilding.getNumChildren()):
            child = aDNAFlatBuilding.at(i)
            if child.getClassType().eq(DNAWall.getClassType()):
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
            self.activeMenu.getInitialState()

        if newString != currString:
            self.updateWallTextureDNA(aDNAWall, dnaString)

    def updateWallStyleNum(self, styleNum):
        if styleNum < 0:
            self.setWallStyle(self.targetDNAObject, 1)
        else:
            self.setWallStyle(self.targetDNAObject, (1 + styleNum))
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
                                 (scaleFactor + 1) * 5.0)

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
            groupClass = dnaGroup.getClassType().getName()
            deltaPos = VBase3(20,0,0)
            deltaHpr = VBase3(0)
            if groupClass == 'DNAFlatBuilding':
                deltaPos.setX(self.getWallWidth())
            elif groupClass == 'DNAStreet':
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
                    deltaPos.setX(30.0)
                elif objectType == 'street_angle_45':
                    deltaPos.setX(30.0)
                elif objectType == 'street_angle_60':
                    deltaPos.setX(30.0)
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
                    eltaPos.setX(40.0)
            elif groupClass == 'DNALandmarkBuilding':
                objectType = self.getDNAString(dnaGroup)
                if objectType == 'toon_landmark_building_01':
                    deltaPos.setX(25.0)
                elif objectType == 'toon_landmark_building_02':
                    deltaPos.setX(15.0)
                elif objectType == 'toon_landmark_building_03':
                    deltaPos.setX(20.0)
            elif groupClass == 'DNAProp':
                objectType = self.getDNAString(dnaGroup)
                if objectType == 'prop_sphere':
                    deltaPos.setX(40.0)

            # Position grid for placing next object
            # Eventually we need to setHpr too
            self.grid.setPos(selectedNode, deltaPos)
            if self.grid.getXyzSnap():
                # Tighten up grid position
                pos = self.grid.getPos()
                roundVal = self.roundTo(self.grid.getGridSpacing(), 1)
                x = self.roundTo(pos[0], roundVal)
                y = self.roundTo(pos[1], roundVal)
                z = self.roundTo(pos[2], roundVal)
                self.grid.setPos(x,y,z)

	# Also move the camera
	taskMgr.removeTasksNamed('autoMoveDelay')
	handlesToCam = self.direct.widget.getPos(self.direct.camera)
	handlesToCam = handlesToCam * ( self.direct.chan.near/handlesToCam[1])
	if ((abs(handlesToCam[0]) > (self.direct.chan.nearWidth * 0.4)) |
            (abs(handlesToCam[2]) > (self.direct.chan.nearHeight * 0.4))):
            taskMgr.removeTasksNamed('manipulateCamera')
            self.direct.cameraControl.centerCamIn(self.direct.chan, 0.5)

    def plantSelectedNodePath(self):
	# Move grid to prepare for placement of next object
	selectedNode = self.direct.selected.last
        if selectedNode:
            # Where is the mouse relative to the grid?
            # MRM NEEDED
            self.getMouseIntersectionPoint(self.hitPt, 0)
            selectedNode.setPos(self.grid, self.hitPt)
            dnaGroup = self.getDNAGroup(selectedNode)
            if dnaGroup:
                # Update props placement to reflect current mouse position
                dnaGroup.setPos(direct.selected.last.getPos())

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

    def selectNodePath(self, aNodePath):
	# Method to handle the select event.
        # MRM: THis or message?
	self.direct.select(aNodePath)
	# Update selectedLevelObject
	self.selectedLevelObject = self.getLevelObject(aNodePath)

    def getWallIntersectionPoint(self, intersectionPoint):
	# Find point of intersection between grid plane and line from cam through mouse
	# Don't do anything if nothing selected
	selectedNode = self.direct.selected.last
        if not selectedNode:
            return 0

        chan = self.direct.chan
    	mouseX = chan.mouseX
  	mouseY = chan.mouseY
   	nearX = math.tan(deg2Rad(chan.fovH)/2.0) * mouseX * chan.near
   	nearZ = math.tan(deg2Rad(chan.fovV)/2.0) * mouseY * chan.near

   	mCam2Grid = chan.camera.getMat(selectedNode)
	mouseOrigin = Point3(0)
   	mouseOrigin.assign(mCam2Grid.getRow3(3))
	mouseDir = Vec3(0)
   	mouseDir.set(nearX, chan.near, nearZ)
   	mouseDir.assign(mCam2Grid.xformVec(mouseDir))
   
   	return self.wallIntersectionPlane.intersectsLine(intersectionPoint, mouseOrigin, mouseDir)

"""

    def initializePropButtons(self):
	# Initialize Hooks and Buttons for the wall type buttons
	newPtopTypes = self.getCatalogCodes('prop')
	methodArray = []
        for
		newPtopTypes collect: [ :prop | 
			{ prop copyFrom: 5 to: prop size . prop asSymbol } ].

	hooksSet = self.hooksDictionary['prop ifAbsent: [ Set new. ].
	methodArray do: [ :pair | hooksSet add: (pair at: 2) ].
	self.hooksDictionary['prop'] = hooksSet.

	# Create wall module buttons
	# First get rid of any existing buttons
	self.clickBoxDictionary['prop ifPresent: [ :clickBoxList | clickBoxList disable ].
	buttons = ClickBoxList new table: methodArray x: 0.95 y: 0.90.
	buttons addButtonWithText: 'back' event: #mainMenuEnable.	
	buttons alignRight.
	buttons.setScale(0.06.
	buttons.setColor(0.6 g: 0.6 b: 0.6 a: 0.8.
	self.clickBoxDictionary['prop'] = buttons.

	self.categorySet add: #prop.
! !

!Level methodsFor: 'initialization' stamp: 'panda 00/00/0000 00:00'!
initializeStreetButtons
	| streetTypes methodArray hooksSet buttons |
	# Initialize Hooks and Buttons for the wall type buttons
	streetTypes = self.getCatalogCodes(#street.	
	methodArray = 
		streetTypes collect: [ :street | { street copyFrom: 8 to: street size . street asSymbol } ].

	hooksSet = self.hooksDictionary['street ifAbsent: [ Set new. ].
	methodArray do: [ :pair | hooksSet add: (pair at: 2) ].
	self.hooksDictionary['street'] = hooksSet.

	# Create wall module buttons
	# First get rid of any existing buttons
	self.clickBoxDictionary['street ifPresent: [ :clickBoxList | clickBoxList disable ].
	buttons = ClickBoxList new table: methodArray x: 0.95 y: 0.90.
	buttons addButtonWithText: 'back' event: #mainMenuEnable.	
	buttons alignRight.
	buttons.setScale(0.06.
	buttons.setColor(0.6 g: 0.6 b: 0.6 a: 0.8.
	self.clickBoxDictionary['street'] = buttons.

	self.categorySet add: #street.
! !

!Level methodsFor: 'initialization' stamp: 'panda 00/00/0000 00:00'!
initializeWallButtons
	| methodArray hooksSet buttons |

	# Initialize Hooks and Buttons for the wall type buttons
	methodArray = { 
		{ 'Random 20' . #random20 } .
		{ 'Random 30' . #random30 } .
		{ '10-10' . #toonTenTen } .
		{ '20' . #toonTwenty } .
		{ '10-20' . #toonTenTwenty } .
		{ '20-10' . #toonTwentyTen } .
		{ '10-10-10' . #toonTenTenTen } .
		{ '30' . #toonThirty } }.

	hooksSet = self.hooksDictionary['wall ifAbsent: [ Set new. ].
	methodArray do: [ :pair | hooksSet add: (pair at: 2) ].
	self.hooksDictionary['wall'] = hooksSet.

	# Create wall module buttons
	# First get rid of any existing buttons
	self.clickBoxDictionary['wall ifPresent: [ :clickBoxList | clickBoxList disable ].
	buttons = ClickBoxList new table: methodArray x: 0.95 y: 0.90.
	buttons addButtonWithText: 'back' event: #mainMenuEnable.	
	buttons.setColor(0.6 g: 0.6 b: 0.6 a: 0.8.
	buttons.setScale(0.06.
	buttons alignRight.
	buttons makeAllWideAsWidest.
	self.clickBoxDictionary['wall'] = buttons.

	# Initialize Hooks and Buttons for the wall width buttons
	methodArray = { { '5 ft' . #fiveFt } .
					{ '10 ft' . #tenFt } .
					{ '15 ft' . #fifteenFt } .
					{ '20 ft' . #twentyFt } .
					{ '25 ft' . #twentyFiveFt } }.

	hooksSet = self.hooksDictionary['wallWidths ifAbsent: [ Set new. ].
	methodArray do: [ :pair | hooksSet add: (pair at: 2) ].
	self.hooksDictionary['wallWidths'] = hooksSet.

	# Create wall width buttons
	# First get rid of any existing buttons
	self.clickBoxDictionary['wallWidths ifPresent: [ :clickBoxList | clickBoxList disable ].
	buttons = ClickBoxList new table: methodArray x: 0.95 y: -0.40.
	buttons.setColor(0.6 g: 0.6 b: 0.6 a: 0.8.
	buttons.setScale(0.06.
	buttons alignRight.
	self.clickBoxDictionary['wallWidths'] = buttons.

	self.categorySet add: #wall.
! !


    def clearHighlightedObjects(self):
	highlightedObjects getChildren forEachPathPerform: #removeNode.! !

!Level methodsFor: 'object operations' stamp: 'panda 00/00/0000 00:00'!
followMouse: aNodePath
	# Plant target object on grid at cursor projection point
	| roundVal |
	roundVal = (self.grid gridSpacing roundTo: 1).
	(self.grid getMouseIntersectionPoint: self.hitPt ) ifTrue: [ 
		self.grid xyzSnap ifTrue: [
			aNodePath setPos: self.grid 
				x: (((self.hitPt at: 0) + (self.offset at: 0)) roundTo: roundVal)
				y: (((self.hitPt at: 1) + (self.offset at: 1)) roundTo: roundVal)
				z: (((self.hitPt at: 2) + (self.offset at: 2)) roundTo: roundVal).
			]
		ifFalse: [
			aNodePath setPos: self.grid pos: (self.hitPt + self.offset).
			].
		]
! !

!Level methodsFor: 'object operations' stamp: 'panda 00/00/0000 00:00'!
followMouseStart: aNodePath
	| gridToObjectHandles hitPtToObjectHandles |

	# Where is the mouse relative to the grid?
	self.grid getMouseIntersectionPoint: self.hitPt.
	
	# Record crank origin
	self.crankOrigin operatorAssign: (direct selectedNodePath getPos: self.grid).

	# Record the offset
	self.offset = self.crankOrigin - self.hitPt.
	# Init crankDir
	self.crankDir operatorAssign: self.offset negated.
	self.crankDir normalize.
	# Compute crankAngle
	startAngle = self.getCrankAngle.
	startH = direct selectedNodePath getH.

	# Transform hitPt into object handles space to determine manipulation mode
	# Where is the mouse relative to the widget? 
	 Don't snap to grid since we want to test hitPt relative to widget
	self.grid getMouseIntersectionPoint: self.hitPt xyzSnap: 0.
	gridToObjectHandles = self.grid getMat: direct objectHandles.
	hitPtToObjectHandles = (Vec3 new: (gridToObjectHandles xformPoint: self.hitPt)) length.

	# Are we inside rotation ring?
	((hitPtToObjectHandles > 0.8) and: [(hitPtToObjectHandles < 1.2)])	
	ifTrue: [[[true] taskWhileTrue: [ self.mouseCrank: aNodePath]] 
				spawnTaskNamed: #levelMouseCrank.]
	ifFalse: [[[true] taskWhileTrue: [ self.followMouse: aNodePath]] 
				spawnTaskNamed: #levelFollowMouse.]

! !

!Level methodsFor: 'object operations' stamp: 'panda 00/00/0000 00:00'!
followMouseStop
	| selectedNode |
	# Stop moving object
	Task removeTasksNamed: #levelFollowMouse.
	Task removeTasksNamed: #levelMouseCrank.

	# Move grid to line up with object
	selectedNode = direct selectedNodePath.
	selectedNode notNone ifTrue: [
		self.updateDNAPosHpr: selectedNode.
		# Position grid for placing next object
		self.autoPositionGrid.
	].

! !

!Level methodsFor: 'object operations' stamp: 'panda 00/00/0000 00:00'!
getCrankAngle
	| newAngle |
	self.crankDir normalize.
	# Just look at Y component (equiv to dot product with (0 1 0)
	newAngle = (self.crankDir at: 1) arcCos radiansToDegrees.
	((self.crankDir at: 0) > 0.0) ifTrue: [ newAngle = newAngle negated. ].
	# Force it to 0 to 360.0 range
 	return newAngle + 180.0.							
! !

!Level methodsFor: 'object operations' stamp: 'panda 00/00/0000 00:00'!
highlightNodePath: aNodePath
	| pose highlightedNode |
	# First clear out old highlighted nodes
	self.clearHighlightedObjects.
	# Place an instance of the object under the highlightedObjects node	
	highlightedNode = aNodePath instanceTo: highlightedObjects.
	pose = aNodePath getMat: self.levelObjects.
	highlightedNode setMat: self.levelObjects mat: pose.
	! !

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
"""
