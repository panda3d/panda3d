from PandaObject import *
from PGTop import *
from PGButton import *
from PGItem import *
from PGFrameStyle import *
import OnscreenText
import OnscreenGeom
import OnscreenImage
import types

# USEFUL GUI CONSTANTS
NORMAL = 'normal'
DISABLED = 'disabled'

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
    return eventString[:eventString.rfind('-')]

ENTER = 'enter'
EXIT = 'exit'
B1CLICK = getGenericMouseEvent('click', 1)
B2CLICK = getGenericMouseEvent('click', 2)
B3CLICK = getGenericMouseEvent('click', 3)
B1PRESS = getGenericMouseEvent('press', 1)
B2PRESS = getGenericMouseEvent('press', 2)
B3PRESS = getGenericMouseEvent('press', 3)
B1RELEASE = getGenericMouseEvent('release', 1)
B2RELEASE = getGenericMouseEvent('release', 2)
B3RELEASE = getGenericMouseEvent('release', 3)

# Constant used to indicate that an option can only be set by a call
# to the constructor.
INITOPT = ['initopt']
