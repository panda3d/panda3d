from PandaObject import *
from PieMenu import *
from OnscreenText import *
from Tkinter import *
from DirectGeometry import *
from SceneGraphExplorer import *
from tkSimpleDialog import askstring
from tkMessageBox import showinfo
from tkFileDialog import *
from whrandom import *
import Pmw
import EntryScale
import VectorWidgets
import string
import os
import __builtin__
import whrandom

# Colors used by all color menus
DEFAULT_COLORS = [
    Vec4(1,1,1,1),
    Vec4(0.75, 0.75, 0.75, 1.0 ),
    Vec4(0.5, 0.5, 0.5, 1.0),
    Vec4(0.25, 0.25, 0.25, 1.0)]
# The list of items with color attributes
COLOR_TYPES = ['wall_color', 'window_color',
               'window_awning_color', 'door_color',
               'door_awning_color', 'cornice_color',
               'prop_color']
# The list of dna components maintained in the style attribute dictionary
DNA_TYPES = ['wall', 'window', 'door', 'cornice', 'toon_landmark',
             'prop', 'street']
BUILDING_TYPES = ['10_10', '20', '10_20', '20_10', '10_10_10']
# The list of neighborhoods to edit
NEIGHBORHOODS = ['toontown_central', 'donalds_dock',
                 'minnies_melody_land', 'the_burrrgh']
NEIGHBORHOOD_CODES = {'toontown_central': 'TT', 'donalds_dock': 'DD',
                      'minnies_melody_land': 'MM', 'the_burrrgh': 'BR'}
OBJECT_SNAP_POINTS = {
    'street_5x20': [(Vec3(5.0,0,0), Vec3(0)),
                    (Vec3(0), Vec3(0))],
    'street_10x20': [(Vec3(10.0,0,0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_20x20': [(Vec3(20.0,0,0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_40x20': [(Vec3(40.0,0,0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_80x20': [(Vec3(80.0,0,0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_5x40': [(Vec3(5.0,0,0), Vec3(0)),
                    (Vec3(0), Vec3(0))],
    'street_10x40': [(Vec3(10.0,0,0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_20x40': [(Vec3(20.0,0,0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_30x40': [(Vec3(30.0,0,0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_40x40': [(Vec3(40.0,0,0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_80x40': [(Vec3(80.0,0,0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_angle_30': [(Vec3(0), Vec3(-30,0,0)),
                        (Vec3(0), Vec3(0))],
    'street_angle_45': [(Vec3(0), Vec3(-45,0,0)),
                        (Vec3(0), Vec3(0))],
    'street_angle_60': [(Vec3(0), Vec3(-60,0,0)),
                        (Vec3(0), Vec3(0))],
    'street_inner_corner': [(Vec3(20.0,0,0), Vec3(0)),
                            (Vec3(0), Vec3(0))],
    'street_outer_corner': [(Vec3(20.0,0,0), Vec3(0)),
                            (Vec3(0), Vec3(0))],
    'street_full_corner': [(Vec3(40.0,0,0), Vec3(0)),
                           (Vec3(0), Vec3(0))],
    'street_t_intersection': [(Vec3(40.0,0,0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_y_intersection': [(Vec3(30.0,0,0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_street_20x20': [(Vec3(20.0,0,0), Vec3(0)),
                            (Vec3(0), Vec3(0))],
    'street_street_40x40': [(Vec3(40.0,0,0), Vec3(0)),
                            (Vec3(0), Vec3(0))],
    'street_sidewalk_20x20': [(Vec3(20.0,0,0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_sidewalk_40x40': [(Vec3(40.0,0,0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_divided_transition': [(Vec3(40.0,0,0), Vec3(0)),
                                  (Vec3(0), Vec3(0))],
    'street_divided_40x70': [(Vec3(40.0,0,0), Vec3(0)),
                             (Vec3(0), Vec3(0))],
    'street_stairs_40x10x5': [(Vec3(40.0,0,0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_4way_intersection': [(Vec3(40.0,0,0), Vec3(0)),
                                 (Vec3(0), Vec3(0))],
    'street_incline_40x40x5': [(Vec3(40.0,0,0), Vec3(0)),
                               (Vec3(0), Vec3(0))],
    'street_courtyard_70': [(Vec3(0.0,0,0), Vec3(45,0,0)),
                            (Vec3(0), Vec3(0))],
    'street_courtyard_70_exit': [(Vec3(0.0,0,0), Vec3(45,0,0)),
                                 (Vec3(0), Vec3(0))],
    'street_courtyard_90': [(Vec3(0.0,0,0), Vec3(45,0,0)),
                            (Vec3(0), Vec3(0))],
    'street_courtyard_90_exit': [(Vec3(0.0,0,0), Vec3(45,0,0)),
                                 (Vec3(0), Vec3(0))],
    'street_50_transition': [(Vec3(10.0,0,0), Vec3(0)),
                             (Vec3(0), Vec3(0))],
    'street_20x50': [(Vec3(20.0,0,0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_40x50': [(Vec3(40.0,0,0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_keyboard_10x40': [(Vec3(10.0,0,0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_keyboard_20x40': [(Vec3(20.0,0,0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_keyboard_40x40': [(Vec3(40.0,0,0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    }
SNAP_ANGLE = 15.0

try:
    if dnaLoaded:
        pass
except NameError:
    # DNAStorage instance for storing level DNA info
    __builtin__.DNASTORE = DNASTORE = DNAStorage()
    # Load the generic storage file
    loadDNAFile(DNASTORE, 'phase_4/dna/storage.dna', CSDefault, 1)
    # Load all the neighborhood specific storage files
    loadDNAFile(DNASTORE, 'phase_4/dna/storage_TT.dna', CSDefault, 1)
    loadDNAFile(DNASTORE, 'phase_6/dna/storage_DD.dna', CSDefault, 1)
    loadDNAFile(DNASTORE, 'phase_6/dna/storage_MM.dna', CSDefault, 1)
    loadDNAFile(DNASTORE, 'phase_6/dna/storage_BR.dna', CSDefault, 1)
    __builtin__.dnaLoaded = 1

# Precompute class types for type comparisons
DNA_CORNICE = DNACornice.getClassType()
DNA_DOOR = DNADoor.getClassType()
DNA_FLAT_BUILDING = DNAFlatBuilding.getClassType()
DNA_NODE = DNANode.getClassType()
DNA_GROUP = DNAGroup.getClassType()
DNA_VIS_GROUP = DNAVisGroup.getClassType()
DNA_LANDMARK_BUILDING = DNALandmarkBuilding.getClassType()
DNA_NODE = DNANode.getClassType()
DNA_PROP = DNAProp.getClassType()
DNA_STREET = DNAStreet.getClassType()
DNA_WALL = DNAWall.getClassType()
DNA_WINDOWS = DNAWindows.getClassType()

# DNA Utility functions (possible class extensions?)
def DNARemoveChildren(dnaObject):
    """ Utility function to delete all the children of a DNANode """
    children = []
    for i in range(dnaObject.getNumChildren()):
        children.append(dnaObject.at(i))
    for child in children:
        dnaObject.remove(child)
        DNASTORE.removeDNAGroup(child)

def DNARemoveChildOfClass(dnaNode, classType):
    """ Remove the first object of that type you come across """
    for i in range(dnaNode.getNumChildren()):
        child = dnaNode.at(i)
        if DNAClassEqual(child, classType):
            dnaNode.remove(child)
            DNASTORE.removeDNAGroup(child)
            return 1
    # None found
    return 0

def DNAGetChild(dnaObject, type = DNA_NODE, childNum = 0):
    childCount = 0
    for i in range(dnaObject.getNumChildren()):
        child = dnaObject.at(i)
        if DNAClassEqual(child, type):
            if childCount == childNum:
                return child
            childCount = childCount + 1
    # Not found
    return None

def DNAGetChildOfClass(dnaNode, classType):
    for i in range(dnaNode.getNumChildren()):
        child = dnaNode.at(i)
        if DNAClassEqual(child, classType):
            return child
    # Not found
    return None

def DNAGetClassType(dnaObject):
    return dnaObject.__class__.getClassType()

def DNAClassEqual(dnaObject, classType):
    return DNAGetClassType(dnaObject).eq(classType)

def DNAIsDerivedFrom(dnaObject, classType):
    return DNAGetClassType(dnaObject).isDerivedFrom(classType)

def DNAGetWallHeights(aDNAFlatBuilding):
    """ Get a list of wall heights for a given flat building """
    # Init variables
    heightList = []
    offsetList = []
    offset = 0.0
    # Compute wall heights
    for i in range(aDNAFlatBuilding.getNumChildren()):
        child = aDNAFlatBuilding.at(i)
        if DNAClassEqual(child, DNA_WALL):
            height = child.getHeight()
            heightList.append(height)
            offsetList.append(offset)
            offset = offset + height
    return heightList, offsetList

class LevelEditor(NodePath, PandaObject):
    """Class used to create a Toontown LevelEditor object"""
    # Primary variables:
    # DNAData: DNA object holding DNA info about level
    # DNAToplevel: Top level DNA Node, all DNA objects descend from this node
    # NPToplevel: Corresponding Node Path
    # DNAParent: Current DNA Node that new objects get added to
    # NPParent: Corresponding Node Path
    # DNAVisGroup: Current DNAVisGroup that new objects get added to
    # NPVisGroup: Corresponding Node Path
    # selectedDNARoot: DNA Node of currently selected object
    # selectedNPRoot: Corresponding Node Path
    # DNATarget: Subcomponent being modified by Pie Menu
    def __init__(self):
        # Make the level editor a node path so that you can show/hide
        # The level editor separately from loading/saving the top level node
        # Initialize superclass
        NodePath.__init__(self)
        # Become the new node path
        self.assign(hidden.attachNewNode('LevelEditor'))
        
        # Create ancillary objects
        # Style manager for keeping track of styles/colors
        self.styleManager = LevelStyleManager()
        # Load neighborhood maps
        self.createLevelMaps()
        # Marker for showing next insertion point
        self.createInsertionMarker()
        # Create level Editor Panel
        self.panel = LevelEditorPanel(self)

        # Used to store whatever edges and points are loaded in the level
        self.edgeDict = {}
        self.pointDict = {}
        self.cellDict = {}

        # Initialize LevelEditor variables DNAData, DNAToplevel, NPToplevel
        # DNAParent, NPParent, groupNum, lastAngle
        # Pass in the new toplevel group and don't clear out the old
        # toplevel group (since it doesn't exist yet)
        self.reset(fDeleteToplevel = 0, fCreateToplevel = 1)
        
        # The list of events the level editor responds to
        self.actionEvents = [
            # Actions in response to DIRECT operations
            ('selectedNodePath', self.selectedNodePathHook),
            ('deselectedNodePath', self.deselectedNodePathHook),
            ('manipulateObjectCleanup', self.updateSelectedPose),
            
            # Actions in response to Level Editor Panel operations
            ('SGENodePath_Set Name', self.getAndSetName),
            ('SGENodePath_Set Parent', self.setActiveParent),
            ('SGENodePath_Reparent', self.reparent),
            ('SGENodePath_Add Group', self.addGroup),
            ('SGENodePath_Add Vis Group', self.addVisGroup),
            ('SGENodePath_Set Color', self.setNPColor),
            # Actions in response to Pie Menu interaction
            ('select_building_style', self.setBuildingStyle),
            ('select_building_type', self.setBuildingType),
            ('select_building_width', self.setBuildingWidth),
            ('select_cornice_color', self.setDNATargetColor),
            ('select_cornice_orientation', self.setDNATargetOrientation),
            ('select_cornice_texture', self.setDNATargetCode, ['cornice']),
            ('select_door_color', self.setDNATargetColor),
            ('select_door_orientation', self.setDNATargetOrientation),
            ('select_door_texture', self.setDNATargetCode, ['door']),
            ('select_door_awning_texture', self.setDNATargetCode,
             ['door_awning']),
            ('select_door_awning_color', self.setDNATargetColor),
            ('select_window_color', self.setDNATargetColor),
            ('select_window_count', self.setWindowCount),
            ('select_window_orientation', self.setDNATargetOrientation),
            ('select_window_texture', self.setDNATargetCode, ['windows']),
            ('select_window_awning_texture', self.setDNATargetCode,
             ['window_awning']),
            ('select_window_awning_color', self.setDNATargetColor),
            ('select_wall_style', self.setWallStyle),
            ('select_wall_color', self.setDNATargetColor),
            ('select_wall_orientation', self.setDNATargetOrientation),
            ('select_wall_texture', self.setDNATargetCode, ['wall']),
            ('select_toon_landmark_texture', self.setDNATargetCode,
             ['landmark']),
            ('select_toon_landmark_door_color', self.setDNATargetColor),
            ('select_toon_landmark_door_orientation',
             self.setDNATargetOrientation),
            ('select_landmark_door_texture', self.setDNATargetCode,
             ['landmark_door']),
            ('select_street_texture', self.setDNATargetCode, ['street']),
            ('select_prop_texture', self.setDNATargetCode, ['prop']),
            ('select_prop_color', self.setDNATargetColor),
            # Hot key actions
            ('a', self.autoPositionGrid),
            ('j', self.jumpToInsertionPoint),
            ('left', self.keyboardXformSelected, ['left']),
            ('right', self.keyboardXformSelected, ['right']),
            ('up', self.keyboardXformSelected, ['up']),
            ('down', self.keyboardXformSelected, ['down']),
            ('S', self.placeSuitPoint),
            ('C', self.placeBattleCell),
            ]
	        
        # Initialize state
	# Make sure direct is running
	direct.enable()
        # And only the appropriate handles are showing
        direct.widget.disableHandles(['x-ring', 'x-disc',
                                           'y-ring', 'y-disc',
                                           'z-post'])
        # Initialize camera
	base.cam.node().setNear(5.0)
	base.cam.node().setFar(3000)
	direct.camera.setPos(0,-10,10)
        # Hide (disable) grid initially
        self.showGrid(0)
        # Create variable for vis groups panel
        self.vgpanel = None
        # Start off enabled
        self.enable()
        # Editing toontown_central
        self.setEditMode('toontown_central')

        # SUIT POINTS
        # Create a sphere model to show suit points
        self.suitPointMarker = loader.loadModel('models/misc/sphere')
        self.suitPointMarker.setScale(0.25)

        # Initialize the suit points
        self.startSuitPoint = None
        self.endSuitPoint = None
        self.currentSuitPointType = DNASuitPoint.STREETPOINT

        # BATTLE CELLS
        self.battleCellMarker = loader.loadModel('models/misc/sphere')
        self.battleCellMarker.setScale(1)
        self.currentBattleCellType = "20w 20l"


    # ENABLE/DISABLE
    def enable(self):
        """ Enable level editing and show level """
        # Make sure level is visible
        self.reparentTo(direct.group)
	self.show()
        # Add all the action events
        for event in self.actionEvents:
            if len(event) == 3:
                self.accept(event[0], event[1], event[2])
            else:
                self.accept(event[0], event[1])
        # Enable mouse interaction (pie menus)
        self.enableMouse()
        # Spawn task to keep insertion marker aligned with grid
        self.spawnInsertionMarkerTask()

    def disable(self):
        """ Disable level editing and hide level """
        # Deselect everything as a precaution
	direct.deselectAll()
        # Hide the level
        self.reparentTo(hidden)
        # Ignore the hooks
        for eventPair in self.actionEvents:
            self.ignore(eventPair[0])
        # These are added outside of actionEvents list
	self.ignore('insert')
	self.ignore('space')
        # Disable Pie Menu mouse interaction
	self.disableMouse()
        # Remove insertion marker task
        taskMgr.removeTasksNamed('insertionMarkerTask')

    def reset(self, fDeleteToplevel = 1, fCreateToplevel = 1):
        """
        Reset level and re-initialize main class variables
        Pass in the new top level group
        """
        if fDeleteToplevel:
            # First destroy existing scene-graph/DNA hierarchy
            self.deleteToplevel()
        
	# Clear DNASTORE
	DNASTORE.resetDNAGroups()
        # Reset DNA VIS Groups
        DNASTORE.resetDNAVisGroups()
        DNASTORE.resetSuitPoints()
        DNASTORE.resetBattleCells()
        
	# Create fresh DNA DATA
	self.DNAData = DNAData('level_data')
        
        # Create new toplevel node path and DNA
        if fCreateToplevel:
            self.createToplevel(DNANode('level'))
        
        # Initialize variables
        # Reset grid
        direct.grid.setPosHprScale(0,0,0,0,0,0,1,1,1)
        # The selected DNA Object/NodePath
        self.selectedDNARoot = None
        self.selectedNPRoot = None
        # Set active target (the subcomponent being modified)
        self.DNATarget = None
        self.DNATargetParent = None
        # Set count of groups added to level
        self.setGroupNum(0)
        # Heading angle of last object added to level
	self.setLastAngle(0.0)
        # Last wall and building modified using pie menus
        self.lastWall = None
        self.lastBuilding = None
        # Code of last selected object (for autopositionGrid)
        self.snapList = []
        # Last menu used
        self.activeMenu = None
        # Reset path markers
        self.resetPathMarkers()
        # Reset battle cell markers
        self.resetBattleCellMarkers()

    def deleteToplevel(self):
        # Destory old toplevel node path and DNA
        # First the toplevel DNA
        self.DNAData.remove(self.DNAToplevel)
        # Then the toplevel Node Path
        self.NPToplevel.reparentTo(hidden)
        self.NPToplevel.remove()

    def createToplevel(self, dnaNode, nodePath = None):
        # When you create a new level, data is added to this node
        # When you load a DNA file, you replace this node with the new data
        self.DNAToplevel = dnaNode
        self.DNAData.add(self.DNAToplevel)
        if nodePath:
            # Node path given, use it
            self.NPToplevel = nodePath
            self.NPToplevel.reparentTo(self)
        else:
            # No node path given, traverse
            self.NPToplevel = self.DNAToplevel.traverse(self, DNASTORE, 1)
        # Update parent pointers
        self.DNAParent = self.DNAToplevel
        self.NPParent = self.NPToplevel
        self.VGParent = None
        # Update scene graph explorer
        # self.panel.sceneGraphExplorer.update()

    def destroy(self):
        """ Disable level editor and destroy node path """
	self.disable()
	self.removeNode()
        self.panel.destroy()
        if self.vgpanel:
            self.vgpanel.destroy()

    def useDirectFly(self):
        """ Disable player camera controls/enable direct camera control """
        self.enableMouse()
        direct.enable()

    def useDriveMode(self):
        """ Lerp down to eye level then switch to Drive mode """
        pos = direct.camera.getPos()
        pos.setZ(4.0)
        hpr = direct.camera.getHpr()
        hpr.set(hpr[0], 0.0, 0.0)
        t = direct.camera.lerpPosHpr(pos, hpr, 1.0, blendType = 'easeInOut',
                                   task = 'manipulateCamera')
        # Note, if this dies an unatural death, this could screw things up
        t.uponDeath = self.switchToDriveMode

    def switchToDriveMode(self,state):
        """ Disable direct camera manipulation and enable player drive mode """
        direct.minimumConfiguration()
        direct.manipulationControl.disableManipulation()
        base.useDrive()
        # Make sure we're where we want to be
        pos = direct.camera.getPos()
        pos.setZ(4.0)
        hpr = direct.camera.getHpr()
        hpr.set(hpr[0], 0.0, 0.0)
        # Fine tune the drive mode
        base.mouseInterface.node().setPos(pos)
        base.mouseInterface.node().setHpr(hpr)
        base.mouseInterface.node().setForwardSpeed(20.0)
        base.mouseInterface.node().setReverseSpeed(20.0)

    def enableMouse(self):
	""" Enable Pie Menu interaction (and disable player camera control) """
	# Turn off player camera control
	base.disableMouse()
	# Handle mouse events for pie menus
	self.accept('handleMouse3',self.levelHandleMouse3)
	self.accept('handleMouse3Up',self.levelHandleMouse3Up)

    def disableMouse(self):
        """ Disable Pie Menu interaction """
	# Disable handling of mouse events
	self.ignore('handleMouse3')
	self.ignore('handleMouse3Up')

    # LEVEL OBJECT MANAGEMENT FUNCTIONS
    def findDNANode(self, nodePath):
        """ Find node path's DNA Object in DNAStorage (if any) """
        return DNASTORE.findDNAGroup(nodePath.id())

    def replaceSelected(self):
        # Update visible geometry using new DNA
        newRoot = self.replace(self.selectedNPRoot, self.selectedDNARoot)
        # Reselect node path and respawn followSelectedNodePathTask
        direct.select(newRoot)

    def replace(self, nodePath, dnaNode):
        """ Replace a node path with the results of a DNANode traversal """
        # Find node path's parent
	parent = nodePath.getParent()
        dnaParent = dnaNode.getParent()
	# Get rid of the old node path and remove its DNA and
        # node relations from the DNA Store
	self.remove(dnaNode, nodePath)
        # Traverse the old (modified) dna to create the new node path
        newNodePath = dnaNode.traverse(parent, DNASTORE, 1)
        # Add it back to the dna parent
        dnaParent.add(dnaNode)
        # Update scene graph explorer
        # self.panel.sceneGraphExplorer.update()
        # Return new node path
        return newNodePath

    def remove(self, dnaNode, nodePath):
        """
        Delete DNA and Node relation from DNA Store and remove the node
        path from the scene graph.
        """
        # First delete DNA and node relation from the DNA Store
        if dnaNode:
            # Get DNANode's parent
            parentDNANode = dnaNode.getParent()
            if parentDNANode:
                # Remove DNANode from its parent
                parentDNANode.remove(dnaNode)
            # Delete DNA and associated Node Relations from DNA Store
            DNASTORE.removeDNAGroup(dnaNode)
        if nodePath:
            # Next deselect nodePath to avoid having bad node paths in the dict
            direct.deselect(nodePath)
            # Now you can get rid of the node path
            nodePath.removeNode()

    def reparent(self, nodePath):
        """ Move node path (and its DNA) to active parent """
        # Do we have a node path?
        if nodePath:
            dnaNode = self.findDNANode(nodePath)
            # Does the node path correspond to a DNA Object
            if dnaNode:
                # Yes, get its parent and see if it has a DNA Object
                parent = nodePath.getParent()
                parentDNAObject = self.findDNANode(parent)
                if parentDNAObject:
                    # Yes it does, move node path (and DNA)
                    # to new parent (if active parent set)
                    if ((self.NPParent != None) and
                        (self.DNAParent != None)):
                        nodePath.reparentTo(self.NPParent)
                        parentDNAObject.remove(dnaNode)
                        self.DNAParent.add(dnaNode)
                        # Update scene graph explorer to reflect change
                        # self.panel.sceneGraphExplorer.update()

    def reparentSelected(self):
        for nodePath in direct.selected:
            self.reparent(nodePath)

    def setActiveParent(self, nodePath = None):
        """ Set NPParent and DNAParent to node path and its DNA """
        # If nothing passed in, try currently selected node path
        if not nodePath:
            nodePath = direct.selected.last
        # If we've got a valid node path
        if nodePath:
            # See if this is in the DNA database
            newDNAParent = self.findDNANode(nodePath)
            if newDNAParent:
                self.DNAParent = newDNAParent
                self.NPParent = nodePath
            else:
                print 'LevelEditor.setActiveParent: nodePath not found'
        else:
            print 'LevelEditor.setActiveParent: nodePath == None'

    def getAndSetName(self, nodePath):
        """ Prompt user for new node path name """
        newName = askstring('Node Path', 'Enter new name:')
        if newName:
            self.setName(nodePath, newName)

    def setName(self, nodePath, newName):
        """ Set name of a node path and its DNA (if it exists) """
        # First, set name of the node path
        nodePath.setName(newName)
        # Now find the DNA that corresponds to this node path
        dnaNode = self.findDNANode(nodePath)
        if dnaNode:
            # If it exists, set the name of the DNA Node
            dnaNode.setName(newName)

    def updateSelectedPose(self):
        """
        Update the DNA database to reflect selected objects current positions
        """
        for selectedNode in direct.selected:
            # Is this a DNA Object in the DNASTORE database?
            dnaObject = self.findDNANode(selectedNode)
            if dnaObject:
                # It is, is it a DNA_NODE (i.e. does it have pos/hpr/scale)?
                if DNAIsDerivedFrom(dnaObject, DNA_NODE):
                    # First snap selected node path to grid
                    pos = selectedNode.getPos(direct.grid)
                    snapPos = direct.grid.computeSnapPoint(pos)
                    if self.panel.fPlaneSnap.get():
                        zheight = 0
                    else:
                        zheight = snapPos[2]
                    selectedNode.setPos(direct.grid,
                                        snapPos[0], snapPos[1], zheight)
                    # Angle snap
                    h = direct.grid.computeSnapAngle(selectedNode.getH())
                    selectedNode.setH(h)
                    if selectedNode == direct.selected.last:
                        self.setLastAngle(h)
                    # Update DNA
                    self.updatePose(dnaObject, selectedNode)

    def updatePose(self, dnaObject, nodePath):
        """
        Update a DNA Object's pos, hpr, and scale based upon
        node path's current pose
        """
        # Set DNA's pos, hpr, and scale
        dnaObject.setPos(nodePath.getPos())
        dnaObject.setHpr(nodePath.getHpr())
        dnaObject.setScale(nodePath.getScale())        
                
    # LEVEL OBJECT CREATION FUNCTIONS
    def initDNANode(self, dnaNode):
        """
        This method adds a new DNA object to the scene and adds hooks that
        allow duplicate copies of this DNA node to be added using the
        space bar. For DNAFlatBuildings, a new copy with random style is
        generated by hitting the insert key.
        """
	# First create the visible geometry for this DNA Node
	self.initNodePath(dnaNode)
        # And add hooks to insert copies of dnaNode
        self.addReplicationHooks(dnaNode)

    def addReplicationHooks(self, dnaNode):
	# Now add hook to allow placement of a new dna Node of this type
        # by simply hitting the space bar or insert key.  Note, extra paramter
        # indicates new dnaNode should be a copy
	self.accept('space', self.initNodePath, [dnaNode, 'space'])
        self.accept('insert', self.initNodePath, [dnaNode, 'insert'])

    def setRandomBuildingStyle(self, dnaNode, name = 'building'):
        """ Initialize a new DNA Flat building to a random building style """
        buildingType = self.getCurrent('building_type')
        # Select a list of wall heights
        if buildingType == 'random20':
            chance = randint(1,100)
            if chance <= 65:
                buildingType = '10_10'
            else:
                buildingType = '20'
        elif buildingType == 'random30':
            chance = randint(1,100)
            if chance <= 40:
                buildingType = '10_20'
            elif (chance > 80):
                buildingType = '10_10_10'
            else:
                buildingType = '20_10'
        
        # The building_style attribute dictionary for this number of stories
        dict = self.getAttribute('building_style_' + buildingType).getDict()
        style = self.getRandomDictionaryEntry(dict)
        self.styleManager.setDNAFlatBuildingStyle(
            dnaNode, style, width = self.getRandomWallWidth(), name = name)

    def getRandomWallWidth(self):
	chance = randint(0,100)
        if chance <= 15:
            return 5.0
        elif (chance > 15) and (chance <= 30):
            return 10.0
        elif (chance > 30) and (chance <= 65):
            return 15.0
        elif (chance > 65) and (chance <= 85):
            return 20.0
        elif (chance > 85):
            return 25.0

    def initNodePath(self, dnaNode, hotKey = None):
        """
        Update DNA to reflect latest style choices and then generate
        new node path and add it to the scene graph
        """
        # Determine dnaNode Class Type
        nodeClass = DNAGetClassType(dnaNode)
        # Did the user hit insert or space?
        if hotKey:
            # Yes, make a new copy of the dnaNode
            dnaNode = dnaNode.__class__(dnaNode)
            # And determine dnaNode type and perform any type specific updates
            if nodeClass.eq(DNA_PROP):
                dnaNode.setCode(self.getCurrent('prop_texture'))
            elif nodeClass.eq(DNA_STREET):
                dnaNode.setCode(self.getCurrent('street_texture'))
            elif nodeClass.eq(DNA_FLAT_BUILDING):
                # If insert, pick a new random style
                if hotKey == 'insert':
                    self.setRandomBuildingStyle(dnaNode, dnaNode.getName())
                    # Get a new building width
                    self.setCurrent('building_width',
                                    self.getRandomWallWidth())
                dnaNode.setWidth(self.getCurrent('building_width'))
        
	# Add the DNA to the active parent
	self.DNAParent.add(dnaNode)
	# And create the geometry
	newNodePath = dnaNode.traverse(self.NPParent, DNASTORE, 1)
        # Update scene graph explorer
        # self.panel.sceneGraphExplorer.update()
        
        # Position it
        # First kill autoposition task so grid can jump to its final
        # destination (part of cleanup
        taskMgr.removeTasksNamed('autoPositionGrid')
        # Now find where to put node path
        if (hotKey is not None) and nodeClass.eq(DNA_PROP):
            # If its a prop and a copy, place it based upon current
            # mouse position
            hitPt = self.getGridIntersectionPoint()
            newNodePath.setPos(direct.grid, hitPt)
        else:
            # Place the new node path at the current grid origin
            newNodePath.setPos(direct.grid,0,0,0)
	# Initialize angle to match last object
	newNodePath.setHpr(self.getLastAngle(), 0, 0)
	# Now update DNA pos and hpr to reflect final pose
	dnaNode.setPos(newNodePath.getPos())
	dnaNode.setHpr(newNodePath.getHpr())

        # Reset last Code (for autoPositionGrid)
        if DNAClassEqual(dnaNode, DNA_STREET):
            self.snapList = OBJECT_SNAP_POINTS[dnaNode.getCode()]
	# Select the instance
	direct.select(newNodePath)
        # Update grid to get ready for the next object
        self.autoPositionGrid()

    def addGroup(self, nodePath):
        """ Add a new DNA Node Group to the specified Node Path """
        # Set the node path as the current parent
        self.setActiveParent(nodePath)
        # Add a new group to the selected parent
        self.createNewGroup()

    def addVisGroup(self, nodePath):
        """ Add a new DNA Group to the specified Node Path """
        # Set the node path as the current parent
        self.setActiveParent(nodePath)
        # Add a new group to the selected parent
        self.createNewGroup(type = 'vis')

    def createNewGroup(self, type = 'dna'):
        """ Create a new DNA Node group under the active parent """
        # Create a new DNA Node group
        if type == 'dna':
            newDNANode = DNANode('group_' + `self.getGroupNum()`)
        else:
            newDNANode = DNAVisGroup('VisGroup_' + `self.getGroupNum()`)
            # Increment group counter
	self.setGroupNum(self.getGroupNum() + 1)
	# Add new DNA Node group to the current parent DNA Object
	self.DNAParent.add(newDNANode)
	# The new Node group becomes the active parent
	self.DNAParent = newDNANode
        # Traverse it to generate the new node path as a child of NPParent
	newNodePath = self.DNAParent.traverse(self.NPParent, DNASTORE, 1)
        # Update NPParent to point to the new node path
	self.NPParent = newNodePath
        # Update scene graph explorer
        # self.panel.sceneGraphExplorer.update()

    def addFlatBuilding(self, buildingType):
	# Create new building
	newDNAFlatBuilding = DNAFlatBuilding()
        self.setRandomBuildingStyle(newDNAFlatBuilding,
                                    name = buildingType + '_DNARoot')
	# Initialize its position and hpr
	newDNAFlatBuilding.setPos(VBase3(0))
	newDNAFlatBuilding.setHpr(VBase3(0))
	# Now place new building in the world
	self.initDNANode(newDNAFlatBuilding)

    def addLandmark(self, landmarkType):
        # Record new landmark type
        self.setCurrent('toon_landmark_texture', landmarkType)
        # And create new landmark building
	newDNALandmarkBuilding = DNALandmarkBuilding(landmarkType + '_DNARoot')
	newDNALandmarkBuilding.setCode(landmarkType)
	newDNALandmarkBuilding.setPos(VBase3(0))
	newDNALandmarkBuilding.setHpr(VBase3(0))
	newDNADoor = self.createDoor()
	newDNALandmarkBuilding.add(newDNADoor)
	# Now place new landmark building in the world
	self.initDNANode(newDNALandmarkBuilding)

    def addProp(self, propType):
        # Record new prop type
	self.setCurrent('prop_texture', propType)
        # And create new prop
	newDNAProp = DNAProp(propType + '_DNARoot')
	newDNAProp.setCode(propType)
	newDNAProp.setPos(VBase3(0))
	newDNAProp.setHpr(VBase3(0))
	# Now place new prop in the world
	self.initDNANode(newDNAProp)

    def addStreet(self, streetType):
        # Record new street type
	self.setCurrent('street_texture', streetType)
        # And create new street
	newDNAStreet = DNAStreet(streetType + '_DNARoot')
	newDNAStreet.setCode(streetType)
	newDNAStreet.setPos(VBase3(0))
	newDNAStreet.setHpr(VBase3(0))
        # Set street texture to neighborhood dependant texture
        newDNAStreet.setStreetTexture(
            'street_street_' + self.neighborhoodCode + '_tex')
        newDNAStreet.setSidewalkTexture(
            'street_sidewalk_' + self.neighborhoodCode + '_tex')
	# Now place new street in the world
	self.initDNANode(newDNAStreet)

    def createCornice(self):
	newDNACornice = DNACornice('cornice')
	newDNACornice.setCode(self.getCurrent('cornice_texture'))
	newDNACornice.setColor(self.getCurrent('cornice_color'))
	return newDNACornice

    def createDoor(self):
        if not (self.getCurrent('door_texture')):
            defaultDoorStyle = self.styleManager.attributeDictionary['door_texture'].getList()[0]
            self.setCurrent('door_texture', defaultDoorStyle)
        newDNADoor = DNADoor('door')
        newDNADoor.setCode(self.getCurrent('door_texture'))
        newDNADoor.setColor(self.getCurrent('door_color'))
        return newDNADoor

    def createWindows(self):
	newDNAWindows = DNAWindows()
	newDNAWindows.setCode(self.getCurrent('window_texture'))
	newDNAWindows.setWindowCount(self.getCurrent('window_count'))
	newDNAWindows.setColor(self.getCurrent('window_color'))
	return newDNAWindows

    def removeCornice(self, cornice, parent):
        self.setCurrent('cornice_color', cornice.getColor())
        DNARemoveChildOfClass(parent, DNA_CORNICE)

    def removeLandmarkDoor(self, door, parent):
        self.setCurrent('door_color', door.getColor())
        DNARemoveChildOfClass(parent, DNA_DOOR)

    def removeDoor(self, door, parent):
        self.setCurrent('door_color', door.getColor())
        DNARemoveChildOfClass(parent, DNA_DOOR)

    def removeWindows(self, windows, parent):
        # And record number of windows
        self.setCurrent('window_color', windows.getColor())
        self.setCurrent('window_count', windows.getWindowCount())
        DNARemoveChildOfClass(parent, DNA_WINDOWS)

    # LEVEL-OBJECT MODIFICATION FUNCTIONS
    def levelHandleMouse3(self):
        # Initialize dna target
        self.DNATarget = None
        
	# If nothing selected, just return
        if not self.selectedNPRoot:
            return
        
	# Next check to see if the selected object is a DNA object
	dnaObject = self.findDNANode(self.selectedNPRoot)
	# Nope, not a DNA object, just return
        if not dnaObject:
            return
        
        # Pick a menu based upon object type
        if DNAClassEqual(dnaObject, DNA_FLAT_BUILDING):
            # FLAT BUILDING OPERATIONS
            menuMode, wallNum = self.getFlatBuildingMode(dnaObject)
            # Check menuMode
            if menuMode == None:
                return
            # Find appropriate target
            wall = self.getWall(dnaObject, wallNum)
            # Record bldg/wall
            self.lastBuilding = dnaObject
            self.lastWall = wall
            if string.find(menuMode,'wall') >= 0:
                self.DNATarget = wall
                self.DNATargetParent = dnaObject
            elif string.find(menuMode,'door') >= 0:
                self.DNATarget = DNAGetChildOfClass(wall, DNA_DOOR)
                self.DNATargetParent = wall
            elif string.find(menuMode, 'window') >= 0:
                self.DNATarget = DNAGetChildOfClass(wall, DNA_WINDOWS)
                self.DNATargetParent = wall
            elif string.find(menuMode,'cornice') >= 0:
                self.DNATarget = DNAGetChildOfClass(wall, DNA_CORNICE)
                self.DNATargetParent = wall
            else:
                self.DNATarget = dnaObject
        elif DNAClassEqual(dnaObject, DNA_PROP):
            # PROP OPERATIONS
            self.DNATarget = dnaObject
            if direct.fControl:
                menuMode = 'prop_color'
            else:
                menuMode = 'prop_texture'
        elif DNAClassEqual(dnaObject, DNA_LANDMARK_BUILDING):
            # INSERT HERE
            # LANDMARK BUILDING OPERATIONS
            self.DNATarget = DNAGetChildOfClass(dnaObject, DNA_DOOR)
            self.DNATargetParent = dnaObject
            if direct.fAlt:
                menuMode = 'door_orientation'
            elif direct.fControl:
                menuMode = 'door_color'
            else:
                menuMode = 'door_texture'                    
        elif DNAClassEqual(dnaObject, DNA_STREET):
            # STREET OPERATIONS
            self.DNATarget = dnaObject
            menuMode = 'street_texture'
        
        # No valid menu mode, get the hell out of here!
        if menuMode == None:
            return
        
        # Now spawn apropriate menu task if menu selected
        self.activeMenu = self.getMenu(menuMode)
        # Set initial state
        state = None
        if self.DNATarget:
            if string.find(menuMode,'texture') >= 0:
                state = self.DNATarget.getCode()
            elif string.find(menuMode, 'color') >= 0:
                state = self.DNATarget.getColor()
                self.panel.setCurrentColor(state)
                self.panel.setResetColor(state)
            elif string.find(menuMode, 'orientation') >= 0:
                state = self.DNATarget.getCode()[-3:]
            elif menuMode == 'building_width' >= 0:
                state = self.DNATarget.getWidth()
            elif menuMode == 'window_count' >= 0:
                state = self.DNATarget.getWindowCount()
            elif menuMode == 'building_style' >= 0:
                # Extract the building style from the current building
                state = DNAFlatBuildingStyle(building = self.DNATarget)
            elif menuMode == 'wall_style' >= 0:
                # Extract the wall style from the current wall
                state = DNAWallStyle(wall = self.DNATarget)
        self.activeMenu.setInitialState(state)
        
        # Spawn active menu's tatsk
        self.activeMenu.spawnPieMenuTask()

    def getFlatBuildingMode(self, dnaObject):
        # Where are we hitting the building?
        hitPt = self.getWallIntersectionPoint(self.selectedNPRoot)
        wallNum = self.computeWallNum(dnaObject, hitPt)
        if wallNum < 0:
            # Do building related operations
            if direct.fShift:
                menuMode = 'building_type'
            elif direct.fAlt:
                menuMode = 'building_width'
            else:
                menuMode = 'building_style'
        else:
            # Otherwise, do wall specific operations
            # Figure out where you are hitting on the wall
            wallHeights, offsetList = DNAGetWallHeights(dnaObject)
            # Find a normalized X and Z coordinate
            xPt = hitPt[0]/dnaObject.getWidth()
            # Adjust zPt depending on what wall you are pointing at
            wallHeight = wallHeights[wallNum]
            zPt = (hitPt[2] - offsetList[wallNum])/wallHeight
            # Record current wall height
            self.setCurrent('wall_height', wallHeight)
            # Determine which zone you are pointing at
            if (zPt > 0.8):
                # Do cornice operations
                if direct.fControl:
                    menuMode = 'cornice_color'
                elif direct.fAlt:
                    menuMode = 'cornice_orientation'
                else:
                    menuMode = 'cornice_texture'
            elif ((xPt < 0.3) or (xPt > 0.7)):
                # Do wall operations
                if direct.fControl:
                    menuMode = 'wall_color'
                elif direct.fAlt:
                    menuMode = 'wall_orientation'
                elif direct.fShift:
                    menuMode = 'wall_texture'
                else:
                    menuMode = 'wall_style'
            elif (zPt < 0.4):
                # Do door operations
                if direct.fControl:
                    menuMode = 'door_color'
                elif direct.fAlt:
                    menuMode = 'door_orientation'
                else:
                    menuMode = 'door_texture'
                # MRM: Temp for now
                menuMode = 'window_texture'
            else:
                # Do window operations
                if direct.fControl:
                    menuMode = 'window_color'
                elif direct.fAlt:
                    menuMode = 'window_orientation'
                elif direct.fShift:
                    menuMode = 'window_count'
                else:
                    menuMode = 'window_texture'
        return menuMode, wallNum

    def levelHandleMouse3Up(self):
        if self.activeMenu:
            self.activeMenu.removePieMenuTask()
        # Update panel color if appropriate
        if self.DNATarget:
            objClass = DNAGetClassType(self.DNATarget)
            if ((objClass.eq(DNA_WALL)) or
                (objClass.eq(DNA_WINDOWS)) or
                (objClass.eq(DNA_DOOR)) or
                (objClass.eq(DNA_CORNICE)) or
                (objClass.eq(DNA_PROP))
                ):
                self.panel.setCurrentColor(self.DNATarget.getColor())
            
    def setDNATargetColor(self, color):
        if self.DNATarget:
            self.DNATarget.setColor(color)
            self.replaceSelected()
            
    def setDNATargetCode(self, type, code):
        if (self.DNATarget != None) and (code != None):
            # Update code
            self.DNATarget.setCode(code)
        elif (self.DNATarget != None) and (code == None):
            # Delete object, record pertinant properties before
            # you delete the object so you can restore them later
            # Remove object
            if (type == 'cornice'):
                self.removeCornice(self.DNATarget, self.DNATargetParent)
            elif (type == 'landmark_door'):
                self.removeLandmarkDoor(self.DNATarget, self.DNATargetParent)
            elif (type == 'door'):
                self.removeDoor(self.DNATarget, self.DNATargetParent)
            elif (type == 'windows'):
                self.removeWindows(self.DNATarget, self.DNATargetParent)
            # Clear out DNATarget
            self.DNATarget = None
        elif (self.DNATarget == None) and (code != None):
            # Add new object
            if (type == 'cornice'):
                self.DNATarget = self.createCornice()
            elif (type == 'landmark_door'):
                self.DNATarget = self.createDoor()
            elif (type == 'door'):
                self.DNATarget = self.createDoor()
            elif (type == 'windows'):
                # Make sure window_count n.e. 0
                if self.getCurrent('window_count') == 0:
                    self.setCurrent(
                        'window_count',
                        self.getRandomWindowCount())
                # Now create the windows
                self.DNATarget = self.createWindows()
            if self.DNATarget:
                self.DNATargetParent.add(self.DNATarget)
        # Update visible representation
        self.replaceSelected()
        
    def setDNATargetOrientation(self, orientation):
        if (self.DNATarget != None) and (orientation != None):
            oldCode = self.DNATarget.getCode()[:-3]
            self.DNATarget.setCode(oldCode + '_' + orientation)
            self.replaceSelected()
            
    def setBuildingStyle(self, style):
        if (self.DNATarget != None) and (style != None):
            self.styleManager.setDNAFlatBuildingStyle(
                self.DNATarget, style, 
                width = self.DNATarget.getWidth(),
                name = self.DNATarget.getName())
            # MRM: Need to disable dna store warning
            self.replaceSelected()
            # Re-add replication hooks so we get right kind of copy
            self.addReplicationHooks(self.DNATarget)
            
    def setBuildingType(self, type):
        print 'setBuildingType: ', `type`
        
    def setBuildingWidth(self, width):
        if self.DNATarget:
            self.DNATarget.setWidth(width)
            self.replaceSelected()
            
    def setWindowCount(self, count):
        if (self.DNATarget != None) and (count != 0):
            self.DNATarget.setWindowCount(count)
        elif (self.DNATarget != None) and (count == 0):
            # Remove windows and clear out DNATarget
            self.removeWindows(self.DNATarget, self.DNATargetParent)
            self.DNATarget = None
        elif (self.DNATarget == None) and (count != 0):
            self.DNATarget = self.createWindows()
            self.DNATargetParent.add(self.DNATarget)
        self.replaceSelected()
            
    def setWallStyle(self, style):
        if (self.DNATarget != None) and (style != None):
            self.styleManager.setDNAWallStyle(
                self.DNATarget, style,
                self.DNATarget.getHeight())
            self.replaceSelected()

    def setNPColor(self, nodePath):
        """ This is used to set color of dnaNode subparts """
        def updateDNANodeColor(color, s = self, np = nodePath):
            # Update node color in DNASTORE here
            # dnaNode = s.findDNANode(np)
            pass
        esg = EntryScale.setColor(nodePath, updateDNANodeColor)
    
    # SELECTION FUNCTIONS
    def selectedNodePathHook(self, nodePath):
        """
        Hook called upon selection of a node path used to restrict
        selection to DNA Objects.  Press control to select any type of
        DNA Object, with no control key pressed, hook selects only
        DNA Root objects
        """
        # Clear out old root variables
        self.selectedDNARoot = None
        self.selectedNPRoot = None
        # Now process newly selected node path
        dnaParent = None
        dnaNode = self.findDNANode(nodePath)
        if direct.fControl:
            # Is the current node a DNA Object?
            if not dnaNode:
                # No it isn't, look for a parent DNA object
                dnaParent = self.findDNAParent(nodePath.getParent())
        else:
            # Is the current node a DNA Root object?
            if nodePath.getName()[-8:] != '_DNARoot':
                # No it isn't, look for a parent DNA Root object
                dnaParent = self.findDNARoot(nodePath.getParent())
        # Do we need to switch selection to a parent object?
        if dnaParent:
            # Yes, deselect currently selected node path
            direct.deselect(nodePath)
            # And select parent
            direct.select(dnaParent, direct.fShift)
        elif dnaNode:
            # We got a valid node path/DNA object, continue
            self.selectedNPRoot = nodePath
            self.selectedDNARoot = dnaNode
            # Reset last Code (for autoPositionGrid)
            if DNAClassEqual(dnaNode, DNA_STREET):
                self.snapList = OBJECT_SNAP_POINTS[dnaNode.getCode()]
        else:
            pass

    def deselectedNodePathHook(self, nodePath):
        # Clear out old root variables
        self.selectedDNARoot = None
        self.selectedNPRoot = None

    def findDNAParent(self, nodePath):
        """ Walk up a node path's ancestry looking for its DNA Root """
        # Check to see if current node is a dna object
        if self.findDNANode(nodePath):
            # Its a root!
            return nodePath
        else:
            # If reached the top: fail
            if not nodePath.hasParent():
                return 0
            else:
                # Try parent
                return self.findDNAParent(nodePath.getParent())

    def findDNARoot(self, nodePath):
        """ Walk up a node path's ancestry looking for its DNA Root """
        # Check current node's name for root marker
        if (nodePath.getName()[-8:] == '_DNARoot'):
            # Its a root!
            return nodePath
        else:
            # If reached the top: fail
            if not nodePath.hasParent():
                return None
            else:
                # Try parent
                return self.findDNARoot(nodePath.getParent())

    # MANIPULATION FUNCTIONS
    def keyboardRotateSelected(self, arrowDirection):
        """ Rotate selected objects using arrow keys """
        if ((arrowDirection == 'left') or (arrowDirection == 'up')):
            self.setLastAngle(self.getLastAngle() + SNAP_ANGLE)
        else:
            self.setLastAngle(self.getLastAngle() - SNAP_ANGLE)
        
        if (self.getLastAngle() < -180.0):
            self.setLastAngle(self.getLastAngle() + 360.0)
        elif (self.getLastAngle() > 180.0):
            self.setLastAngle(self.getLastAngle() - 360.0)
        # Move selected objects
        for selectedNode in direct.selected:
            selectedNode.setHpr(self.getLastAngle(), 0, 0)
        # Snap objects to grid and update DNA if necessary
        self.updateSelectedPose()

    def keyboardTranslateSelected(self, arrowDirection):
        gridToCamera = direct.grid.getMat(direct.camera)
        camXAxis = gridToCamera.xformVec(X_AXIS)
        xxDot = camXAxis.dot(X_AXIS)
        xzDot = camXAxis.dot(Z_AXIS)

        # what is the current grid spacing?
        deltaMove = direct.grid.gridSpacing

        # Compute the specified delta
        deltaPos = Vec3(0)
        if (abs(xxDot) > abs(xzDot)):
            if (xxDot < 0.0):
                deltaMove = -deltaMove
            # Compute delta
            if (arrowDirection == 'right'):
                deltaPos.setX(deltaPos[0] + deltaMove)
            elif (arrowDirection == 'left'):
                deltaPos.setX(deltaPos[0] - deltaMove)
            elif (arrowDirection == 'up'):
                deltaPos.setY(deltaPos[1] + deltaMove)
            elif (arrowDirection == 'down'):
                deltaPos.setY(deltaPos[1] - deltaMove)
        else:
            if (xzDot < 0.0):
                deltaMove = -deltaMove
            # Compute delta
            if (arrowDirection == 'right'):
                deltaPos.setY(deltaPos[1] - deltaMove)
            elif (arrowDirection == 'left'):
                deltaPos.setY(deltaPos[1] + deltaMove)
            elif (arrowDirection == 'up'):
                deltaPos.setX(deltaPos[0] + deltaMove)
            elif (arrowDirection == 'down'):
                deltaPos.setX(deltaPos[0] - deltaMove)

        # Move selected objects
        for selectedNode in direct.selected:
            # Move it
            selectedNode.setPos(direct.grid,
                                selectedNode.getPos(direct.grid) + deltaPos)
        # Snap objects to grid and update DNA if necessary
        self.updateSelectedPose()

    def keyboardXformSelected(self, arrowDirection):
        if direct.fControl:
            self.keyboardRotateSelected(arrowDirection)
        else:
            self.keyboardTranslateSelected(arrowDirection)

    # VISIBILITY FUNCTIONS
    def editDNAVisGroups(self):
        visGroups = self.getDNAVisGroups(self.NPToplevel)
        if visGroups:
            self.vgpanel = VisGroupsEditor(self, visGroups)
        else:
            showinfo('Vis Groups Editor','No DNA Vis Groups Found!')
        
    def getDNAVisGroups(self, nodePath):
        """ Find the highest level vis groups in the scene graph """
        dnaNode = self.findDNANode(nodePath)
        if dnaNode:
            if DNAClassEqual(dnaNode, DNA_VIS_GROUP):
                return [[nodePath, dnaNode]]
        childVisGroups = []
        children = nodePath.getChildrenAsList()
        for child in children:
            childVisGroups = (childVisGroups + self.getDNAVisGroups(child))
        return childVisGroups
    
    def showGrid(self,flag):
        """ toggle direct grid """
        if flag:
            direct.grid.enable()
        else:
            direct.grid.disable()

    # LEVEL MAP/MARKER FUNCTIONS
    def createLevelMaps(self):
        """
        Load up the various neighborhood maps
        """
        # For neighborhood maps
        self.levelMap = hidden.attachNewNode('level-map')
        self.activeMap = None
        self.mapDictionary = {}
        for neighborhood in NEIGHBORHOODS:
            self.createMap(neighborhood)

    def createMap(self, neighborhood):
	map = loader.loadModel('models/level_editor/' + neighborhood +
                               '_layout')
	map.arc().setTransition(TransparencyTransition(1))
	map.setColor(Vec4(1,1,1,.4))
        self.mapDictionary[neighborhood] = map
        # Make sure this item isn't pickable
        direct.addUnpickable(neighborhood + '_layout')

    def selectMap(self, neighborhood):
        if self.activeMap:
            self.activeMap.reparentTo(hidden)
        self.activeMap = self.mapDictionary[neighborhood]
        self.activeMap.reparentTo(self.levelMap)

    def toggleMapVis(self, flag):
        if flag:
            self.levelMap.reparentTo(direct.group)
        else:
            self.levelMap.reparentTo(hidden)

    def createInsertionMarker(self):
	self.insertionMarker = LineNodePath(self)
        self.insertionMarker.lineNode.setName('insertionMarker')
        self.insertionMarker.setColor(VBase4(0.785, 0.785, 0.5,1))
	self.insertionMarker.setThickness(1)
        self.insertionMarker.reset()
        self.insertionMarker.moveTo(-75,0,0)
        self.insertionMarker.drawTo(75,0,0)
        self.insertionMarker.moveTo(0,-75,0)
        self.insertionMarker.drawTo(0,75,0)
        self.insertionMarker.moveTo(0,0,-75)
        self.insertionMarker.drawTo(0,0,75)
        self.insertionMarker.create()

    def spawnInsertionMarkerTask(self):
        t = Task.Task(self.insertionMarkerTask)
        taskMgr.spawnTaskNamed(t, 'insertionMarkerTask')

    def insertionMarkerTask(self, state):
        self.insertionMarker.setPosHpr(direct.grid, 0,0,0, 0,0,0)
        # MRM: Why is this necessary?
        self.insertionMarker.setScale(1,1,1)
        return Task.cont

    # UTILITY FUNCTIONS
    def getRandomDictionaryEntry(self,dict):
        numKeys = len(dict)
        if numKeys > 0:
            keys = dict.keys()
            key = keys[randint(0,numKeys - 1)]
            return dict[key]
        else:
            return None

    def getRandomWindowCount(self):
        if ((self.lastWall != None) and (self.lastBuilding != None)):
            h = ROUND_INT(self.lastWall.getHeight())
            w = ROUND_INT(self.lastBuilding.getWidth())
            # Otherwise....
            if w == 5:
                # 5 ft walls can have 1 window
                return 1
            elif h == 10:
                # All other 10 ft high bldgs can have 1 or 2
                return randint(1,2)
            else:
                # All others can have 1 - 4
                return randint(1,4)
        else:
            return 1

    def autoPositionGrid(self):
        taskMgr.removeTasksNamed('autoPositionGrid')
	# Move grid to prepare for placement of next object
	selectedNode = direct.selected.last
        if selectedNode:
            dnaNode = self.findDNANode(selectedNode)
            if dnaNode == None:
                return
            nodeClass = DNAGetClassType(dnaNode)
            deltaPos = Point3(20,0,0)
            deltaHpr = VBase3(0)
            if nodeClass.eq(DNA_FLAT_BUILDING):
                deltaPos.setX(dnaNode.getWidth())
            elif nodeClass.eq(DNA_STREET):
                objectCode = dnaNode.getCode()
                deltas = self.getNextSnapPoint()
                deltaPos.assign(deltas[0])
                deltaHpr.assign(deltas[1])
            elif nodeClass.eq(DNA_LANDMARK_BUILDING):
                objectCode = dnaNode.getCode()
                if objectCode[-2:-1] == 'A':
                    deltaPos.setX(25.0)
                elif objectCode[-2:-1] == 'B':
                    deltaPos.setX(15.0)
                elif objectCode[-2:-1] == 'C':
                    deltaPos.setX(20.0)

            # Position grid for placing next object
            # Eventually we need to setHpr too
            t = direct.grid.lerpPosHpr(
                deltaPos, deltaHpr, 0.25,
                other = selectedNode,
                blendType = 'easeInOut',
                task = 'autoPositionGrid')
            t.deltaPos = deltaPos
            t.deltaHpr = deltaHpr
            t.selectedNode = selectedNode
            t.uponDeath = self.autoPositionCleanup
            
	# Also move the camera
	taskMgr.removeTasksNamed('autoMoveDelay')
	handlesToCam = direct.widget.getPos(direct.camera)
	handlesToCam = handlesToCam * ( direct.dr.near/handlesToCam[1])
	if ((abs(handlesToCam[0]) > (direct.dr.nearWidth * 0.4)) or
            (abs(handlesToCam[2]) > (direct.dr.nearHeight * 0.4))):
            taskMgr.removeTasksNamed('manipulateCamera')
            direct.cameraControl.centerCamIn(0.5)

    def autoPositionCleanup(self,state):
        direct.grid.setPosHpr(state.selectedNode, state.deltaPos,
                              state.deltaHpr)
        if direct.grid.getHprSnap():
            # Clean up grid angle
            direct.grid.setH(ROUND_TO(direct.grid.getH(), SNAP_ANGLE))

    def getNextSnapPoint(self):
        """ Pull next pos hpr deltas off of snap list then rotate list """
        if self.snapList:
            deltas = self.snapList[0]
            # Rotate list by one
            self.snapList = self.snapList[1:] + self.snapList[:1]
            return deltas
        else:
            return (ZERO_VEC, ZERO_VEC)

    def getWallIntersectionPoint(self, selectedNode):
	"""
        Return point of intersection between building's wall and line from cam
        through mouse. 
        """
        # Find mouse point on near plane
    	mouseX = direct.dr.mouseX
  	mouseY = direct.dr.mouseY
   	nearX = (math.tan(deg2Rad(direct.dr.fovH)/2.0) *
                 mouseX * direct.dr.near)
   	nearZ = (math.tan(deg2Rad(direct.dr.fovV)/2.0) *
                 mouseY * direct.dr.near)
        # Initialize points
   	mCam2Wall = direct.camera.getMat(selectedNode)
	mouseOrigin = Point3(0)
   	mouseOrigin.assign(mCam2Wall.getRow3(3))
	mouseDir = Vec3(0)
   	mouseDir.set(nearX, direct.dr.near, nearZ)
   	mouseDir.assign(mCam2Wall.xformVec(mouseDir))
        # Calc intersection point
        return planeIntersect(mouseOrigin, mouseDir, ZERO_POINT, NEG_Y_AXIS)

    def getGridSnapIntersectionPoint(self):
	"""
        Return point of intersection between ground plane and line from cam
        through mouse. Return false, if nothing selected. Snap to grid.
        """
        return direct.grid.computeSnapPoint(self.getGridIntersectionPoint())

    def getGridIntersectionPoint(self):
	"""
        Return point of intersection between ground plane and line from cam
        through mouse. Return false, if nothing selected
        """
        # Find mouse point on near plane
    	mouseX = direct.dr.mouseX
  	mouseY = direct.dr.mouseY
   	nearX = (math.tan(deg2Rad(direct.dr.fovH)/2.0) *
                 mouseX * direct.dr.near)
   	nearZ = (math.tan(deg2Rad(direct.dr.fovV)/2.0) *
                 mouseY * direct.dr.near)
        # Initialize points
   	mCam2Grid = direct.camera.getMat(direct.grid)
	mouseOrigin = Point3(0)
   	mouseOrigin.assign(mCam2Grid.getRow3(3))
	mouseDir = Vec3(0)
   	mouseDir.set(nearX, direct.dr.near, nearZ)
   	mouseDir.assign(mCam2Grid.xformVec(mouseDir))
        # Calc intersection point
        return planeIntersect(mouseOrigin, mouseDir, ZERO_POINT, Z_AXIS)

    def jumpToInsertionPoint(self):
        """ Move selected object to insertion point """
        selectedNode = direct.selected.last
        if selectedNode:
            # Check if its a dna node
            dnaNode = self.findDNANode(selectedNode)
            if dnaNode:
                # Place the new node path at the current grid origin
                selectedNode.setPos(direct.grid,0,0,0)
                # Initialize angle to match last object
                selectedNode.setHpr(self.getLastAngle(), 0, 0)
                # Now update DNA pos and hpr to reflect final pose
                dnaNode.setPos(selectedNode.getPos())
                dnaNode.setHpr(selectedNode.getHpr())
                # Update grid to get ready for the next object
                self.autoPositionGrid()

    # STYLE/DNA FILE FUNCTIONS
    def loadSpecifiedDNAFile(self):
        f = Filename(self.styleManager.stylePathPrefix +
                     '/alpha/DIRECT/LevelEditor/DNAFiles')
        path = os.path.join(f.toOsSpecific(), self.outputDir)
        if not os.path.isdir(path):
            print 'LevelEditor Warning: Invalid default DNA directory!'
            print 'Using current directory'
            path = '.'
        dnaFilename = askopenfilename(
            defaultextension = '.dna',
            filetypes = (('DNA Files', '*.dna'),('All files', '*')),
            initialdir = path,
            title = 'Load DNA File',
            parent = self.panel.component('hull'))
        if dnaFilename:
            self.loadDNAFromFile(dnaFilename)

    def saveToSpecifiedDNAFile(self):
        f = Filename(self.styleManager.stylePathPrefix +
                     '/alpha/DIRECT/LevelEditor/DNAFiles')
        path = os.path.join(f.toOsSpecific(), self.outputDir)
        if not os.path.isdir(path):
            print 'LevelEditor Warning: Invalid DNA save directory!'
            print 'Using current directory'
            path = '.'
        dnaFilename = asksaveasfilename(
            defaultextension = '.dna',
            filetypes = (('DNA Files', '*.dna'),('All files', '*')),
            initialdir = path,
            title = 'Save DNA File as',
            parent = self.panel.component('hull'))
        if dnaFilename:
            self.outputDNA(dnaFilename)

    def loadDNAFromFile(self, filename):
	# Reset level, destroying existing scene/DNA hierarcy
	self.reset(fDeleteToplevel = 1, fCreateToplevel = 0)
	# Now load in new file
	newNPToplevel = loadDNAFile(DNASTORE, filename, CSDefault, 1)
 	# Make sure the topmost file DNA object gets put under DNARoot
 	newDNAToplevel = self.findDNANode(newNPToplevel)
        # Update toplevel variables
        self.createToplevel(newDNAToplevel, newNPToplevel)
        # Create visible representations of all the paths and battle cells
        self.createSuitPaths()
        self.hideSuitPaths()
        self.createBattleCells()
        self.hideBattleCells()

    def outputDNADefaultFile(self):
        f = Filename(self.styleManager.stylePathPrefix +
                     '/alpha/DIRECT/LevelEditor/DNAFiles')
        file = os.path.join(f.toOsSpecific(), self.outputDir, self.outputFile)
        self.outputDNA(file)
        
    def outputDNA(self,filename):
	print 'Saving DNA to: ', filename
        binaryFilename = Filename(filename)
        binaryFilename.setBinary()
	self.DNAData.writeDna(binaryFilename, Notify.out(),DNASTORE)
        
    def saveColor(self):
        self.appendColorToColorPaletteFile(self.panel.colorEntry.get())

    def appendColorToColorPaletteFile(self, color):
        obj = self.DNATarget
        if obj:
            classType = DNAGetClassType(obj)
            if classType.eq(DNA_WALL):
                tag = 'wall_color:'
            elif classType.eq(DNA_WINDOWS):
                tag = 'window_color:'
            elif classType.eq(DNA_DOOR):
                tag = 'door_color:'
            elif classType.eq(DNA_CORNICE):
                tag = 'cornice_color:'
            elif classType.eq(DNA_PROP):
                tag = 'prop_color:'
            else:
                return
            # Valid type, add color to file
            filename = self.neighborhood + '_colors.txt'
            fname = Filename(self.styleManager.stylePathPrefix +
                             '/alpha/DIRECT/LevelEditor/StyleFiles/' +
                             filename)
            f = open(fname.toOsSpecific(), 'a')
            f.write('%s Vec4(%.2f, %.2f, %.2f, 1.0)\n' %
                    (tag,
                     color[0]/255.0,
                     color[1]/255.0,
                     color[2]/255.0))
            f.close()

    def saveWallStyle(self):
        if self.lastWall:
            # Valid wall, add style to file
            filename = self.neighborhood + '_wall_styles.txt'
            fname = Filename(self.styleManager.stylePathPrefix +
                             '/alpha/DIRECT/LevelEditor/StyleFiles/' +
                             filename)
            f = open(fname.toOsSpecific(), 'a')
            # Add a blank line
            f.write('\n')
            # Now output style details to file
            style = DNAWallStyle(wall = self.lastWall)
            style.output(f)
            # Close the file
            f.close()

    def saveBuildingStyle(self):
        dnaObject = self.selectedDNARoot
        if dnaObject:
            if DNAClassEqual(dnaObject, DNA_FLAT_BUILDING):
                # Valid wall, add style to file
                filename = self.neighborhood + '_building_styles.txt'
                fname = Filename(self.styleManager.stylePathPrefix +
                                 '/alpha/DIRECT/LevelEditor/StyleFiles/' +
                                 filename)
                f = open(fname.toOsSpecific(), 'a')
                # Add a blank line
                f.write('\n')
                # Now output style details to file
                style = DNAFlatBuildingStyle(building = dnaObject)
                style.output(f)
                # Close the file
                f.close()
                return
        print 'Must select building before saving building style'

    # GET/SET
    # DNA Object elements
    def getWall(self, dnaFlatBuilding, wallNum):
	wallCount = 0
        for i in range(dnaFlatBuilding.getNumChildren()):
            child = dnaFlatBuilding.at(i)
            if DNAClassEqual(child, DNA_WALL):
                if wallCount == wallNum:
                    return child
                wallCount = wallCount + 1
        # Not found
	return None

    def computeWallNum(self, aDNAFlatBuilding, hitPt):
        """
        Given a hitPt, return wall number if cursor is over building
        Return -1 if cursor is outside of building
        """
        xPt = hitPt[0]
        zPt = hitPt[2]
        # Left or right of building
        if ((xPt < 0) or (xPt > aDNAFlatBuilding.getWidth())):
            return -1
        # Below the building
        if zPt < 0:
            return -1
        # Above z = 0 and within wall width, check height of walls
	heightList, offsetList = DNAGetWallHeights(aDNAFlatBuilding)
	wallNum = 0
        for i in range(len(heightList)):
            # Compute top of wall segment
            top = offsetList[i] + heightList[i]
            if zPt < top:
                return wallNum
            wallNum = wallNum + 1
	return -1

    def getWindowCount(self, dnaWall):
	windowCount = 0
        for i in range(dnaWall.getNumChildren()):
            child = dnaWall.at(i)
            if DNAClassEqual(child, DNA_WINDOWS):
                windowCount = windowCount + 1
	# Not found
	return windowCount

    # Style manager edit mode
    def setEditMode(self, neighborhood):
        self.neighborhood = neighborhood
        self.neighborhoodCode = NEIGHBORHOOD_CODES[self.neighborhood]
        self.outputFile = neighborhood + '_working.dna'
        if neighborhood == 'toontown_central':
            self.outputDir = 'ToontownCentral'
        elif neighborhood == 'donalds_dock':
            self.outputDir = 'DonaldsDock'
        elif neighborhood == 'minnies_melody_land':
            self.outputDir = 'MinniesMelodyLand'
        elif neighborhood == 'the_burrgh':
            self.outputDir = 'TheBurrrgh'
        self.panel.editMenu.selectitem(neighborhood)
        self.styleManager.setEditMode(neighborhood)
        self.selectMap(neighborhood)
        
    def getEditMode(self):
        return self.styleManager.getEditMode()
    
    # Level Style Attributes
    def __getitem__(self,attribute):
        """ Return top level entry in attribute dictionary """
        return self.styleManager.attributeDictionary[attribute]
    
    def getAttribute(self, attribute):
        """ Return specified attribute for current neighborhood """
        return self.styleManager.getAttribute(attribute)
    
    def getCurrent(self, attribute):
        """ Return neighborhood's current selection for specified attribute """
        return self.getAttribute(attribute).getCurrent()
    
    def setCurrent(self, attribute, newCurrent):
        """ Set neighborhood's current selection for specified attribute """
        self.getAttribute(attribute).setCurrent(newCurrent, fEvent = 0)
    
    def getMenu(self, attribute):
        """ Return neighborhood's Pie Menu object for specified attribute """
        return self.getAttribute(attribute).getMenu()
    
    def getDict(self, attribute):
        """ Return neighborhood's Dictionary for specified attribute """
        return self.getAttribute(attribute).getDict()
    
    def getList(self, attribute):
        """ Return neighborhood's List for specified attribute """
        return self.getAttribute(attribute).getList()
    
    # DNA variables
    def getDNAData(self):
        return self.DNAData
    
    def getDNAToplevel(self):
        return self.DNAToplevel
    
    def getDNAParent(self):
        return self.DNAParent
    
    def getDNATarget(self):
        return self.DNATarget
    
    # Node Path variables
    def getNPToplevel(self):
        return self.NPToplevel
    
    def getNPParent(self):
        return self.NPParent
    
    # Count of groups added to level
    def setGroupNum(self,num):
	self.groupNum = num
    
    def getGroupNum(self):
	return self.groupNum
    
    # Angle of last object added to level
    def setLastAngle(self, angle):
        self.lastAngle = angle
    
    def getLastAngle(self):
        return self.lastAngle

    def drawSuitEdge(self, edge, parent):
        # Draw a line from start to end
        edgeLine = LineNodePath(parent)
        edgeLine.lineNode.setName('suitEdge')
        edgeLine.setColor(VBase4(0.0, 0.0, 0.5 ,1))
        edgeLine.setThickness(1)
        edgeLine.reset()
        # We need to draw the arrow relative to the parent, but the
        # point positions are relative to the NPToplevel. So get the
        # start and end positions relative to the parent, then draw
        # the arrow using those points
        tempNode = self.NPToplevel.attachNewNode('tempNode')
        mat = self.NPToplevel.getMat(parent)
        relStartPos = Point3(mat.xformPoint(edge.getStartPoint().getPos()))
        relEndPos = Point3(mat.xformPoint(edge.getEndPoint().getPos()))
        edgeLine.drawArrow(relStartPos,
                           relEndPos,
                           15, # arrow angle
                           1) # arrow length
        edgeLine.create()
        return edgeLine

    def drawSuitPoint(self, pos, type, parent):
        marker = self.suitPointMarker.instanceTo(parent)
        marker.setPos(pos)
        if (type == DNASuitPoint.STREETPOINT):
            marker.setColor(0,0,0.6)
            marker.setScale(0.25)
        elif (type == DNASuitPoint.FRONTDOORPOINT):
            marker.setColor(0,0,1)
            marker.setScale(0.5)
        elif (type == DNASuitPoint.SIDEDOORPOINT):
            marker.setColor(0,0.2,0.4)
            marker.setScale(0.5)
        # v is relative to the grid
        return marker

    def placeSuitPoint(self):
        v = self.getGridSnapIntersectionPoint()
        # get the absolute pos relative to the top level. That is what gets stored in the point
        mat = direct.grid.getMat(self.NPToplevel)
        absPos = Point3(mat.xformPoint(v))
        print 'Suit point: ' + `absPos`
        # Store the point in the DNA. If this point is already in there, it returns
        # the existing point
        suitPoint = DNASTORE.storeSuitPoint(self.currentSuitPointType, absPos)
        if not self.pointDict.has_key(suitPoint):
            marker = self.drawSuitPoint(absPos, self.currentSuitPointType, self.NPToplevel)
            self.pointDict[suitPoint] = marker
        self.currentSuitPointIndex = suitPoint.getIndex()

        if self.startSuitPoint:
            self.endSuitPoint = suitPoint
            # Make a new dna edge
            if DNAClassEqual(self.DNAParent, DNA_VIS_GROUP):
                zoneId = self.DNAParent.getName()
                suitEdge = DNASuitEdge(self.startSuitPoint, self.endSuitPoint, zoneId)
                DNASTORE.storeSuitEdge(suitEdge)
                # Add the edge to the current vis group so it can be written out
                self.DNAParent.addSuitEdge(suitEdge)
                # Draw a line to represent the edge
                edgeLine = self.drawSuitEdge(suitEdge, self.NPParent)
                # Store the line in a dict so we can hide/show them
                self.edgeDict[suitEdge] = edgeLine
                print 'Added dnaSuitEdge to zone: ' + zoneId
            else:
                print 'Error: DNAParent is not a dnaVisGroup. Did not add edge'
            # Reset
            self.startSuitPoint = None
            self.endSuitPoint = None

        else:
            # First point, store it
            self.startSuitPoint = suitPoint

    def drawBattleCell(self, cell, parent):
        marker = self.battleCellMarker.instanceTo(parent)
        # Greenish
        marker.setColor(0.25,0.75,0.25,0.5)
        marker.setPos(cell.getPos())
        # scale to radius which is width/2
        marker.setScale(cell.getWidth()/2.0,
                        cell.getHeight()/2.0,
                        1)
        return marker

    def placeBattleCell(self):
        # Store the battle cell in the current vis group
        if not DNAClassEqual(self.DNAParent, DNA_VIS_GROUP):
            print 'Error: DNAParent is not a dnaVisGroup. Did not add battle cell'
            return

        v = self.getGridSnapIntersectionPoint()
        mat = direct.grid.getMat(self.NPParent)
        absPos = Point3(mat.xformPoint(v))
        if (self.currentBattleCellType == '20w 20l'):
            cell = DNABattleCell(20, 20, absPos)
        elif (self.currentBattleCellType == '20w 30l'):
            cell = DNABattleCell(20, 30, absPos)
        elif (self.currentBattleCellType == '30w 20l'):
            cell = DNABattleCell(30, 20, absPos)
        elif (self.currentBattleCellType == '30w 30l'):
            cell = DNABattleCell(30, 30, absPos)
        # Store the battle cell in the storage
        DNASTORE.storeBattleCell(cell)
        # Draw the battle cell
        marker = self.drawBattleCell(cell, self.NPParent)
        # Keep a handy dict of the visible markers
        self.cellDict[cell] = marker
        # Store the battle cell in the current vis group
        self.DNAParent.addBattleCell(cell)

    def createSuitPaths(self):
        # Points
        numPoints = DNASTORE.getNumSuitPoints()
        for i in range(numPoints):
            point = DNASTORE.getSuitPoint(i)
            marker = self.drawSuitPoint(point.getPos(), point.getPointType(), self.NPToplevel)
            self.pointDict[point] = marker

        # Edges
        visGroups = self.getDNAVisGroups(self.NPToplevel)
        for visGroup in visGroups:
            np = visGroup[0]
            dnaVisGroup = visGroup[1]
            numSuitEdges = dnaVisGroup.getNumSuitEdges()
            for i in range(numSuitEdges):
                edge = dnaVisGroup.getSuitEdge(i)
                edgeLine = self.drawSuitEdge(edge, np)
                self.edgeDict[edge] = edgeLine

    def resetPathMarkers(self):
        for edge, edgeLine in self.edgeDict.items():
            edgeLine.remove()
        self.edgeDict = {}
        for point, marker in self.pointDict.items():
            marker.remove()
        self.pointDict = {}
                
    def hideSuitPaths(self):
        for edge, edgeLine in self.edgeDict.items():
            edgeLine.hide()
        for point, marker in self.pointDict.items():
            marker.hide()

    def showSuitPaths(self):
        for edge, edgeLine in self.edgeDict.items():
            edgeLine.show()
        for point, marker in self.pointDict.items():
            marker.show()

    def createBattleCells(self):
        # Edges
        visGroups = self.getDNAVisGroups(self.NPToplevel)
        for visGroup in visGroups:
            np = visGroup[0]
            dnaVisGroup = visGroup[1]
            numCells = dnaVisGroup.getNumBattleCells()
            for i in range(numCells):
                cell = dnaVisGroup.getBattleCell(i)
                marker = self.drawBattleCell(cell, np)
                self.cellDict[cell] = marker

    def resetBattleCellMarkers(self):
        for cell, marker in self.cellDict.items():
            marker.remove()
        self.cellDict = {}

    def hideBattleCells(self):
        for cell, marker in self.cellDict.items():
            marker.hide()

    def showBattleCells(self):
        for cell, marker in self.cellDict.items():
            marker.show()
    
    def colorZones(self):
        # Give each zone a random color to see them better
        visGroups = self.getDNAVisGroups(self.NPToplevel)
        for visGroup in visGroups:
            np = visGroup[0]
            np.setColor(0.5 + random()/2.0,
                        0.5 + random()/2.0,
                        0.5 + random()/2.0)
            
   

            
class LevelStyleManager:
    """Class which reads in style files and manages class variables"""
    def __init__(self):
        # Used to locate the alpha mount on windows (i.e. on what drive)
        self.stylePathPrefix = base.config.GetString('style-path-prefix', '')
        # The main dictionary holding all attribute objects
        self.attributeDictionary = {}
        # Create the style samples
        self.createWallStyleAttributes()
        self.createBuildingStyleAttributes()
        self.createColorAttributes()
        self.createDNAAttributes()
        self.createMiscAttributes()

    # WALL STYLE FUNCTIONS
    def createWallStyleAttributes(self):
        """
        Create a wallStyle entry in the attribute dictionary
        This will be a dictionary of style attributes, one per neighborhood
        """
        # First create an empty dictionary
        dict = self.attributeDictionary['wall_style'] = {}
        # Create a attribute object for each neighborhood
        for neighborhood in NEIGHBORHOODS:
            attribute = LevelAttribute('wall_style')
            attribute.setDict(
                # Create a wall style dictionary for each neighborhood
                self.createWallStyleDictionary(neighborhood))
            # Using this dictionary, create color pie menus
            attribute.setMenu(
                self.createWallStyleMenu(neighborhood, attribute.getDict()))
            dict[neighborhood] = attribute
    
    def createWallStyleDictionary(self, neighborhood):
        """
        Create a dictionary of wall styles for a neighborhood
        """
        filename = neighborhood + '_wall_styles.txt'
        print 'Loading wall styles from: ' + filename
        styleData = self.getStyleFileData(filename)
        return self.initializeWallStyleDictionary(styleData, neighborhood)    
    
    def initializeWallStyleDictionary(self, styleData, neighborhood):
        """
        Fill in the wall style dictionary with data from the style file
        """
        styleDictionary = {}
        styleCount = 0
        code = NEIGHBORHOOD_CODES[neighborhood]
        while styleData:
            l = styleData[0]
            if l == 'wallStyle':
                # Start of new style, strip off first line then extract style
                style, styleData = self.extractWallStyle(styleData)
                style.name = code + '_wall_style_' + `styleCount`
                # Store style in dictionary
                styleDictionary[style.name] = style
                styleCount = styleCount + 1
            # Move to next line
            styleData = styleData[1:]
        return styleDictionary

    def extractWallStyle(self, styleData):
        """
        Pull out one style from a list of style data.  Will keep
        processing data until endWallStyle of end of data is reached.
        Returns a wall style and remaining styleData.
        """
        # Create default style
        style = DNAWallStyle()
        # Strip off first line
        styleData = styleData[1:]
        while styleData:
            l = styleData[0]
            if l == 'endWallStyle':
                # End of style found, break out of while loop
                # Note, endWallStyle line is *not* stripped off
                return style, styleData
            else:
                pair = map(string.strip, l.split(':'))
                if style.__dict__.has_key(pair[0]):
                    # Convert colors and count strings to numerical values
                    if ((string.find(pair[0],'_color') >= 0) or
                        (string.find(pair[0],'_count') >= 0)):
                        style[pair[0]] = eval(pair[1])
                    else:
                        style[pair[0]] = pair[1]
                else:
                    print 'getStyleDictionaryFromStyleData: Invalid Key'
                    print pair[0]
            styleData = styleData[1:]
        # No end of style found, return style data as is
        return style, None
        
    def createWallStyleMenu(self, neighborhood, dictionary):
        """
        Create a wall style pie menu
        """
	numItems = len(dictionary)
	newStyleMenu = hidden.attachNewNode(neighborhood + '_style_menu')
	radius = 0.7
	angle = deg2Rad(360.0/numItems)
        keys = dictionary.keys()
        keys.sort()
        styles = map(lambda x, d = dictionary: d[x], keys)
        sf = 0.03
        aspectRatio = (direct.dr.width/float(direct.dr.height))
        for i in range(numItems):
            # Get the node
            node = self.createWallStyleSample(styles[i])
            bounds = node.getBounds()
            center = bounds.getCenter()
            center = center * sf
            # Reposition it
            node.setPos((radius * math.cos(i * angle)) - center[0],
                        0.0,
                        ((radius * aspectRatio * math.sin(i * angle)) -
                         center[2]))
            # Scale it
            node.setScale(sf)
            # Add it to the styleMenu
            node.reparentTo(newStyleMenu)
	# Scale the whole shebang down by 0.5
	newStyleMenu.setScale(0.5)
        # Create and return a pie menu
        return PieMenu(newStyleMenu, styles)
    
    def createWallStyleSample(self, wallStyle):
        """
        Create a style sample using the DNA info in the style
        """
        bldg = DNAFlatBuilding()
        bldgStyle = DNAFlatBuildingStyle(styleList = [(wallStyle, 10.0)])
        self.setDNAFlatBuildingStyle(bldg, bldgStyle, width = 12.0,
                                     name = 'wall_style_sample')
        return bldg.traverse(hidden, DNASTORE, 1)
    
    # BUILDING STYLE FUNCTIONS
    def createBuildingStyleAttributes(self):
        """
        Create a buildingStyle entry in the attribute dictionary
        This will be a dictionary of style attributes, one per neighborhood
        """
        # First create an empty dictionary
        styleDict = self.attributeDictionary['building_style'] = {}
        # Create an attribute object of all styles for each neighborhood
        for neighborhood in NEIGHBORHOODS:
            attribute = LevelAttribute('building_style')
            attribute.setDict(
                # Create a wall style dictionary for each neighborhood
                self.createBuildingStyleDictionary(neighborhood))
            # Using this dictionary, create color pie menus
            attribute.setMenu(
                self.createBuildingStyleMenu(neighborhood,
                                             attribute.getDict()))
            styleDict[neighborhood] = attribute
            
        # Now create attribute entries sorted according to building
        # height styles
        attrDict = {}
        # Create an attribute dictionary entry for each building height type
        for type in BUILDING_TYPES:
            key = 'building_style_' + type
            attrDict[type] = self.attributeDictionary[key] = {}
        # For each neighborhood create attribute for each height type
        for neighborhood in NEIGHBORHOODS:
            # Temp lists to accumulate neighborhood styles
            # sorted by height type
            styleLists = {}
            for type in BUILDING_TYPES:
                styleLists[type] = []
                
            # Sort through the styles and store in separate lists
            for style in styleDict[neighborhood].getList():
                heightType = string.strip(string.split(style.name, ':')[1])
                styleLists[heightType].append(style)
                
            # Now put these lists in appropriate neighborhood attribute
            for type in BUILDING_TYPES:
                attribute = LevelAttribute('building_style_' + type)
                attribute.setList(styleLists[type])
                # Store them according to neighborhood
                attrDict[type][neighborhood] = attribute
    
    def createBuildingStyleDictionary(self, neighborhood):
        """
        Create a dictionary of wall styles for a neighborhood
        """
        filename = neighborhood + '_building_styles.txt'
        print 'Loading building styles from: ' + filename
        styleData = self.getStyleFileData(filename)
        return self.initializeBuildingStyleDictionary(styleData, neighborhood)

    def initializeBuildingStyleDictionary(self, styleData, neighborhood):
        """
        Fill in the building style dictionary with data from the style file
        """
        # Create a dictionary of all building styles, this will later be
        # split out into a separate dictionary for each building type
        # e.g. 20, 10-10, 10-20....
        bldgStyleDictionary = {}
        styleCount = 0
        code = NEIGHBORHOOD_CODES[neighborhood]
        while styleData:
            # Pull out first line
            l = styleData[0]
            if l[:13] == 'buildingStyle':
                # Start with empty style list
                bldgStyle = DNAFlatBuildingStyle(styleList = [])
                # Extract height information found at end of line
                heightCode = string.strip(string.split(l, ':')[1])
                heightList = map(string.atof, string.split(heightCode, '_'))
                # Construct name for building style.  Tack on height code
                # to be used later to split styles by heightCode
                bldgStyle.name = (
                    code + '_building_style_' + `styleCount` +
                    ':' + heightCode)
                # Increment counter
                styleCount = styleCount + 1
                # Reset wall counter to zero
                wallCount = 0
                
            elif l == 'endBuildingStyle':
                # Done, add new style to dictionary
                bldgStyleDictionary[bldgStyle.name] = bldgStyle
                
            elif l[:9] == 'wallStyle':
                # Beginning of next wall style  
                wallStyle, styleData = self.extractWallStyle(styleData)
                wallStyle.name = bldgStyle.name + '_wall_' + `wallCount`
                try:
                    height = heightList[wallCount]
                except IndexError:
                    height = 10.0
                # Add wall style to building style
                bldgStyle.add(wallStyle, height)
                # Increment wall counter
                wallCount = wallCount + 1
            # Move to next line
            styleData = styleData[1:]
        return bldgStyleDictionary
        
    def createBuildingStyleMenu(self, neighborhood, dictionary):
        """
        Create a wall style pie menu
        """
	numItems = len(dictionary)
	newStyleMenu = hidden.attachNewNode(neighborhood + '_style_menu')
	radius = 0.7
	angle = deg2Rad(360.0/numItems)
        keys = dictionary.keys()
        keys.sort()
        styles = map(lambda x, d = dictionary: d[x], keys)
        sf = 0.02
        aspectRatio = (direct.dr.width/float(direct.dr.height))
        for i in range(numItems):
            # Get the node
            node = self.createBuildingStyleSample(styles[i])
            bounds = node.getBounds()
            center = bounds.getCenter()
            center = center * sf
            # Reposition it
            node.setPos((radius * math.cos(i * angle)) - center[0],
                        0.0,
                        ((radius * aspectRatio * math.sin(i * angle)) -
                         center[2]))
            # Scale it
            node.setScale(sf)
            # Add it to the styleMenu
            node.reparentTo(newStyleMenu)
	# Scale the whole shebang down by 0.5
	newStyleMenu.setScale(0.5)
        # Create and return a pie menu
        return PieMenu(newStyleMenu, styles)
    
    def createBuildingStyleSample(self, bldgStyle):
        """
        Create a style sample using the DNA info in the style
        """
        bldg = DNAFlatBuilding()
        self.setDNAFlatBuildingStyle(bldg, bldgStyle, width = 10.0,
                                     name = 'building_style_sample')
        return bldg.traverse(hidden, DNASTORE, 1)
    
    def setDNAFlatBuildingStyle(self, fb, bldgStyle, width = 10.0,
                                name = 'building'):
        """ Set DNAFlatBuilding style. """
        # Remove flat building's children
        DNARemoveChildren(fb)
        # Update the name
        fb.setName(name)
        # Create the walls
        styleList = bldgStyle.styleList
        heightList = bldgStyle.heightList
        for i in range(len(styleList)):
            wallStyle = styleList[i]
            # Get Style
            if not wallStyle:
                # If set to None use default style
                wallStyle = DNAWallStyle()
            # Try to get height
            try:
                wallHeight = heightList[i]
            except IndexError:
                wallHeight = 10.0
            # Create wall accordingly
            wall = DNAWall()
            self.setDNAWallStyle(wall, wallStyle, wallHeight)
            # Add it to building DNA
            fb.add(wall)
        # Set the buildings width
        fb.setWidth(width)

    def setDNAWallStyle(self, wall, style, height = 10.0):
        """ Set DNAWall to input style. """
        # Remove wall's children
        DNARemoveChildren(wall)
        # Update wall attributes
        wall.setCode(style['wall_texture'])
        wall.setColor(style['wall_color'])
        wall.setHeight(height)
        # Add windows if necessary
        if style['window_texture']:
            windows = DNAWindows()
            windows.setWindowCount(style['window_count'])
            # Set window's attributes
            windows.setCode(style['window_texture'])
            windows.setColor(style['window_color'])
            # Add windows to the wall
            wall.add(windows)
            # Add a window awning if necessary
            if style['window_awning_texture']:
                awning = DNAProp()
                # Update awning's attributes
                awning.setCode(style['window_awning_texture'])
                awning.setColor(style['window_awning_color'])
                # Add awning to window
                windows.add(awning)
        # Add a door if necessary
        if style['door_texture']:
            door = DNADoor()
            # Set the door's attributes
            door.setCode(style['door_texture'])
            door.setColor(style['door_color'])
            # Add door to wall
            wall.add(door)
            # Add a door awning if necessary
            if style['door_awning_texture']:
                awning = DNAProp()
                awning.setCode(style['door_awning_texture'])
                awning.setColor(style['door_awning_color'])
                door.add(awning)
        # And a cornice if necessary
        if style['cornice_texture']:
            cornice = DNACornice()
            # Set the cornice's attributes
            cornice.setCode(style['cornice_texture'])
            cornice.setColor(style['cornice_color'])
            # Add cornice to wall
            wall.add(cornice)

    def printFlatBuildingStyle(self, building):
        for i in range(building.getNumChildren()):
            child = building.at(i)
            if DNAClassEqual(child, DNA_WALL):
                self.printWallStyle(child)
                
    def printWallStyle(self, wall):
        print 'wall_texture: ' + wall.getCode()
        color = wall.getColor()
        print ('wall_color: Vec4(%.3f, %.3f, %.3f, 1.0)' %
               (color[0], color[1], color[2]))
        for i in range(wall.getNumChildren()):
            child = wall.at(i)
            if DNAClassEqual(child, DNA_WINDOWS):
                print 'window_texture: ' + child.getCode()
                color = child.getColor()
                print ('window_color: Vec4(%.3f, %.3f, %.3f, 1.0)' %
                       (color[0], color[1], color[2]))
                # MRM: Check for awnings here
            elif DNAClassEqual(child, DNA_DOOR):
                print 'door_texture: ' + child.getCode()
                color = child.getColor()
                print ('door_color: Vec4(%.3f, %.3f, %.3f, 1.0)' %
                       (color[0], color[1], color[2]))
                # MRM: Check for awnings here
            elif DNAClassEqual(child, DNA_CORNICE):
                print 'cornice_texture: ' + child.getCode()
                color = child.getColor()
                print ('cornice_color: Vec4(%.3f, %.3f, %.3f, 1.0)' %
                       (color[0], color[1], color[2]))

    # COLOR PALETTE FUNCTIONS
    def createColorAttributes(self):
        # First compile color information for each neighborhood
        colorDictionary = {}
        colorMenuDictionary = {}
        for neighborhood in NEIGHBORHOODS:
            colorDictionary[neighborhood] = (
                # Create a wall style dictionary for each neighborhood
                self.createColorDictionary(neighborhood))
            # Using this dictionary, create color pie menus
            colorMenuDictionary[neighborhood] = (
                self.createColorMenus(
                neighborhood, colorDictionary[neighborhood]))
        # Now store this info in the appropriate place in the attribute dict
        for colorType in COLOR_TYPES:
            neighborhoodDict = self.attributeDictionary[colorType] = {}
            for neighborhood in NEIGHBORHOODS:
                attribute = LevelAttribute(colorType)
                dict = {}
                # Add colors to attribute dictionary
                colorList = colorDictionary[neighborhood][colorType]
                for i in range(len(colorList)):
                    dict[i] = colorList[i]
                attribute.setDict(dict)
                attribute.setMenu(
                    colorMenuDictionary[neighborhood][colorType])
                neighborhoodDict[neighborhood] = attribute
    
    def createColorDictionary(self, neighborhood):
        filename = neighborhood + '_colors.txt'
        print 'Loading Color Palettes from: ' + filename
        colorData = self.getStyleFileData(filename)
        return self.getColorDictionary(colorData)
    
    def getColorDictionary(self, colorData):
        # Initialze neighborhod color dictionary
        dict = {}
        for colorType in COLOR_TYPES:
            dict[colorType] = DEFAULT_COLORS[:] 
        # Add color information to appropriate sub-list
        for line in colorData:
            pair = map(string.strip, line.split(':'))
            key = pair[0]
            if dict.has_key(key):
                dict[key].append(eval(pair[1]))
            else:
                print 'LevelStyleManager.getColorDictionary key not found'
        return dict
    
    def createColorMenus(self, neighborhood, dictionary):
        menuDict = {}
        keys = dictionary.keys()
        for key in keys:
            menuDict[key] = (
                self.createColorMenu(neighborhood + key, dictionary[key]))
        return menuDict
    
    def createColorMenu(self, menuName, colorList, radius = 0.7, sf = 2.0):
        # Create color chips for each color
	numItems = len(colorList)
	# Attach it to hidden for now
	newColorMenu = hidden.attachNewNode(menuName + 'Menu')
        # Compute the angle per item
	angle = deg2Rad(360.0/float(numItems))
        aspectRatio = (direct.dr.width / float(direct.dr.height))
	# Attach the color chips to the new menu and adjust sizes
        for i in range (numItems):
            # Create the node and set its color
            node = OnscreenText('    ', 0.0, 0.0)
            node.setColor(colorList[i])
            bounds = node.getBounds()
            center = bounds.getCenter()
            center = center * (sf * node.getScale()[0])
            # Reposition it
            node.setXY((radius * math.cos(i * angle)) - center[0],
                       (radius * aspectRatio * math.sin(i * angle)) -
                       center[2])
            node.setScale(node.getScale() * sf)
            # Add it to the wallColorMenu
            node.reparentTo(newColorMenu)
	# Scale the whole shebang down by 0.5
	newColorMenu.setScale(0.5)
	# Create and return resulting pie menu
        return PieMenu(newColorMenu, colorList)
    
    # DNA ATTRIBUTES
    def createDNAAttributes(self):
        # Create the DNA Attribute entries
        # Most objects are oriented with graphical menu items
        # Street and props aren't oiented and use text menus
        for dnaType in DNA_TYPES:
            # Create a dictionary of dna types
            dict = {}
            if ((dnaType == 'street') or (dnaType == 'prop') or
                (dnaType == 'toon_landmark')):
                dnaList = self.getCatalogCodes(dnaType)
            else:
                dnaList = [None] + self.getCatalogCodesSuffix(dnaType, '_ur')
            # Add dnaCodes to attribute dictionary
            for i in range(len(dnaList)):
                dict[i] = dnaList[i]
            # Create a LevelAttribute
            attribute = LevelAttribute(dnaType + '_texture')
            attribute.setDict(dict)
            # Prepend None to allow option of no item
            if ((dnaType == 'street') or (dnaType == 'prop') or
                (dnaType == 'toon_landmark')):
                attribute.setMenu(self.createTextPieMenu(dnaType, dnaList))
            elif (dnaType == 'wall'):
                attribute.setMenu(self.createDNAPieMenu(dnaType, dnaList,
                                                         sf = 0.25))
            elif (dnaType == 'door'):
                attribute.setMenu(self.createDNAPieMenu(dnaType, dnaList,
                                                         sf = 0.035))
            elif (dnaType == 'cornice'):
                attribute.setMenu(self.createDNAPieMenu(dnaType, dnaList,
                                                         sf = 0.5))
            elif (dnaType == 'window'):
                attribute.setMenu(self.createDNAPieMenu(dnaType, dnaList,
                                                         sf = 0.125))
            else:
                print 'unknown attribute'
            # Add it to the attributeDictionary
            self.attributeDictionary[dnaType + '_texture'] = attribute
    
    def createDNAPieMenu(self, dnaType, dnaList, radius = 0.7, sf = 1.0):
	# Get the currently available window options	
	numItems = len(dnaList)
        # Create a top level node to hold the menu
	newMenu = hidden.attachNewNode(dnaType + 'Menu')
        # Compute angle increment per item
	angle = deg2Rad(360.0/numItems)
        aspectRatio = direct.dr.width /float(direct.dr.height)
        # Add items
        for i in range(0, numItems):
            if dnaList[i]:
                # Get the node
                node = DNASTORE.findNode(dnaList[i])
            else:
                node = None
            if node:
                # Add it to the window menu
                path = node.instanceTo(newMenu)
                # Place menu nodes in a circle, offset each in X and Z
                # by half node width/height
                bounds = path.getBounds()
                center = bounds.getCenter()
                center = center * sf
                path.setPos((radius * math.cos(i * angle)) - center[0],
                            0.0,
                            ((radius * aspectRatio * math.sin(i * angle)) -
                             center[2]))
                path.setScale(sf)
	# Scale the whole shebang down by 0.5
	newMenu.setScale(0.5)
        # Create and return a pie menu
        return PieMenu(newMenu, dnaList)

    def createTextPieMenu(self, dnaType, textList, radius = 0.7, sf = 1.0):
	numItems = len(textList)
        # Create top level node for new menu
	newMenu = hidden.attachNewNode(dnaType + 'Menu')
        # Compute angle per item
	angle = deg2Rad(360.0/numItems)
        aspectRatio = direct.dr.width/float(direct.dr.height)
        # Add items
        for i in range (numItems):
            # Create onscreen text node for each item
            if (textList[i] != None):
                node = OnscreenText(str(textList[i]),0,0)
            else:
                node = None
            if node:
                # Reposition it
                bounds = node.getBounds()
                center = bounds.getCenter()
                center = center * (sf * node.getScale()[0])
                node.setXY(radius * math.cos(i * angle) - center[0],
                           ((radius * aspectRatio * math.sin(i * angle)) -
                            center[2]))
                node.setScale(node.getScale() * sf)
                # Add it to the newMenu
                node.reparentTo(newMenu)
	# Scale the whole shebang down by 0.5
	newMenu.setScale(0.5)
        # Create and return a pie menu
        return PieMenu(newMenu, textList)

    # MISCELLANEOUS MENUS
    def createMiscAttributes(self):
        # Num windows menu
        self.createMiscAttribute('window_count',[0,1,2,3,4])
        # Building width menu
        self.createMiscAttribute('building_width',[5,10,15,15.6,20,20.7,25])
        # Building types
        self.createMiscAttribute('building_type', BUILDING_TYPES)
        # MRM: Need offset on these menus
        # Wall orientation menu
        self.createMiscAttribute('wall_orientation', ['ur','ul','dl','dr'])
        # Wall height menu
        self.createMiscAttribute('wall_height', [10, 20])
        # Window orientation menu
        self.createMiscAttribute('window_orientation', ['ur','ul',None,None])
        # Door orientation menu
        self.createMiscAttribute('door_orientation', ['ur','ul',None,None])
        # Cornice orientation menu
        self.createMiscAttribute('cornice_orientation', ['ur','ul',None,None])
        
    def createMiscAttribute(self, miscType, miscList, sf = 3.0):
        # Create a dictionary from miscList
        dict = {}
        # Add items to attribute dictionary
        for i in range(len(miscList)):
            dict[i] = miscList[i]
        # Create the miscellaneous Attribute entries
        attribute = LevelAttribute(miscType)
        attribute.setDict(dict)
        # Now create a pie menu 
        attribute.setMenu(self.createTextPieMenu(miscType, miscList,
                                                  sf = sf))
        # Add it to the attributeDictionary
        self.attributeDictionary[miscType] = attribute

    # GENERAL FUNCTIONS
    def getStyleFileData(self, filename):
        """
        Open the specified file and strip out unwanted whitespace and
        empty lines.  Return file as list, one file line per element.
        """
        fname = Filename(self.stylePathPrefix +
                         '/alpha/DIRECT/LevelEditor/StyleFiles/' +
                         filename)
        f = open(fname.toOsSpecific(), 'r')
        rawData = f.readlines()
        f.close()
        styleData = []
        for line in rawData:
            l = string.strip(line)
            if l:
                styleData.append(l)
        return styleData

    # UTILITY FUNCTIONS
    def getAttribute(self, attribute):
        """ Return specified attribute for current neighborhood """
        levelAttribute = self.attributeDictionary[attribute]
        # Get attribute for current neighborhood
        if (type(levelAttribute) == types.DictionaryType):
            levelAttribute = levelAttribute[self.getEditMode()]
        return levelAttribute

    def getCatalogCode(self, category, i):
        return DNASTORE.getCatalogCode(category, i)
    
    def getCatalogCodes(self, category):
	numCodes = DNASTORE.getNumCatalogCodes(category)
	codes = []
        for i in range(numCodes):
            codes.append(DNASTORE.getCatalogCode(category, i))
	return codes

    def getCatalogCodesSuffix(self, category, suffix):
	codes = self.getCatalogCodes(category)
	orientedCodes = []
        for code in codes:
            if code[-3:] == suffix:
                orientedCodes.append(code)
	return orientedCodes

    def setEditMode(self, mode):
        """Set current neighborhood editing mode"""
        self._editMode = mode
    
    def getEditMode(self):
        """Get current neighborhood editing mode"""
        return self._editMode

class LevelAttribute:
    """Class which encapsulates a pie menu and a set of items"""
    def __init__(self, name):
        # Record name
        self.name = name
        # Pie menu used to pick an option
        self._menu = None
        # Dictionary of available options
        self._dict = None
        self._list = []
        # Currently selected option
        self._current = None
    def setCurrent(self, newValue, fEvent = 1):
        self._current = newValue
        # Send event if specified
        if fEvent:
            messenger.send('select_' + self.name, [self._current])
    def setMenu(self,menu):
        self._menu = menu
        self._menu.action = self.setCurrent
    def setDict(self,dict):
        self._dict = dict
        # Create a list from the dictionary
        self._list = dict.values()
        # Initialize current to first item
        if (len(self._list) > 0):
            self._current = self._list[0]
    def setList(self,list):
        self._list = list
        # Create a dictionary from the list
        self._dict = {}
        count = 0
        for item in list:
            self._dict[count] = item
            count = count + 1
        # Initialize current to first item
        if (len(self._list) > 0):
            self._current = self._list[0]
    def getCurrent(self):
        return self._current
    def getMenu(self):
        return self._menu
    def getDict(self):
        return self._dict
    def getList(self):
        return self._list

class DNAFlatBuildingStyle:
    """Class to hold attributes of a building style"""
    def __init__(self, building = None, styleList = None, name = 'bldg_style'):
        self.name = name
        if building:
            # Passed in a building
            self.copy(building)
        elif styleList != None:
            # Passed in a list of style-height pairs
            self.styleList = []
            self.heightList = []
            for pair in styleList:
                self.add(pair[0], pair[1])
        else:
            # Use default style/height
            self.styleList = [DNAWallStyle()]
            self.heightList = [10]

    def add(self, style, height):
        self.styleList.append(style)
        self.heightList.append(height)
            
    def copy(self, building):
        self.styleList = []
        self.heightList = DNAGetWallHeights(building)[0]
        for i in range(building.getNumChildren()):
            child = building.at(i)
            if DNAClassEqual(child, DNA_WALL):
                wallStyle = DNAWallStyle(wall = child)
                self.styleList.append(wallStyle)

    def output(self, file = sys.stdout):
        def createHeightCode(s = self):
            def joinHeights(h1,h2):
                return '%s_%s' % (h1, h2)
            hl = map(ROUND_INT, s.heightList)
            return reduce(joinHeights, hl)
        file.write('buildingStyle: %s\n' % createHeightCode())
        for style in self.styleList:
            style.output(file)
        file.write('endBuildingStyle\n')

class DNAWallStyle:
    """Class to hold attributes of a wall style (textures, colors, etc)"""
    def __init__(self, wall = None, name = 'wall_style'):
        # First initialize everything
        self.name = name
        self.wall_texture = 'wall_md_blank_ur'
        self.wall_color = Vec4(1.0)
        self.window_count = 2
        self.window_texture = None
        self.window_color = Vec4(1.0)
        self.window_awning_texture = None
        self.window_awning_color = Vec4(1.0)
        self.door_texture = None
        self.door_color = Vec4(1.0)
        self.door_awning_texture = None
        self.door_awning_color = Vec4(1.0)
        self.cornice_texture = None
        self.cornice_color = Vec4(1.0)
        # Then copy the specifics for the wall if input
        if wall:
            self.copy(wall)

    def copy(self, wall):
        self.wall_texture = wall.getCode()
        self.wall_color = wall.getColor()
        for i in range(wall.getNumChildren()):
            child = wall.at(i)
            if DNAClassEqual(child, DNA_WINDOWS):
                self.window_count = child.getWindowCount()
                self.window_texture = child.getCode()
                self.window_color = child.getColor()
                # MRM: Check for awnings here
            elif DNAClassEqual(child, DNA_DOOR):
                self.door_texture = child.getCode()
                self.door_color = child.getColor()
                # MRM: Check for awnings here
            elif DNAClassEqual(child, DNA_CORNICE):
                self.cornice_texture = child.getCode()
                self.cornice_color = child.getColor()

    def output(self, file = sys.stdout):
        """ Output style description to a file """
        def writeAttributes(f, type, s = self):
            color = s[type + '_color']
            f.write('%s_texture: %s\n' % (type, s[type + '_texture']))
            f.write('%s_color: Vec4(%.2f, %.2f, %.2f, 1.0)\n' %
                    (type, color[0], color[1], color[2]))
        file.write('wallStyle\n')
        writeAttributes(file, 'wall')
        if self['window_texture']:
            writeAttributes(file, 'window')
            file.write('window_count: %s\n' % self['window_count'])
        if self['window_awning_texture']:
            writeAttributes(file, 'window_awning')
        if self['door_texture']:
            writeAttributes(file, 'door')
        if self['door_awning_texture']:
            writeAttributes(file, 'door_awning')
        if self['cornice_texture']:
            writeAttributes(file, 'cornice')
        file.write('endWallStyle\n')

    # Convience functions to facilitate class use
    def __setitem__(self, index, item):
        self.__dict__[index] = item
    
    def __getitem__(self, index):
        return self.__dict__[index]
    
    def __repr__(self):
        return(
            'Name: %s\n' % self.name +
            'Wall Texture: %s\n' % self.wall_texture +
            'Wall Color: %s\n' % self.wall_color +
            'Window Texture: %s\n' % self.window_texture +
            'Window Color: %s\n' % self.window_color +
            'Window Awning Texture: %s\n' % self.window_awning_texture +
            'Window Awning Color: %s\n' % self.window_awning_color +
            'Door Texture: %s\n' % self.door_texture +
            'Door Color: %s\n' % self.door_color +
            'Door Awning Texture: %s\n' % self.door_awning_texture +
            'Door Awning Color: %s\n' % self.door_awning_color +
            'Cornice Texture: %s\n' % self.cornice_texture +
            'Cornice Color: %s\n' % self.cornice_color
            )

class OldLevelEditor(NodePath, PandaObject):
    pass

class LevelEditorPanel(Pmw.MegaToplevel):
    def __init__(self, levelEditor, parent = None, **kw):

        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',       'Toontown Level Editor',       None),
            )
        self.defineoptions(kw, optiondefs)

        Pmw.MegaToplevel.__init__(self, parent, title = self['title'])

        self.levelEditor = levelEditor
        self.styleManager = self.levelEditor.styleManager
        self.fUpdateSelected = 1
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
                            'Edit Visibility Groups',
                            label = 'Edit Vis Groups',
                            command = self.levelEditor.editDNAVisGroups)
        menuBar.addmenuitem('Level Editor', 'command',
                            'Reset level',
                            label = 'Reset level',
                            command = self.levelEditor.reset)
        menuBar.addmenuitem('Level Editor', 'command',
                            'Exit Level Editor Panel',
                            label = 'Exit',
                            command = self.levelEditor.destroy)

        menuBar.addmenu('Style', 'Style Operations')
        menuBar.addmenuitem('Style', 'command',
                            "Save Selected Object's Color",
                            label = 'Save Color',
                            command = self.levelEditor.saveColor)
        menuBar.addmenuitem('Style', 'command',
                            "Save Selected Wall's Style",
                            label = 'Save Wall Style',
                            command = self.levelEditor.saveWallStyle)
        menuBar.addmenuitem('Style', 'command',
                            "Save Selected Buildings's Style",
                            label = 'Save Bldg Style',
                            command = self.levelEditor.saveBuildingStyle)
        menuBar.addmenuitem('Style', 'command',
                            'Reload Color Palettes',
                            label = 'Reload Colors',
                            command = self.styleManager.createColorAttributes)
        menuBar.addmenuitem('Style', 'command',
                            'Reload Wall Style Palettes',
                            label = 'Reload Wall Styles',
                            command = self.styleManager.createWallStyleAttributes)
        menuBar.addmenuitem('Style', 'command',
                            'Reload Building Style Palettes',
                            label = 'Reload Bldg Styles',
                            command = self.styleManager.createBuildingStyleAttributes)

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
            selectioncommand = self.levelEditor.setEditMode, history = 0,
            scrolledlist_items = NEIGHBORHOODS)
        self.editMenu.selectitem(NEIGHBORHOODS[0])
        self.editMenu.pack(side = 'left', expand = 0)
                                     

        # Create the notebook pages
        notebook = Pmw.NoteBook(hull)
        notebook.pack(fill = BOTH, expand = 1)
        streetsPage = notebook.add('Streets')
        toonBuildingsPage = notebook.add('Toon Bldgs')
        landmarkBuildingsPage = notebook.add('Landmark Bldgs')
        suitPathPage = notebook.add('Paths')
        battleCellPage = notebook.add('Cells')
        # suitBuildingsPage = notebook.add('Suit Buildings')
        propsPage = notebook.add('Props')
        sceneGraphPage = notebook.add('SceneGraph')

        self.fPaths = IntVar()
        self.fPaths.set(0)
        self.pathButton = Checkbutton(suitPathPage,
                                      text = 'Show Paths',
                                      width = 6,
                                      variable = self.fPaths,
                                      command = self.toggleSuitPaths)
        self.pathButton.pack(side = 'top', expand = 1, fill = 'x')
        self.suitPointSelector = Pmw.ComboBox(
            suitPathPage,
            dropdown = 0,
            listheight = 200,
            labelpos = W,
            label_text = 'Point type:',
            label_width = 12,
            label_anchor = W,
            entry_width = 30,
            selectioncommand = self.setSuitPointType,
            scrolledlist_items = ['street', 'front door', 'side door']
            )
        self.suitPointSelector.selectitem('street')
        self.suitPointSelector.pack(expand = 1, fill = 'both')

        self.fCells = IntVar()
        self.fCells.set(0)
        self.cellButton = Checkbutton(battleCellPage,
                                      text = 'Show Cells',
                                      width = 6,
                                      variable = self.fCells,
                                      command = self.toggleBattleCells)
        self.cellButton.pack(side = 'top', expand = 1, fill = 'x')
        self.battleCellSelector = Pmw.ComboBox(
            battleCellPage,
            dropdown = 0,
            listheight = 200,
            labelpos = W,
            label_text = 'Cell type:',
            label_width = 12,
            label_anchor = W,
            entry_width = 30,
            selectioncommand = self.setBattleCellType,
            scrolledlist_items = ['20w 20l', '20w 30l', '30w 20l', '30w 30l']
            )
        self.battleCellSelector.selectitem('20w 20l')
        self.battleCellSelector.pack(expand = 1, fill = 'both')

        self.addStreetButton = Button(
            streetsPage,
            text = 'ADD STREET',
            command = self.addStreet)
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
                                     self.styleManager.getCatalogCodes(
            'street'))
            )
        self.streetModuleType = self.styleManager.getCatalogCode('street',0)
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
            scrolledlist_items = ['random20', 'random30'] + BUILDING_TYPES
            )
        self.toonBuildingType = 'random20'
        self.toonBuildingSelector.selectitem(self.toonBuildingType)
        self.toonBuildingSelector.pack(expand = 1, fill = 'both')
        
        #self.toonBuildingWidthScale = EntryScale.EntryScale(
        #toonBuildingsPage, min = 1.0, max = 30.0,
        #   resolution = 0.01, text = 'Wall Width',
        #   command = self.updateSelectedWallWidth)
        #self.toonBuildingWidthScale.pack(fill = 'x')
        
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
                                     self.styleManager.getCatalogCodes(
            'toon_landmark'))
            )
        self.landmarkType = self.styleManager.getCatalogCode(
            'toon_landmark',0)
        self.landmarkBuildingSelector.selectitem(
            self.styleManager.getCatalogCode('toon_landmark',0)[14:])
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
                                     self.styleManager.getCatalogCodes('prop'))
            )
        self.propType = self.styleManager.getCatalogCode('prop',0)
        self.propSelector.selectitem(
            self.styleManager.getCatalogCode('prop',0)[5:])
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
        self.fMapVis = IntVar()
        self.fMapVis.set(0)
        self.mapSnapButton = Checkbutton(buttonFrame,
                                      text = 'Map Vis',
                                      width = 6,
                                      variable = self.fMapVis,
                                      command = self.toggleMapVis)
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

        self.fPlaneSnap = IntVar()
        self.fPlaneSnap.set(1)
        self.planeSnapButton = Checkbutton(buttonFrame,
                                           text = 'PlaneSnap',
                                           width = 6,
                                           variable = self.fPlaneSnap)
        self.planeSnapButton.pack(side = 'left', expand = 1, fill = 'x')

        self.fGrid = IntVar()
        self.fGrid.set(0)
        direct.gridButton = Checkbutton(buttonFrame,
                                      text = 'Show Grid',
                                      width = 6,
                                      variable = self.fGrid,
                                      command = self.toggleGrid)
        direct.gridButton.pack(side = 'left', expand = 1, fill = 'x')

        buttonFrame.pack(expand = 1, fill = 'x')

        buttonFrame2 = Frame(hull)
        self.groupButton = Button(
            buttonFrame2,
            text = 'New Group',
            command = self.levelEditor.createNewGroup)
        self.groupButton.pack(side = 'left', expand = 1, fill = 'x')
        
        self.visGroupButton = Button(
            buttonFrame2,
            text = 'New Vis Group',
            command = self.createNewVisGroup)
        self.visGroupButton.pack(side = 'left', expand = 1, fill = 'x')
        
        self.setParentButton = Button(
            buttonFrame2,
            text = 'Set Parent',
            command = self.levelEditor.setActiveParent)
        self.setParentButton.pack(side = 'left', expand = 1, fill = 'x')

        self.reparentButton = Button(
            buttonFrame2,
            text = 'Re Parent',
            command = self.levelEditor.reparentSelected)
        self.reparentButton.pack(side = 'left', expand = 1, fill = 'x')

        buttonFrame2.pack(fill = 'x')

        buttonFrame3 = Frame(hull)
        self.isolateButton = Button(
            buttonFrame3,
            text = 'Isolate Selected',
            command = direct.isolate)
        self.isolateButton.pack(side = 'left', expand = 1, fill = 'x')

        self.showAllButton = Button(
            buttonFrame3,
            text = 'Show All',
            command = self.showAll)
        self.showAllButton.pack(side = 'left', expand = 1, fill = 'x')

        buttonFrame3.pack(fill = 'x')

        buttonFrame4 = Frame(hull)
        self.driveMode = IntVar()
        self.driveMode.set(1)
        self.driveModeButton = Radiobutton(
            buttonFrame4,
            text = 'Drive Mode',
            value = 0,
            variable = self.driveMode,
            command = self.levelEditor.useDriveMode)
        self.driveModeButton.pack(side = 'left', expand = 1, fill = 'x')
        directModeButton = Radiobutton(
            buttonFrame4,
            text = 'DIRECT Fly',
            value = 1,
            variable = self.driveMode,
            command = self.levelEditor.useDirectFly)
        directModeButton.pack(side = 'left', expand = 1, fill = 'x')
        buttonFrame4.pack(fill = 'x')

        self.sceneGraphExplorer = SceneGraphExplorer(
            parent = sceneGraphPage,
            menuItems = ['Set Parent', 'Reparent', 'Add Group',
                         'Add Vis Group', 'Set Color',
                         'Set Name'])
        self.sceneGraphExplorer.pack(expand = 1, fill = 'both')
        
        # Make sure input variables processed 
        self.initialiseoptions(LevelEditorPanel)

    def toggleGrid(self):
        if self.fGrid.get():
            direct.grid.enable()
        else:
            direct.grid.disable()

    def toggleSuitPaths(self):
        if self.fPaths.get():
            self.levelEditor.showSuitPaths()
        else:
            self.levelEditor.hideSuitPaths()

    def toggleBattleCells(self):
        if self.fCells.get():
            self.levelEditor.showBattleCells()
        else:
            self.levelEditor.hideBattleCells()

    def toggleXyzSnap(self):
        direct.grid.setXyzSnap(self.fXyzSnap.get())

    def toggleHprSnap(self):
        direct.grid.setHprSnap(self.fXyzSnap.get())
        
    def toggleMapVis(self):
        self.levelEditor.toggleMapVis(self.fMapVis.get())

    def showAll(self):
        direct.showAll(self.levelEditor.NPToplevel)

    def createNewVisGroup(self):
        self.levelEditor.createNewGroup(type = 'vis')
        
    def setStreetModuleType(self,name):
        self.streetModuleType = 'street_' + name
        self.levelEditor.setCurrent('street_texture', self.streetModuleType)

    def addStreet(self):
        self.levelEditor.addStreet(self.streetModuleType)

    def setFlatBuildingType(self,name):
        self.toonBuildingType = name
        self.levelEditor.setCurrent('building_type', self.toonBuildingType)
        
    def addFlatBuilding(self):
        self.levelEditor.addFlatBuilding(self.toonBuildingType)

    def setLandmarkType(self,name):
        self.landmarkType = 'toon_landmark_' + name
        self.levelEditor.setCurrent('toon_landmark_texture', self.landmarkType)

    def addLandmark(self):
        self.levelEditor.addLandmark(self.landmarkType)

    def setPropType(self,name):
        self.propType = 'prop_' + name
        self.levelEditor.setCurrent('prop_texture', self.propType)
        
    def addProp(self):
        self.levelEditor.addProp(self.propType)

    def updateSelectedWallWidth(self, strVal):
        self.levelEditor.updateSelectedWallWidth(string.atof(strVal))

    def setCurrentColor(self, colorVec, fUpdate = 0):
        # Turn on/off update of selected before updating entry
        self.fUpdateSelected = fUpdate
        self.colorEntry.set([int(colorVec[0] * 255.0),
                             int(colorVec[1] * 255.0),
                             int(colorVec[2] * 255.0),
                             255])
        
    def setResetColor(self, colorVec):
        self.colorEntry['resetValue'] = (
            [int(colorVec[0] * 255.0),
             int(colorVec[1] * 255.0),
             int(colorVec[2] * 255.0),
             255])

    def setSuitPointType(self,name):
        if (name == "street"):
            self.levelEditor.currentSuitPointType = DNASuitPoint.STREETPOINT
        elif (name == "front door"):
            self.levelEditor.currentSuitPointType = DNASuitPoint.FRONTDOORPOINT
        elif (name == "side door"):
            self.levelEditor.currentSuitPointType = DNASuitPoint.SIDEDOORPOINT
        print self.levelEditor.currentSuitPointType

    def setBattleCellType(self,name):
        self.levelEditor.currentBattleCellType = name

    def updateSelectedObjColor(self, color):
        try:
            obj = self.levelEditor.DNATarget
            if self.fUpdateSelected and (obj != None):
                objClass = DNAGetClassType(obj)
                if ((objClass.eq(DNA_WALL)) or
                    (objClass.eq(DNA_WINDOWS)) or
                    (objClass.eq(DNA_DOOR)) or
                    (objClass.eq(DNA_CORNICE)) or
                    (objClass.eq(DNA_PROP))
                    ):
                    self.levelEditor.setDNATargetColor(
                        VBase4((color[0]/255.0),
                               (color[1]/255.0),
                               (color[2]/255.0),
                               1.0))
        except AttributeError:
            pass
        # Default is to update selected
        self.fUpdateSelected = 1

    def toggleBalloon(self):
        if self.toggleBalloonVar.get():
            self.balloon.configure(state = 'balloon')
        else:
            self.balloon.configure(state = 'none')

class VisGroupsEditor(Pmw.MegaToplevel):
    def __init__(self, levelEditor, visGroups = ['None'],
                 parent = None, **kw):

        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',       'Visability Groups Editor',       None),
            )
        self.defineoptions(kw, optiondefs)

        Pmw.MegaToplevel.__init__(self, parent, title = self['title'])

        self.levelEditor = levelEditor
        self.visGroups = visGroups
        self.visGroupNames = map(lambda pair: pair[1].getName(),
                                 self.visGroups)
        # Initialize dictionary of visibility relationships
        self.visDict = {}
        # Group we are currently setting visGroups for
        self.target = None
        # Flag to enable/disable toggleVisGroup command
        self.fCommand = 1

        # Handle to the toplevels hull
        hull = self.component('hull')

        balloon = self.balloon = Pmw.Balloon(hull)
        # Start with balloon help disabled
        self.balloon.configure(state = 'none')
        
        menuFrame = Frame(hull, relief = GROOVE, bd = 2)
        menuFrame.pack(fill = X, expand = 1)

        menuBar = Pmw.MenuBar(menuFrame, hotkeys = 1, balloon = balloon)
        menuBar.pack(side = LEFT, expand = 1, fill = X)
        menuBar.addmenu('Vis Groups Editor',
                        'Visability Groups Editor Operations')
        menuBar.addmenuitem('Vis Groups Editor', 'command',
                            'Exit Visability Groups Editor',
                            label = 'Exit',
                            command = self.preDestroy)

        menuBar.addmenu('Help', 'Visability Groups Editor Help Operations')
        self.toggleBalloonVar = IntVar()
        self.toggleBalloonVar.set(0)
        menuBar.addmenuitem('Help', 'checkbutton',
                            'Toggle balloon help',
                            label = 'Balloon Help',
                            variable = self.toggleBalloonVar,
                            command = self.toggleBalloon)

        # Create a combo box to choose target vis group
        self.targetSelector = Pmw.ComboBox(
            hull, labelpos = W, label_text = 'Target Vis Group:',
            entry_width = 12, selectioncommand = self.selectVisGroup,
            scrolledlist_items = self.visGroupNames)
        self.targetSelector.selectitem(self.visGroupNames[0])
        self.targetSelector.pack(expand = 1, fill = X)

        # Scrolled frame to hold radio selector
        sf = Pmw.ScrolledFrame(hull, horizflex = 'elastic',
                               usehullsize = 1, hull_width = 200,
                               hull_height = 400)
        frame = sf.interior()
        sf.pack(padx=5, pady=3, fill = 'both', expand = 1)

        # Add vis groups selector
        self.selected = Pmw.RadioSelect(frame, selectmode=MULTIPLE,
                                        orient = VERTICAL,
                                        pady = 0,
                                        command = self.toggleVisGroup)
        for groupInfo in self.visGroups:
            nodePath = groupInfo[0]
            group = groupInfo[1]
            name = group.getName()
            self.selected.add(name, width = 12)
            # Assemble list of groups visible from this group
            visible = []
            for i in range(group.getNumVisibles()):
                visible.append(group.getVisibleName(i))
            visible.sort()
            self.visDict[name] = [nodePath, group, visible]
        # Pack the widget
        self.selected.pack(expand = 1, fill = X)
        # And make sure scrolled frame is happy
        sf.reposition()

        buttonFrame = Frame(hull)
        buttonFrame.pack(fill='x', expand = 1)

        self.showMode = IntVar()
        self.showMode.set(0)
        self.showAllButton = Radiobutton(buttonFrame, text = 'Show All',
                                         value = 0, indicatoron = 1,
                                         variable = self.showMode,
                                         command = self.refreshVisibility)
        self.showAllButton.pack(side = LEFT, fill = 'x', expand = 1)
        self.showActiveButton = Radiobutton(buttonFrame, text = 'Show Target',
                                            value = 1, indicatoron = 1,
                                            variable = self.showMode,
                                            command = self.refreshVisibility)
        self.showActiveButton.pack(side = LEFT, fill = 'x', expand = 1)

        # Make sure input variables processed 
        self.initialiseoptions(VisGroupsEditor)
        
        # Switch to current target's list
        self.selectVisGroup(self.visGroupNames[0])

    def selectVisGroup(self, target):
        print 'Setting vis options for group:', target
        # Record current target
        oldTarget = self.target
        # Record new target
        self.target = target
        # Deselect buttons from old target (first deactivating command)
        self.fCommand = 0
        if oldTarget:
            visList = self.visDict[oldTarget][2]
            for group in visList:
                self.selected.invoke(self.selected.index(group))
        # Now set buttons to reflect state of new target
        visList = self.visDict[target][2]
        for group in visList:
            self.selected.invoke(self.selected.index(group))
        # Reactivate command
        self.fCommand = 1
        # Update scene
        self.refreshVisibility()

    def toggleVisGroup(self, groupName, state):
        if self.fCommand:
            targetInfo = self.visDict[self.target]
            target = targetInfo[1]
            visList = targetInfo[2]
            groupNP = self.visDict[groupName][0]
            group = self.visDict[groupName][1]
            # MRM: Add change in visibility here
            # Show all vs. show active
            if state == 1:
                print 'Vis Group:', self.target, 'adding group:', groupName
                if groupName not in visList:
                    visList.append(groupName)
                    target.addVisible(groupName)
                    # Update vis and color
                    groupNP.show()
                    groupNP.setColor(1,0,0,1)
            else:
                print 'Vis Group:', self.target, 'removing group:', groupName
                if groupName in visList:
                    visList.remove(groupName)
                    target.removeVisible(groupName)
                    # Update vis and color
                    if self.showMode.get() == 1:
                        groupNP.hide()
                    groupNP.clearColor()

    def refreshVisibility(self):
        # Get current visibility list for target
        targetInfo = self.visDict[self.target]
        visList = targetInfo[2]
        for key in self.visDict.keys():
            groupNP = self.visDict[key][0]
            if key in visList:
                groupNP.show()
                if key == self.target:
                    groupNP.setColor(0,1,0,1)
                else:
                    groupNP.setColor(1,0,0,1)
            else:
                if self.showMode.get() == 0:
                    groupNP.show()
                else:
                    groupNP.hide()
                groupNP.clearColor()

    def preDestroy(self):
        # First clear level editor variable
        self.levelEditor.vgpanel = None
        self.destroy()

    def toggleBalloon(self):
        if self.toggleBalloonVar.get():
            self.balloon.configure(state = 'balloon')
        else:
            self.balloon.configure(state = 'none')

        
