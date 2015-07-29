#!/usr/bin/env python

# Author: Shao Zhang and Phil Saltzman
# Last Updated: 2015-03-13
#
# This tutorial is intended as a initial panda scripting lesson going over
# display initialization, loading models, placing objects, and the scene graph.
#
# Step 1: ShowBase contains the main Panda3D modules. Importing it
# initializes Panda and creates the window. The run() command causes the
# real-time simulation to begin

from direct.showbase.ShowBase import ShowBase
base = ShowBase()

base.run()
