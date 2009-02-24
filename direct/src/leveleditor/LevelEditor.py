
from pandac.PandaModules import *
from direct.directbase.DirectStart import *
from direct.showbase.DirectObject import DirectObject
from PieMenu import *
import direct.gui.DirectGuiGlobals as DGG
from direct.showbase.TkGlobal import *
from direct.directtools.DirectUtil import *
from direct.directtools.DirectGeometry import *
from direct.tkwidgets.SceneGraphExplorer import *
from direct.directnotify import DirectNotifyGlobal
from tkMessageBox import showinfo
from tkFileDialog import *
from Tkinter import *
#from whrandom import *
from random import *
from direct.tkwidgets import Floater
from direct.tkwidgets import VectorWidgets
import string
import os
import getopt
import sys
#import whrandom
import random
import types
from direct.task import Task
import Pmw
import __builtin__

# [gjeon] to control avatar movement in drive mode
from direct.controls import ControlManager
from direct.controls import GravityWalker
from direct.controls import NonPhysicsWalker
from direct.interval.LerpInterval import LerpFunctionInterval

from otp.avatar import LocalAvatar

from toontown.toon import RobotToon
from otp.otpbase import OTPGlobals

from LevelStyleManager import *

# Force direct and tk to be on
base.startDirect(fWantDirect = 1, fWantTk = 1)

visualizeZones = base.config.GetBool("visualize-zones", 0)
dnaDirectory = Filename.expandFrom(base.config.GetString("dna-directory", "$TTMODELS/src/dna"))
fUseCVS = base.config.GetBool("level-editor-use-cvs", 1)
useSnowTree = base.config.GetBool("use-snow-tree", 0)

# Colors used by all color menus
DEFAULT_COLORS = [
    Vec4(1, 1, 1, 1),
    Vec4(0.75, 0.75, 0.75, 1.0),
    Vec4(0.5, 0.5, 0.5, 1.0),
    Vec4(0.25, 0.25, 0.25, 1.0)
    ]
# The list of items with color attributes
COLOR_TYPES = ['wall_color', 'window_color',
               'window_awning_color', 'sign_color', 'door_color',
               'door_awning_color', 'cornice_color',
               'prop_color']
# The list of dna components maintained in the style attribute dictionary
DNA_TYPES = ['wall', 'window', 'sign', 'door_double', 'door_single', 'cornice', 'toon_landmark',
             'prop', 'street']
BUILDING_TYPES = ['10_10', '20', '10_20', '20_10', '10_10_10',
                  '4_21', '3_22', '4_13_8', '3_13_9', '10',
                  '12_8', '13_9_8', '4_10_10',  '4_10', '4_20',
                  ]
BUILDING_HEIGHTS = [10, 14, 20, 24, 25, 30]
NUM_WALLS = [1, 2, 3]
LANDMARK_SPECIAL_TYPES = ['', 'hq', 'gagshop', 'clotheshop', 'petshop', 'kartshop']

OBJECT_SNAP_POINTS = {
    'street_5x20': [(Vec3(5.0, 0, 0), Vec3(0)),
                    (Vec3(0), Vec3(0))],
    'street_10x20': [(Vec3(10.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_20x20': [(Vec3(20.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_30x20': [(Vec3(30.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_40x20': [(Vec3(40.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_80x20': [(Vec3(80.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_5x40': [(Vec3(5.0, 0, 0), Vec3(0)),
                    (Vec3(0), Vec3(0))],
    'street_10x40': [(Vec3(10.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_20x40': [(Vec3(20.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_20x40_15': [(Vec3(20.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_30x40': [(Vec3(30.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_40x40': [(Vec3(40.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_20x60': [(Vec3(20.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_40x60': [(Vec3(40.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_40x40_15': [(Vec3(40.0, 0, 0), Vec3(0)),
                        (Vec3(0), Vec3(0))],
    'street_80x40': [(Vec3(80.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_angle_30': [(Vec3(0), Vec3(-30, 0, 0)),
                        (Vec3(0), Vec3(0))],
    'street_angle_45': [(Vec3(0), Vec3(-45, 0, 0)),
                        (Vec3(0), Vec3(0))],
    'street_angle_60': [(Vec3(0), Vec3(-60, 0, 0)),
                        (Vec3(0), Vec3(0))],
    'street_inner_corner': [(Vec3(20.0, 0, 0), Vec3(0)),
                            (Vec3(0), Vec3(0))],
    'street_outer_corner': [(Vec3(20.0, 0, 0), Vec3(0)),
                            (Vec3(0), Vec3(0))],
    'street_full_corner': [(Vec3(40.0, 0, 0), Vec3(0)),
                           (Vec3(0), Vec3(0))],
    'street_tight_corner': [(Vec3(40.0, 0, 0), Vec3(0)),
                           (Vec3(0), Vec3(0))],
    'street_tight_corner_mirror': [(Vec3(40.0, 0, 0), Vec3(0)),
                           (Vec3(0), Vec3(0))],
    'street_double_corner': [(Vec3(40.0, 0, 0), Vec3(0)),
                           (Vec3(0), Vec3(0))],
    'street_curved_corner': [(Vec3(40.0, 0, 0), Vec3(0)),
                           (Vec3(0), Vec3(0))],
    'street_curved_corner_15': [(Vec3(40.0, 0, 0), Vec3(0)),
                           (Vec3(0), Vec3(0))],
    'street_t_intersection': [(Vec3(40.0, 0, 0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_y_intersection': [(Vec3(30.0, 0, 0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_street_20x20': [(Vec3(20.0, 0, 0), Vec3(0)),
                            (Vec3(0), Vec3(0))],
    'street_street_40x40': [(Vec3(40.0, 0, 0), Vec3(0)),
                            (Vec3(0), Vec3(0))],
    'street_sidewalk_20x20': [(Vec3(20.0, 0, 0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_sidewalk_40x40': [(Vec3(40.0, 0, 0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_divided_transition': [(Vec3(40.0, 0, 0), Vec3(0)),
                                  (Vec3(0), Vec3(0))],
    'street_divided_40x70': [(Vec3(40.0, 0, 0), Vec3(0)),
                             (Vec3(0), Vec3(0))],
    'street_divided_transition_15': [(Vec3(40.0, 0, 0), Vec3(0)),
                                  (Vec3(0), Vec3(0))],
    'street_divided_40x70_15': [(Vec3(40.0, 0, 0), Vec3(0)),
                             (Vec3(0), Vec3(0))],
    'street_stairs_40x10x5': [(Vec3(40.0, 0, 0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_4way_intersection': [(Vec3(40.0, 0, 0), Vec3(0)),
                                 (Vec3(0), Vec3(0))],
    'street_incline_40x40x5': [(Vec3(40.0, 0, 0), Vec3(0)),
                               (Vec3(0), Vec3(0))],
    'street_square_courtyard': [(Vec3(0.0, 0, 0), Vec3(0)),
                            (Vec3(0), Vec3(0))],
    'street_courtyard_70': [(Vec3(0.0, 0, 0), Vec3(0)),
                            (Vec3(0), Vec3(0))],
    'street_courtyard_70_exit': [(Vec3(0.0, 0, 0), Vec3(0)),
                                 (Vec3(0), Vec3(0))],
    'street_courtyard_90': [(Vec3(0.0, 0, 0), Vec3(0)),
                            (Vec3(0), Vec3(0))],
    'street_courtyard_90_exit': [(Vec3(0.0, 0, 0), Vec3(0)),
                                 (Vec3(0), Vec3(0))],
    'street_courtyard_70_15': [(Vec3(0.0, 0, 0), Vec3(0)),
                               (Vec3(0), Vec3(0))],
    'street_courtyard_70_15_exit': [(Vec3(0.0, 0, 0), Vec3(0)),
                                    (Vec3(0), Vec3(0))],
    'street_courtyard_90_15': [(Vec3(0.0, 0, 0), Vec3(0)),
                            (Vec3(0), Vec3(0))],
    'street_courtyard_90_15_exit': [(Vec3(0.0, 0, 0), Vec3(0)),
                                 (Vec3(0), Vec3(0))],
    'street_50_transition': [(Vec3(10.0, 0, 0), Vec3(0)),
                             (Vec3(0), Vec3(0))],
    'street_20x50': [(Vec3(20.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_40x50': [(Vec3(40.0, 0, 0), Vec3(0)),
                     (Vec3(0), Vec3(0))],
    'street_keyboard_10x40': [(Vec3(10.0, 0, 0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_keyboard_20x40': [(Vec3(20.0, 0, 0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_keyboard_40x40': [(Vec3(40.0, 0, 0), Vec3(0)),
                              (Vec3(0), Vec3(0))],
    'street_sunken_40x40': [(Vec3(40.0, 0, 0), Vec3(0)),
                            (Vec3(0), Vec3(0))],
    }

# NEIGHBORHOOD DATA
# If you run this from the command line you can pass in the hood codes
# you want to load. For example:
#    ppython LevelEditor.py DD TT BR
#
if sys.argv[1:]:
    try:
        opts, pargs = getopt.getopt(sys.argv[1:], '')
        hoods = pargs
    except Exception, e:
        print e
# If you do not run from the command line, we just load all of them
# or you can hack this up for your own purposes.
else:
    hoodString = base.config.GetString('level-editor-hoods',
                                       'TT DD BR DG DL MM CC CL CM CS GS GZ OZ PA')
    hoods = string.split(hoodString)

# The list of neighborhoods to edit
hoodIds = {'TT': 'toontown_central',
           'DD': 'donalds_dock',
           'MM': 'minnies_melody_land',
           'BR': 'the_burrrgh',
           'DG': 'daisys_garden',
           'DL': 'donalds_dreamland',
           'CC': 'cog_hq_bossbot',
           'CL': 'cog_hq_lawbot',
           'CM': 'cog_hq_cashbot',
           'CS': 'cog_hq_sellbot',
           'GS': 'goofy_speedway',
           'OZ': 'outdoor_zone',
           'GZ': 'golf_zone',
           'PA': 'party_zone',
           }

# Init neighborhood arrays
NEIGHBORHOODS = []
NEIGHBORHOOD_CODES = {}
for hoodId in hoods:
    if hoodIds.has_key(hoodId):
        hoodName = hoodIds[hoodId]
        NEIGHBORHOOD_CODES[hoodName] = hoodId
        NEIGHBORHOODS.append(hoodName)
    else:
        print 'Error: no hood defined for: ', hoodId

# Load DNA
try:
    if dnaLoaded:
        pass
except NameError:
    print "Loading LevelEditor for hoods: ", hoods
    # DNAStorage instance for storing level DNA info

    # We need to use the __builtin__.foo syntax, not the
    # __builtins__["foo"] syntax, since this file runs at the top
    # level.
    __builtin__.DNASTORE = DNASTORE = DNAStorage()

    # Load the generic storage files
    loadDNAFile(DNASTORE, 'dna/storage.dna', CSDefault, 1)
    loadDNAFile(DNASTORE, 'phase_4/dna/storage.dna', CSDefault, 1)
    loadDNAFile(DNASTORE, 'phase_5/dna/storage_town.dna', CSDefault, 1)
    # loadDNAFile(DNASTORE, 'phase_5.5/dna/storage_estate.dna', CSDefault, 1)
    # loadDNAFile(DNASTORE, 'phase_5.5/dna/storage_house_interior.dna', CSDefault, 1)
    # Load all the neighborhood specific storage files
    if 'TT' in hoods:
        loadDNAFile(DNASTORE, 'phase_4/dna/storage_TT.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_4/dna/storage_TT_sz.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_5/dna/storage_TT_town.dna', CSDefault, 1)
    if 'DD' in hoods:
        loadDNAFile(DNASTORE, 'phase_6/dna/storage_DD.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_6/dna/storage_DD_sz.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_6/dna/storage_DD_town.dna', CSDefault, 1)
    if 'MM' in hoods:
        loadDNAFile(DNASTORE, 'phase_6/dna/storage_MM.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_6/dna/storage_MM_sz.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_6/dna/storage_MM_town.dna', CSDefault, 1)
    if 'BR' in hoods:
        loadDNAFile(DNASTORE, 'phase_8/dna/storage_BR.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_8/dna/storage_BR_sz.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_8/dna/storage_BR_town.dna', CSDefault, 1)
    if 'DG' in hoods:
        loadDNAFile(DNASTORE, 'phase_8/dna/storage_DG.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_8/dna/storage_DG_sz.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_8/dna/storage_DG_town.dna', CSDefault, 1)
    if 'DL' in hoods:
        loadDNAFile(DNASTORE, 'phase_8/dna/storage_DL.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_8/dna/storage_DL_sz.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_8/dna/storage_DL_town.dna', CSDefault, 1)
    if 'CS' in hoods:
        loadDNAFile(DNASTORE, 'phase_9/dna/storage_CS.dna', CSDefault, 1)
    if 'GS' in hoods:
        loadDNAFile(DNASTORE, 'phase_4/dna/storage_GS.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_4/dna/storage_GS_sz.dna', CSDefault, 1)
    if 'OZ' in hoods:
        loadDNAFile(DNASTORE, 'phase_6/dna/storage_OZ.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_6/dna/storage_OZ_sz.dna', CSDefault, 1)
    if 'GZ' in hoods:
        loadDNAFile(DNASTORE, 'phase_6/dna/storage_GZ.dna', CSDefault, 1)
        loadDNAFile(DNASTORE, 'phase_6/dna/storage_GZ_sz.dna', CSDefault, 1)
    if 'CC' in hoods:
        loadDNAFile(DNASTORE, 'phase_12/dna/storage_CC_sz.dna', CSDefault, 1)        
    if 'PA' in hoods:
        loadDNAFile(DNASTORE, 'phase_13/dna/storage_party_sz.dna', CSDefault, 1)
    __builtin__.dnaLoaded = 1


class LevelEditor(NodePath, DirectObject):
    """Class used to create a Toontown LevelEditor object"""
    notify = DirectNotifyGlobal.directNotify.newCategory('LevelEditor')
        
    # Init the list of callbacks:
    selectedNodePathHookHooks=[]
    deselectedNodePathHookHooks=[]

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
    def __init__(self, hoods = hoods):
        # Make the level editor a node path so that you can show/hide
        # The level editor separately from loading/saving the top level node
        # Initialize superclass
        NodePath.__init__(self)
        # Become the new node path
        self.assign(hidden.attachNewNode('LevelEditor'))

        # Enable replaceSelected by default:
        self.replaceSelectedEnabled=1

        self.removeHookList=[self.landmarkBlockRemove]

        # Start block ID at 0 (it will be incremented before use (to 1)):
        self.landmarkBlock=0

        # Create ancillary objects
        # Style manager for keeping track of styles/colors
        self.styleManager = LevelStyleManager(NEIGHBORHOODS, NEIGHBORHOOD_CODES)
        # Load neighborhood maps
        self.createLevelMaps()
        # Marker for showing next insertion point
        self.createInsertionMarker()
        # Create level Editor Panel
        self.panel = LevelEditorPanel(self)

        # Used to store whatever edges and points are loaded in the level
        self.edgeDict = {}
        self.np2EdgeDict = {}
        self.pointDict = {}
        self.point2edgeDict = {}
        self.cellDict = {}

        self.visitedPoints = []
        self.visitedEdges = []

        self.zoneLabels = []



        # Initialize LevelEditor variables DNAData, DNAToplevel, NPToplevel
        # DNAParent, NPParent, groupNum, lastAngle
        # Pass in the new toplevel group and don't clear out the old
        # toplevel group (since it doesn't exist yet)
        self.reset(fDeleteToplevel = 0, fCreateToplevel = 1)

        # The list of events the level editor responds to
        self.actionEvents = [
            # Node path events
            ('preRemoveNodePath', self.removeNodePathHook),
            # Actions in response to DIRECT operations
            ('DIRECT_selectedNodePath', self.selectedNodePathHook),
            ('DIRECT_deselectedNodePath', self.deselectedNodePathHook),
            ('DIRECT_manipulateObjectCleanup', self.updateSelectedPose),
            ('DIRECT_nodePathSetName', self.setName),
            ('DIRECT_activeParent', self.setActiveParent),
            ('DIRECT_reparent', self.reparent),
            ('RGBPanel_setColor', self.setColor),
            # Actions in response to Level Editor Panel operations
            ('SGE_Add Group', self.addGroup),
            ('SGE_Add Vis Group', self.addVisGroup),
            # Actions in response to Pie Menu interaction
            ('select_building_style_all', self.setBuildingStyle),
            ('select_building_type', self.setBuildingType),
            ('select_building_width', self.setBuildingWidth),
            ('select_cornice_color', self.setDNATargetColor),
            ('select_cornice_orientation', self.setDNATargetOrientation),
            ('select_cornice_texture', self.setDNATargetCode, ['cornice']),
            ('select_sign_color', self.setDNATargetColor),
            ('select_sign_orientation', self.setDNATargetOrientation),
            ('select_sign_texture', self.setDNATargetCode, ['sign']),
            ('select_baseline_style', self.panel.setSignBaselineStyle),
            ('select_door_color', self.setDNATargetColor),
            ('select_door_orientation', self.setDNATargetOrientation),
            ('select_door_single_texture', self.setDNATargetCode, ['door']),
            ('select_door_double_texture', self.setDNATargetCode, ['door']),
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
            ('arrow_left', self.keyboardXformSelected, ['left', 'xlate']),
            ('arrow_right', self.keyboardXformSelected, ['right', 'xlate']),
            ('arrow_up', self.keyboardXformSelected, ['up','xlate']),
            ('arrow_down', self.keyboardXformSelected, ['down','xlate']),
            ('control-arrow_left', self.keyboardXformSelected, ['left', 'rotate']),
            ('control-arrow_right', self.keyboardXformSelected, ['right', 'rotate']),
            ('control-arrow_up', self.keyboardXformSelected, ['up', 'rotate']),
            ('control-arrow_down', self.keyboardXformSelected, ['down', 'rotate']),
            ('shift-s', self.placeSuitPoint),
            ('shift-c', self.placeBattleCell),
            ('k', self.addToLandmarkBlock),
            ('shift-k', self.toggleShowLandmarkBlock),
            ('%', self.pdbBreak),

            ('page_up', self.pageUp),
            ('page_down', self.pageDown),
            ]

        self.overrideEvents = [
            ('page_up', base.direct),
            ('page_down',  base.direct)
            ]

        # Initialize state
        # Make sure direct is running
        base.direct.enable()
        # And only the appropriate handles are showing
        base.direct.widget.disableHandles(['x-ring', 'x-disc',
                                           'y-ring', 'y-disc',
                                           'z-post'])
        # Initialize camera
        base.camLens.setNear(1.0)
        base.camLens.setFar(3000)
        base.direct.camera.setPos(0, -10, 10)
        # Initialize drive mode
        self.configureDriveModeCollisionData()
        # Init visibility variables
        self.__zoneId = None
        # Hide (disable) grid initially
        self.showGrid(0)
        # Create variable for vis groups panel
        self.vgpanel = None
        # Start off enabled
        self.enable()

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
        self.battleCellMarker.setName('battleCellMarker')
        self.battleCellMarker.setScale(1)
        self.currentBattleCellType = "20w 20l"

        # Update panel
        # Editing the first hood id on the list
        self.outputFile = None
        self.setEditMode(NEIGHBORHOODS[0])
        # Start of with first item in lists
        self.panel.streetSelector.selectitem(0)
        self.panel.streetSelector.invoke()
        self.panel.toonBuildingSelector.selectitem(0)
        self.panel.toonBuildingSelector.invoke()
        self.panel.landmarkBuildingSelector.selectitem(0)
        self.panel.landmarkBuildingSelector.invoke()
        self.panel.propSelector.selectitem(0)
        self.panel.propSelector.invoke()
        # Start off with 20 foot buildings
        self.panel.twentyFootButton.invoke()
        # Update scene graph explorer
        self.panel.sceneGraphExplorer.update()

        # Karting
        # the key is the barricade number,  the data is a two element list,
        # first number is the first bldg group that uses this
        # the second is the last bldg group that uses this
        self.outerBarricadeDict = {}
        self.innerBarricadeDict = {}

        # [gjeon] to find out currently moving camera in maya mode
        self.mouseMayaCamera = True

        # [gjeon] to find out drive mode stat
        self.fDrive = False

        # [gjeon] to control drive mode
        self.controlManager = None
        self.avatar = None

        self.fov = 60
        self.isPageUp=0
        self.isPageDown=0        
 
    # ENABLE/DISABLE
    def enable(self):
        """ Enable level editing and show level """
        # Make sure level is visible
        self.reparentTo(base.direct.group)
        self.show()

        # [gjeon] Ignore overridden events
        for event in self.overrideEvents:
            event[1].ignore(event[0])
        
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
        base.direct.deselectAll()
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
        taskMgr.remove('insertionMarkerTask')

    def reset(self, fDeleteToplevel = 1, fCreateToplevel = 1,
              fUpdateExplorer = 1):
        """
        Reset level and re-initialize main class variables
        Pass in the new top level group
        """
        # Reset path markers
        self.resetPathMarkers()
        # Reset battle cell markers
        self.resetBattleCellMarkers()

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
        base.direct.grid.setPosHprScale(0, 0, 0, 0, 0, 0, 1, 1, 1)
        # The selected DNA Object/NodePath
        self.selectedDNARoot = None
        self.selectedNPRoot = None
        self.selectedSuitPoint = None
        self.lastLandmarkBuildingDNA = None
        self.showLandmarkBlockToggleGroup = None
        # Set active target (the subcomponent being modified)
        self.DNATarget = None
        self.DNATargetParent = None
        # Set count of groups added to level
        self.setGroupNum(0)
        # Heading angle of last object added to level
        self.setLastAngle(0.0)
        # Last wall and building modified using pie menus
        self.lastSign = None
        self.lastWall = None
        self.lastBuilding = None
        # Code of last selected object (for autopositionGrid)
        self.snapList = []
        # Last menu used
        self.activeMenu = None
        # For highlighting suit paths
        self.visitedPoints = []
        self.visitedEdges = []
        # Update scene graph explorer
        if fUpdateExplorer:
            self.panel.sceneGraphExplorer.update()

        self.outputFile = None
        self.panel["title"] = 'Level Editor: No file loaded'

    def deleteToplevel(self):
        # Destory old toplevel node path and DNA
        # First the toplevel DNA
        self.DNAData.remove(self.DNAToplevel)
        # Then the toplevel Node Path
        self.NPToplevel.removeNode()

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
        # Add toplevel node path for suit points
        self.suitPointToplevel = self.NPToplevel.attachNewNode('suitPoints')

    def destroy(self):
        """ Disable level editor and destroy node path """
        self.disable()
        self.removeNode()
        self.panel.destroy()
        if self.vgpanel:
            self.vgpanel.destroy()

    def useDirectFly(self):
        """ Disable player camera controls/enable direct camera control """
        # Turn off collision traversal
        self.traversalOff()
        # Turn on collisions
        self.collisionsOff()
        # Turn on visiblity
        self.visibilityOff()
        base.camera.wrtReparentTo(render)
        # Reset cam
        base.camera.iPos(base.cam)
        base.cam.iPosHpr()
        # Renable mouse
        self.enableMouse()
        base.direct.enable()

        # [gjeon]  disable avatar and controlManager
        if (self.avatar):
            self.avatar.reparentTo(hidden)
            self.avatar.stopUpdateSmartCamera()
        if (self.controlManager):
            self.controlManager.disable()

        self.fDrive = False

    def lerpCameraP(self, p, time):
        """
        lerp the camera P over time (used by the battle)
        """
        taskMgr.remove('cam-p-lerp')
        if self.avatar:
            self.avatar.stopUpdateSmartCamera()
        def setCamP(p):
            base.camera.setP(p)

        if self.isPageUp:
            fromP = 36.8699
        elif self.isPageDown:
            fromP = -27.5607
        else:
            fromP = 0

        self.camLerpInterval = LerpFunctionInterval(setCamP,
            fromData=fromP, toData=p, duration=time,
            name='cam-p-lerp')
        self.camLerpInterval.start()

    def clearPageUpDown(self):
        if self.isPageDown or self.isPageUp:
            self.lerpCameraP(0, 0.6)
            self.isPageDown = 0
            self.isPageUp = 0
            #self.setCameraPositionByIndex(self.cameraIndex)

        if self.avatar:
            self.avatar.startUpdateSmartCamera()
            
    def pageUp(self):
        if not self.isPageUp:
            self.lerpCameraP(36.8699, 0.6)
            self.isPageDown = 0
            self.isPageUp = 1
            #self.setCameraPositionByIndex(self.cameraIndex)
        else:
            self.clearPageUpDown()

    def pageDown(self):
        if not self.isPageDown:
            self.lerpCameraP(-27.5607, 0.6)
            self.isPageUp = 0
            self.isPageDown = 1
            #self.setCameraPositionByIndex(self.cameraIndex)
        else:
            self.clearPageUpDown()            

    def useDriveMode(self):
        """ Lerp down to eye level then switch to Drive mode """
        # create avatar and set it's location to the camera
        if (self.avatar == None):
            #self.avatar = RobotToon.RobotToon()
            self.avatar = LEAvatar(None, None, None)
            base.localAvatar = self.avatar
            self.avatar.doId = 0
            self.avatar.robot = RobotToon.RobotToon()
            self.avatar.robot.reparentTo(self.avatar)
            self.avatar.setHeight(self.avatar.robot.getHeight())
            self.avatar.setName("The Inspector")
            self.avatar.robot.loop('neutral')

        self.avatar.setPos(base.camera.getPos())
        self.avatar.reparentTo(render)

##         pos = base.direct.camera.getPos()
##         pos.setZ(4.0)
##         hpr = base.direct.camera.getHpr()
##         hpr.set(hpr[0], 0.0, 0.0)
##         t = base.direct.camera.lerpPosHpr(pos, hpr, 1.0, blendType = 'easeInOut',
##                                    task = 'manipulateCamera')
        # Note, if this dies an unatural death, this could screw things up
        # t.uponDeath = self.switchToDriveMode
        self.switchToDriveMode(None)
        self.fDrive = True
        #[gjeon] deselect
        base.direct.selected.deselect(base.direct.selected.last)

    def switchToDriveMode(self, state):
        """ Disable direct camera manipulation and enable player drive mode """
        #base.direct.minimumConfiguration()
        #base.direct.manipulationControl.disableManipulation()
        # Update vis data
        self.initVisibilityData()
##         # Switch to drive mode
##         base.useDrive()
##         # Move cam up and back
##         base.cam.setPos(0, -5, 4)
##         # And move down and forward to compensate
##         base.camera.setPos(base.camera, 0, 5, -4)
##         # Make sure we're where we want to be
##         pos = base.direct.camera.getPos()
##         pos.setZ(0.0)
##         hpr = base.direct.camera.getHpr()
##         hpr.set(hpr[0], 0.0, 0.0)
##         # Fine tune the drive mode
##         base.mouseInterface.node().setPos(pos)
##         base.mouseInterface.node().setHpr(hpr)
##         base.mouseInterface.node().setForwardSpeed(0)
##         base.mouseInterface.node().setReverseSpeed(0)

        base.camera.wrtReparentTo(self.avatar)
        base.camera.setHpr(0, 0, 0)
        #base.camera.setPos(0, 0, 0)
        base.camera.setPos(0, -11.8125, 3.9375)

        base.camLens.setFov(VBase2(60, 46.8265))

        #self.initializeSmartCameraCollisions()
        #self._smartCamEnabled = False
        
        # Turn on collisions
        if self.panel.fColl.get():
            self.collisionsOn()
        # Turn on visiblity
        if self.panel.fVis.get():
            self.visibilityOn()
        # Turn on collision traversal
        if self.panel.fColl.get() or self.panel.fVis.get():
            self.traversalOn()

        if (self.controlManager == None):
            # create player movement controls,camera controls, and avatar
            self.controlManager = ControlManager.ControlManager()
            avatarRadius = 1.4
            floorOffset = OTPGlobals.FloorOffset
            reach = 4.0

            #walkControls=GravityWalker.GravityWalker(gravity = -32.1740 * 2.0)
            walkControls=NonPhysicsWalker.NonPhysicsWalker()
            walkControls.setWallBitMask(OTPGlobals.WallBitmask)
            walkControls.setFloorBitMask(OTPGlobals.FloorBitmask)
            walkControls.initializeCollisions(self.cTrav, self.avatar,
                                              avatarRadius, floorOffset, reach)
            self.controlManager.add(walkControls, "walk")
            self.controlManager.use("walk", self)

            # set speeds after adding controls to the control manager
            self.controlManager.setSpeeds(
                OTPGlobals.ToonForwardSpeed,
                OTPGlobals.ToonJumpForce,
                OTPGlobals.ToonReverseSpeed,
                OTPGlobals.ToonRotateSpeed
                )
        else:
            self.controlManager.enable()

        self.avatarAnimTask = taskMgr.add(self.avatarAnimate, 'avatarAnimTask', 24)
        self.avatar.startUpdateSmartCamera()
        
        self.avatarMoving = 0

    #--------------------------------------------------------------------------
    # Function:   animate avatar model based on if it is moving
    # Parameters: none
    # Changes:
    # Returns:
    #--------------------------------------------------------------------------
    def avatarAnimate(self,task=None):
        if (self.controlManager):
            moving = self.controlManager.currentControls.speed or self.controlManager.currentControls.slideSpeed or self.controlManager.currentControls.rotationSpeed
            if (moving and
                self.avatarMoving == 0):
                self.clearPageUpDown()
                # moving, play walk anim
                if (self.controlManager.currentControls.speed < 0 or
                    self.controlManager.currentControls.rotationSpeed):
                    self.avatar.robot.loop('walk')
                else:
                    self.avatar.robot.loop('run')
                self.avatarMoving = 1
            elif (moving == 0 and
                  self.avatarMoving == 1):
                # no longer moving, play neutral anim
                self.avatar.robot.loop('neutral')
                self.avatarMoving = 0
        return Task.cont

    def configureDriveModeCollisionData(self):
        """
        Set up the local avatar for collisions
        """
        # Set up the collision sphere
        # This is a sphere on the ground to detect barrier collisions
        self.cSphere = CollisionSphere(0.0, 0.0, 0.0, 1.5)
        self.cSphereNode = CollisionNode('cSphereNode')
        self.cSphereNode.addSolid(self.cSphere)
        self.cSphereNodePath = camera.attachNewNode(self.cSphereNode)
        self.cSphereNodePath.hide()
        self.cSphereBitMask = BitMask32.bit(0)
        self.cSphereNode.setFromCollideMask(self.cSphereBitMask)
        self.cSphereNode.setIntoCollideMask(BitMask32.allOff())

        # Set up the collison ray
        # This is a ray cast from your head down to detect floor polygons
        self.cRay = CollisionRay(0.0, 0.0, 6.0, 0.0, 0.0, -1.0)
        self.cRayNode = CollisionNode('cRayNode')
        self.cRayNode.addSolid(self.cRay)
        self.cRayNodePath = camera.attachNewNode(self.cRayNode)
        self.cRayNodePath.hide()
        self.cRayBitMask = BitMask32.bit(1)
        self.cRayNode.setFromCollideMask(self.cRayBitMask)
        self.cRayNode.setIntoCollideMask(BitMask32.allOff())

        # set up wall collision mechanism
        self.pusher = CollisionHandlerPusher()
        self.pusher.setInPattern("enter%in")
        self.pusher.setOutPattern("exit%in")

        # set up floor collision mechanism
        self.lifter = CollisionHandlerFloor()
        self.lifter.setInPattern("on-floor")
        self.lifter.setOutPattern("off-floor")
        self.floorOffset = 0.1
        self.lifter.setOffset(self.floorOffset)

        # Limit our rate-of-fall with the lifter.
        # If this is too low, we actually "fall" off steep stairs
        # and float above them as we go down. I increased this
        # from 8.0 to 16.0 to prevent this
        self.lifter.setMaxVelocity(16.0)

        # set up the collision traverser
        self.cTrav = CollisionTraverser("LevelEditor")
        self.cTrav.setRespectPrevTransform(1)

        # activate the collider with the traverser and pusher
        #self.pusher.addCollider(self.cSphereNodePath, base.camera, base.drive.node())
        #self.lifter.addCollider(self.cRayNodePath, base.camera, base.drive.node())
        # A map of zone ID's to a list of nodes that are visible from
        # that zone.
        self.nodeDict = {}
        # A map of zone ID's to the particular node that corresponds
        # to that zone.
        self.zoneDict = {}
        # A list of all visible nodes
        self.nodeList = []
        # Flag for bootstrapping visibility
        self.fVisInit = 0

    def traversalOn(self):
        base.cTrav = self.cTrav

    def traversalOff(self):
        base.cTrav = 0

    def collisionsOff(self):
        self.cTrav.removeCollider(self.cSphereNodePath)

    def collisionsOn(self):
        self.collisionsOff()
        self.cTrav.addCollider(self.cSphereNodePath, self.pusher)

    def toggleCollisions(self):
        if self.panel.fColl.get():
            self.collisionsOn()
            self.traversalOn()
        else:
            self.collisionsOff()
            if (not (self.panel.fColl.get() or self.panel.fVis.get())):
                self.traversalOff()

    def initVisibilityData(self):
        # First make sure everything is shown
        self.showAllVisibles()
        # A map of zone ID's to a list of nodes that are visible from
        # that zone.
        self.nodeDict = {}
        # A map of zone ID's to the particular node that corresponds
        # to that zone.
        self.zoneDict = {}
        # A list of all visible nodes
        self.nodeList = []
        # NOTE: this should change to find the groupnodes in
        # the dna storage instead of searching through the tree
        for i in range(DNASTORE.getNumDNAVisGroups()):
            groupFullName = DNASTORE.getDNAVisGroupName(i)
            groupName = self.extractGroupName(groupFullName)
            zoneId = int(groupName)
            self.nodeDict[zoneId] = []
            self.zoneDict[zoneId] = self.NPToplevel.find("**/" + groupName)

            # TODO: we only need to look from the top of the hood
            # down one level to find the vis groups as an optimization
            groupNode = self.NPToplevel.find("**/" + groupFullName)
            if groupNode.isEmpty():
                print "Could not find visgroup"
            self.nodeList.append(groupNode)
            for j in range(DNASTORE.getNumVisiblesInDNAVisGroup(i)):
                visName = DNASTORE.getVisibleName(i, j)
                visNode = self.NPToplevel.find("**/" + visName)
                self.nodeDict[zoneId].append(visNode)
        # Rename the floor polys to have the same name as the
        # visgroup they are in... This makes visibility possible.
        self.renameFloorPolys(self.nodeList)
        # Init vis flag
        self.fVisInit = 1

    def extractGroupName(self, groupFullName):
        # The Idea here is that group names may have extra flags associated
        # with them that tell more information about what is special about
        # the particular vis zone. A normal vis zone might just be "13001",
        # but a special one might be "14356:safe_zone" or
        # "345:safe_zone:exit_zone"... These are hypotheticals. The main
        # idea is that there are colon separated flags after the initial
        # zone name.
        return(string.split(groupFullName, ":", 1)[0])

    def renameFloorPolys(self, nodeList):
        for i in nodeList:
            # Get all the collision nodes in the vis group
            collNodePaths = i.findAllMatches("**/+CollisionNode")
            numCollNodePaths = collNodePaths.getNumPaths()
            visGroupName = i.node().getName()
            for j in range(numCollNodePaths):
                collNodePath = collNodePaths.getPath(j)
                bitMask = collNodePath.node().getIntoCollideMask()
                if bitMask.getBit(1):
                    # Bit 1 is the floor collision bit. This renames
                    # all floor collision polys to the same name as their
                    # visgroup.
                    collNodePath.node().setName(visGroupName)

    def hideAllVisibles(self):
        for i in self.nodeList:
            i.hide()

    def showAllVisibles(self):
        for i in self.nodeList:
            i.show()
            i.clearColor()

    def visibilityOn(self):
        self.visibilityOff()
        # Accept event
        self.accept("on-floor", self.enterZone)
        # Add collider
        self.cTrav.addCollider(self.cRayNodePath, self.lifter)
        # Reset lifter
        self.lifter.clear()
        # Reset flag
        self.fVisInit = 1

    def visibilityOff(self):
        self.ignore("on-floor")
        self.cTrav.removeCollider(self.cRayNodePath)
        self.showAllVisibles()

    def toggleVisibility(self):
        if self.panel.fVis.get():
            self.visibilityOn()
            self.traversalOn()
        else:
            self.visibilityOff()
            if (not (self.panel.fColl.get() or self.panel.fVis.get())):
                self.traversalOff()

    def enterZone(self, newZone):
        return
        """
        Puts the toon in the indicated zone.  newZone may either be a
        CollisionEntry object as determined by a floor polygon, or an
        integer zone id.  It may also be None, to indicate no zone.
        """
        # First entry into a zone, hide everything
        if self.fVisInit:
            self.hideAllVisibles()
            self.fVisInit = 0
        # Get zone id
        if isinstance(newZone, CollisionEntry):
            # Get the name of the collide node
            try:
                newZoneId = int(newZone.getIntoNode().getName())
            except:
                newZoneId = 0
        else:
            newZoneId = newZone
        # Ensure we have vis data
        assert self.nodeDict
        # Hide the old zone (if there is one)
        if self.__zoneId != None:
            for i in self.nodeDict[self.__zoneId]:
                i.hide()
        # Show the new zone
        if newZoneId != None:
            for i in self.nodeDict[newZoneId]:
                i.show()
        # Make sure we changed zones
        if newZoneId != self.__zoneId:
            if self.panel.fVisZones.get():
                # Set a color override on our zone to make it obvious what
                # zone we're in.
                if self.__zoneId != None:
                    self.zoneDict[self.__zoneId].clearColor()
                if newZoneId != None:
                    self.zoneDict[newZoneId].setColor(0, 0, 1, 1, 100)
            # The new zone is now old
            self.__zoneId = newZoneId

    def enableMouse(self):
        """ Enable Pie Menu interaction (and disable player camera control) """
        # Turn off player camera control
        base.disableMouse()
        # Handle mouse events for pie menus
        self.accept('DIRECT-mouse3', self.levelHandleMouse3)
        self.accept('DIRECT-mouse3Up', self.levelHandleMouse3Up)

    def disableMouse(self):
        """ Disable Pie Menu interaction """
        # Disable handling of mouse events
        self.ignore('DIRECT-mouse3')
        self.ignore('DIRECT-mouse3Up')

    # LEVEL OBJECT MANAGEMENT FUNCTIONS
    def findDNANode(self, nodePath):
        """ Find node path's DNA Object in DNAStorage (if any) """
        if nodePath:
            return DNASTORE.findDNAGroup(nodePath.node())
        else:
            return None

    def replaceSelected(self):
        if self.replaceSelectedEnabled:
            # Update visible geometry using new DNA
            newRoot = self.replace(self.selectedNPRoot, self.selectedDNARoot)
            # Reselect node path and respawn followSelectedNodePathTask
            base.direct.select(newRoot)

    def replace(self, nodePath, dnaNode):
        """ Replace a node path with the results of a DNANode traversal """
        # Find node path's parent
        if not nodePath:
            return None
        parent = nodePath.getParent()
        dnaParent = dnaNode.getParent()
        # Get rid of the old node path and remove its DNA and
        # node relations from the DNA Store
        self.remove(dnaNode, nodePath)
        # Traverse the old (modified) dna to create the new node path
        try:
            newNodePath = dnaNode.traverse(parent, DNASTORE, 1)
        except Exception:
            self.notify.debug("Couldn't traverse existing DNA! Do not trust replace")
            return None
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
            base.direct.deselect(nodePath)
            # Now you can get rid of the node path
            nodePath.removeNode()

    def removeNodePathHook(self, nodePath):
        dnaNode = self.findDNANode(nodePath)
        # Does the node path correspond to a DNA Object
        if dnaNode:
            for hook in self.removeHookList:
                hook(dnaNode, nodePath)
            # Get DNANode's parent
            parentDNANode = dnaNode.getParent()
            if parentDNANode:
                # Remove DNANode from its parent
                parentDNANode.remove(dnaNode)
            # Delete DNA and associated Node Relations from DNA Store
            DNASTORE.removeDNAGroup(dnaNode)
        else:
            pointOrCell, type = self.findPointOrCell(nodePath)
            if pointOrCell and type:
                if (type == 'suitPointMarker'):
                    print 'Suit Point:', pointOrCell
                    if DNASTORE.removeSuitPoint(pointOrCell):
                        print "Removed from DNASTORE"
                    else:
                        print "Not found in DNASTORE"
                    # Remove point from pointDict
                    del(self.pointDict[pointOrCell])
                    # Remove point from visitedPoints list
                    if pointOrCell in self.visitedPoints:
                        self.visitedPoints.remove(pointOrCell)
                    # Update edge related dictionaries
                    for edge in self.point2edgeDict[pointOrCell]:
                        # Is it still in edge dict?
                        oldEdgeLine = self.edgeDict.get(edge, None)
                        if oldEdgeLine:
                            del self.edgeDict[edge]
                            oldEdgeLine.reset()
                            del oldEdgeLine
                        # Find other endpoints of edge and clear out
                        # corresponding point2edgeDict entry
                        startPoint = edge.getStartPoint()
                        endPoint = edge.getEndPoint()
                        if pointOrCell == startPoint:
                            self.point2edgeDict[endPoint].remove(edge)
                        elif pointOrCell == endPoint:
                            self.point2edgeDict[startPoint].remove(edge)
                        # Is it in the visited edges list?
                        if edge in self.visitedEdges:
                            self.visitedEdges.remove(edge)
                    # Now delete point2edgeDict entry for this point
                    del(self.point2edgeDict[pointOrCell])
                elif (type == 'battleCellMarker'):
                    # Get parent vis group
                    visGroupNP, visGroupDNA = self.findParentVisGroup(
                        nodePath)
                    print 'Battle Cell:', pointOrCell
                    # Remove cell from vis group
                    if visGroupNP and visGroupDNA:
                        if visGroupDNA.removeBattleCell(pointOrCell):
                            print "Removed from Vis Group"
                        else:
                            print "Not found in Vis Group"
                    else:
                        print "Parent Vis Group not found"
                    # Remove cell from DNASTORE
                    if DNASTORE.removeBattleCell(pointOrCell):
                        print "Removed from DNASTORE"
                    else:
                        print "Not found in DNASTORE"
                    # Remove cell from cellDict
                    del(self.cellDict[pointOrCell])

    def reparent(self, nodePath, oldParent, newParent):
        """ Move node path (and its DNA) to active parent """
        # Does the node path correspond to a DNA Object
        dnaNode = self.findDNANode(nodePath)
        if dnaNode:
            # Find old parent DNA
            oldParentDNANode = self.findDNANode(oldParent)
            # Remove DNA from old parent
            if oldParentDNANode:
                oldParentDNANode.remove(dnaNode)
            if newParent:
                # Update active parent just to be safe
                self.setActiveParent(newParent)
                # Move DNA to new parent (if active parent set)
                if self.DNAParent != None:
                    self.DNAParent.add(dnaNode)
                    # It is, is it a DNA_NODE (i.e. it has pos/hpr/scale)?
                    # Update pose to reflect new relationship
                    if DNAIsDerivedFrom(dnaNode, DNA_NODE):
                        # Update DNA
                        self.updatePose(dnaNode, nodePath)
        elif newParent:
            # See if this node path is a suit edge
            suitEdge, oldVisGroup = self.np2EdgeDict.get(nodePath.id(), (None, None))
            # And see if the new parent is a vis group
            newVisGroupNP, newVisGroupDNA = self.findParentVisGroup(newParent)
            if suitEdge and DNAClassEqual(newVisGroupDNA, DNA_VIS_GROUP):
                # If so, remove suit edge from old vis group and add it to the new group
                oldVisGroup.removeSuitEdge(suitEdge)
                # Update suit edge to reflect new zone ID
                suitEdge.setZoneId(newVisGroupDNA.getName())
                newVisGroupDNA.addSuitEdge(suitEdge)
                # Update np2EdgeDict to reflect changes
                self.np2EdgeDict[nodePath.id()] = [suitEdge, newVisGroupDNA]

    def setActiveParent(self, nodePath = None):
        """ Set NPParent and DNAParent to node path and its DNA """
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

    def setName(self, nodePath, newName):
        """ Set name of nodePath's DNA (if it exists) """
        # Find the DNA that corresponds to this node path
        dnaNode = self.findDNANode(nodePath)
        if dnaNode:
            # If it exists, set the name of the DNA Node
            dnaNode.setName(newName)

    def updateSelectedPose(self, nodePathList):
        """
        Update the DNA database to reflect selected objects current positions
        """
        for selectedNode in nodePathList:
            # Is this a DNA Object in the DNASTORE database?
            dnaObject = self.findDNANode(selectedNode)
            if dnaObject:
                # It is, is it a DNA_NODE (i.e. does it have pos/hpr/scale)?
                if DNAIsDerivedFrom(dnaObject, DNA_NODE):
                    # First snap selected node path to grid
                    pos = selectedNode.getPos(base.direct.grid)
                    snapPos = base.direct.grid.computeSnapPoint(pos)
                    if self.panel.fPlaneSnap.get():
                        zheight = 0
                    else:
                        zheight = snapPos[2]
                    selectedNode.setPos(base.direct.grid,
                                        snapPos[0], snapPos[1], zheight)
                    # Angle snap
                    h = base.direct.grid.computeSnapAngle(selectedNode.getH())
                    if base.direct.grid.getHprSnap():
                        selectedNode.setH(h)
                    if selectedNode == base.direct.selected.last:
                        self.setLastAngle(h)
                    # Update DNA
                    self.updatePose(dnaObject, selectedNode)
            else:
                pointOrCell, type = self.findPointOrCell(selectedNode)
                if pointOrCell and type:
                    # First snap selected node path to grid
                    pos = selectedNode.getPos(base.direct.grid)
                    snapPos = base.direct.grid.computeSnapPoint(pos)
                    if self.panel.fPlaneSnap.get():
                        zheight = 0
                    else:
                        zheight = snapPos[2]
                    selectedNode.setPos(
                        base.direct.grid,
                        snapPos[0], snapPos[1], zheight)
                    newPos = selectedNode.getPos(self.NPToplevel)
                    # Update DNA
                    pointOrCell.setPos(newPos)
                    if (type == 'suitPointMarker'):
                        print "Found suit point!", pointOrCell
                        # Ok, now update all the lines into that node
                        for edge in self.point2edgeDict[pointOrCell]:
                            # Is it still in edge dict?
                            oldEdgeLine = self.edgeDict.get(edge, None)
                            if oldEdgeLine:
                                del self.edgeDict[edge]
                                oldEdgeLine.reset()
                                oldEdgeLine.removeNode()
                                del oldEdgeLine
                                newEdgeLine = self.drawSuitEdge(
                                    edge, self.NPParent)
                                self.edgeDict[edge] = newEdgeLine
                    elif (type == 'battleCellMarker'):
                        print "Found battle cell!", pointOrCell

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
        # What is the current building type?
        buildingType = self.getCurrent('building_type')
        # If random
        if buildingType == 'random':
            # Generate height list based on current building height
            buildingHeight = self.getCurrent('building_height')
            heightList = self.getRandomHeightList(buildingHeight)
            # Convert height list to building type
            buildingType = createHeightCode(heightList)
        else:
            # Use specified height list
            heightList = map(string.atof, string.split(buildingType, '_'))
            height = calcHeight(heightList)
            # Is this a never before seen height list?  If so, record it.
            try:
                attr = self.getAttribute(`height` + '_ft_wall_heights')
                if heightList not in attr.getList():
                    print 'Adding new height list entry'
                    attr.add(heightList)
            except KeyError:
                print 'Non standard height building'

        # See if this building type corresponds to existing style dict
        try:
            dict = self.getDict(buildingType + '_styles')
        except KeyError:
            # Nope
            dict = {}

        # No specific dict or empty dict, try to pick a dict
        # based on number of walls
        if not dict:
            # How many walls?
            numWalls = len(heightList)
            # Get the building_style attribute dictionary for
            # this number of walls
            dict = self.getDict(`numWalls` + '_wall_styles')

        if not dict:
            # Still no dict, create new random style using height list
            styleList = []
            # Build up wall styles
            for height in heightList:
                wallStyle = self.getRandomDictionaryEntry(
                    self.getDict('wall_style'))
                styleList.append((wallStyle, height))
            # Create new random flat building style
            style = DNAFlatBuildingStyle(styleList = styleList)
        else:
            # Pick a style
            style = self.getRandomDictionaryEntry(dict)

        # Set style....finally
        self.styleManager.setDNAFlatBuildingStyle(
            dnaNode, style, width = self.getRandomWallWidth(),
            heightList = heightList, name = name)

    def getRandomHeightList(self, buildingHeight):
        # Select a list of wall heights
        heightLists = self.getList(`buildingHeight` + '_ft_wall_heights')
        l = len(heightLists)
        if l > 0:
            # If a list exists for this building height, pick randomly
            return heightLists[randint(0, l - 1)]
        else:
            # No height lists exists for this building height, generate
            chance = randint(0, 100)
            if buildingHeight <= 10:
                return [buildingHeight]
            elif buildingHeight <= 14:
                return [4, 10]
            elif buildingHeight <= 20:
                if chance <= 30:
                    return [20]
                elif chance <= 80:
                    return [10, 10]
                else:
                    return [12, 8]
            elif buildingHeight <= 24:
                if chance <= 50:
                    return [4, 10, 10]
                else:
                    return [4, 20]
            elif buildingHeight <= 25:
                if chance <= 25:
                    return [3, 22]
                elif chance <= 50:
                    return [4, 21]
                elif chance <= 75:
                    return [3, 13, 9]
                else:
                    return [4, 13, 8]
            else:
                if chance <= 20:
                    return [10, 20]
                elif chance <= 35:
                    return [20, 10]
                elif chance <= 75:
                    return [10, 10, 10]
                else:
                    return [13, 9, 8]

    def getRandomWallWidth(self):
        chance = randint(0, 100)
        if chance <= 15:
            return 5.0
        elif chance <= 30:
            return 10.0
        elif chance <= 65:
            return 15.0
        elif chance <= 85:
            return 20.0
        else:
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

        # Position it
        # First kill autoposition task so grid can jump to its final
        # destination (part of cleanup
        taskMgr.remove('autoPositionGrid')
        # Now find where to put node path
        if (hotKey is not None) and nodeClass.eq(DNA_PROP):
            # If its a prop and a copy, place it based upon current
            # mouse position
            hitPt = self.getGridIntersectionPoint()
            # Attach a node, so we can set pos with respect to:
            tempNode = hidden.attachNewNode('tempNode')
            # Place it:
            tempNode.setPos(base.direct.grid, hitPt)
            # Copy the pos to where we really want it:
            dnaNode.setPos(tempNode.getPos())
            # Clean up:
            tempNode.removeNode()
        else:
            # Place the new node path at the current grid origin
            dnaNode.setPos(base.direct.grid.getPos())
        # Initialize angle to match last object
        dnaNode.setHpr(Vec3(self.getLastAngle(), 0, 0))

        # Add the DNA to the active parent
        self.DNAParent.add(dnaNode)
        # And create the geometry
        newNodePath = dnaNode.traverse(self.NPParent, DNASTORE, 1)
        # Update scene graph explorer
        # self.panel.sceneGraphExplorer.update()

        # Reset last Code (for autoPositionGrid)
        if DNAClassEqual(dnaNode, DNA_STREET):
            self.snapList = self.getSnapPoint(dnaNode.getCode())
        # Select the instance
        base.direct.select(newNodePath)
        self.lastNodePath = newNodePath
        # Update grid to get ready for the next object
        self.autoPositionGrid()

    def getSnapPoint(self, code):
        return OBJECT_SNAP_POINTS.get(code, [(Vec3(0.0, 0, 0), Vec3(0)), (Vec3(0), Vec3(0))])

    def addGroup(self, nodePath):
        """ Add a new DNA Node Group to the specified Node Path """
        # Set the node path as the current parent
        base.direct.setActiveParent(nodePath)
        # Add a new group to the selected parent
        self.createNewGroup()

    def addVisGroup(self, nodePath):
        """ Add a new DNA Group to the specified Node Path """
        # Set the node path as the current parent
        base.direct.setActiveParent(nodePath)
        # Add a new group to the selected parent
        self.createNewGroup(type = 'vis')

    def createNewGroup(self, type = 'dna'):
        print "createNewGroup"
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
                                    name = 'tb0:'+ buildingType + '_DNARoot')
        # Now place new building in the world
        self.initDNANode(newDNAFlatBuilding)

    def getNextLandmarkBlock(self):
        self.landmarkBlock=self.landmarkBlock+1
        return str(self.landmarkBlock)

    def addLandmark(self, landmarkType, specialType):
        # Record new landmark type
        self.setCurrent('toon_landmark_texture', landmarkType)
        # And create new landmark building
        block=self.getNextLandmarkBlock()
        newDNALandmarkBuilding = DNALandmarkBuilding(
            'tb'+block+':'+landmarkType + '_DNARoot')
        newDNALandmarkBuilding.setCode(landmarkType)
        newDNALandmarkBuilding.setBuildingType(specialType)
        newDNALandmarkBuilding.setPos(VBase3(0))
        newDNALandmarkBuilding.setHpr(VBase3(0))
        # Headquarters do not have doors
        if specialType not in ['hq', 'kartshop']:
            newDNADoor = self.createDoor('landmark_door')
            newDNALandmarkBuilding.add(newDNADoor)
        # Now place new landmark building in the world
        self.initDNANode(newDNALandmarkBuilding)

    def addProp(self, propType):
        print "addProp %s " % propType
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
        newDNAStreet.setCurbTexture(
            'street_curb_' + self.neighborhoodCode + '_tex')
        # Now place new street in the world
        self.initDNANode(newDNAStreet)

    def createCornice(self):
        newDNACornice = DNACornice('cornice')
        newDNACornice.setCode(self.getCurrent('cornice_texture'))
        newDNACornice.setColor(self.getCurrent('cornice_color'))
        return newDNACornice

    def createDoor(self, type):
        if (type == 'landmark_door'):
            newDNADoor = DNADoor('door')
            print "createDoor %s" % type
            if not (self.getCurrent('door_double_texture')):
                doorStyles = self.styleManager.attributeDictionary['door_double_texture'].getList()[1:]
                defaultDoorStyle = random.choice(doorStyles)
                self.setCurrent('door_double_texture', defaultDoorStyle)
            newDNADoor.setCode(self.getCurrent('door_double_texture'))
            print "doorcolor = %s" % self.getCurrent('door_color')
            newDNADoor.setColor(self.getCurrent('door_color'))
        elif (type == 'door'):
            newDNADoor = DNAFlatDoor('door')
            if not (self.getCurrent('door_single_texture')):
                doorStyles = self.styleManager.attributeDictionary['door_single_texture'].getList()[1:]
                defaultDoorStyle = random.choice(doorStyles)
                self.setCurrent('door_single_texture', defaultDoorStyle)
            newDNADoor.setCode(self.getCurrent('door_single_texture'))
            newDNADoor.setColor(self.getCurrent('door_color'))
        return newDNADoor

    def createSign(self):
        if not (self.getCurrent('sign_texture')):
            defaultSignStyle = self.styleManager.attributeDictionary['sign_texture'].getList()[0]
            self.setCurrent('sign_texture', defaultSignStyle)
        newDNASign = DNASign('sign')
        newDNASign.setCode(self.getCurrent('sign_texture'))
        newDNASign.setColor(self.getCurrent('sign_color'))
        #newDNASign.setColor(VBase4(0.0, 1.0, 0.0, 1.0))
        #newDNASign.setScale(VBase3(2.0, 1.0, 2.0))

        baseline = DNASignBaseline('baseline')
        baseline.setCode("humanist")
        baseline.setColor(VBase4(0.0, 0.0, 0.0, 1.0))
        #baseline.setKern(1.0)
        #baseline.setWiggle(30.0)
        #baseline.setStumble(1.0)
        #baseline.setStomp(10.0)
        #baseline.setWidth(16.0)
        #baseline.setHeight(16.0)
        baseline.setScale(VBase3(0.7, 1.0, 0.7))
        newDNASign.add(baseline)

        #graphic = DNASignGraphic('graphic')
        #graphic.setCode("sign_general1")
        #graphic.setColor(VBase4(1.0, 1.0, 0.0, 0.5))
        #graphic.setScale(VBase3(.2, 1.0, .2))
        #graphic.setHpr(VBase3(0.0, 0.0, 90.0))
        #baseline.add(graphic)

        DNASetBaselineString(baseline, "Toon Shop")

        return newDNASign

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

    def removeSign(self, sign, parent):
        self.setCurrent('sign_color', sign.getColor())
        DNARemoveChildOfClass(parent, DNA_SIGN)

    def removeDoor(self, door, parent):
        self.setCurrent('door_color', door.getColor())
        DNARemoveChildOfClass(parent, DNA_FLAT_DOOR)

    def removeWindows(self, windows, parent):
        # And record number of windows
        self.setCurrent('window_color', windows.getColor())
        self.setCurrent('window_count', windows.getWindowCount())
        DNARemoveChildOfClass(parent, DNA_WINDOWS)

    # LEVEL-OBJECT MODIFICATION FUNCTIONS
    def levelHandleMouse3(self, modifiers):
        # import pdb; pdb.set_trace()
        if base.direct.cameraControl.useMayaCamControls and modifiers == 4: # alt is down, use maya controls
            self.mouseMayaCamera = True
            return
        else:
            self.mouseMayaCamera = False
        
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
            menuMode, wallNum = self.getFlatBuildingMode(dnaObject, modifiers)
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
                self.DNATarget = DNAGetChildOfClass(wall, DNA_FLAT_DOOR)
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
            if base.direct.gotControl(modifiers):
                menuMode = 'prop_color'
            elif base.direct.gotAlt(modifiers) and self.panel.currentBaselineDNA:
                menuMode = 'baseline_style'
            elif base.direct.gotShift(modifiers):
                menuMode = 'sign_texture'
                self.DNATarget = DNAGetChildOfClass(dnaObject, DNA_SIGN)
                self.DNATargetParent = dnaObject
            else:
                menuMode = 'prop_texture'
        elif DNAClassEqual(dnaObject, DNA_LANDMARK_BUILDING):
            # INSERT HERE
            # LANDMARK BUILDING OPERATIONS
            menuMode = self.getLandmarkBuildingMode(dnaObject, modifiers)
            if string.find(menuMode, 'door') >= 0:
                self.DNATarget = DNAGetChildOfClass(dnaObject, DNA_DOOR)
                self.DNATargetParent = dnaObject
            elif string.find(menuMode, 'sign') >= 0:
                self.DNATarget = DNAGetChildOfClass(dnaObject, DNA_SIGN)
                self.DNATargetParent = dnaObject
            else:
                self.DNATarget = dnaObject
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
                state = self.DNATarget.getCode()[-2:]
            elif menuMode == 'building_width':
                state = self.DNATarget.getWidth()
            elif menuMode == 'window_count':
                state = self.DNATarget.getWindowCount()
            elif menuMode == 'building_style_all':
                # Extract the building style from the current building
                state = DNAFlatBuildingStyle(building = self.DNATarget)
            elif menuMode == 'baseline_style':
                # Extract the baseline style
                state = DNABaselineStyle(
                    baseline = self.panel.currentBaselineDNA)
            elif menuMode == 'wall_style':
                # Extract the wall style from the current wall
                state = DNAWallStyle(wall = self.DNATarget)
        self.activeMenu.setInitialState(state)

        # Spawn active menu's task
        self.activeMenu.spawnPieMenuTask()

    def getLandmarkBuildingMode(self, dnaObject, modifiers):
        # Where are we hitting the building?
        hitPt = self.getWallIntersectionPoint(self.selectedNPRoot)
        if hitPt[2] < 10.0:
            # Do door operations
            if base.direct.gotControl(modifiers):
                menuMode = 'door_color'
            elif base.direct.gotAlt(modifiers):
                menuMode = 'door_orientation'
            else:
                menuMode = 'door_double_texture'
        else:
            # Do sign operations
            if base.direct.gotControl(modifiers):
                menuMode = 'sign_color'
            elif base.direct.gotAlt(modifiers) and self.panel.currentBaselineDNA:
                menuMode = 'baseline_style'
            elif base.direct.gotAlt(modifiers):
                menuMode = 'sign_orientation'
            else:
                menuMode = 'sign_texture'
        return menuMode

    def getFlatBuildingMode(self, dnaObject, modifiers):
        # Where are we hitting the building?
        hitPt = self.getWallIntersectionPoint(self.selectedNPRoot)
        wallNum = self.computeWallNum(dnaObject, hitPt)
        if wallNum < 0:
            # Do building related operations
            if base.direct.gotAlt(modifiers):
                menuMode = 'building_width'
            else:
                menuMode = 'building_style_all'
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
                if base.direct.gotControl(modifiers):
                    menuMode = 'cornice_color'
                elif base.direct.gotAlt(modifiers):
                    menuMode = 'cornice_orientation'
                else:
                    menuMode = 'cornice_texture'
            elif ((xPt < 0.3) or (xPt > 0.7)):
                # Do wall operations
                if base.direct.gotControl(modifiers):
                    menuMode = 'wall_color'
                elif base.direct.gotAlt(modifiers):
                    menuMode = 'wall_orientation'
                elif base.direct.gotShift(modifiers):
                    menuMode = 'wall_texture'
                else:
                    menuMode = 'wall_style'
            elif (zPt < 0.4):
                # Do door operations
                if base.direct.gotControl(modifiers):
                    menuMode = 'door_color'
                elif base.direct.gotAlt(modifiers):
                    menuMode = 'door_orientation'
                else:
                    menuMode = 'door_single_texture'
            else:
                # Do window operations
                if base.direct.gotControl(modifiers):
                    menuMode = 'window_color'
                elif base.direct.gotAlt(modifiers):
                    menuMode = 'window_orientation'
                elif base.direct.gotShift(modifiers):
                    menuMode = 'window_count'
                else:
                    menuMode = 'window_texture'
        return menuMode, wallNum

    def levelHandleMouse3Up(self):
        if self.mouseMayaCamera:
            return
        
        if self.activeMenu:
            self.activeMenu.removePieMenuTask()
        # Update panel color if appropriate
        if self.DNATarget:
            objClass = DNAGetClassType(self.DNATarget)
            if ((objClass.eq(DNA_WALL)) or
                (objClass.eq(DNA_WINDOWS)) or
                (objClass.eq(DNA_DOOR)) or
                (objClass.eq(DNA_FLAT_DOOR)) or
                (objClass.eq(DNA_CORNICE)) or
                (objClass.eq(DNA_PROP))
                ):
                self.panel.setCurrentColor(self.DNATarget.getColor())
                
##        b1 = DirectButton(text = ("Button1", "click!", "roll", "disabled"),
##                  text_scale=0.1, borderWidth = (0.01, 0.01),
##                  relief=2)
##
##        b2 = DirectButton(text = ("Button2", "click!", "roll", "disabled"),
##                  text_scale=0.1, borderWidth = (0.01, 0.01),
##                  relief=2)
##
##        l1 = DirectLabel(text = "Test1", text_scale=0.1)
##        l2 = DirectLabel(text = "Test2", text_scale=0.1)
##        l3 = DirectLabel(text = "Test3", text_scale=0.1)
##
##        numItemsVisible = 4
##        itemHeight = 0.11
##
##        myScrolledList = DirectScrolledList(
##                decButton_pos= (0.35, 0, 0.53),
##                decButton_text = "Dec",
##                decButton_text_scale = 0.04,
##                decButton_borderWidth = (0.005, 0.005),
##
##                incButton_pos= (0.35, 0, -0.02),
##                incButton_text = "Inc",
##                incButton_text_scale = 0.04,
##                incButton_borderWidth = (0.005, 0.005),
##
##                frameSize = (0.0, 0.7, -0.05, 0.59),
##                frameColor = (1,0,0,0.5),
##                pos = (-1, 0, 0),
##                items = [b1, b2],
##                numItemsVisible = numItemsVisible,
##                forceHeight = itemHeight,
##                itemFrame_frameSize = (-0.2, 0.2, -0.37, 0.11),
##                itemFrame_pos = (0.35, 0, 0.4),
##        )
##
##        myScrolledList.addItem(l1)
##        myScrolledList.addItem(l2)
##        myScrolledList.addItem(l3)
##
##        for fruit in ['apple', 'pear', 'banana', 'orange']:
##            l = DirectLabel(text = fruit, text_scale=0.1)
##            myScrolledList.addItem(l)
##
##        myScrolledList.reparentTo(aspect2d)


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
            elif (type == 'sign'):
                self.removeSign(self.DNATarget, self.DNATargetParent)
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
            elif (type == 'sign'):
                self.DNATarget = self.createSign()
            elif (type == 'landmark_door'):
                self.DNATarget = self.createDoor('landmark_door')
            elif (type == 'door'):
                self.DNATarget = self.createDoor('door')
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
            oldCode = self.DNATarget.getCode()[:-2]
            # Suit walls only have two orientations!
            if oldCode.find('wall_suit') >= 0:
                orientation = 'u' + orientation[1]
            self.DNATarget.setCode(oldCode+orientation)
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

    def setColor(self, nodePath, r, g, b, a):
        """ This is used to set color of dnaNode subparts """
        dnaNode = self.findDNANode(nodePath)
        if dnaNode:
            objClass = DNAGetClassType(dnaNode)
            if ((objClass.eq(DNA_WALL)) or
                (objClass.eq(DNA_WINDOWS)) or
                (objClass.eq(DNA_DOOR)) or
                (objClass.eq(DNA_FLAT_DOOR)) or
                (objClass.eq(DNA_CORNICE)) or
                (objClass.eq(DNA_PROP)) or
                (objClass.eq(DNA_SIGN)) or
                (objClass.eq(DNA_SIGN_BASELINE)) or
                (objClass.eq(DNA_SIGN_TEXT)) or
                (objClass.eq(DNA_SIGN_GRAPHIC))
                ):
                # Update dna information
                dnaNode.setColor(VBase4(r/255.0, g/255.0, b/255.0, a/255.0))

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
        self.selectedSuitPoint = None
        # Now process newly selected node path
        dnaParent = None
        dnaNode = self.findDNANode(nodePath)
        if base.direct.fControl:
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
            base.direct.deselect(nodePath)
            # And select parent
            base.direct.select(dnaParent, base.direct.fShift)
        elif dnaNode:
            # We got a valid node path/DNA object, continue
            self.selectedNPRoot = nodePath
            self.selectedDNARoot = dnaNode
            # Reset last landmark
            if DNAClassEqual(dnaNode, DNA_LANDMARK_BUILDING):
                self.lastLandmarkBuildingDNA=dnaNode
                if self.showLandmarkBlockToggleGroup:
                    # Toggle old highlighting off:
                    self.toggleShowLandmarkBlock()
                    # Toggle on the the new highlighting:
                    self.toggleShowLandmarkBlock()
            # Reset last Code (for autoPositionGrid)
            if DNAClassEqual(dnaNode, DNA_STREET):
                self.snapList = self.getSnapPoint(dnaNode.getCode())
        else:
            pointOrCell, type = self.findPointOrCell(nodePath)
            if pointOrCell and (type == 'suitPointMarker'):
                print "Found suit point!", pointOrCell
                self.selectedSuitPoint = pointOrCell
            elif pointOrCell and (type == 'battleCellMarker'):
                print "Found battle cell!", pointOrCell
            else:
                if nodePath.getName() != 'suitEdge':
                    suitEdge = self.findSuitEdge(nodePath.getParent())
                    if suitEdge:
                        # Yes, deselect currently selected node path
                        base.direct.deselect(nodePath)
                        # And select parent
                        base.direct.select(suitEdge, base.direct.fShift)

        # Let others know that something new may be selected:
        for i in self.selectedNodePathHookHooks:
                i()

        if self.fDrive:
            base.direct.deselect(nodePath)

    def deselectedNodePathHook(self, nodePath):
        # Clear out old root variables
        self.selectedDNARoot = None
        self.selectedNPRoot = None
        self.selectedSuitPoint = None
        # Let others know:
        for i in self.deselectedNodePathHookHooks:
                i()

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

    def findSuitEdge(self, nodePath):
        """ Walk up a node path's ancestry looking for a suit edge """
        # Check current node's name for suitEdge marker
        if (nodePath.getName() == 'suitEdge'):
            # Its a suitEdge
            return nodePath
        else:
            # If reached the top: fail
            if not nodePath.hasParent():
                return None
            else:
                # Try parent
                return self.findSuitEdge(nodePath.getParent())

    def findPointOrCell(self, nodePath):
        """
        Walk up a node path's ancestry to see if its a suit point marker
        or a battle cell marker
        """
        # Check current node's name for root marker
        if (nodePath.getName() == 'suitPointMarker'):
            # Its a suit point marker!
            # See if point is in pointDict
            point = self.getSuitPointFromNodePath(nodePath)
            return point, 'suitPointMarker'
        elif (nodePath.getName() == 'battleCellMarker'):
            # Its a battle cell marker
            # See if cell is in cell Dict
            cell = self.getBattleCellFromNodePath(nodePath)
            return cell, 'battleCellMarker'
        else:
            # If reached the top: fail
            if not nodePath.hasParent():
                return None, None
            else:
                # Try parent
                return self.findPointOrCell(nodePath.getParent())

    # MANIPULATION FUNCTIONS
    def keyboardRotateSelected(self, arrowDirection):
        """ Rotate selected objects using arrow keys """
        # Get snap angle
        if base.direct.fShift:
            oldSnapAngle = base.direct.grid.snapAngle
            base.direct.grid.setSnapAngle(1.0)
        snapAngle = base.direct.grid.snapAngle
        # Compute new angle
        if ((arrowDirection == 'left') or (arrowDirection == 'up')):
            self.setLastAngle(self.getLastAngle() + snapAngle)
        else:
            self.setLastAngle(self.getLastAngle() - snapAngle)

        if (self.getLastAngle() < -180.0):
            self.setLastAngle(self.getLastAngle() + 360.0)
        elif (self.getLastAngle() > 180.0):
            self.setLastAngle(self.getLastAngle() - 360.0)
        # Move selected objects
        for selectedNode in base.direct.selected:
            selectedNode.setHpr(self.getLastAngle(), 0, 0)
        # Snap objects to grid and update DNA if necessary
        self.updateSelectedPose(base.direct.selected.getSelectedAsList())
        if base.direct.fShift:
            base.direct.grid.setSnapAngle(oldSnapAngle)

    def keyboardTranslateSelected(self, arrowDirection):
        gridToCamera = base.direct.grid.getMat(base.direct.camera)
        camXAxis = gridToCamera.xformVec(X_AXIS)
        xxDot = camXAxis.dot(X_AXIS)
        xzDot = camXAxis.dot(Z_AXIS)

        # what is the current grid spacing?
        if base.direct.fShift:
            # If shift, divide grid spacing by 10.0
            oldGridSpacing = base.direct.grid.gridSpacing
            # Use back door to set grid spacing to avoid grid update
            base.direct.grid.gridSpacing = base.direct.grid.gridSpacing/10.0
        deltaMove = base.direct.grid.gridSpacing

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
        for selectedNode in base.direct.selected:
            # Move it
            selectedNode.setPos(base.direct.grid,
                                selectedNode.getPos(base.direct.grid) + deltaPos)
        # Snap objects to grid and update DNA if necessary
        self.updateSelectedPose(base.direct.selected.getSelectedAsList())
        # Restore grid spacing
        if base.direct.fShift:
            # Use back door to set grid spacing to avoid grid update
            base.direct.grid.gridSpacing = oldGridSpacing

    def keyboardXformSelected(self, arrowDirection, mode):
        if mode == 'rotate':
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
        children = nodePath.getChildren()
        for child in children:
            childVisGroups = (childVisGroups + self.getDNAVisGroups(child))
        return childVisGroups

    def findParentVisGroup(self, nodePath):
        """ Find the containing vis group """
        dnaNode = self.findDNANode(nodePath)
        if dnaNode:
            if DNAClassEqual(dnaNode, DNA_VIS_GROUP):
                return nodePath, dnaNode
        elif nodePath.hasParent():
            return self.findParentVisGroup(nodePath.getParent())
        # Fall through
        return None, None

    def showGrid(self, flag):
        """ toggle direct grid """
        if flag:
            base.direct.grid.enable()
        else:
            base.direct.grid.disable()

    # LEVEL MAP/MARKER FUNCTIONS
    def createLevelMaps(self):
        """
        Load up the various neighborhood maps
        """
        # For neighborhood maps
        self.levelMap = hidden.attachNewNode('level-map')
        self.activeMap = None
        self.mapDictionary = {}
        """
        for neighborhood in NEIGHBORHOODS:
            self.createMap(neighborhood)
        """

    def createMap(self, neighborhood):
        map = loader.loadModel('models/level_editor/' + neighborhood +
                               '_layout')
        if map:
            map.setTransparency(1)
            map.setColor(Vec4(1, 1, 1, .4))
            self.mapDictionary[neighborhood] = map
            # Make sure this item isn't pickable
            base.direct.addUnpickable(neighborhood + '_layout')

    def selectMap(self, neighborhood):
        if self.activeMap:
            self.activeMap.reparentTo(hidden)
        if self.mapDictionary.has_key(neighborhood):
            self.activeMap = self.mapDictionary[neighborhood]
            self.activeMap.reparentTo(self.levelMap)

    def toggleMapVis(self, flag):
        if flag:
            self.levelMap.reparentTo(base.direct.group)
        else:
            self.levelMap.reparentTo(hidden)

    def createInsertionMarker(self):
        self.insertionMarker = LineNodePath(self)
        self.insertionMarker.lineNode.setName('insertionMarker')
        self.insertionMarker.setColor(VBase4(0.785, 0.785, 0.5, 1))
        self.insertionMarker.setThickness(1)
        self.insertionMarker.reset()
        self.insertionMarker.moveTo(-75, 0, 0)
        self.insertionMarker.drawTo(75, 0, 0)
        self.insertionMarker.moveTo(0, -75, 0)
        self.insertionMarker.drawTo(0, 75, 0)
        self.insertionMarker.moveTo(0, 0, -75)
        self.insertionMarker.drawTo(0, 0, 75)
        self.insertionMarker.create()

    def spawnInsertionMarkerTask(self):
        taskMgr.add(self.insertionMarkerTask, 'insertionMarkerTask')

    def insertionMarkerTask(self, state):
        self.insertionMarker.setPosHpr(base.direct.grid, 0, 0, 0, 0, 0, 0)
        # MRM: Why is this necessary?
        self.insertionMarker.setScale(1, 1, 1)
        return Task.cont

    # UTILITY FUNCTIONS
    def getRandomDictionaryEntry(self, dict):
        numKeys = len(dict)
        if numKeys > 0:
            keys = dict.keys()
            key = keys[randint(0, numKeys - 1)]
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
                return randint(1, 2)
            else:
                # All others can have 1 - 4
                return randint(1, 4)
        else:
            return 1

    def autoPositionGrid(self, fLerp = 1):
        taskMgr.remove('autoPositionGrid')
        # Move grid to prepare for placement of next object
        selectedNode = base.direct.selected.last
        if selectedNode:
            dnaNode = self.findDNANode(selectedNode)
            if dnaNode == None:
                return
            nodeClass = DNAGetClassType(dnaNode)
            deltaPos = Point3(20, 0, 0)
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

            if fLerp:
                # Position grid for placing next object
                # Eventually we need to setHpr too
                t = base.direct.grid.lerpPosHpr(
                    deltaPos, deltaHpr, 0.25,
                    other = selectedNode,
                    blendType = 'easeInOut',
                    task = 'autoPositionGrid')
                t.deltaPos = deltaPos
                t.deltaHpr = deltaHpr
                t.selectedNode = selectedNode
                t.setUponDeath(self.autoPositionCleanup)
            else:
                base.direct.grid.setPosHpr(selectedNode, deltaPos, deltaHpr)

        if self.mouseMayaCamera:
            return
        # Also move the camera
        taskMgr.remove('autoMoveDelay')
        handlesToCam = base.direct.widget.getPos(base.direct.camera)
        handlesToCam = handlesToCam * (base.direct.dr.near/handlesToCam[1])
        if ((abs(handlesToCam[0]) > (base.direct.dr.nearWidth * 0.4)) or
            (abs(handlesToCam[2]) > (base.direct.dr.nearHeight * 0.4))):
            taskMgr.remove('manipulateCamera')
            base.direct.cameraControl.centerCamIn(0.5)

    def autoPositionCleanup(self, state):
        base.direct.grid.setPosHpr(state.selectedNode, state.deltaPos,
                              state.deltaHpr)
        if base.direct.grid.getHprSnap():
            # Clean up grid angle
            base.direct.grid.setH(ROUND_TO(base.direct.grid.getH(),
                                      base.direct.grid.snapAngle))

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
        mouseX = base.direct.dr.mouseX
        mouseY = base.direct.dr.mouseY
        nearX = (math.tan(deg2Rad(base.direct.dr.fovH)/2.0) *
                 mouseX * base.direct.dr.near)
        nearZ = (math.tan(deg2Rad(base.direct.dr.fovV)/2.0) *
                 mouseY * base.direct.dr.near)
        # Initialize points
        mCam2Wall = base.direct.camera.getMat(selectedNode)
        mouseOrigin = Point3(0)
        mouseOrigin.assign(mCam2Wall.getRow3(3))
        mouseDir = Vec3(0)
        mouseDir.set(nearX, base.direct.dr.near, nearZ)
        mouseDir.assign(mCam2Wall.xformVec(mouseDir))
        # Calc intersection point
        return planeIntersect(mouseOrigin, mouseDir, ZERO_POINT, NEG_Y_AXIS)

    def getGridSnapIntersectionPoint(self):
        """
        Return point of intersection between ground plane and line from cam
        through mouse. Return false, if nothing selected. Snap to grid.
        """
        return base.direct.grid.computeSnapPoint(self.getGridIntersectionPoint())

    def getGridIntersectionPoint(self):
        """
        Return point of intersection between ground plane and line from cam
        through mouse. Return false, if nothing selected
        """
        # Find mouse point on near plane
        mouseX = base.direct.dr.mouseX
        mouseY = base.direct.dr.mouseY
        nearX = (math.tan(deg2Rad(base.direct.dr.fovH)/2.0) *
                 mouseX * base.direct.dr.near)
        nearZ = (math.tan(deg2Rad(base.direct.dr.fovV)/2.0) *
                 mouseY * base.direct.dr.near)
        # Initialize points
        mCam2Grid = base.direct.camera.getMat(base.direct.grid)
        mouseOrigin = Point3(0)
        mouseOrigin.assign(mCam2Grid.getRow3(3))
        mouseDir = Vec3(0)
        mouseDir.set(nearX, base.direct.dr.near, nearZ)
        mouseDir.assign(mCam2Grid.xformVec(mouseDir))
        # Calc intersection point
        return planeIntersect(mouseOrigin, mouseDir, ZERO_POINT, Z_AXIS)

    def jumpToInsertionPoint(self):
        """ Move selected object to insertion point """
        selectedNode = base.direct.selected.last
        if selectedNode:
            # Check if its a dna node
            dnaNode = self.findDNANode(selectedNode)
            if dnaNode:
                # Place the new node path at the current grid origin
                selectedNode.setPos(base.direct.grid, 0, 0, 0)
                # Initialize angle to match last object
                selectedNode.setHpr(self.getLastAngle(), 0, 0)
                # Now update DNA pos and hpr to reflect final pose
                dnaNode.setPos(selectedNode.getPos())
                dnaNode.setHpr(selectedNode.getHpr())
                # Update grid to get ready for the next object
                self.autoPositionGrid()

    # CVS OPERATIONS
    def cvsUpdate(self, filename):
        dirname = os.path.dirname(filename)
        if not os.path.isdir(dirname):
            print 'Cannot CVS update %s: invalid directory' % (filename)
            return

        basename = os.path.basename(filename)
        cwd = os.getcwd()
        os.chdir(dirname)
        cvsCommand = 'cvs update ' + basename
        print cvsCommand
        os.system(cvsCommand)
        os.chdir(cwd)

    def cvsAdd(self, filename):
        dirname = os.path.dirname(filename)
        if not os.path.isdir(dirname):
            print 'Cannot CVS add %s: invalid directory' % (filename)
            return

        basename = os.path.basename(filename)
        cwd = os.getcwd()
        os.chdir(dirname)
        cvsCommand = 'cvs add ' + basename
        print cvsCommand
        os.system(cvsCommand)
        os.chdir(cwd)

    def cvsUpdateAll(self):
        # Update the entire dna source directory.
        dirname = dnaDirectory.toOsSpecific()
        if not os.path.isdir(dirname):
            print 'Cannot CVS commit: invalid directory'
            return

        cwd = os.getcwd()
        os.chdir(dirname)
        cvsCommand = 'cvs update -dP'
        print cvsCommand
        os.system(cvsCommand)
        os.chdir(cwd)

    def cvsCommitAll(self):
        # cvs commit always commits the entire dna source directory.
        dirname = dnaDirectory.toOsSpecific()
        if not os.path.isdir(dirname):
            print 'Cannot CVS commit: invalid directory'
            return

        cwd = os.getcwd()
        os.chdir(dirname)
        cvsCommand = 'cvs commit -m "level editor"'
        print cvsCommand
        os.system(cvsCommand)
        os.chdir(cwd)

    # STYLE/DNA FILE FUNCTIONS
    def loadSpecifiedDNAFile(self):
        path = dnaDirectory.toOsSpecific()
        if not os.path.isdir(path):
            print 'LevelEditor Warning: Invalid default DNA directory!'
            print 'Using current directory'
            path = '.'
        dnaFilename = askopenfilename(
            defaultextension = '.dna',
            filetypes = (('DNA Files', '*.dna'), ('All files', '*')),
            initialdir = path,
            title = 'Load DNA File',
            parent = self.panel.component('hull'))
        if dnaFilename:
            self.loadDNAFromFile(dnaFilename)
            self.outputFile = dnaFilename
        print "Finished Load: ", dnaFilename

    def saveToSpecifiedDNAFile(self):
        path = dnaDirectory.toOsSpecific()
        if not os.path.isdir(path):
            print 'LevelEditor Warning: Invalid DNA save directory!'
            print 'Using current directory'
            path = '.'
        dnaFilename = asksaveasfilename(
            defaultextension = '.dna',
            filetypes = (('DNA Files', '*.dna'), ('All files', '*')),
            initialdir = path,
            title = 'Save DNA File as',
            parent = self.panel.component('hull'))
        if dnaFilename:
            self.outputDNA(dnaFilename)
            self.outputFile = dnaFilename

    def loadDNAFromFile(self, filename):
        print filename
        # Reset level, destroying existing scene/DNA hierarcy
        self.reset(fDeleteToplevel = 1, fCreateToplevel = 0,
                   fUpdateExplorer = 0)
        # Now load in new file
        if fUseCVS:
            self.cvsUpdate(filename)
        try:
            node = loadDNAFile(DNASTORE, Filename.fromOsSpecific(filename).cStr(), CSDefault, 1)
        except Exception:
            self.notify.debug("Couldn't load specified DNA file. Please make sure storage code has been specified in Config.prc file")
        if node.getNumParents() == 1:
            # If the node already has a parent arc when it's loaded, we must
            # be using the level editor and we want to preserve that arc.
            newNPToplevel = NodePath(node)
            newNPToplevel.reparentTo(hidden)
        else:
            # Otherwise, we should create a new arc for the node.
            newNPToplevel = hidden.attachNewNode(node)

        # Make sure the topmost file DNA object gets put under DNARoot
        newDNAToplevel = self.findDNANode(newNPToplevel)

        # reset the landmark block number:
        (self.landmarkBlock, needTraverse)=self.findHighestLandmarkBlock(
            newDNAToplevel, newNPToplevel)

        # Update toplevel variables
        if needTraverse:
            self.createToplevel(newDNAToplevel)
        else:
            self.createToplevel(newDNAToplevel, newNPToplevel)

        # Create visible representations of all the paths and battle cells
        self.createSuitPaths()
        self.hideSuitPaths()
        self.createBattleCells()
        self.hideBattleCells()
        # Set the title bar to have the filename to make it easier
        # to remember what file you are working on
        self.panel["title"] = 'Level Editor: ' + os.path.basename(filename)
        self.panel.sceneGraphExplorer.update()

    def outputDNADefaultFile(self):
        outputFile = self.outputFile
        if outputFile == None:
            outputFile = self.neighborhood + '_working.dna'
        file = os.path.join(dnaDirectory.toOsSpecific(), outputFile)
        self.outputDNA(file)

    def outputDNA(self, filename):
        print 'Saving DNA to: ', filename
        binaryFilename = Filename(filename)
        binaryFilename.setBinary()
        self.DNAData.writeDna(binaryFilename, Notify.out(), DNASTORE)

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
            elif classType.eq(DNA_FLAT_DOOR):
                tag = 'door_color:'
            elif classType.eq(DNA_CORNICE):
                tag = 'cornice_color:'
            elif classType.eq(DNA_PROP):
                tag = 'prop_color:'
            else:
                return
            # Valid type, add color to file
            filename = self.neighborhood + '_colors.txt'
            fname = Filename(dnaDirectory.getFullpath() +
                             '/stylefiles/' + filename)
            f = open(fname.toOsSpecific(), 'ab')
            f.write('%s Vec4(%.2f, %.2f, %.2f, 1.0)\n' %
                    (tag,
                     color[0]/255.0,
                     color[1]/255.0,
                     color[2]/255.0))
            f.close()

    def saveStyle(self, filename, style):
        # A generic routine to append a new style definition to one of
        # the style files.

        fname = Filename(dnaDirectory.getFullpath() +
                         '/stylefiles/' + filename)
        # We use binary mode to avoid Windows' end-of-line convention
        f = open(fname.toOsSpecific(), 'ab')
        # Add a blank line
        f.write('\n')
        # Now output style details to file
        style.output(f)
        # Close the file
        f.close()

    def saveBaselineStyle(self):
        if self.panel.currentBaselineDNA:
            # Valid baseline, add style to file
            filename = self.neighborhood + '_baseline_styles.txt'
            style = DNABaselineStyle(self.panel.currentBaselineDNA)
            self.saveStyle(filename, style)

    def saveWallStyle(self):
        if self.lastWall:
            # Valid wall, add style to file
            filename = self.neighborhood + '_wall_styles.txt'
            style = DNAWallStyle(self.lastWall)
            self.saveStyle(filename, style)

    def saveBuildingStyle(self):
        dnaObject = self.selectedDNARoot
        if dnaObject:
            if DNAClassEqual(dnaObject, DNA_FLAT_BUILDING):
                # Valid wall, add style to file
                filename = self.neighborhood + '_building_styles.txt'
                style = DNAFlatBuildingStyle(dnaObject)
                self.saveStyle(filename, style)
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
        if neighborhood == 'toontown_central':
            self.outputDir = 'ToontownCentral'
        elif neighborhood == 'donalds_dock':
            self.outputDir = 'DonaldsDock'
        elif neighborhood == 'minnies_melody_land':
            self.outputDir = 'MinniesMelodyLand'
        elif neighborhood == 'the_burrrgh':
            self.outputDir = 'TheBurrrgh'
        elif neighborhood == 'daisys_garden':
            self.outputDir = 'DaisysGarden'
        elif neighborhood == 'donalds_dreamland':
            self.outputDir = 'DonaldsDreamland'
        self.panel.editMenu.selectitem(neighborhood)
        self.styleManager.setEditMode(neighborhood)
        self.panel.updateHeightList(self.getCurrent('building_height'))
        self.selectMap(neighborhood)

    def getEditMode(self):
        return self.styleManager.getEditMode()

    # Level Style Attributes
    def __getitem__(self, attribute):
        """ Return top level entry in attribute dictionary """
        return self.styleManager.attributeDictionary[attribute]

    def getAttribute(self, attribute):
        """ Return specified attribute for current neighborhood """
        return self.styleManager.getAttribute(attribute)

    def hasAttribute(self, attribute):
        """ Return specified attribute for current neighborhood """
        return self.styleManager.hasAttribute(attribute)

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
    def setGroupNum(self, num):
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
        edgeLine.setColor(VBase4(0.0, 0.0, 0.5, 1))
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
        # Compute offset: a vector rotated 90 degrees clockwise
        offset = Vec3(relEndPos - relStartPos)
        offset.normalize()
        offset *= 0.1
        a = offset[0]
        offset.setX(offset[1])
        offset.setY(-1 * a)
        # Just to get it above the street
        offset.setZ(0.05)
        # Add offset to start and end to help differentiate lines
        relStartPos += offset
        relEndPos += offset
        # Draw arrow
        edgeLine.drawArrow(relStartPos,
                           relEndPos,
                           15, # arrow angle
                           1) # arrow length
        edgeLine.create()
        # Add a clickable sphere
        marker = self.suitPointMarker.copyTo(edgeLine)
        marker.setName('suitEdgeMarker')
        midPos = (relStartPos + relEndPos)/2.0
        marker.setPos(midPos)
        # Adjust color of highlighted lines
        if edge in self.visitedEdges:
            NodePath.setColor(edgeLine, 1, 0, 0, 1)
        # Clean up:
        tempNode.removeNode()
        return edgeLine

    def drawSuitPoint(self, suitPoint, pos, type, parent):
        marker = self.suitPointMarker.copyTo(parent)
        marker.setName("suitPointMarker")
        marker.setPos(pos)
        if (type == DNASuitPoint.STREETPOINT):
            marker.setColor(0, 0, 0.6)
            marker.setScale(0.4)
        elif (type == DNASuitPoint.FRONTDOORPOINT):
            marker.setColor(0, 0, 1)
            marker.setScale(0.5)
        elif (type == DNASuitPoint.SIDEDOORPOINT):
            marker.setColor(0, 0.6, 0.2)
            marker.setScale(0.5)
        # Highlight if necessary
        if suitPoint in self.visitedPoints:
            marker.setColor(1, 0, 0, 1)
        return marker

    def placeSuitPoint(self):
        v = self.getGridSnapIntersectionPoint()
        # get the absolute pos relative to the top level.
        # That is what gets stored in the point
        mat = base.direct.grid.getMat(self.NPToplevel)
        absPos = Point3(mat.xformPoint(v))
        print 'Suit point: ' + `absPos`
        # Store the point in the DNA. If this point is already in there,
        # it returns the existing point
        suitPoint = DNASTORE.storeSuitPoint(self.currentSuitPointType, absPos)
        print "placeSuitPoint: ", suitPoint
        # In case the point returned is a different type, update our type
        self.currentSuitPointType = suitPoint.getPointType()
        if not self.pointDict.has_key(suitPoint):
            marker = self.drawSuitPoint(suitPoint,
                absPos, self.currentSuitPointType,
                self.suitPointToplevel)
            self.pointDict[suitPoint] = marker
        self.currentSuitPointIndex = suitPoint.getIndex()

        if self.startSuitPoint:
            self.endSuitPoint = suitPoint
            # Make a new dna edge
            if DNAClassEqual(self.DNAParent, DNA_VIS_GROUP):
                zoneId = self.DNAParent.getName()

                suitEdge = DNASuitEdge(
                    self.startSuitPoint, self.endSuitPoint, zoneId)
                DNASTORE.storeSuitEdge(suitEdge)
                # Add edge to the current vis group so it can be written out
                self.DNAParent.addSuitEdge(suitEdge)
                # Draw a line to represent the edge
                edgeLine = self.drawSuitEdge(suitEdge, self.NPParent)
                # Store the line in a dict so we can hide/show them
                self.edgeDict[suitEdge] = edgeLine
                self.np2EdgeDict[edgeLine.id()] = [suitEdge, self.DNAParent]
                # Store the edge on each point in case we move the point
                # we can update the edge
                for point in [self.startSuitPoint, self.endSuitPoint]:
                    if self.point2edgeDict.has_key(point):
                        self.point2edgeDict[point].append(suitEdge)
                    else:
                        self.point2edgeDict[point] = [suitEdge]

                # If this is a building point, you need edges in both directions
                # so just make the other edge automatically
                if ((self.startSuitPoint.getPointType() == DNASuitPoint.FRONTDOORPOINT)
                    or (self.startSuitPoint.getPointType() == DNASuitPoint.SIDEDOORPOINT)):

                    suitEdge = DNASuitEdge(
                        self.endSuitPoint, self.startSuitPoint, zoneId)
                    DNASTORE.storeSuitEdge(suitEdge)
                    # Add edge to the current vis group so it can be written out
                    self.DNAParent.addSuitEdge(suitEdge)
                    # Draw a line to represent the edge
                    edgeLine = self.drawSuitEdge(suitEdge, self.NPParent)
                    # Store the line in a dict so we can hide/show them
                    self.edgeDict[suitEdge] = edgeLine
                    self.np2EdgeDict[edgeLine.id()] = [suitEdge, self.DNAParent]
                    for point in [self.startSuitPoint, self.endSuitPoint]:
                        if self.point2edgeDict.has_key(point):
                            self.point2edgeDict[point].append(suitEdge)
                        else:
                            self.point2edgeDict[point] = [suitEdge]

                print 'Added dnaSuitEdge to zone: ' + zoneId
            else:
                print 'Error: DNAParent is not a dnaVisGroup. Did not add edge'
            # Reset
            self.startSuitPoint = None
            self.endSuitPoint = None

        else:
            # First point, store it
            self.startSuitPoint = suitPoint

    def highlightConnected(self, nodePath = None, fReversePath = 0):
        if nodePath == None:
            nodePath = base.direct.selected.last
        if nodePath:
            suitPoint = self.findPointOrCell(nodePath)[0]
            if suitPoint:
                self.clearPathHighlights()
                self.highlightConnectedRec(suitPoint, fReversePath)

    def highlightConnectedRec(self, suitPoint, fReversePath):
        nodePath = self.pointDict.get(suitPoint, None)
        if nodePath:
            # highlight marker
            nodePath.setColor(1, 0, 0, 1)
            # Add point to visited points
            self.visitedPoints.append(suitPoint)
            # highlight connected edges
            for edge in self.point2edgeDict[suitPoint]:
                if ((fReversePath or (suitPoint == edge.getStartPoint())) and
                    (edge not in self.visitedEdges)):
                    edgeLine = self.edgeDict[edge]
                    # Call node path not LineNodePath setColor
                    NodePath.setColor(edgeLine, 1, 0, 0, 1)
                    # Add edge to visited edges
                    self.visitedEdges.append(edge)
                    # Color components connected to the edge
                    if fReversePath:
                        startPoint = edge.getStartPoint()
                        if startPoint not in self.visitedPoints:
                            self.highlightConnectedRec(startPoint,
                                                       fReversePath)
                    endPoint = edge.getEndPoint()
                    type = endPoint.getPointType()
                    if ((endPoint not in self.visitedPoints) and
                        (fReversePath or (type == DNASuitPoint.STREETPOINT))):
                        self.highlightConnectedRec(endPoint,
                                                   fReversePath)

    def clearPathHighlights(self):
        for point in self.pointDict.keys():
            type = point.getPointType()
            marker = self.pointDict[point]
            if (type == DNASuitPoint.STREETPOINT):
                marker.setColor(0, 0, 0.6)
            elif (type == DNASuitPoint.FRONTDOORPOINT):
                marker.setColor(0, 0, 1)
            elif (type == DNASuitPoint.SIDEDOORPOINT):
                marker.setColor(0, 0.6, 0.2)
        for edge in self.edgeDict.values():
            edge.clearColor()
        self.visitedPoints = []
        self.visitedEdges = []

    def drawBattleCell(self, cell, parent):
        marker = self.battleCellMarker.copyTo(parent)
        # Greenish
        marker.setColor(0.25, 0.75, 0.25, 0.5)
        marker.setTransparency(1)
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
        mat = base.direct.grid.getMat(self.NPParent)
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
            point = DNASTORE.getSuitPointAtIndex(i)
            marker = self.drawSuitPoint(point,
                point.getPos(), point.getPointType(),
                self.suitPointToplevel)
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
                self.np2EdgeDict[edgeLine.id()] = [edge, dnaVisGroup]
                # Store the edge on each point in case we move the point
                # we can update the edge
                for point in [edge.getStartPoint(), edge.getEndPoint()]:
                    if self.point2edgeDict.has_key(point):
                        self.point2edgeDict[point].append(edge)
                    else:
                        self.point2edgeDict[point] = [edge]

    def getSuitPointFromNodePath(self, nodePath):
        """
        Given a node path, attempt to find the point, nodePath pair
        in the pointDict. If it is there, return the point. If we
        cannot find it, return None.
        TODO: a reverse lookup pointDict would speed this up quite a bit
        """
        for point, marker in self.pointDict.items():
            if marker.eq(nodePath):
                return point
        return None

    def resetPathMarkers(self):
        for edge, edgeLine in self.edgeDict.items():
            if not edgeLine.isEmpty():
                edgeLine.reset()
                edgeLine.removeNode()
        self.edgeDict = {}
        self.np2EdgeDict = {}
        for point, marker in self.pointDict.items():
            if not marker.isEmpty():
                marker.removeNode()
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
            if not marker.isEmpty():
                marker.remove()
        self.cellDict = {}

    def hideBattleCells(self):
        for cell, marker in self.cellDict.items():
            marker.hide()

    def showBattleCells(self):
        for cell, marker in self.cellDict.items():
            marker.show()

    def getBattleCellFromNodePath(self, nodePath):
        """
        Given a node path, attempt to find the battle cell, nodePath pair
        in the cellDict. If it is there, return the cell. If we
        cannot find it, return None.
        TODO: a reverse lookup cellDict would speed this up quite a bit
        """
        for cell, marker in self.cellDict.items():
            if marker.eq(nodePath):
                return cell
        return None

    def toggleZoneColors(self):
        if self.panel.zoneColor.get():
            self.colorZones()
        else:
            self.clearZoneColors()

    def colorZones(self):
        # Give each zone a random color to see them better
        visGroups = self.getDNAVisGroups(self.NPToplevel)
        for visGroup in visGroups:
            np = visGroup[0]
            np.setColor(0.5 + random()/2.0,
                        0.5 + random()/2.0,
                        0.5 + random()/2.0)

    def clearZoneColors(self):
        # Clear random colors
        visGroups = self.getDNAVisGroups(self.NPToplevel)
        for visGroup in visGroups:
            np = visGroup[0]
            np.clearColor()

    def labelZones(self):
        # Label each zone
        # First clear out old labels if any
        self.clearZoneLabels()
        visGroups = self.getDNAVisGroups(self.NPToplevel)
        from direct.gui import DirectGui
        for np, dna in visGroups:
            name = dna.getName()
            label = DirectGui.DirectLabel(text = name,
                                          parent = np.getParent(),
                                          relief = None, scale = 3)
            label.setBillboardPointEye(0)
            center = np.getBounds().getCenter()
            label.setPos(center[0], center[1], .1)
            self.zoneLabels.append(label)

    def clearZoneLabels(self):
        for label in self.zoneLabels:
            label.removeNode()
        self.zoneLabels = []

    def getBlockFromName(self, name):
        block=name[2:name.find(':')]
        return block

    def addToLandmarkBlock(self):
        dnaRoot=self.selectedDNARoot
        if dnaRoot and self.lastLandmarkBuildingDNA:
            if DNAClassEqual(dnaRoot, DNA_FLAT_BUILDING):
                n=dnaRoot.getName()
                n=n[n.find(':'):]
                block=self.lastLandmarkBuildingDNA.getName()
                block=block[2:block.find(':')]
                dnaRoot.setName('tb'+block+n)
                self.replaceSelected()
                # If we're highlighting the landmark blocks:
                if self.showLandmarkBlockToggleGroup:
                    # then highlight this one:
                    np=self.selectedNPRoot
                    self.showLandmarkBlockToggleGroup.append(np)
                    np.setColor(1, 0, 0, 1)
        elif self.selectedSuitPoint and self.lastLandmarkBuildingDNA:
            block=self.lastLandmarkBuildingDNA.getName()
            block=block[2:block.find(':')]
            print ("associate point with building: " + str(block))
            self.selectedSuitPoint.setLandmarkBuildingIndex(int(block))
            marker = self.pointDict[self.selectedSuitPoint]
            marker.setColor(1, 0, 0, 1)
            marker.setScale(1.0)

    def findHighestLandmarkBlock(self, dnaRoot, npRoot):
        npc=npRoot.findAllMatches("**/*:toon_landmark_*")
        highest=0
        for i in range(npc.getNumPaths()):
            path=npc.getPath(i)
            block=path.getName()
            block=int(block[2:block.find(':')])
            if (block > highest):
                highest=block
        # Make a list of flat building names, outside of the
        # recursive function:
        self.flatNames=['random'] + BUILDING_TYPES
        self.flatNames=map(lambda n: n+'_DNARoot', self.flatNames)
        # Search/recurse the dna:
        newHighest=self.convertToLandmarkBlocks(highest, dnaRoot)
        # Get rid of the list of flat building names:
        del self.flatNames

        needToTraverse = (highest!=newHighest)
        return (newHighest, needToTraverse)

    def convertToLandmarkBlocks(self, block, dnaRoot):
        """
        Find all the buildings without landmark blocks and
        assign them one.
        """
        for i in range(dnaRoot.getNumChildren()):
            child = dnaRoot.at(i)
            if DNAClassEqual(child, DNA_LANDMARK_BUILDING):
                # Landmark buildings:
                name=child.getName()
                if name.find('toon_landmark_')==0:
                    block=block+1
                    child.setName('tb'+str(block)+':'+name)
            elif DNAClassEqual(child, DNA_FLAT_BUILDING):
                # Flat buildings:
                name=child.getName()
                if (name in self.flatNames):
                    child.setName('tb0:'+name)
            else:
                block = self.convertToLandmarkBlocks(block, child)
        return block

    def revertLandmarkBlock(self, block):
        """
        un-block flat buildings (set them to block zero).
        """
        npc=self.NPToplevel.findAllMatches("**/tb"+block+":*_DNARoot")
        for i in range(npc.getNumPaths()):
            nodePath=npc.getPath(i)
            name=nodePath.getName()
            if name[name.find(':'):][:15] != ':toon_landmark_':
                name='tb0'+name[name.find(':'):]
                dna=self.findDNANode(nodePath)
                dna.setName(name)
                nodePath=self.replace(nodePath, dna)
                # If we're highlighting the landmark blocks:
                if self.showLandmarkBlockToggleGroup:
                    # then highlight this one:
                    self.showLandmarkBlockToggleGroup.append(nodePath)
                    nodePath.setColor(0, 1, 0, 1)

    def landmarkBlockRemove(self, dna, nodePath):
        if dna:
            name=dna.getName()
            # Get the underscore index within the name:
            usIndex=name.find(':')
            if name[usIndex:][:15] == ':toon_landmark_':
                block=name[2:usIndex]
                self.lastLandmarkBuildingDNA=None
                self.revertLandmarkBlock(block)

    def toggleShowLandmarkBlock(self):
        dna=self.lastLandmarkBuildingDNA
        if dna:
            if not self.showLandmarkBlockToggleGroup:
                group=[]
                block=dna.getName()
                block=block[2:block.find(':')]

                # Get current landmark buildings:
                npc=self.NPToplevel.findAllMatches("**/tb"+block+":*_DNARoot")
                for i in range(npc.getNumPaths()):
                    nodePath=npc.getPath(i)
                    group.append(nodePath)
                    nodePath.setColor(1, 0, 0, 1)

                # Get block zero buildings (i.e. non-blocked):
                npc=self.NPToplevel.findAllMatches("**/tb0:*_DNARoot")
                for i in range(npc.getNumPaths()):
                    nodePath=npc.getPath(i)
                    group.append(nodePath)
                    nodePath.setColor(0, 1, 0, 1)

                # Get the suit point for this lb
                for point, marker in self.pointDict.items():
                    if ((point.getPointType() == DNASuitPoint.FRONTDOORPOINT)
                        or (point.getPointType() == DNASuitPoint.SIDEDOORPOINT)):
                        lbIndex = point.getLandmarkBuildingIndex()
                        if (lbIndex == int(block)):
                            marker.setColor(1, 0, 0, 1)
                            marker.setScale(1.0)
                            # There should only be one, so break now
                        elif (lbIndex == -1):
                            # This point belongs to no block
                            marker.setColor(0, 1, 0, 1)

                self.showLandmarkBlockToggleGroup=group
            else:
                for i in self.showLandmarkBlockToggleGroup:
                    i.clearColor()
                for point, marker in self.pointDict.items():
                    if (point.getPointType() == DNASuitPoint.FRONTDOORPOINT):
                        marker.setColor(0, 0, 1, 1)
                        marker.setScale(0.5)
                    elif (point.getPointType() == DNASuitPoint.SIDEDOORPOINT):
                        marker.setColor(0, 0.6, 0.2, 1)
                        marker.setScale(0.5)
                self.showLandmarkBlockToggleGroup=None

    def pdbBreak(self):
        pdb.set_trace()

    def reparentStreetBuildings(self, nodePath):
        dnaNode = self.findDNANode(nodePath)
        if dnaNode:
            if (DNAClassEqual(dnaNode, DNA_FLAT_BUILDING) or
                DNAClassEqual(dnaNode, DNA_LANDMARK_BUILDING)):
                base.direct.reparent(nodePath, fWrt = 1)
        children = nodePath.getChildren()
        for child in children:
            self.reparentStreetBuildings(child)

    def consolidateStreetBuildings(self):
        # First put everything under the ATR group so the leftover
        # can be easily deleted
        originalChildren = self.NPToplevel.getChildren()
        self.addGroup(self.NPToplevel)
        atrGroup = self.NPParent
        atrGroup.setName('ATR')
        self.setName(atrGroup, 'ATR')
        base.direct.setActiveParent(atrGroup)
        for child in originalChildren:
            base.direct.reparent(child)
        # Now create a new group with just the buildings
        self.addGroup(self.NPToplevel)
        newGroup = self.NPParent
        newGroup.setName('LongStreet')
        self.setName(newGroup, 'LongStreet')
        base.direct.setActiveParent(newGroup)
        self.reparentStreetBuildings(self.NPToplevel)
        return newGroup

    def makeNewBuildingGroup(self, sequenceNum, side, curveName):
        print "-------------------------- new building group %s  curveName=%s------------------------" % (sequenceNum, curveName)
        # Now create a new group with just the buildings
        self.addGroup(self.NPToplevel)
        newGroup = self.NPParent
        groupName = ''
        #if curveName == "urban_curveside_inner_1_1":
        #    import pdb; pdb.set_trace()

        if 'curveside' in curveName:
            #we want to preserve which group the side street is closest to
            print "special casing %s" % curveName
            parts = curveName.split('_')
            groupName = 'Buildings_' + side + "-" + parts[3] + "_" + parts[4]
            print "groupname = %s" % groupName
        else:
            groupName = 'Buildings_' + side + "-" + str(sequenceNum)
        newGroup.setName(groupName)
        self.setName(newGroup, groupName)
        base.direct.setActiveParent(newGroup)

        if 'barricade_curve' in curveName:
            parts = curveName.split('_')
            origBarricadeNum = parts[3]
            self.updateBarricadeDict(side, int(origBarricadeNum),  sequenceNum)

    def adjustPropChildren(self, nodePath, maxPropOffset = -4):
        for np in nodePath.getChildren():
            dnaNode = self.findDNANode(np)
            if dnaNode:
                if DNAClassEqual(dnaNode, DNA_PROP):
                    if np.getY() < maxPropOffset:
                        np.setY(maxPropOffset)
                        self.updateSelectedPose([np])

    def getBuildingWidth(self, bldg):
        dnaNode = self.findDNANode(bldg)
        bldgWidth = 0
        if DNAClassEqual(dnaNode, DNA_FLAT_BUILDING):
            bldgWidth = dnaNode.getWidth()
        elif DNAClassEqual(dnaNode, DNA_LANDMARK_BUILDING):
            objectCode = dnaNode.getCode()
            if objectCode[-2:-1] == 'A':
                bldgWidth = 25.0
            elif objectCode[-2:-1] == 'B':
                bldgWidth = 15.0
            elif objectCode[-2:-1] == 'C':
                bldgWidth = 20.0
        return bldgWidth

    def calcLongStreetLength(self, bldgs):
        streetLength = 0
        for bldg in bldgs:
            streetLength += self.getBuildingWidth(bldg)
        return streetLength

    def addStreetUnits(self, streetLength):
        base.direct.grid.setPosHpr(0, -40, 0, 0, 0, 0)
        currLength = 0
        while (currLength < streetLength):
            self.addStreet('street_80x40')
            currLength += 80

    def makeLongStreet(self):
        bldgGroup = self.consolidateStreetBuildings()
        bldgs = bldgGroup.getChildren()
        numBldgs = len(bldgs)
        streetLength = self.calcLongStreetLength(bldgs)/2.0
        ref = None
        base.direct.grid.fXyzSnap = 0
        currLength = 0
        for i in range(numBldgs):
            bldg = bldgs[i]
            if ref == None:
                base.direct.grid.iPosHpr(bldgGroup)
            else:
                ref.select()
                self.autoPositionGrid(fLerp = 0)
            if base.direct.grid.getX() >= streetLength:
                base.direct.grid.setPosHpr(base.direct.grid, 0, -40, 0, 180, 0, 0)
            bldg.iPosHpr(base.direct.grid)
            self.updateSelectedPose([bldg])
            self.adjustPropChildren(bldg)
            ref = bldg
        self.addStreetUnits(streetLength)

    def loadStreetCurve(self):
        path = '.'
        streetCurveFilename = askopenfilename(
            defaultextension = '.egg',
            filetypes = (('Egg files', '*.egg'),
                         ('Bam files', '*.bam'),
                         ('Maya files', '*.mb'),
                         ('All files', '*')),
            initialdir = path,
            title = 'Load Curve File',
            parent = self.panel.component('hull'))
        if streetCurveFilename:
            modelFile = loader.loadModel(Filename.fromOsSpecific(streetCurveFilename))
            #curves = modelFile.findAllMatches('**/+ClassicNurbsCurve')
            curves = {'inner':[], 'outer':[],'innersidest':[], 'outersidest':[]}
            curvesInner = modelFile.findAllMatches('**/*curve_inner*')
            print("-------------- curvesInner-----------------")
            print curvesInner
            curvesOuter = modelFile.findAllMatches('**/*curve_outer*')
            print("---------------- curvesOuter---------------")
            print curvesOuter
            curveInnerSideSts = modelFile.findAllMatches('**/*curveside_inner*')
            print("--------- curveInnerSideSts----------")
            print curveInnerSideSts

            curveOuterSideSts = modelFile.findAllMatches('**/*curveside_outer*')
            print("----------- curveOuterSideSits ----------")
            print curveOuterSideSts

            # return an ordered list
            for i in range(len(curvesInner) +1): #RAU don't forget, these curves are 1 based
                curve = modelFile.find('**/*curve_inner_'+str(i))
                if not curve.isEmpty():
                    # Mark whether it is a section of buildings or trees
                    curveType = curve.getName().split("_")[0]
                    curves['inner'].append([curve.node(), curveType])

            for i in range(len(curvesOuter) +1):
                curve = modelFile.find('**/*curve_outer_'+str(i))
                if not curve.isEmpty():
                    # Mark whether it is a section of buildings or trees
                    curveType = curve.getName().split("_")[0]
                    curves['outer'].append([curve.node(), curveType])

            maxNum = len(curvesInner)
            if len(curvesOuter) > maxNum:
                maxNum = len(curvesOuter)

            maxNum += 2; #track ends in a barricade, and add 1 since 1 based
            #RAU also do special processing for the side streets
            # side streets are numbered differently and could be non consecutive
            # curveside_inner_28_1, curveside_outer_28_1, curveside_inner_28_2,
            # curveside_outer_28_2 (two side streets closest to main building track 28)
            for i in range(maxNum):
                for barricade in ['innerbarricade','outerbarricade']:
                    curve = modelFile.find('**/*curveside_inner_'+ barricade + '_' + str(i))
                    if not curve.isEmpty():
                        # Mark whether it is a section of buildings or trees
                        curveType = curve.getName().split("_")[0]
                        curves['innersidest'].append([curve.node(), curveType])
                        print "adding innersidest %s" % curve.getName()

            for i in range(maxNum):
                for barricade in ['innerbarricade','outerbarricade']:
                    curve = modelFile.find('**/*curveside_outer_'+ barricade + '_' + str(i))
                    if not curve.isEmpty():
                        # Mark whether it is a section of buildings or trees
                        curveType = curve.getName().split("_")[0]
                        curves['outersidest'].append([curve.node(), curveType])
                        print "adding outersidest %s" % curve.getName()


            print "loaded curves: %s" % curves
            return curves
        else:
            return None

    def duplicateFlatBuilding(self, oldDNANode):
        # Yes, make a new copy of the dnaNode
        print "a"
        dnaNode = oldDNANode.__class__(oldDNANode)
        print "b"
        dnaNode.setWidth(oldDNANode.getWidth())
        # Add the DNA to the active parent
        print "c"
        self.DNAParent.add(dnaNode)
        # And create the geometry
        print "d %s" % (oldDNANode)
        newNodePath = dnaNode.traverse(self.NPParent, DNASTORE, 1)
        print "e"
        return newNodePath

    def getBldg(self, bldgIndex, bldgs, forceDuplicate = False):
        numBldgs = len(bldgs)
        if bldgIndex < numBldgs and not forceDuplicate:
            # Use original
            print "using original bldg"
            bldg = bldgs[bldgIndex]
            bldgIndex += 1
        else:
            # Make a copy
            oldBldg = bldgs[bldgIndex % numBldgs]
            bldgIndex += 1

            oldBldg.select()
            oldDNANode = self.findDNANode(oldBldg)
            nodeClass = DNAGetClassType(oldDNANode)
            if nodeClass.eq(DNA_LANDMARK_BUILDING):
                print "making landmark copy"
                # Remove white and dark grey doors from color list
                colorList = self.getAttribute('door_color').getList()
                colorList = colorList[1:3] + colorList[4:len(colorList)]
                # Set a random door color
                doorColor = random.choice(colorList)
                self.setCurrent('door_color', doorColor)
                self.addLandmark(oldDNANode.getCode(), oldDNANode.getBuildingType())
                bldg = self.lastNodePath
            else:
                print "making flatbuilding copy"
                bldg = self.duplicateFlatBuilding(oldDNANode)
        return bldg, bldgIndex

    def updateBarricadeDict(self, side, barricadeOrigNum, curBldgGroupIndex):
        barricadeDict = None
        if (side == 'outer'):
            barricadeDict = self.outerBarricadeDict
        elif (side == 'inner'):
            barricadeDict = self.innerBarricadeDict
        else:
            print("unhandled side %s" % side)
            return

        if not barricadeDict.has_key(barricadeOrigNum):
            barricadeDict[barricadeOrigNum] = [curBldgGroupIndex, curBldgGroupIndex]

        if curBldgGroupIndex < barricadeDict[barricadeOrigNum][0]:
            barricadeDict[barricadeOrigNum][0] = curBldgGroupIndex

        if barricadeDict[barricadeOrigNum][1] < curBldgGroupIndex:
            barricadeDict[barricadeOrigNum][1] = curBldgGroupIndex

        print "---------- %s barricadeDict origNum=%d  data=(%d, %d)" %(side, barricadeOrigNum, barricadeDict[barricadeOrigNum][0], barricadeDict[barricadeOrigNum][1])

    def makeStreetAlongCurve(self):
        curves = self.loadStreetCurve()
        if curves == None:
            return

        self.outerBarricadeDict = {}
        self.innerBarricadeDict = {}


        base.direct.grid.fXyzSnap = 0
        base.direct.grid.fHprSnap = 0
        self.panel.fPlaneSnap.set(0)
        bldgGroup = self.consolidateStreetBuildings()
        bldgs = bldgGroup.getChildren()

        # streetWidth puts buildings on the edge of the street, not the middle
        currPoint = Point3(0)
        bldgIndex = 0

        #populate side streets
        self.makeSideStreets(curves)

        # Populate buildings on both sides of the street
        #sides = ['inner', 'outer','innersidest','outersidest']
        sides = ['inner', 'outer']
        maxGroupWidth = 500
        for side in sides:
            print "Building street for %s side" % side
            # Subdivide the curve into different groups.
            bldgGroupIndex = 0
            curGroupWidth = 0
            curveName = '';
            if len(curves[side]):
                initialCurve, initialCurveType = curves[side][0]
                if initialCurve:
                    curveName = initialCurve.getName()
            self.makeNewBuildingGroup(bldgGroupIndex, side, curveName)

            for curve, curveType in curves[side]:
                print "----------------- curve(%s, %s): %s --------------- " % (side, curve.getName(), curve)
                #import pdb; pdb.set_trace()
                currT = 0
                endT = curve.getMaxT()

                while currT < endT:
                    if curveType == 'urban':
                        bldg, bldgIndex = self.getBldg(bldgIndex, bldgs)
                        curve.getPoint(currT, currPoint)

                        if side == "inner" or side == "innersidest":
                            heading = 90
                        else:
                            heading = -90
                        bldg.setPos(currPoint)
                        bldgWidth = self.getBuildingWidth(bldg)

                        curGroupWidth += bldgWidth
                        # Adjust grid orientation based upon next point along curve
                        currT, currPoint = self.findBldgEndPoint(bldgWidth, curve, currT, currPoint, rd = 0)
                        bldg.lookAt(Point3(currPoint))
                        bldg.setH(bldg, heading)

                        # Shift building forward if it is on the out track, since we just turned it away from
                        # the direction of the track
                        if side == "outer" or side == "outersidest":
                            bldg.setPos(currPoint)

                        self.updateSelectedPose([bldg])
                        self.adjustPropChildren(bldg)
                        base.direct.reparent(bldg, fWrt = 1)
                        print bldgIndex
                    elif curveType == 'trees':
                        curve.getPoint(currT, currPoint)
                        # trees are spaced anywhere from 40-80 ft apart
                        treeWidth = random.randint(40, 80)
                        curGroupWidth += treeWidth
                        # Adjust grid orientation based upon next point along curve
                        currT, currPoint = self.findBldgEndPoint(treeWidth, curve, currT, currPoint, rd = 0)

                        # Add some trees
                        tree = random.choice(["prop_tree_small_ul",
                                                "prop_tree_small_ur",
                                                "prop_tree_large_ur",
                                                "prop_tree_large_ul"])

                        #use snow if necessaryy

                        if (useSnowTree):
                            tree = random.choice(["prop_snow_tree_small_ul",
                                                    "prop_snow_tree_small_ur",
                                                    "prop_snow_tree_large_ur",
                                                    "prop_snow_tree_large_ul"])


                        self.addProp(tree)
                        for selectedNode in base.direct.selected:
                            # Move it
                            selectedNode.setPos(currPoint)
                            # Snap objects to grid and update DNA if necessary
                            self.updateSelectedPose(base.direct.selected.getSelectedAsList())
                    elif curveType == 'bridge':
                        # Don't add any dna for the bridge sections, but add the length
                        # of the bridge so we can increment our building groups correctly
                        print "adding bridge (%s), curT = %s" % (side, currT)
                        bridgeWidth = 1050
                        curGroupWidth += bridgeWidth
                        #currT, currPoint = self.findBldgEndPoint(bridgeWidth, curve, currT, currPoint, rd = 0)
                        print "currT after adding bridge = %s" % currT
                        # force move to next curve
                        currT = endT + 1
                    elif curveType == 'tunnel':
                        # Don't add any dna for the tunnel sections, but add the length
                        # of the bridge so we can increment our building groups correctly
                        print "adding tunnel (%s), curT = %s" % (side, currT)
                        tunnelWidth = 775
                        curGroupWidth += tunnelWidth
                        #currT, currPoint = self.findBldgEndPoint(tunnelWidth, curve, currT, currPoint, rd = 0)
                        print "currT after adding tunnel = %s" % currT
                        # force move to next curve
                        currT = endT + 1
                    elif curveType == 'barricade':
                        print "adding barricade (%s) %s, curT = %d" % (side, curve.getName(), currT)
                        barricadeWidth = curve.calcLength()
                        print "barricade width = %f" % barricadeWidth

                        simple =1
                        if (simple):
                            curGroupWidth += barricadeWidth
                            # force move to next curve
                            currT = endT + 1
                        else:
                            #add a prop_tree to force it to be shown
                            curve.getPoint(currT, currPoint)
                            #trees are spaced anywhere from 40-80 ft apart
                            #treeWidth = random.randint(40, 80)
                            treeWidth = barricadeWidth
                            curGroupWidth += treeWidth
                            # Adjust grid orientation based upon next point along curve
                            currT, currPoint = self.findBldgEndPoint(treeWidth, curve, currT, currPoint, rd = 0)

                            # Add some trees
                            tree = random.choice(["prop_snow_tree_small_ul",
                                                "prop_snow_tree_small_ur",
                                                "prop_snow_tree_large_ur",
                                                "prop_snow_tree_large_ul"])
                            self.addProp(tree)
                            for selectedNode in base.direct.selected:
                                # Move it
                                selectedNode.setPos(currPoint)
                                # Snap objects to grid and update DNA if necessary
                                self.updateSelectedPose(base.direct.selected.getSelectedAsList())





                    # Check if we need a new group yet
                    if curGroupWidth > maxGroupWidth:
                        print "curGroupWidth %s > %s" % (curGroupWidth, maxGroupWidth)
                        diffGroup = curGroupWidth - maxGroupWidth
                        while diffGroup > 0:
                            bldgGroupIndex += 1
                            self.makeNewBuildingGroup(bldgGroupIndex, side, curve.getName())
                            print "adding group %s (%s)" % (bldgGroupIndex, diffGroup)
                            diffGroup -= maxGroupWidth
                        curGroupWidth = 0
                    print currT, curGroupWidth


    def makeSideStreets(self, curves):
        """ Each side in a sidestreet MUST be in 1 building group, otherwise the 2nd half
        of a building group could be very far away. This would cause the stashing and
        unstashing code to go off kilter.
        """

        base.direct.grid.fXyzSnap = 0
        base.direct.grid.fHprSnap = 0
        self.panel.fPlaneSnap.set(0)
        bldgGroup = self.consolidateStreetBuildings()
        bldgs = bldgGroup.getChildren()

        # streetWidth puts buildings on the edge of the street, not the middle
        currPoint = Point3(0)
        bldgIndex = 0

        # Populate buildings on both sides of the street
        #sides = ['inner', 'outer','innersidest','outersidest']
        sides = ['innersidest', 'outersidest']
        maxGroupWidth = 50000
        for side in sides:
            print "Building street for %s side" % side
            # Subdivide the curve into different groups.
            bldgGroupIndex = 0
            curGroupWidth = 0


            for curve, curveType in curves[side]:
                print "----------------- curve(%s, %s): %s --------------- " % (side, curve.getName(), curve)
                #import pdb; pdb.set_trace()
                currT = 0
                endT = curve.getMaxT()

                #RAU side streets still too long, lets try arbitrarily dividing it in half
                #endT = endT / 2

                print ("endT = %f" % endT)
                #if (maxGroupWidth < endT):
                #    self.notify.debug("changing endT from %f to %f" % (endT, maxGroupWidth))
                #    endT = maxGroupWidth


                currGroupWidth = 0
                self.makeNewBuildingGroup(bldgGroupIndex, side, curve.getName())

                while currT < endT:
                    if curveType == 'urban':
                        bldg, bldgIndex = self.getBldg(bldgIndex, bldgs, forceDuplicate = True)
                        curve.getPoint(currT, currPoint)

                        if side == "inner" or side == "innersidest":
                            heading = 90
                        else:
                            heading = -90
                        bldg.setPos(currPoint)
                        bldgWidth = self.getBuildingWidth(bldg)

                        curGroupWidth += bldgWidth
                        # Adjust grid orientation based upon next point along curve
                        currT, currPoint = self.findBldgEndPoint(bldgWidth, curve, currT, currPoint, rd = 0)
                        bldg.lookAt(Point3(currPoint))
                        bldg.setH(bldg, heading)

                        # Shift building forward if it is on the out track, since we just turned it away from
                        # the direction of the track
                        if side == "outer" or side == "outersidest":
                            bldg.setPos(currPoint)

                        self.updateSelectedPose([bldg])
                        self.adjustPropChildren(bldg)
                        base.direct.reparent(bldg, fWrt = 1)
                        print bldgIndex
                    elif curveType == 'trees':
                        curve.getPoint(currT, currPoint)
                        # trees are spaced anywhere from 40-80 ft apart
                        treeWidth = random.randint(40, 80)
                        curGroupWidth += treeWidth
                        # Adjust grid orientation based upon next point along curve
                        currT, currPoint = self.findBldgEndPoint(treeWidth, curve, currT, currPoint, rd = 0)

                        # Add some trees
                        tree = random.choice(["prop_tree_small_ul",
                                                "prop_tree_small_ur",
                                                "prop_tree_large_ur",
                                                "prop_tree_large_ul"])

                        #use snow tree if necessary
                        if (useSnowTree):
                            tree = random.choice(["prop_snow_tree_small_ul",
                                                    "prop_snow_tree_small_ur",
                                                    "prop_snow_tree_large_ur",
                                                    "prop_snow_tree_large_ul"])


                        self.addProp(tree)
                        for selectedNode in base.direct.selected:
                            # Move it
                            selectedNode.setPos(currPoint)
                            # Snap objects to grid and update DNA if necessary
                            self.updateSelectedPose(base.direct.selected.getSelectedAsList())
                    elif curveType == 'bridge':
                        # Don't add any dna for the bridge sections, but add the length
                        # of the bridge so we can increment our building groups correctly
                        print "adding bridge (%s), curT = %s" % (side, currT)
                        bridgeWidth = 1050
                        curGroupWidth += bridgeWidth
                        #currT, currPoint = self.findBldgEndPoint(bridgeWidth, curve, currT, currPoint, rd = 0)
                        print "currT after adding bridge = %s" % currT
                        # force move to next curve
                        currT = endT + 1
                    elif curveType == 'tunnel':
                        # Don't add any dna for the tunnel sections, but add the length
                        # of the bridge so we can increment our building groups correctly
                        print "adding tunnel (%s), curT = %s" % (side, currT)
                        tunnelWidth = 775
                        curGroupWidth += tunnelWidth
                        #currT, currPoint = self.findBldgEndPoint(tunnelWidth, curve, currT, currPoint, rd = 0)
                        print "currT after adding tunnel = %s" % currT
                        # force move to next curve
                        currT = endT + 1
                    elif curveType == 'barricade':
                        print "adding barricade (%s) %s, curT = %d" % (side, curve.getName(), currT)
                        barricadeWidth = curve.calcLength()
                        print "barricade width = %f" % barricadeWidth

                        simple =1
                        if (simple):
                            curGroupWidth += barricadeWidth
                            # force move to next curve
                            currT = endT + 1
                        else:
                            #add a prop_tree to force it to be shown
                            curve.getPoint(currT, currPoint)
                            #trees are spaced anywhere from 40-80 ft apart
                            #treeWidth = random.randint(40, 80)
                            treeWidth = barricadeWidth
                            curGroupWidth += treeWidth
                            # Adjust grid orientation based upon next point along curve
                            currT, currPoint = self.findBldgEndPoint(treeWidth, curve, currT, currPoint, rd = 0)

                            # Add some trees
                            tree = random.choice(["prop_snow_tree_small_ul",
                                                "prop_snow_tree_small_ur",
                                                "prop_snow_tree_large_ur",
                                                "prop_snow_tree_large_ul"])
                            self.addProp(tree)
                            for selectedNode in base.direct.selected:
                                # Move it
                                selectedNode.setPos(currPoint)
                                # Snap objects to grid and update DNA if necessary
                                self.updateSelectedPose(base.direct.selected.getSelectedAsList())

                #done with for loop, increment bldgGroupIndex
                bldgGroupIndex += 1

    def findBldgEndPoint(self, bldgWidth, curve, currT, currPoint,
                         startT = None, endT = None, tolerance = 0.1, rd = 0):
        if startT == None:
            startT = currT
        if endT == None:
            endT = curve.getMaxT()
        if rd > 100:
            import pdb
            pdb.set_trace()
        midT = (startT + endT)/2.0
        midPoint = Point3(0)
        curve.getPoint(midT, midPoint)
        separation = Vec3(midPoint - currPoint).length()
        error = separation - bldgWidth
        #print error, startT, midT
        if abs(error) < tolerance:
            return midT, midPoint
        elif error > 0:
            # Mid point was beyond building end point, focus on first half
            return self.findBldgEndPoint(bldgWidth, curve, currT, currPoint, startT = startT, endT = midT,
                                         rd = rd + 1)
        else:
            # End point beyond Mid point, focus on second half
            # But make sure buildind end point is not beyond curve end point
            endPoint = Point3(0)
            curve.getPoint(endT, endPoint)
            separation = Vec3(endPoint - currPoint).length()
            if bldgWidth > separation:
                # Must have reached end of the curve
                return endT, endPoint
            else:
                return self.findBldgEndPoint(bldgWidth, curve, currT, currPoint, startT = midT, endT = endT,
                                             rd = rd + 1)

class OldLevelEditor(NodePath, DirectObject):
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
        hull.geometry('400x625')

        balloon = self.balloon = Pmw.Balloon(hull)
        # Start with balloon help disabled
        self.balloon.configure(state = 'none')

        menuFrame = Frame(hull, relief = GROOVE, bd = 2)
        menuFrame.pack(fill = X)

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
                            'CVS update directory',
                            label = 'CVS update',
                            command = self.levelEditor.cvsUpdateAll)
        menuBar.addmenuitem('Level Editor', 'command',
                            'CVS commit directory',
                            label = 'CVS commit',
                            command = self.levelEditor.cvsCommitAll)
        menuBar.addmenuitem('Level Editor', 'command',
                            'Edit Visibility Groups',
                            label = 'Edit Vis Groups',
                            command = self.levelEditor.editDNAVisGroups)
        menuBar.addmenuitem('Level Editor', 'command',
                            'Reset level',
                            label = 'Reset level',
                            command = self.levelEditor.reset)
        menuBar.addmenuitem('Level Editor', 'command',
                            'Make Long Street',
                            label = 'Make Long Street',
                            command = self.levelEditor.makeLongStreet)
        menuBar.addmenuitem('Level Editor', 'command',
                            'Make Street Along Curve',
                            label = 'Make Street Along Curve',
                            command = self.levelEditor.makeStreetAlongCurve)
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
                            "Save Selected Baseline's Style",
                            label = 'Save Baseline Style',
                            command = self.levelEditor.saveBaselineStyle)
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
                            'Reload Baseline Style Palettes',
                            label = 'Reload Baseline Styles',
                            command = self.styleManager.createBaselineStyleAttributes)
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
            label_text = 'Edit Mode:', entry_width = 18,
            selectioncommand = self.levelEditor.setEditMode, history = 0,
            scrolledlist_items = NEIGHBORHOODS)
        self.editMenu.selectitem(NEIGHBORHOODS[0])
        self.editMenu.pack(side = LEFT, expand = 0)


        # Create the notebook pages
        self.notebook = Pmw.NoteBook(hull)
        self.notebook.pack(fill = BOTH, expand = 1)
        streetsPage = self.notebook.add('Streets')
        toonBuildingsPage = self.notebook.add('Toon Bldgs')
        landmarkBuildingsPage = self.notebook.add('Landmark Bldgs')
        # suitBuildingsPage = self.notebook.add('Suit Buildings')
        propsPage = self.notebook.add('Props')
        signPage = self.notebook.add('Signs')
        suitPathPage = self.notebook.add('Paths')
        battleCellPage = self.notebook.add('Cells')
        sceneGraphPage = self.notebook.add('SceneGraph')
        self.notebook['raisecommand'] = self.updateInfo

        # STREETS
        Label(streetsPage, text = 'Streets',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        self.addStreetButton = Button(
            streetsPage,
            text = 'ADD STREET',
            command = self.addStreet)
        self.addStreetButton.pack(fill = X, padx = 20, pady = 10)
        streets = map(lambda s: s[7:],
                      self.styleManager.getCatalogCodes(
            'street'))
        streets.sort()
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
            scrolledlist_items = streets
            )
        self.streetModuleType = self.styleManager.getCatalogCode('street', 0)
        self.streetSelector.selectitem(self.streetModuleType[7:])
        self.streetSelector.pack(expand = 1, fill = BOTH)

        # TOON BUILDINGS
        Label(toonBuildingsPage, text = 'Toon Buildings',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        self.addToonBuildingButton = Button(
            toonBuildingsPage,
            text = 'ADD TOON BUILDING',
            command = self.addFlatBuilding)
        self.addToonBuildingButton.pack(fill = X, padx = 20, pady = 10)
        self.toonBuildingSelector = Pmw.ComboBox(
            toonBuildingsPage,
            dropdown = 0,
            listheight = 200,
            labelpos = W,
            label_width = 12,
            label_anchor = W,
            label_text = 'Bldg type:',
            entry_width = 30,
            selectioncommand = self.setFlatBuildingType,
            scrolledlist_items = (['random'] + BUILDING_TYPES)
            )
        bf = Frame(toonBuildingsPage)
        Label(bf, text = 'Building Height:').pack(side = LEFT, expand = 0)
        self.heightMode = IntVar()
        self.heightMode.set(20)
        self.tenFootButton = Radiobutton(
            bf,
            text = '10 ft',
            value = 10,
            variable = self.heightMode,
            command = self.setFlatBuildingHeight)
        self.tenFootButton.pack(side = LEFT, expand = 1, fill = X)
        self.fourteenFootButton = Radiobutton(
            bf,
            text = '14 ft',
            value = 14,
            variable = self.heightMode,
            command = self.setFlatBuildingHeight)
        self.fourteenFootButton.pack(side = LEFT, expand = 1, fill = X)
        self.twentyFootButton = Radiobutton(
            bf,
            text = '20 ft',
            value = 20,
            variable = self.heightMode,
            command = self.setFlatBuildingHeight)
        self.twentyFootButton.pack(side = LEFT, expand = 1, fill = X)
        self.twentyFourFootButton = Radiobutton(
            bf,
            text = '24 ft',
            value = 24,
            variable = self.heightMode,
            command = self.setFlatBuildingHeight)
        self.twentyFourFootButton.pack(side = LEFT, expand = 1, fill = X)
        self.twentyFiveFootButton = Radiobutton(
            bf,
            text = '25 ft',
            value = 25,
            variable = self.heightMode,
            command = self.setFlatBuildingHeight)
        self.twentyFiveFootButton.pack(side = LEFT, expand = 1, fill = X)
        self.thirtyFootButton = Radiobutton(
            bf,
            text = '30 ft',
            value = 30,
            variable = self.heightMode,
            command = self.setFlatBuildingHeight)
        self.thirtyFootButton.pack(side = LEFT, expand = 1, fill = X)
        bf.pack(fill = X)

        self.toonBuildingType = 'random'
        self.toonBuildingSelector.selectitem(self.toonBuildingType)
        self.toonBuildingSelector.pack(expand = 1, fill = BOTH)

        # LANDMARK BUILDINGS
        Label(landmarkBuildingsPage, text = 'Landmark Buildings',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)

        """
        self.landmarkHQIntVar = IntVar()
        self.landmarkHQIntVar.set(0)
        self.landmarkHQButton = Checkbutton(
            landmarkBuildingsPage,
            text = 'HQ',
            variable=self.landmarkHQIntVar,
            command=self.setLandmarkHQ)
        self.landmarkHQButton.pack(side = LEFT, expand = 1, fill = X)
        """

        self.addLandmarkBuildingButton = Button(
            landmarkBuildingsPage,
            text = 'ADD LANDMARK BUILDING',
            command = self.addLandmark)
        self.addLandmarkBuildingButton.pack(fill = X, padx = 20, pady = 10)
        bldgs = map(lambda s: s[14:],
                    self.styleManager.getCatalogCodes(
            'toon_landmark'))
        bldgs.sort()
        self.landmarkBuildingSelector = Pmw.ComboBox(
            landmarkBuildingsPage,
            dropdown = 0,
            listheight = 200,
            labelpos = W,
            label_width = 12,
            label_anchor = W,
            label_text = 'Bldg type:',
            entry_width = 30,
            selectioncommand = self.setLandmarkType,
            scrolledlist_items = bldgs
            )
        self.landmarkType = self.styleManager.getCatalogCode(
            'toon_landmark', 0)
        self.landmarkBuildingSelector.selectitem(
            self.styleManager.getCatalogCode('toon_landmark', 0)[14:])
        self.landmarkBuildingSelector.pack(expand = 1, fill = BOTH)

        self.landmarkBuildingSpecialSelector = Pmw.ComboBox(
            landmarkBuildingsPage,
            dropdown = 0,
            listheight = 100,
            labelpos = W,
            label_width = 12,
            label_anchor = W,
            label_text = 'Special type:',
            entry_width = 30,
            selectioncommand = self.setLandmarkSpecialType,
            scrolledlist_items = LANDMARK_SPECIAL_TYPES
            )
        self.landmarkSpecialType = LANDMARK_SPECIAL_TYPES[0]
        self.landmarkBuildingSpecialSelector.selectitem(
            LANDMARK_SPECIAL_TYPES[0])
        self.landmarkBuildingSpecialSelector.pack(expand = 0)

        # SIGNS
        Label(signPage, text = 'Signs',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        self.currentSignDNA=None
        self.currentBaselineDNA=None
        self.levelEditor.selectedNodePathHookHooks.append(self.updateSignPage)

        gridFrame = Frame(signPage)
        signSelectedFrame = Frame(gridFrame)

        self.currentBaselineIndex=0
        self.baselineMenu = Pmw.ComboBox(
            signSelectedFrame,
            labelpos = W,
            label_text = 'Selected:', entry_width = 24,
            selectioncommand = self.selectSignBaseline,
            history = 0, # unique = 0,
            scrolledlist_items = ['<the sign>'])
        self.baselineMenu.selectitem(self.currentBaselineIndex)
        self.baselineMenu.pack(side = LEFT, expand = 1, fill = X)

        self.baselineAddButton = Button(
            signSelectedFrame,
            text="Add Baseline", command=self.addBaseline)
        self.baselineAddButton.pack(side = LEFT, expand = 1, fill = X)

        self.baselineDeleteButton = Button(
            signSelectedFrame,
            text="Del", command=self.deleteSignItem)
        self.baselineDeleteButton.pack(side = LEFT, expand = 1, fill = X)

        signSelectedFrame.grid(row=0, column=0, columnspan=6)

        self.baselineString=StringVar()
        self.baselineString.trace("wu", self.signBaselineTrace)
        self.baselineTextBox = Entry(
            gridFrame, width = 24,
            textvariable=self.baselineString)
        self.baselineTextBox.grid(row=1, column=0, columnspan=3)

        fontList = [""]+self.styleManager.getCatalogCodes('font')
        self.fontMenu = Pmw.ComboBox(
            gridFrame, labelpos = W,
            label_text = 'Font:', entry_width = 12,
            selectioncommand = self.setSignBaslineFont, history = 0,
            scrolledlist_items = fontList)
        self.fontMenu.selectitem(0)
        self.fontMenu.grid(row=1, column=3, columnspan=3)

        graphicList = self.styleManager.getCatalogCodes('graphic')
        self.graphicMenu = Pmw.ComboBox(
            gridFrame, labelpos = W,
            label_text = 'Add Graphic:', entry_width = 24,
            selectioncommand = self.addSignGraphic, history = 0,
            scrolledlist_items = graphicList)
        self.graphicMenu.selectitem(0)
        self.graphicMenu.grid(row=2, column=0, columnspan=4)

        signButtonFrame = Frame(gridFrame)

        self.bigFirstLetterIntVar = IntVar()
        self.bigFirstLetterCheckbutton = Checkbutton(
            signButtonFrame,
            text = 'Big First Letter',
            variable=self.bigFirstLetterIntVar, command=self.setBigFirstLetter)
        self.bigFirstLetterCheckbutton.pack(
            side = LEFT, expand = 1, fill = X)

        self.allCapsIntVar = IntVar()
        self.allCapsCheckbutton = Checkbutton(
            signButtonFrame,
            text = 'All Caps',
            variable=self.allCapsIntVar, command=self.setAllCaps)
        self.allCapsCheckbutton.pack(side = LEFT, expand = 1, fill = X)

        self.dropShadowIntVar = IntVar()
        self.dropShadowCheckbutton = Checkbutton(
            signButtonFrame,
            text = 'Drop Shadow',
            variable=self.dropShadowIntVar, command=self.setDropShadow)
        self.dropShadowCheckbutton.pack(side = LEFT, expand = 1, fill = X)

        signButtonFrame.grid(row=3, column=0, columnspan=6)

        self.addKernFloater = Floater.Floater(
            gridFrame,
            text='Kern',
            #maxVelocity=1.0,
            command=self.setSignBaselineKern)
        self.addKernFloater.grid(row=4, column=0, rowspan=2, columnspan=3,
                                 sticky = EW)
        self.addWiggleFloater = Floater.Floater(
            gridFrame,
            text='Wiggle',
            #maxVelocity=10.0,
            command=self.setSignBaselineWiggle)
        self.addWiggleFloater.grid(row=6, column=0, rowspan=2, columnspan=3,
                                 sticky = EW)
        self.addStumbleFloater = Floater.Floater(
            gridFrame,
            text='Stumble',
            #maxVelocity=1.0,
            command=self.setSignBaselineStumble)
        self.addStumbleFloater.grid(row=8, column=0, rowspan=2, columnspan=3,
                                 sticky = EW)
        self.addStompFloater = Floater.Floater(
            gridFrame,
            text='Stomp',
            #maxVelocity=1.0,
            command=self.setSignBaselineStomp)
        self.addStompFloater.grid(row=10, column=0, rowspan=2, columnspan=3,
                                 sticky = EW)
        self.addCurveFloater = Floater.Floater(
            gridFrame,
            text='Curve',
            #maxVelocity = 1.0,
            command=self.setSignBaselineCurve)
        self.addCurveFloater.grid(row=12, column=0, rowspan=2, columnspan=3,
                                 sticky = EW)
        self.addXFloater = Floater.Floater(
            gridFrame,
            text='X',
            #maxVelocity=1.0,
            command=self.setDNATargetX)
        self.addXFloater.grid(row=4, column=3, rowspan=2, columnspan=3,
                                 sticky = EW)
        self.addZFloater = Floater.Floater(
            gridFrame,
            text='Z',
            #maxVelocity=1.0,
            command=self.setDNATargetZ)
        self.addZFloater.grid(row=6, column=3, rowspan=2, columnspan=3,
                                 sticky = EW)
        self.addScaleXFloater = Floater.Floater(
            gridFrame,
            text='Scale X',
            #maxVelocity=1.0,
            command=self.setDNATargetScaleX)
        self.addScaleXFloater.grid(row=8, column=3, rowspan=2, columnspan=3,
                                 sticky = EW)
        self.addScaleZFloater = Floater.Floater(
            gridFrame,
            text='Scale Z',
            #maxVelocity=1.0,
            command=self.setDNATargetScaleZ)
        self.addScaleZFloater.grid(row=10, column=3, rowspan=2, columnspan=3,
                                 sticky = EW)
        self.addRollFloater = Floater.Floater(
            gridFrame,
            text='Roll',
            #maxVelocity=10.0,
            command=self.setDNATargetRoll)
        self.addRollFloater.grid(row=12, column=3, rowspan=2, columnspan=3,
                                 sticky = EW)

        gridFrame.pack(fill=BOTH)

        # PROPS
        Label(propsPage, text = 'Props',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        self.addPropsButton = Button(
            propsPage,
            text = 'ADD PROP',
            command = self.addProp)
        self.addPropsButton.pack(fill = X, padx = 20, pady = 10)
        codes = (self.styleManager.getCatalogCodes('prop') +
                 self.styleManager.getCatalogCodes('holiday_prop'))
        codes.sort()
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
            scrolledlist_items = codes
            )
        self.propType = self.styleManager.getCatalogCode('prop', 0)
        self.propSelector.selectitem(
            self.styleManager.getCatalogCode('prop', 0))
        self.propSelector.pack(expand = 1, fill = BOTH)

        # SUIT PATHS
        Label(suitPathPage, text = 'Suit Paths',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)

        spButtons = Frame(suitPathPage)
        self.fPaths = IntVar()
        self.fPaths.set(0)
        self.pathButton = Checkbutton(spButtons,
                                      text = 'Show Paths',
                                      width = 6,
                                      variable = self.fPaths,
                                      command = self.toggleSuitPaths)
        self.pathButton.pack(side = LEFT, expand = 1, fill = X)

        self.zoneColor = IntVar()
        self.zoneColor.set(0)
        self.colorZoneButton1 = Checkbutton(
            spButtons,
            text = 'Color Zones', width = 6,
            variable = self.zoneColor,
            command = self.levelEditor.toggleZoneColors)
        self.colorZoneButton1.pack(side = LEFT, expand = 1, fill = X)
        spButtons.pack(fill = X)

        spButtons = Frame(suitPathPage)
        Label(spButtons, text = 'Highlight:').pack(side = LEFT, fill = X)
        self.highlightConnectedButton = Button(
            spButtons,
            text = 'Forward',
            width = 6,
            command = self.levelEditor.highlightConnected)
        self.highlightConnectedButton.pack(side = LEFT, expand = 1, fill = X)

        self.highlightConnectedButton2 = Button(
            spButtons,
            text = 'Connected',
            width = 6,
            command = lambda s = self: s.levelEditor.highlightConnected(fReversePath = 1))
        self.highlightConnectedButton2.pack(side = LEFT, expand = 1, fill = X)

        self.clearHighlightButton = Button(
            spButtons,
            text = 'Clear',
            width = 6,
            command = self.levelEditor.clearPathHighlights)
        self.clearHighlightButton.pack(side = LEFT, expand = 1, fill = X)
        spButtons.pack(fill = X, pady = 4)

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
        self.suitPointSelector.pack(expand = 1, fill = BOTH)

        # BATTLE CELLS
        Label(battleCellPage, text = 'Battle Cells',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        bcButtons = Frame(battleCellPage)
        self.fCells = IntVar()
        self.fCells.set(0)
        self.cellButton = Checkbutton(bcButtons,
                                      text = 'Show Cells',
                                      width = 6,
                                      variable = self.fCells,
                                      command = self.toggleBattleCells)
        self.cellButton.pack(side = LEFT, expand = 1, fill = X)

        self.colorZoneButton2 = Checkbutton(
            bcButtons,
            text = 'Color Zones', width = 6,
            variable = self.zoneColor,
            command = self.levelEditor.toggleZoneColors)
        self.colorZoneButton2.pack(side = LEFT, expand = 1, fill = X)
        bcButtons.pack(fill = X)

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
        self.battleCellSelector.pack(expand = 1, fill = BOTH)

        # SCENE GRAPH EXPLORER
        Label(sceneGraphPage, text = 'Level Scene Graph',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        self.sceneGraphExplorer = SceneGraphExplorer(
            parent = sceneGraphPage,
            nodePath = self.levelEditor,
            menuItems = ['Add Group', 'Add Vis Group'])
        self.sceneGraphExplorer.pack(expand = 1, fill = BOTH)

        # Compact down notebook
        self.notebook.setnaturalsize()

        self.colorEntry = VectorWidgets.ColorEntry(
            hull, text = 'Select Color',
            command = self.updateSelectedObjColor)
        self.colorEntry.menu.add_command(
            label = 'Save Color', command = self.levelEditor.saveColor)
        self.colorEntry.pack(fill = X)

        buttonFrame = Frame(hull)
        self.fMapVis = IntVar()
        self.fMapVis.set(0)
        self.mapSnapButton = Checkbutton(buttonFrame,
                                      text = 'Map Vis',
                                      width = 6,
                                      variable = self.fMapVis,
                                      command = self.toggleMapVis)
        #self.mapSnapButton.pack(side = LEFT, expand = 1, fill = X)

        self.fXyzSnap = IntVar()
        self.fXyzSnap.set(1)
        self.xyzSnapButton = Checkbutton(buttonFrame,
                                      text = 'XyzSnap',
                                      width = 6,
                                      variable = self.fXyzSnap,
                                      command = self.toggleXyzSnap)
        self.xyzSnapButton.pack(side = LEFT, expand = 1, fill = X)

        self.fHprSnap = IntVar()
        self.fHprSnap.set(1)
        self.hprSnapButton = Checkbutton(buttonFrame,
                                      text = 'HprSnap',
                                      width = 6,
                                      variable = self.fHprSnap,
                                      command = self.toggleHprSnap)
        self.hprSnapButton.pack(side = LEFT, expand = 1, fill = X)

        def toggleWidgetHandles(s = self):
            if s.fPlaneSnap.get():
                base.direct.widget.disableHandles(['x-ring', 'x-disc',
                                              'y-ring', 'y-disc',
                                              'z-post'])
            else:
                base.direct.widget.enableHandles('all')
        self.fPlaneSnap = IntVar()
        self.fPlaneSnap.set(1)
        self.planeSnapButton = Checkbutton(buttonFrame,
                                           text = 'PlaneSnap',
                                           width = 6,
                                           variable = self.fPlaneSnap,
                                           command = toggleWidgetHandles)
        self.planeSnapButton.pack(side = LEFT, expand = 1, fill = X)

        self.fGrid = IntVar()
        self.fGrid.set(0)
        base.direct.gridButton = Checkbutton(buttonFrame,
                                      text = 'Show Grid',
                                      width = 6,
                                      variable = self.fGrid,
                                      command = self.toggleGrid)
        base.direct.gridButton.pack(side = LEFT, expand = 1, fill = X)

        self.fMaya = IntVar()
        self.fMaya.set(1)
        self.mayaButton = Checkbutton(buttonFrame,
                                      text = 'Maya Cam',
                                      width = 6,
                                      variable = self.fMaya,
                                      command = self.toggleMaya)
        self.mayaButton.pack(side = LEFT, expand = 1, fill = X)
        
        #Make maya mode on by default
        self.toggleMaya()

        buttonFrame.pack(fill = X)

        buttonFrame4 = Frame(hull)
        self.driveMode = IntVar()
        self.driveMode.set(1)
        self.directModeButton = Radiobutton(
            buttonFrame4,
            text = 'DIRECT Fly',
            value = 1,
            variable = self.driveMode,
            command = self.levelEditor.useDirectFly)
        self.directModeButton.pack(side = LEFT, fill = X, expand = 1)
        self.driveModeButton = Radiobutton(
            buttonFrame4,
            text = 'Drive',
            value = 0,
            variable = self.driveMode,
            command = self.levelEditor.useDriveMode)
        self.driveModeButton.pack(side = LEFT, fill = X, expand = 1)

        self.fColl = IntVar()
        self.fColl.set(1)
        base.direct.collButton = Checkbutton(
            buttonFrame4,
            text = 'Collide',
            variable = self.fColl,
            command = self.levelEditor.toggleCollisions)
        base.direct.collButton.pack(side = LEFT, expand = 1, fill = X)

        self.fVis = IntVar()
        self.fVis.set(1)
        base.direct.visButton = Checkbutton(
            buttonFrame4,
            text = 'Visibility',
            variable = self.fVis,
            command = self.levelEditor.toggleVisibility)
        base.direct.visButton.pack(side = LEFT, expand = 1, fill = X)

        self.fVisZones = IntVar()
        self.fVisZones.set(visualizeZones)
        base.direct.visZonesButton = Checkbutton(
            buttonFrame4,
            text = 'Show Zones',
            variable = self.fVisZones)
        base.direct.visZonesButton.pack(side = LEFT, expand = 1, fill = X)

        buttonFrame4.pack(fill = X, padx = 5)

        buttonFrame = Frame(hull)
        self.fLabel = IntVar()
        self.fLabel.set(0)
        self.labelButton = Checkbutton(buttonFrame,
                                       text = 'Show Zone Labels',
                                       width = 6,
                                       variable = self.fLabel,
                                       command = self.toggleZoneLabels)
        self.labelButton.pack(side = LEFT, expand = 1, fill = X)

        self.selectButton = Button(buttonFrame,
                                   text = 'Place Selected',
                                   width = 6,
                                   command = lambda: last.place()
                                   )
        self.selectButton.pack(side = LEFT, expand = 1, fill = X)
        buttonFrame.pack(fill = X)

        # Make sure input variables processed
        self.initialiseoptions(LevelEditorPanel)

    def updateInfo(self, page):
        if page == 'Signs':
            self.updateSignPage()

    # [gjeon] to toggle maya cam mode
    def toggleMaya(self):
        base.direct.cameraControl.lockRoll = self.fMaya.get()
        direct.cameraControl.useMayaCamControls = self.fMaya.get()

    def toggleGrid(self):
        if self.fGrid.get():
            base.direct.grid.enable()
        else:
            base.direct.grid.disable()

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

    def toggleZoneLabels(self):
        if self.fLabel.get():
            self.levelEditor.labelZones()
        else:
            self.levelEditor.clearZoneLabels()

    def toggleXyzSnap(self):
        base.direct.grid.setXyzSnap(self.fXyzSnap.get())

    def toggleHprSnap(self):
        base.direct.grid.setHprSnap(self.fXyzSnap.get())

    def toggleMapVis(self):
        self.levelEditor.toggleMapVis(self.fMapVis.get())

    def setStreetModuleType(self, name):
        self.streetModuleType = 'street_' + name
        self.levelEditor.setCurrent('street_texture', self.streetModuleType)

    def addStreet(self):
        self.levelEditor.addStreet(self.streetModuleType)

    def setFlatBuildingType(self, name):
        self.toonBuildingType = name
        self.levelEditor.setCurrent('building_type', self.toonBuildingType)

    def setFlatBuildingHeight(self):
        height = self.heightMode.get()
        self.levelEditor.setCurrent('building_height', height)
        self.updateHeightList(height)

    def updateHeightList(self, height):
        # Update combo box
        heightList = self.levelEditor.getList(`height` + '_ft_wall_heights')
        self.toonBuildingSelector.setlist(
            ['random'] + map(createHeightCode, heightList))
        self.toonBuildingSelector.selectitem(0)
        self.toonBuildingSelector.invoke()

    def addFlatBuilding(self):
        self.levelEditor.addFlatBuilding(self.toonBuildingType)

    def setLandmarkType(self, name):
        self.landmarkType = 'toon_landmark_' + name
        self.levelEditor.setCurrent('toon_landmark_texture', self.landmarkType)

    def signPanelSync(self):
        self.baselineMenu.delete(1, END)
        sign=self.findSignFromDNARoot()
        if not sign:
            return
        baselineList = DNAGetChildren(sign, DNA_SIGN_BASELINE)
        for baseline in baselineList:
            s=DNAGetBaselineString(baseline)
            self.baselineMenu.insert(END, s)
        self.baselineMenu.selectitem(0)
        self.selectSignBaseline(0)

    def updateSignPage(self):
        if (self.notebook.getcurselection() == 'Signs'):
            sign=self.findSignFromDNARoot()
            # Only update if it's not the current sign:
            if (self.currentSignDNA != sign):
                self.currentSignDNA = sign
                self.signPanelSync()

    def findSignFromDNARoot(self):
        dnaRoot=self.levelEditor.selectedDNARoot
        if not dnaRoot:
            return
        objClass=DNAGetClassType(dnaRoot)
        if (objClass.eq(DNA_LANDMARK_BUILDING)
                or objClass.eq(DNA_PROP)):
            target=DNAGetChildRecursive(dnaRoot, DNA_SIGN)
            return target

    def selectSignBaseline(self, val):
        if not self.currentSignDNA:
            return
        # Temporarily undefine DNATarget (this will speed
        # up setting the values, because the callbacks won't
        # call self.levelEditor.replaceSelected() with each
        # setting):
        self.levelEditor.DNATarget=None
        self.currentBaselineDNA=None
        target=None
        index=self.currentBaselineIndex=int((self.baselineMenu.curselection())[0])
        if (index==0):
            target=self.currentSignDNA
            # Unset some ui elements:
            self.baselineString.set('')
            self.fontMenu.selectitem(0)
            self.addCurveFloater.set(0)
            self.addKernFloater.set(0)
            self.addWiggleFloater.set(0)
            self.addStumbleFloater.set(0)
            self.addStompFloater.set(0)
            self.bigFirstLetterIntVar.set(0)
            self.allCapsIntVar.set(0)
            self.dropShadowIntVar.set(0)
        else:
            target=DNAGetChild(self.currentSignDNA, DNA_SIGN_BASELINE, index-1)
            if target:
                # Update panel info:
                s = DNAGetBaselineString(target)
                self.baselineString.set(s)
                self.fontMenu.selectitem(target.getCode())
                try:
                    val = 1.0/target.getWidth()
                except ZeroDivisionError:
                    val = 0.0
                self.addCurveFloater.set(val)
                self.addKernFloater.set(target.getKern())
                self.addWiggleFloater.set(target.getWiggle())
                self.addStumbleFloater.set(target.getStumble())
                self.addStompFloater.set(target.getStomp())
                flags=target.getFlags()
                self.bigFirstLetterIntVar.set('b' in flags)
                self.allCapsIntVar.set('c' in flags)
                self.dropShadowIntVar.set('d' in flags)

                self.currentBaselineDNA=target
        if target:
            pos=target.getPos()
            self.addXFloater.set(pos[0])
            self.addZFloater.set(pos[2])
            scale=target.getScale()
            self.addScaleXFloater.set(scale[0])
            self.addScaleZFloater.set(scale[2])
            hpr=target.getHpr()
            self.addRollFloater.set(hpr[2])

            self.levelEditor.DNATarget=target

    def deleteSignItem(self):
        """Delete the selected sign or sign baseline"""
        if (self.currentBaselineDNA):
            # Remove the baseline:
            assert int((self.baselineMenu.curselection())[0]) == self.currentBaselineIndex
            DNARemoveChildOfClass(self.currentSignDNA, DNA_SIGN_BASELINE,
                self.currentBaselineIndex-1)
            self.baselineMenu.delete(self.currentBaselineIndex)
            self.baselineMenu.selectitem(0)
            self.currentBaselineIndex=0
            self.currentBaselineDNA=None
            self.selectSignBaseline(0)
            self.levelEditor.replaceSelected()
        elif (self.currentSignDNA):
            # Remove the sign:
            assert int((self.baselineMenu.curselection())[0]) == 0
            le = self.levelEditor
            le.removeSign(le.DNATarget, le.DNATargetParent)
            self.currentBaselineDNA=None
            self.currentSignDNA=None
            self.levelEditor.replaceSelected()

    def signBaselineTrace(self, a, b, mode):
        #print self, a, b, mode, self.baselineString.get()
        baseline=self.currentBaselineDNA
        if baseline:
            s = self.baselineString.get()
            self.setBaselineString(s)

    def addSignGraphic(self, code):
        """
        Create a new baseline with a graphic and
        add it to the current sign
        """
        sign=self.findSignFromDNARoot()
        if sign:
            graphic=DNASignGraphic()
            graphic.setCode(code)
            baseline=DNASignBaseline()
            baseline.add(graphic)
            sign.add(baseline)
            # Show the UI to the new baseline:
            self.levelEditor.DNATarget=baseline
            self.baselineMenu.insert(END, '['+code+']')
            current=self.baselineMenu.size()-1
            self.baselineMenu.selectitem(current)
            self.selectSignBaseline(current)
            self.levelEditor.replaceSelected()

    def addBaseline(self):
        sign=self.findSignFromDNARoot()
        if sign:
            baseline=DNASignBaseline()
            text="Zoo"
            DNASetBaselineString(baseline, text)
            sign.add(baseline)
            # Show the UI to the new baseline:
            self.levelEditor.DNATarget=baseline
            self.baselineMenu.insert(END, text)
            current=self.baselineMenu.size()-1
            self.baselineMenu.selectitem(current)
            self.selectSignBaseline(current)
            self.levelEditor.replaceSelected()

    def addBaselineItem(self):
        pass

    def selectSignBaselineItem(self, val):
        baseline=self.currentBaselineDNA
        if baseline:
            baseline.setCode(val)
            self.levelEditor.replaceSelected()

    def setSignBaselineStyle(self, val):
        baseline=self.currentBaselineDNA
        if baseline == None:
            print "\n\nbaseline == None"
            return #skyler: This isn't working yet.
            #               As a workaround, select the baseline from the tk panel.
            # Try to find the first baseline in the sign:
            sign=self.findSignFromDNARoot()
            if sign:
                self.currentSignDNA = sign
                baseline=DNAGetChild(sign, DNA_SIGN_BASELINE)
        if baseline and val:
            self.levelEditor.DNATarget=baseline
            self.currentBaselineDNA=baseline
            settings=val
            self.levelEditor.replaceSelectedEnabled=0

            # Don't set string: self.baselineString.set('')
            if settings['curve'] != None:
                self.addCurveFloater.set(settings['curve'])
            if settings['kern'] != None:
                self.addKernFloater.set(settings['kern'])
            if settings['wiggle'] != None:
                self.addWiggleFloater.set(settings['wiggle'])
            if settings['stumble'] != None:
                self.addStumbleFloater.set(settings['stumble'])
            if settings['stomp'] != None:
                self.addStompFloater.set(settings['stomp'])

            flags=settings['flags']
            if flags != None:
                self.bigFirstLetterIntVar.set('b' in flags)
                self.setBigFirstLetter()

                self.allCapsIntVar.set('c' in flags)
                self.setAllCaps()

                self.dropShadowIntVar.set('d' in flags)
                self.setDropShadow()

            code = settings['code']
            if code != None:
                self.fontMenu.selectitem(code)
                self.setSignBaslineFont(code)

            if settings['x'] != None:
                self.addXFloater.set(settings['x'])
            if settings['z'] != None:
                self.addZFloater.set(settings['z'])
            if settings['scaleX'] != None:
                self.addScaleXFloater.set(settings['scaleX'])
            if settings['scaleZ'] != None:
                self.addScaleZFloater.set(settings['scaleZ'])
            if settings['roll'] != None:
                self.addRollFloater.set(settings['roll'])

            color = settings['color']
            if color != None:
                #self.updateSelectedObjColor(settings['color'])
                self.setCurrentColor(color)
                self.setResetColor(color)
                baseline.setColor(color)

            self.levelEditor.replaceSelectedEnabled=1
            self.levelEditor.replaceSelected()


    def setBaselineString(self, val):
        baseline=self.currentBaselineDNA
        if baseline:
            DNASetBaselineString(baseline, val)
            self.baselineMenu.delete(self.currentBaselineIndex)
            self.baselineMenu.insert(self.currentBaselineIndex, val)
            self.baselineMenu.selectitem(self.currentBaselineIndex)
            self.levelEditor.replaceSelected()

    def adjustBaselineFlag(self, newValue, flagChar):
        baseline=self.currentBaselineDNA
        if baseline:
            flags=baseline.getFlags()
            if newValue:
                 if not flagChar in flags:
                     # Add the flag:
                     baseline.setFlags(flags+flagChar)
            elif flagChar in flags:
                 # Remove the flag:
                 flags=string.join(flags.split(flagChar), '')
                 baseline.setFlags(flags)
            self.levelEditor.replaceSelected()

    def setLandmarkSpecialType(self, type):
        self.landmarkSpecialType = type
        if self.levelEditor.lastLandmarkBuildingDNA:
            self.levelEditor.lastLandmarkBuildingDNA.setBuildingType(self.landmarkSpecialType)

    def setBigFirstLetter(self):
        self.adjustBaselineFlag(self.bigFirstLetterIntVar.get(), 'b')

    def setAllCaps(self):
        self.adjustBaselineFlag(self.allCapsIntVar.get(), 'c')

    def setDropShadow(self):
        self.adjustBaselineFlag(self.dropShadowIntVar.get(), 'd')

    def setSignBaslineFont(self, val):
        target=self.levelEditor.DNATarget
        if target and (DNAGetClassType(target).eq(DNA_SIGN_BASELINE)
                or DNAGetClassType(target).eq(DNA_SIGN_TEXT)):
            target.setCode(val)
            self.levelEditor.replaceSelected()

    def setSignBaselineCurve(self, val):
        baseline=self.currentBaselineDNA
        if baseline:
            try:
                val=1.0/val
            except ZeroDivisionError:
                val=0.0
            baseline.setWidth(val)
            baseline.setHeight(val)
            self.levelEditor.replaceSelected()

    def setSignBaselineKern(self, val):
        baseline=self.currentBaselineDNA
        if baseline:
            baseline.setKern(val)
            self.levelEditor.replaceSelected()

    def setSignBaselineWiggle(self, val):
        baseline=self.currentBaselineDNA
        if baseline:
            baseline.setWiggle(val)
            self.levelEditor.replaceSelected()

    def setSignBaselineStumble(self, val):
        baseline=self.currentBaselineDNA
        if baseline:
            baseline.setStumble(val)
            self.levelEditor.replaceSelected()

    def setSignBaselineStomp(self, val):
        baseline=self.currentBaselineDNA
        if baseline:
            baseline.setStomp(val)
            self.levelEditor.replaceSelected()

    def setDNATargetX(self, val):
        target=self.levelEditor.DNATarget
        if target:
            pos=target.getPos()
            pos=VBase3(val, pos[1], pos[2])
            target.setPos(pos)
            self.levelEditor.replaceSelected()

    def setDNATargetZ(self, val):
        target=self.levelEditor.DNATarget
        if target:
            pos=target.getPos()
            pos=VBase3(pos[0], pos[1], val)
            target.setPos(pos)
            self.levelEditor.replaceSelected()

    def setDNATargetScaleX(self, val):
        target=self.levelEditor.DNATarget
        if target:
            scale=target.getScale()
            scale=VBase3(val, scale[1], scale[2])
            target.setScale(scale)
            self.levelEditor.replaceSelected()

    def setDNATargetScaleZ(self, val):
        target=self.levelEditor.DNATarget
        if target:
            scale=target.getScale()
            scale=VBase3(scale[0], scale[1], val)
            target.setScale(scale)
            self.levelEditor.replaceSelected()

    def setDNATargetRoll(self, val):
        target=self.levelEditor.DNATarget
        if target:
            hpr=target.getHpr()
            hpr=VBase3(hpr[0], hpr[1], val)
            target.setHpr(hpr)
            self.levelEditor.replaceSelected()

    def addLandmark(self):
        self.levelEditor.addLandmark(self.landmarkType, self.landmarkSpecialType)

    def setPropType(self, name):
        self.propType = name
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

    def setSuitPointType(self, name):
        if (name == "street"):
            self.levelEditor.currentSuitPointType = DNASuitPoint.STREETPOINT
        elif (name == "front door"):
            self.levelEditor.currentSuitPointType = DNASuitPoint.FRONTDOORPOINT
        elif (name == "side door"):
            self.levelEditor.currentSuitPointType = DNASuitPoint.SIDEDOORPOINT
        print self.levelEditor.currentSuitPointType

    def setBattleCellType(self, name):
        self.levelEditor.currentBattleCellType = name

    def updateSelectedObjColor(self, color):
        try:
            obj = self.levelEditor.DNATarget
            if self.fUpdateSelected and (obj != None):
                objClass = DNAGetClassType(obj)
                if ((objClass.eq(DNA_WALL)) or
                    (objClass.eq(DNA_WINDOWS)) or
                    (objClass.eq(DNA_DOOR)) or
                    (objClass.eq(DNA_FLAT_DOOR)) or
                    (objClass.eq(DNA_CORNICE)) or
                    (objClass.eq(DNA_PROP)) or
                    (objClass.eq(DNA_SIGN)) or
                    (objClass.eq(DNA_SIGN_BASELINE)) or
                    (objClass.eq(DNA_SIGN_TEXT)) or
                    (objClass.eq(DNA_SIGN_GRAPHIC))
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
        sf.pack(padx=5, pady=3, fill = BOTH, expand = 1)

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
        buttonFrame.pack(fill=X, expand = 1)

        self.showMode = IntVar()
        self.showMode.set(0)
        self.showAllButton = Radiobutton(buttonFrame, text = 'Show All',
                                         value = 0, indicatoron = 1,
                                         variable = self.showMode,
                                         command = self.refreshVisibility)
        self.showAllButton.pack(side = LEFT, fill = X, expand = 1)
        self.showActiveButton = Radiobutton(buttonFrame, text = 'Show Target',
                                            value = 1, indicatoron = 1,
                                            variable = self.showMode,
                                            command = self.refreshVisibility)
        self.showActiveButton.pack(side = LEFT, fill = X, expand = 1)

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
                    groupNP.setColor(1, 0, 0, 1)
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
                    groupNP.setColor(0, 1, 0, 1)
                else:
                    groupNP.setColor(1, 0, 0, 1)
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

# [gjeon] for LevelEditor specific Avatar
class LEAvatar(LocalAvatar.LocalAvatar):
    def __init__(self, cr, chatMgr, chatAssistant, passMessagesThrough = False):
        LocalAvatar.LocalAvatar.__init__(self,  cr, chatMgr, chatAssistant, passMessagesThrough)

    def getAutoRun(self):
        return 0

l = LevelEditor()
run()

