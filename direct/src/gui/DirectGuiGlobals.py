"""
Global definitions used by Direct Gui Classes and handy constants
that can be used during widget construction
"""

from PandaObject import *
from PGTop import *
from PGButton import *
# Import these after PGButton to get actual class definitions
from PGItem import *
from PGFrameStyle import *
# Helper classes used as components of Direct Gui Widgets
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

# For setting the sorting order of a widget's visible components
IMAGE_SORT_INDEX = 10
GEOM_SORT_INDEX = 20
TEXT_SORT_INDEX = 30

