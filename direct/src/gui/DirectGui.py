"""Imports all of the :ref:`directgui` classes."""

from . import DirectGuiGlobals as DGG
from .OnscreenText import *
from .OnscreenGeom import *
from .OnscreenImage import *

# MPG DirectStart should call this?
# Set up default font
#defaultFont = getDefaultFont()
#if defaultFont:
#    PGItem.getTextNode().setFont(defaultFont)

# Direct Gui Classes
from .DirectFrame import *
from .DirectButton import *
from .DirectEntry import *
from .DirectEntryScroll import *
from .DirectLabel import *
from .DirectScrolledList import *
from .DirectDialog import *
from .DirectWaitBar import *
from .DirectSlider import *
from .DirectScrollBar import *
from .DirectScrolledFrame import *
from .DirectCheckButton import *
from .DirectOptionMenu import *
from .DirectRadioButton import *
