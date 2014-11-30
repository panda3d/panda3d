"""
The RibbonBar library is a set of classes for writing a ribbon user interface.
At the most generic level, this is a combination of a tab control with a toolbar.
At a more functional level, it is similar to the user interface present in recent
versions of Microsoft Office.

A ribbon user interface typically has a ribbon bar, which contains one or more
RibbonPages, which in turn each contains one or more RibbonPanels, which in turn
contain controls.
"""

from art import *
from art_aui import *
from art_internal import *
from art_msw import *
from art_default import *

from bar import *
from buttonbar import *
from control import *
from gallery import *

from page import *
from panel import *
from toolbar import *

