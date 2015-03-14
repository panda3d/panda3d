#!/usr/bin/env python

# Author: Shao Zhang and Phil Saltzman
# Last Updated: 2015-03-13
#
# This tutorial is intended as a initial panda scripting lesson going over
# display initialization, loading models, placing objects, and the scene graph.
#
# Step 2: After initializing panda, we define a class called World. We put
# all of our code in a class to provide a convenient way to keep track of
# all of the variables our project will use, and in later tutorials to handle
# keyboard input.
# The code contained in the __init__ method is executed when we instantiate
# the class (at the end of this file).  Inside __init__ we will first change
# the background color of the window.  We then disable the mouse-based camera
# control and set the camera position.

# Initialize Panda and create a window
from direct.showbase.ShowBase import ShowBase
base = ShowBase()

from panda3d.core import *  # Contains most of Panda's modules
from direct.gui.DirectGui import *  # Imports Gui objects we use for putting
# text on the screen
import sys


class World(object):  # Our main class
    def __init__(self):  # The initialization method caused when a
                         # world object is created

        # Create some text overlayed on our screen.
        # We will use similar commands in all of our tutorials to create titles and
        # instruction guides.
        self.title = OnscreenText(
            text="Panda3D: Tutorial 1 - Solar System",
            parent=base.a2dBottomRight, align=TextNode.A_right,
            style=1, fg=(1, 1, 1, 1), pos=(-0.1, 0.1), scale=.07)

        # Make the background color black (R=0, G=0, B=0)
        # instead of the default grey
        base.setBackgroundColor(0, 0, 0)

        # By default, the mouse controls the camera. Often, we disable that so that
        # the camera can be placed manually (if we don't do this, our placement
        # commands will be overridden by the mouse control)
        base.disableMouse()

        # Set the camera position (x, y, z)
        camera.setPos(0, 0, 45)

        # Set the camera orientation (heading, pitch, roll) in degrees
        camera.setHpr(0, -90, 0)
# end class world

# Now that our class is defined, we create an instance of it.
# Doing so calls the __init__ method set up above
w = World()
# As usual - run() must be called before anything can be shown on screen
base.run()
