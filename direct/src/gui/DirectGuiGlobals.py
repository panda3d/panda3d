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

# Use to extract common prefix of mouse click, press, and release events
def getGenericMouseEvent(event, mouse):
    if mouse == 1:
        mb = MouseButton.one()
    elif mouse == 2:
        mb = MouseButton.two()
    elif mouse == 3:
        mb = MouseButton.three()
    if event == 'click':
        eventFunc = PGButton().getClickEvent
    elif event == 'press':
        eventFunc = PGButton().getPressEvent
    elif event == 'release':
        eventFunc = PGButton().getReleaseEvent
    eventString = eventFunc(mb)
    return eventString[:eventString.rfind('-') + 1]

# User can bind commands to these gui events
ENTER = 'enter-'
EXIT = 'exit-'
B1CLICK = getGenericMouseEvent('click', 1)
B2CLICK = getGenericMouseEvent('click', 2)
B3CLICK = getGenericMouseEvent('click', 3)
B1PRESS = getGenericMouseEvent('press', 1)
B2PRESS = getGenericMouseEvent('press', 2)
B3PRESS = getGenericMouseEvent('press', 3)
B1RELEASE = getGenericMouseEvent('release', 1)
B2RELEASE = getGenericMouseEvent('release', 2)
B3RELEASE = getGenericMouseEvent('release', 3)

# For setting the sorting order of a widget's visible components
IMAGE_SORT_INDEX = 10
GEOM_SORT_INDEX = 20
TEXT_SORT_INDEX = 30

