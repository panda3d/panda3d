import sys, string, math, types
from pandac.PandaModules import *
import direct.gui.DirectGuiGlobals as DGG
from direct.gui.DirectGui import *

from PieMenu import *
from ScrollMenu import *

dnaDirectory = Filename.expandFrom(base.config.GetString("dna-directory", "$TTMODELS/src/dna"))


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


# Precompute class types for type comparisons
DNA_CORNICE = DNACornice.getClassType()
DNA_DOOR = DNADoor.getClassType()
DNA_FLAT_DOOR = DNAFlatDoor.getClassType()
DNA_FLAT_BUILDING = DNAFlatBuilding.getClassType()
DNA_NODE = DNANode.getClassType()
DNA_GROUP = DNAGroup.getClassType()
DNA_VIS_GROUP = DNAVisGroup.getClassType()
DNA_LANDMARK_BUILDING = DNALandmarkBuilding.getClassType()
DNA_NODE = DNANode.getClassType()
DNA_PROP = DNAProp.getClassType()
DNA_SIGN = DNASign.getClassType()
DNA_SIGN_BASELINE = DNASignBaseline.getClassType()
DNA_SIGN_TEXT = DNASignText.getClassType()
DNA_SIGN_GRAPHIC = DNASignGraphic.getClassType()
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

def DNARemoveChildOfClass(dnaNode, classType, childNum = 0):
    """ Remove the nth object of that type you come across """
    childCount = 0
    for i in range(dnaNode.getNumChildren()):
        child = dnaNode.at(i)
        if DNAClassEqual(child, classType):
            if childCount == childNum:
                dnaNode.remove(child)
                DNASTORE.removeDNAGroup(child)
                return 1
            childCount = childCount + 1
    # None found
    return 0

def DNARemoveAllChildrenOfClass(dnaNode, classType):
    """ Remove the objects of that type """
    children = []
    for i in range(dnaNode.getNumChildren()):
        child=dnaNode.at(i)
        if DNAClassEqual(child, classType):
            children.append(child)
    for child in children:
        dnaNode.remove(child)
        DNASTORE.removeDNAGroup(child)

def DNAGetChildren(dnaNode, classType=None):
    """ Return the objects of that type """
    children = []
    for i in range(dnaNode.getNumChildren()):
        child=dnaNode.at(i)
        if ((not classType)
            or DNAClassEqual(child, classType)):
            children.append(child)
    return children

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

def DNAGetChildRecursive(dnaObject, type = DNA_NODE, childNum = 0):
    childCount = 0
    for i in range(dnaObject.getNumChildren()):
        child = dnaObject.at(i)
        if DNAClassEqual(child, type):
            if childCount == childNum:
                return child
            childCount = childCount + 1
        else:
            child = DNAGetChildRecursive(child, type, childNum-childCount)
            if child:
                return child

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

def DNAGetBaselineString(baseline):
    s=""
    for i in range(baseline.getNumChildren()):
        child = baseline.at(i)
        if DNAClassEqual(child, DNA_SIGN_TEXT):
            s=s+child.getLetters()
        elif DNAClassEqual(child, DNA_SIGN_GRAPHIC):
            s=s+'['+child.getCode()+']'
    return s

def DNASetBaselineString(baseline, text):
    # TODO: Instead of removing all the text and replacing it,
    # replace each text item and then add or remove at the end.
    # This should allow inlined graphics to stay in place.
    # end of todo.
    DNARemoveAllChildrenOfClass(baseline, DNA_SIGN_TEXT)

    # We can't just blindly iterate through the text, because it might
    # be utf-8 encoded, meaning some characters are represented using
    # multi-byte sequences.  Instead, create a TextNode and use it to
    # iterate through the characters of the text.
    t = TextNode('')
    t.setText(text)
    for i in range(t.getNumChars()):
        ch = t.getEncodedChar(i)
        text=DNASignText("text")
        text.setLetters(ch)
        baseline.add(text)

class LevelStyleManager:
    """Class which reads in style files and manages class variables"""
    def __init__(self, NEIGHBORHOODS = [], NEIGHBORHOOD_CODES = {} ):
        self.NEIGHBORHOODS = NEIGHBORHOODS
        self.NEIGHBORHOOD_CODES = NEIGHBORHOOD_CODES
        # The main dictionary holding all attribute objects
        self.attributeDictionary = {}
        # Create the style samples
        self.createBaselineStyleAttributes()
        self.createWallStyleAttributes()
        self.createBuildingStyleAttributes()
        self.createColorAttributes()
        self.createDNAAttributes()
        self.createMiscAttributes()

    # BASELINE STYLE FUNCTIONS
    def createBaselineStyleAttributes(self):
        """
        Create a baselineStyle entry in the attribute dictionary
        This will be a dictionary of style attributes, one per neighborhood
        """
        # First create an empty dictionary
        dict = self.attributeDictionary['baseline_style'] = {}
        # Create a attribute object for each neighborhood
        for neighborhood in self.NEIGHBORHOODS:
            attribute = LevelAttribute('baseline_style')
            attribute.setDict(
                # Create a baseline style dictionary for each neighborhood
                self.createBaselineStyleDictionary(neighborhood))
            # Using this dictionary, create style pie menus
            attribute.setMenu(
                self.createBaselineStyleMenu(neighborhood, attribute.getDict()))
            dict[neighborhood] = attribute

    def createBaselineStyleDictionary(self, neighborhood):
        """
        Create a dictionary of baseline styles for a neighborhood
        """
        filename = neighborhood + '_baseline_styles.txt'
        print 'Loading baseline styles from: ' + filename
        styleData = self.getStyleFileData(filename)
        return self.initializeBaselineStyleDictionary(styleData, neighborhood)

    def initializeBaselineStyleDictionary(self, styleData, neighborhood):
        """
        Fill in the baseline style dictionary with data from the style file
        """
        styleDictionary = {}
        styleCount = 0
        code = self.NEIGHBORHOOD_CODES[neighborhood]
        while styleData:
            l = styleData[0]
            if l == 'baselineStyle':
                # Start of new style, strip off first line then extract style
                style, styleData = self.extractBaselineStyle(styleData)
                style.name = code + '_baseline_style_' + `styleCount`
                # Store style in dictionary
                styleDictionary[style.name] = style
                styleCount = styleCount + 1
            # Move to next line
            styleData = styleData[1:]
        return styleDictionary

    def extractBaselineStyle(self, styleData):
        """
        Pull out one style from a list of style data.  Will keep
        processing data until endBaselineStyle of end of data is reached.
        Returns a baseline style and remaining styleData.
        """
        # Create default style
        style = DNABaselineStyle()
        # Strip off first line
        styleData = styleData[1:]
        while styleData:
            l = styleData[0]
            if l == 'endBaselineStyle':
                # End of style found, break out of while loop
                # Note, endBaselineStyle line is *not* stripped off
                return style, styleData
            else:
                pair = map(string.strip, l.split(':'))
                if style.__dict__.has_key(pair[0]):
                    pair_0 = pair[0]
                    # Convert some numerical values
                    if (pair_0 == 'color'
                        or pair_0 == 'kern'
                        or pair_0 == 'wiggle'
                        or pair_0 == 'stumble'
                        or pair_0 == 'stomp'
                        or pair_0 == 'curve'
                        or pair_0 == 'x'
                        or pair_0 == 'z'
                        or pair_0 == 'scaleX'
                        or pair_0 == 'scaleZ'
                        or pair_0 == 'roll'):
                        style[pair_0] = eval(pair[1])
                    else:
                        style[pair_0] = pair[1]
                else:
                    print 'extractBaselineStyle: Invalid Key'
                    print pair[0]
            styleData = styleData[1:]
        # No end of style found, return style data as is
        return style, None

    def createBaselineStyleMenu(self, neighborhood, dictionary):
        """
        Create a baseline style pie menu
        """
        numItems = len(dictionary)
        newStyleMenu = hidden.attachNewNode(neighborhood + '_style_menu')
        radius = 0.7
        angle = deg2Rad(360.0/numItems)
        keys = dictionary.keys()
        keys.sort()
        styles = map(lambda x, d = dictionary: d[x], keys)
        sf = 0.1
        aspectRatio = (base.direct.dr.getWidth()/float(base.direct.dr.getHeight()))
        for i in range(numItems):
            # Get the node
            node = self.createBaselineStyleSample(styles[i])
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

    def createBaselineStyleSample(self, baselineStyle):
        """
        Create a style sample using the DNA info in the style
        """
        baseline = DNASignBaseline()
        # Set some example text:
        DNASetBaselineString(baseline, "Example Text")
        baselineStyle.copyTo(baseline)
        return baseline.traverse(hidden, DNASTORE, 1)

    # WALL STYLE FUNCTIONS
    def createWallStyleAttributes(self):
        """
        Create a wallStyle entry in the attribute dictionary
        This will be a dictionary of style attributes, one per neighborhood
        """
        # First create an empty dictionary
        dict = self.attributeDictionary['wall_style'] = {}
        # Create a attribute object for each neighborhood
        for neighborhood in self.NEIGHBORHOODS:
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
        code = self.NEIGHBORHOOD_CODES[neighborhood]
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
        aspectRatio = (base.direct.dr.getWidth()/float(base.direct.dr.getHeight()))
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
        This information is sorted and stored by num walls, building height,
        and building type (e.g. 10_20, or 10_10_10).
        """
        # First create an attribute which holds one dictionary per
        # neighborhood which stores all of the styles for each neighborhood.
        styleDict = self.attributeDictionary['building_style_all'] = {}
        # Keep track of all building types
        typeDict = {}
        masterTypeList = []
        for neighborhood in self.NEIGHBORHOODS:
            attribute = LevelAttribute('building_style_all')
            # Create a building style dictionary for each neighborhood
            attribute.setDict(
                self.createBuildingStyleDictionary(neighborhood))
            # Using this dictionary, create building style pie menus
            attribute.setMenu(
                self.createBuildingStyleMenu(neighborhood,
                                             attribute.getDict()))
            # Store attribute in dictionary keyed by neighborhood
            styleDict[neighborhood] = attribute
            # Generate a list of building types for this neighborhood
            # and a list of building types for all neighborhoods,
            # to be used in creating attribute dicts below
            typeList = typeDict[neighborhood] = []
            for style in attribute.getList():
                heightType = string.strip(string.split(style.name, ':')[1])
                if heightType not in typeList:
                    typeList.append(heightType)
                if heightType not in masterTypeList:
                    masterTypeList.append(heightType)

        # Now sort styles according to the building type and number of walls
        # in the building style.  Each of these attributes is also sorted by
        # neighborhood
        for i in masterTypeList:
            typeKey = i + '_styles'
            self.attributeDictionary[typeKey] = {}
        for i in NUM_WALLS:
            numWallKey = `i` + '_wall_styles'
            self.attributeDictionary[numWallKey] = {}
        # Also sort height lists according to total height of the building
        for i in BUILDING_HEIGHTS:
            heightKey = `i` + '_ft_wall_heights'
            self.attributeDictionary[heightKey] = {}
        # Now distribute data for each neighborhood
        for neighborhood in self.NEIGHBORHOODS:
            # Create temp dicts to accumulate styles and heights for that
            # neighborhood
            typeAttributes = {}
            numWallsAttributes = {}
            heightAttributes = {}
            # Store atributes for the different categories
            # Building type
            for i in typeDict[neighborhood]:
                typeAttrName = neighborhood + '_' + i + '_styles'
                typeAttributes[i] = LevelAttribute(typeAttrName)
            # Number of walls
            for i in NUM_WALLS:
                styleAttrName = neighborhood + '_' + `i` + '_wall_styles'
                numWallsAttributes[i] = LevelAttribute(styleAttrName)
            # Building height
            for i in BUILDING_HEIGHTS:
                heightAttrName = neighborhood + '_' + `i` + '_ft_wall_heights'
                heightAttributes[i] = LevelAttribute(heightAttrName)
            # Sort through the styles and store in separate lists
            for style in styleDict[neighborhood].getList():
                # Put in code for number of walls into building styles
                heightType = string.strip(string.split(style.name, ':')[1])
                heightList = map(string.atof, string.split(heightType, '_'))
                numWalls = len(heightList)
                # This one stores styles sorted by type
                typeAttributes[heightType].add(style)
                # This one stores styles sorted by number of walls
                numWallsAttributes[numWalls].add(style)
                # A record of all the unique height lists
                height = calcHeight(heightList)
                if heightList not in heightAttributes[height].getList():
                    heightAttributes[height].add(heightList)
            # Now store these sorted style and height attributes
            # in the appropriate top-level attribute dictionary
            for i in typeDict[neighborhood]:
                # Styles
                typeKey = i + '_styles'
                self.attributeDictionary[typeKey][neighborhood] = (
                    typeAttributes[i])
            for i in NUM_WALLS:
                # Styles
                numWallKey = `i` + '_wall_styles'
                self.attributeDictionary[numWallKey][neighborhood] = (
                    numWallsAttributes[i])
            for i in BUILDING_HEIGHTS:
                # Heights
                heightKey = `i` + '_ft_wall_heights'
                self.attributeDictionary[heightKey][neighborhood] = (
                    heightAttributes[i])

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
        code = self.NEIGHBORHOOD_CODES[neighborhood]
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
        aspectRatio = (base.direct.dr.getWidth()/float(base.direct.dr.getHeight()))
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
                                heightList = None, name = 'building'):
        """ Set DNAFlatBuilding style. """
        # Remove flat building's children
        DNARemoveChildren(fb)
        # Update the name
        fb.setName(name)
        # Create the walls
        styleList = bldgStyle.styleList
        # Height list not specified, use styles default heights
        if not heightList:
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
            self.setDNAWallStyle(wall, wallStyle, wallHeight,
                                 width = width)
            # Add it to building DNA
            fb.add(wall)
        # Set the buildings width
        fb.setWidth(width)

    def setDNAWallStyle(self, wall, style, height = 10.0, width = None):
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
            windowCount = style['window_count']
            if width:
                if (width < 15.0):
                    windowCount = min(1, windowCount)
            windows.setWindowCount(windowCount)
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
            door = DNAFlatDoor()
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
            elif DNAClassEqual(child, DNA_FLAT_DOOR):
                print 'door_texture: ' + child.getCode()
                color = child.getColor()
                print ('door_color: Vec4(%.3f, %.3f, %.3f, 1.0)' %
                       (color[0], color[1], color[2]))
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
        for neighborhood in self.NEIGHBORHOODS:
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
            for neighborhood in self.NEIGHBORHOODS:
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
        aspectRatio = (base.direct.dr.getWidth() / float(base.direct.dr.getHeight()))
        # Attach the color chips to the new menu and adjust sizes
        for i in range (numItems):
            # Create a text node--just a card, really--of the right color.
            tn = TextNode('colorChip')
            tn.setFont(DGG.getDefaultFont())
            tn.setTransform(Mat4.scaleMat(0.07, 0.07, 0.07 * aspectRatio))
            tn.setCardColor(colorList[i])
            tn.setCardActual(0, 1.1111, 0, 0.8333)
            tn.setText(' ')

            # Reposition it
            card = tn.getCardTransformed()
            center = (card[1] - card[0], card[3] - card[2])

            node = newColorMenu.attachNewNode(tn)
            node.setScale(sf)
            node.setPos(radius * math.cos(i * angle) - center[0], 0.0,
                        ((radius * aspectRatio * math.sin(i * angle)) -
                         center[1]))
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
                if dnaType == 'prop':
                    dnaList = dnaList + self.getCatalogCodes('holiday_prop')
            elif (dnaType == 'sign'):
                dnaList = [''] + self.getCatalogCodes(dnaType)
            elif (dnaType == 'wall'):
                # Add in suit walls here for now
                dnaList = ([None] +
                           self.getCatalogCodesSuffix(dnaType, '_ur') +
                           self.getCatalogCodesSuffix('suit_wall', '_ur'))
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
                # attribute.setMenu(self.createTextPieMenu(dnaType, dnaList))
                attribute.setMenu(self.createScrollMenu(dnaType, dnaList))
                attribute.getMenu().createScrolledList()
            elif (dnaType == 'wall'):
                attribute.setMenu(self.createDNAPieMenu(dnaType, dnaList,
                                                         sf = 0.25))
            elif (dnaType == 'sign'):
                attribute.setMenu(self.createDNAPieMenu(dnaType, dnaList,
                                                         sf = 0.05))
            elif (dnaType == 'door_double'):
                attribute.setMenu(self.createDNAPieMenu(dnaType, dnaList,
                                                         sf = 0.035))
            elif (dnaType == 'door_single'):
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
        aspectRatio = base.direct.dr.getWidth() /float(base.direct.dr.getHeight())
        # Add items
        for i in range(0, numItems):
            if dnaList[i]:
                # Get the node
                node = DNASTORE.findNode(dnaList[i])
            else:
                node = None
            if node:
                # Add it to the window menu
                # This instance was causing problems for dna nodes that were
                # loaded as singletons, so I changed it to copyTo.
                # path = node.instanceTo(newMenu)
                path = node.copyTo(newMenu)
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
        if numItems == 0:
            angle = 0.0
        else:
            angle = deg2Rad(360.0/numItems)
        aspectRatio = base.direct.dr.getWidth()/float(base.direct.dr.getHeight())
        # Add items
        for i in range (numItems):
            # Create text node for each item
            if (textList[i] != None):
                tn = TextNode('TextItem')
                tn.setFont(DGG.getDefaultFont())
                tn.setTransform(Mat4.scaleMat(0.07, 0.07, 0.07 * aspectRatio))
                tn.setTextColor(0, 0, 0, 1)
                tn.setCardColor(1, 1, 1, 1)
                tn.setCardAsMargin(0.1, 0.1, 0.1, 0.1)
                tn.setText(str(textList[i]))

                # Reposition it
                card = tn.getCardTransformed()
                center = (card[1] - card[0], card[3] - card[2])

                node = newMenu.attachNewNode(tn)
                node.setScale(sf)
                node.setPos(radius * math.cos(i * angle) - center[0], 0.0,
                            ((radius * aspectRatio * math.sin(i * angle)) -
                            center[1]))

        # Scale the whole shebang down by 0.5
        newMenu.setScale(0.5)
        # Create and return a pie menu
        return PieMenu(newMenu, textList)
    
    def createScrollMenu(self, dnaType, textList):
        newMenu = hidden.attachNewNode(dnaType+'ScrollMenu')
        
        return ScrollMenu(newMenu, textList)

    # MISCELLANEOUS MENUS
    def createMiscAttributes(self):
        # Num windows menu
        self.createMiscAttribute('window_count', [0, 1, 2, 3, 4])
        # Building width menu
        self.createMiscAttribute('building_width', [5, 10, 15, 15.6, 20, 20.7, 25])
        # Building types
        self.createMiscAttribute('building_type', BUILDING_TYPES)
        # Building heights
        self.createMiscAttribute('building_height', [10, 14, 20, 24, 25, 30])
        # MRM: Need offset on these menus
        # Wall orientation menu
        self.createMiscAttribute('wall_orientation', ['ur','ul','dl','dr'])
        # Wall height menu
        self.createMiscAttribute('wall_height', [10, 20])
        # Window orientation menu
        self.createMiscAttribute('window_orientation', ['ur','ul', None, None])
        # Sign orientation menu
        self.createMiscAttribute('sign_orientation', ['ur','ul', None, None])
        # Door orientation menu
        self.createMiscAttribute('door_orientation', ['ur','ul', None, None])
        # Cornice orientation menu
        self.createMiscAttribute('cornice_orientation', ['ur','ul', None, None])

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
        fname = Filename(dnaDirectory.getFullpath() +
                         '/stylefiles/' + filename)

        # We use binary mode to avoid Windows' end-of-line convention
        f = open(fname.toOsSpecific(), 'rb')
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

    # UTILITY FUNCTIONS
    def hasAttribute(self, attribute):
        """ Return specified attribute for current neighborhood """
        if not self.attributeDictionary.has_key(attribute):
            return 0
        else:
            levelAttribute = self.attributeDictionary[attribute]
            # Get attribute for current neighborhood
            if (type(levelAttribute) == types.DictionaryType):
                editMode = self.getEditMode()
                return levelAttribute.has_key(editMode)
            else:
                return 1

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
        self._dict = {}
        self._list = []
        self.count = 0
        # Currently selected option
        self._current = None
    def setCurrent(self, newValue, fEvent = 1):
        self._current = newValue
        # Send event if specified
        if fEvent:
            messenger.send('select_' + self.name, [self._current])
    def setMenu(self, menu):
        self._menu = menu
        self._menu.action = self.setCurrent
    def setDict(self, dict):
        self._dict = dict
        # Create a list from the dictionary
        self._list = dict.values()
        # Update count
        self.count = len(self._list)
        # Initialize current to first item
        if (len(self._list) > 0):
            self._current = self._list[0]
    def setList(self, list):
        self._list = list
        # Create a dictionary from the list
        self._dict = {}
        self.count = 0
        for item in list:
            self._dict[self.count] = item
            self.count += 1
        # Initialize current to first item
        if (self.count > 0):
            self._current = self._list[0]
    def add(self, item):
        self._dict[self.count] = item
        self._list.append(item)
        self.count += 1
    def getCurrent(self):
        return self._current
    def getMenu(self):
        return self._menu
    def getDict(self):
        return self._dict
    def getList(self):
        return self._list
    def getCount(self):
        return self.count

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
            self.numWalls = 0
            for pair in styleList:
                self.add(pair[0], pair[1])
        else:
            # Use default style/height
            self.styleList = [DNAWallStyle()]
            self.heightList = [10]
            self.numWalls = 1

    def add(self, style, height):
        self.styleList.append(style)
        self.heightList.append(height)
        self.numWalls += 1

    def copy(self, building):
        self.styleList = []
        self.heightList = DNAGetWallHeights(building)[0]
        self.numWalls = 0
        for i in range(building.getNumChildren()):
            child = building.at(i)
            if DNAClassEqual(child, DNA_WALL):
                wallStyle = DNAWallStyle(wall = child)
                self.styleList.append(wallStyle)
                self.numWalls += 1

    def output(self, file = sys.stdout):
        file.write('buildingStyle: %s\n' % createHeightCode(self.heightList))
        for style in self.styleList:
            style.output(file)
        file.write('endBuildingStyle\n')

def createHeightCode(heightList):
    def joinHeights(h1, h2):
        return '%s_%s' % (h1, h2)
    hl = map(ROUND_INT, heightList)
    if len(hl) == 1:
        return `hl[0]`
    return reduce(joinHeights, hl)

def calcHeight(heightList):
    height = 0
    for h in heightList:
        height = height + h
    return int(height)

class DNABaselineStyle:
    """Class to hold attributes of a baseline style (wiggle, colors, etc)"""
    def __init__(self, baseline = None, name = 'baseline_style'):
        # First initialize everything
        self.name = name
        self.code = None # i.e. font
        self.kern = None
        self.wiggle = None
        self.stumble = None
        self.stomp = None
        self.curve = None
        self.x = None
        self.z = None
        self.scaleX = 1.0
        self.scaleZ = 1.0
        self.roll = None
        self.flags = None
        self.color = Vec4(1.0)
        # Then copy the specifics for the baseline if input
        if baseline:
            self.copy(baseline)

    def copy(self, baseline):
        self.code = baseline.getCode()
        self.kern = baseline.getKern()
        self.wiggle = baseline.getWiggle()
        self.stumble = baseline.getStumble()
        self.stomp = baseline.getStomp()
        width=baseline.getWidth()
        if width:
            self.curve = 1.0/width
        else:
            self.curve = 0.0
        pos = baseline.getPos()
        self.x = pos[0]
        self.z = pos[2]
        scale = baseline.getScale()
        self.scaleX = scale[0]
        self.scaleZ = scale[2]
        hpr = baseline.getHpr()
        self.roll = hpr[2]
        self.flags = baseline.getFlags()
        self.color = baseline.getColor()

    def copyTo(self, baseline):
        if self.code != None:
            baseline.setCode(self.code)
        if self.kern != None:
            baseline.setKern(self.kern)
        if self.wiggle != None:
            baseline.setWiggle(self.wiggle)
        if self.stumble != None:
            baseline.setStumble(self.stumble)
        if self.stomp != None:
            baseline.setStomp(self.stomp)
        if self.curve != None:
            diameter=0.0
            if self.curve:
                diameter=1.0/self.curve
            baseline.setWidth(diameter)
            baseline.setHeight(diameter)
        if (self.x != None) or (self.z != None):
            pos=baseline.getPos()
            if self.x != None:
                pos=Vec3(self.x, pos[1], pos[2])
            if self.z != None:
                pos=Vec3(pos[0], pos[1], self.z)
            baseline.setPos(pos)
        if self.roll != None:
            baseline.setHpr(Vec3(0.0, 0.0, self.roll))
        if (self.scaleX != None) or (self.scaleZ != None):
            scale=baseline.getScale()
            if self.scaleX != None:
                scale=Vec3(self.scaleX, scale[1], scale[2])
            if self.scaleZ != None:
                scale=Vec3(scale[0], scale[1], self.scaleZ)
            baseline.setScale(scale)
        if self.flags != None:
            baseline.setFlags(self.flags)
        if self.color != None:
            baseline.setColor(self.color)

    def output(self, file = sys.stdout):
        """ Output style description to a file """
        file.write('baselineStyle\n')
        if self.name != None:
            file.write('name: %s\n' % self.name)
        if self.code != None:
            file.write('code: %s\n' % self.code)
        if self.kern != None:
            file.write('kern: %s\n' % self.kern)
        if self.wiggle != None:
            file.write('wiggle: %s\n' % self.wiggle)
        if self.stumble != None:
            file.write('stumble: %s\n' % self.stumble)
        if self.stomp != None:
            file.write('stomp: %s\n' % self.stomp)
        if self.curve != None:
            file.write('curve: %s\n' % self.curve)
        if self.x != None:
            file.write('x: %s\n' % self.x)
        if self.z != None:
            file.write('z: %s\n' % self.z)
        if self.scaleX != None:
            file.write('scaleX: %s\n' % self.scaleX)
        if self.scaleZ != None:
            file.write('scaleZ: %s\n' % self.scaleZ)
        if self.roll != None:
            file.write('roll: %s\n' % self.roll)
        if self.flags != None:
            file.write('flags: %s\n' % self.flags)
        if self.color != None:
            file.write('color: Vec4(%.2f, %.2f, %.2f, 1.0)\n'
                % (self.color[0], self.color[1], self.color[2]))
        file.write('endBaselineStyle\n')

    # Convience functions to facilitate class use
    def __setitem__(self, index, item):
        self.__dict__[index] = item

    def __getitem__(self, index):
        return self.__dict__[index]

    def __repr__(self):
        return(
            'name: %s\n' % self.name +
            'code: %s\n' % self.code +
            'kern: %s\n' % self.kern +
            'wiggle: %s\n' % self.wiggle +
            'stumble: %s\n' % self.stumble +
            'stomp: %s\n' % self.stomp +
            'curve: %s\n' % self.curve +
            'x: %s\n' % self.x +
            'z: %s\n' % self.z +
            'scaleX: %s\n' % self.scaleX +
            'scaleZ: %s\n' % self.scaleZ +
            'roll: %s\n' % self.roll +
            'flags: %s\n' % self.flags +
            'color: %s\n' % self.color
            )

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
            elif DNAClassEqual(child, DNA_FLAT_DOOR):
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
