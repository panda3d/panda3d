from DirectGuiGlobals import *

# Initialize global variable guiTop
import __builtin__
__builtin__.guiTop = aspect2d.attachNewNode(PGTop('DirectGuiTop'))
guiTop.node().setMouseWatcher(base.mouseWatcherNode)
base.mouseWatcherNode.addRegion(PGMouseWatcherBackground())

# Set up default font
PGItem.getTextNode().setFont(getDefaultFont())

# Direct Gui Classes
from DirectFrame import *
from DirectButton import *
from DirectEntry import *
from DirectLabel import *
from DirectScrolledList import *
from DirectDialog import *
