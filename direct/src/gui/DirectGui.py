from DirectGuiGlobals import *

# Initialize global variable guiTop
import __builtin__
__builtin__.guiTop = aspect2d.attachNewNode(PGTop('DirectGuiTop'))
guiTop.node().setMouseWatcher(base.mouseWatcher.node())

# Direct Gui Classes
from DirectFrame import *
from DirectButton import *
from DirectLabel import *

