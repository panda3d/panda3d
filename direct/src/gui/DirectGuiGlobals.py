"""
Global definitions used by Direct Gui Classes and handy constants
that can be used during widget construction
"""

from PandaModules import *
import OnscreenText
import OnscreenGeom
import OnscreenImage
import types

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

FrameStyleDict = {'flat' : FLAT, 'raised' : RAISED, 'sunken': SUNKEN,
                  'groove' : GROOVE, 'ridge' : RIDGE 
                  }

# User can bind commands to these gui events
DESTROY = 'destroy-'
PRINT = 'print-'
ENTER = PGButton.getEnterPrefix()
EXIT = PGButton.getExitPrefix()
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
TYPE = PGEntry.getTypePrefix()
ERASE = PGEntry.getErasePrefix()


# For setting the sorting order of a widget's visible components
IMAGE_SORT_INDEX = 10
GEOM_SORT_INDEX = 20
TEXT_SORT_INDEX = 30

defaultFont = None
defaultClickSound = None
defaultRolloverSound = None
defaultDialogGeom = None

def getDefaultRolloverSound():
    global defaultRolloverSound
    if not base.wantSfx:
        return None
    if defaultRolloverSound == None:
        defaultRolloverSound = base.loadSfx(
            "phase_3/audio/sfx/GUI_rollover.mp3")
    return defaultRolloverSound

def getDefaultClickSound():
    global defaultClickSound
    if not base.wantSfx:
        return None
    if defaultClickSound == None:
        defaultClickSound = base.loadSfx(
            "phase_3/audio/sfx/GUI_create_toon_fwd.mp3")
    return defaultClickSound

def getDefaultFont():
    global defaultFont
    if defaultFont == None:
        defaultFont = loader.loadFont("models/fonts/Comic")
    return defaultFont

def setDefaultFont(newFont):
    global defaultFont
    defaultFont = newFont

def getDefaultDialogGeom():
    global defaultDialogGeom
    if defaultDialogGeom == None:
        defaultDialogGeom = loader.loadModelOnce(
            'phase_3/models/gui/dialog_box_gui')
    return defaultDialogGeom

def setDefaultDialogGeom(newDialogGeom):
    global defaultDialogGeom
    defaultDialogGeom = newDialogGeom
