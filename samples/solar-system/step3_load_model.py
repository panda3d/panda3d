#!/usr/bin/env python

# Author: Shao Zhang and Phil Saltzman
# Last Updated: 2015-03-13
#
# This tutorial is intended as a initial panda scripting lesson going over
# display initialization, loading models, placing objects, and the scene graph.
#
# Step 3: In this step, we create a function called loadPlanets, which will
# eventually be used to load all of the planets in our simulation. For now
# we will load just the sun and and the sky-sphere we use to create the
# star-field.

from direct.showbase.ShowBase import ShowBase
base = ShowBase()

from panda3d.core import NodePath, TextNode
from direct.gui.DirectGui import *
import sys


class World(object):
    def __init__(self):
        # This is the initialization we had before
        self.title = OnscreenText(  # Create the title
            text="Panda3D: Tutorial 1 - Solar System",
            parent=base.a2dBottomRight, align=TextNode.A_right,
            style=1, fg=(1, 1, 1, 1), pos=(-0.1, 0.1), scale=.07)

        base.setBackgroundColor(0, 0, 0)  # Set the background to black
        base.disableMouse()  # disable mouse control of the camera
        camera.setPos(0, 0, 45)  # Set the camera position (X, Y, Z)
        camera.setHpr(0, -90, 0)  # Set the camera orientation
        #(heading, pitch, roll) in degrees

        # We will now define a variable to help keep a consistent scale in
        # our model. As we progress, we will continue to add variables here as we
        # need them

        # The value of this variable scales the size of the planets. True scale size
        # would be 1
        self.sizescale = 0.6

        # Now that we have finished basic initialization, we call loadPlanets which
        # will handle actually getting our objects in the world
        self.loadPlanets()

    def loadPlanets(self):
        # Here, inside our class, is where we are creating the loadPlanets function
        # For now we are just loading the star-field and sun. In the next step we
        # will load all of the planets

        # Loading objects in Panda is done via the command loader.loadModel, which
        # takes one argument, the path to the model file. Models in Panda come in
        # two types, .egg (which is readable in a text editor), and .bam (which is
        # not readable but makes smaller files). When you load a file you leave the
        # extension off so that it can choose the right version

        # Load model returns a NodePath, which you can think of as an object
        # containing your model

        # Here we load the sky model. For all the planets we will use the same
        # sphere model and simply change textures. However, even though the sky is
        # a sphere, it is different from the planet model because its polygons
        #(which are always one-sided in Panda) face inside the sphere instead of
        # outside (this is known as a model with reversed normals). Because of
        # that it has to be a separate model.
        self.sky = loader.loadModel("models/solar_sky_sphere")

        # After the object is loaded, it must be placed in the scene. We do this by
        # changing the parent of self.sky to render, which is a special NodePath.
        # Each frame, Panda starts with render and renders everything attached to
        # it.
        self.sky.reparentTo(render)

        # You can set the position, orientation, and scale on a NodePath the same
        # way that you set those properties on the camera. In fact, the camera is
        # just another special NodePath
        self.sky.setScale(40)

        # Very often, the egg file will know what textures are needed and load them
        # automatically. But sometimes we want to set our textures manually, (for
        # instance we want to put different textures on the same planet model)
        # Loading textures works the same way as loading models, but instead of
        # calling loader.loadModel, we call loader.loadTexture
        self.sky_tex = loader.loadTexture("models/stars_1k_tex.jpg")

        # Finally, the following line sets our new sky texture on our sky model.
        # The second argument must be one or the command will be ignored.
        self.sky.setTexture(self.sky_tex, 1)

        # Now we load the sun.
        self.sun = loader.loadModel("models/planet_sphere")
        # Now we repeat our other steps
        self.sun.reparentTo(render)
        self.sun_tex = loader.loadTexture("models/sun_1k_tex.jpg")
        self.sun.setTexture(self.sun_tex, 1)
        # The sun is really much bigger than
        self.sun.setScale(2 * self.sizescale)
        # this, but to be able to see the
        # planets we're making it smaller

    # end loadPlanets()

# end class world

# instantiate the class
w = World()
base.run()
