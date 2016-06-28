#!/usr/bin/env python

# Author: Shao Zhang, Phil Saltzman
# Last Updated: 2015-03-13
#
# This file contians the minimum code needed to load the particle panel tool
# See readme.txt for more information

import sys
try:
    import _tkinter
except:
    sys.exit("Please install python module 'Tkinter'")

try:
    import Pmw
except:
    sys.exit("Please install Python megawidgets")

# Makes sure that Panda is configured to play nice with Tkinter
from panda3d.core import *
loadPrcFileData("", "want-tk true")

# Open the Panda window
from direct.showbase.ShowBase import ShowBase
base = ShowBase()

from direct.tkpanels.ParticlePanel import ParticlePanel

pp = ParticlePanel()             # Create the panel
base.disableMouse()              # Disable camera control to place it
base.camera.setY(-10)            # Place the camera
base.setBackgroundColor(0, 0, 0) # Most particle systems show up better on black backgrounds
base.run()
