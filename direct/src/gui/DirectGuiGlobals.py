"""Undocumented Module"""

__all__ = []


"""
Global definitions used by Direct Gui Classes and handy constants
that can be used during widget construction
"""
from pandac.PandaModules import *

defaultFont = None
defaultFontFunc = TextNode.getDefaultFont
defaultClickSound = None
defaultRolloverSound = None
defaultDialogGeom = None
drawOrder = 100
panel = None

# USEFUL GUI CONSTANTS
# Constant used to indicate that an option can only be set by a call
# to the constructor.
INITOPT = ['initopt']

# Mouse buttons
LMB = 0
MMB = 1
RMB = 2

# Widget state
NORMAL = 'normal'
DISABLED = 'disabled'

# Frame style
FLAT = PGFrameStyle.TFlat
RAISED = PGFrameStyle.TBevelOut
SUNKEN = PGFrameStyle.TBevelIn
GROOVE = PGFrameStyle.TGroove
RIDGE = PGFrameStyle.TRidge

FrameStyleDict = {'flat': FLAT, 'raised': RAISED, 'sunken': SUNKEN,
                  'groove': GROOVE, 'ridge': RIDGE
                  }

# Orientation of DirectSlider and DirectScrollBar
HORIZONTAL = 'horizontal'
VERTICAL = 'vertical'
VERTICAL_INVERTED = 'vertical_inverted'

# Dialog button values
DIALOG_NO = 0
DIALOG_OK = DIALOG_YES = DIALOG_RETRY = 1
DIALOG_CANCEL = -1

# User can bind commands to these gui events
DESTROY = 'destroy-'
PRINT = 'print-'
ENTER = PGButton.getEnterPrefix()
EXIT = PGButton.getExitPrefix()
WITHIN = PGButton.getWithinPrefix()
WITHOUT = PGButton.getWithoutPrefix()
B1CLICK = PGButton.getClickPrefix() + MouseButton.one().getName() + '-'
B2CLICK = PGButton.getClickPrefix() + MouseButton.two().getName() + '-'
B3CLICK = PGButton.getClickPrefix() + MouseButton.three().getName() + '-'
B1PRESS = PGButton.getPressPrefix() + MouseButton.one().getName() + '-'  
B2PRESS = PGButton.getPressPrefix() + MouseButton.two().getName() + '-'  
B3PRESS = PGButton.getPressPrefix() + MouseButton.three().getName() + '-'
B1RELEASE = PGButton.getReleasePrefix() + MouseButton.one().getName() + '-'  
B2RELEASE = PGButton.getReleasePrefix() + MouseButton.two().getName() + '-'  
B3RELEASE = PGButton.getReleasePrefix() + MouseButton.three().getName() + '-'
# For DirectEntry widgets
OVERFLOW = PGEntry.getOverflowPrefix()
ACCEPT = PGEntry.getAcceptPrefix() + KeyboardButton.enter().getName() + '-'
ACCEPTFAILED = PGEntry.getAcceptFailedPrefix() + KeyboardButton.enter().getName() + '-'
TYPE = PGEntry.getTypePrefix()
ERASE = PGEntry.getErasePrefix()
CURSORMOVE = PGEntry.getCursormovePrefix()
# For DirectSlider and DirectScrollBar widgets
ADJUST = PGSliderBar.getAdjustPrefix()


# For setting the sorting order of a widget's visible components
IMAGE_SORT_INDEX = 10
GEOM_SORT_INDEX = 20
TEXT_SORT_INDEX = 30

# Handy conventions for organizing top-level gui objects in loose buckets.
BACKGROUND_SORT_INDEX = -100
MIDGROUND_SORT_INDEX = 0
FOREGROUND_SORT_INDEX = 100

# Symbolic constants for the indexes into an optionInfo list.
_OPT_DEFAULT         = 0
_OPT_VALUE           = 1
_OPT_FUNCTION        = 2

# DirectButton States:
BUTTON_READY_STATE     = PGButton.SReady       # 0
BUTTON_DEPRESSED_STATE = PGButton.SDepressed   # 1
BUTTON_ROLLOVER_STATE  = PGButton.SRollover    # 2
BUTTON_INACTIVE_STATE  = PGButton.SInactive    # 3

def getDefaultRolloverSound():
    global defaultRolloverSound
    if defaultRolloverSound == None:
        defaultRolloverSound = base.loadSfx("audio/sfx/GUI_rollover.wav")
    return defaultRolloverSound

def setDefaultRolloverSound(newSound):
    global defaultRolloverSound
    defaultRolloverSound = newSound

def getDefaultClickSound():
    global defaultClickSound
    if defaultClickSound == None:
        defaultClickSound = base.loadSfx("audio/sfx/GUI_click.wav")
    return defaultClickSound

def setDefaultClickSound(newSound):
    global defaultClickSound
    defaultClickSound = newSound

def getDefaultFont():
    global defaultFont
    if defaultFont == None:
        defaultFont = defaultFontFunc()
    return defaultFont

def setDefaultFont(newFont):
    global defaultFont
    defaultFont = newFont

def setDefaultFontFunc(newFontFunc):
    global defaultFontFunc
    defaultFontFunc = newFontFunc

def getDefaultDialogGeom():
    global defaultDialogGeom
    if defaultDialogGeom == None:
        defaultDialogGeom = loader.loadModel('models/gui/dialog_box_gui')
    return defaultDialogGeom

def setDefaultDialogGeom(newDialogGeom):
    global defaultDialogGeom
    defaultDialogGeom = newDialogGeom

def getDefaultDrawOrder():
    return drawOrder

def setDefaultDrawOrder(newDrawOrder):
    global drawOrder
    drawOrder = newDrawOrder

def getDefaultPanel():
    return panel

def setDefaultPanel(newPanel):
    global panel
    panel = newPanel

#from OnscreenText import *
#from OnscreenGeom import *
#from OnscreenImage import *
