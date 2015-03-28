#!/usr/bin/env python

# Author: Shao Zhang and Phil Saltzman
# Last Updated: 2015-03-13
#
# This tutorial is intended as a initial panda scripting lesson going over
# display initialization, loading models, placing objects, and the scene graph.
#
# Step 4: In this step, we will load the rest of the planets up to Mars.
# In addition to loading them, we will organize how the planets are grouped
# hierarchically in the scene. This will help us rotate them in the next step
# to give a rough simulation of the solar system.  You can see them move by
# running step_5_complete_solar_system.py.

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

        # This section has our variables. This time we are adding a variable to
        # control the relative size of the orbits.
        self.sizescale = 0.6  # relative size of planets
        self.orbitscale = 10  # relative size of orbits

        self.loadPlanets()  # Load our models and make them render

    def loadPlanets(self):
        # Here is where we load all of the planets, and place them.
        # The first thing we do is create a dummy node for each planet. A dummy
        # node is simply a node path that does not have any geometry attached to it.
        # This is done by <NodePath>.attachNewNode('name_of_new_node')

        # We do this because positioning the planets around a circular orbit could
        # be done with a lot of messy sine and cosine operations. Instead, we define
        # our planets to be a given distance from a dummy node, and when we turn the
        # dummy, the planets will move along with it, kind of like turning the
        # center of a disc and having an object at its edge move. Most attributes,
        # like position, orientation, scale, texture, color, etc., are inherited
        # this way. Panda deals with the fact that the objects are not attached
        # directly to render (they are attached through other NodePaths to render),
        # and makes sure the attributes inherit.

        # This system of attaching NodePaths to each other is called the Scene
        # Graph
        self.orbit_root_mercury = render.attachNewNode('orbit_root_mercury')
        self.orbit_root_venus = render.attachNewNode('orbit_root_venus')
        self.orbit_root_mars = render.attachNewNode('orbit_root_mars')
        self.orbit_root_earth = render.attachNewNode('orbit_root_earth')

        # orbit_root_moon is like all the other orbit_root dummy nodes except that
        # it will be parented to orbit_root_earth so that the moon will orbit the
        # earth instead of the sun. So, the moon will first inherit
        # orbit_root_moon's position and then orbit_root_earth's. There is no hard
        # limit on how many objects can inherit from each other.
        self.orbit_root_moon = (
            self.orbit_root_earth.attachNewNode('orbit_root_moon'))

        ###############################################################

        # These are the same steps used to load the sky model that we used in the
        # last step
        # Load the model for the sky
        self.sky = loader.loadModel("models/solar_sky_sphere")
        # Load the texture for the sky.
        self.sky_tex = loader.loadTexture("models/stars_1k_tex.jpg")
        # Set the sky texture to the sky model
        self.sky.setTexture(self.sky_tex, 1)
        # Parent the sky model to the render node so that the sky is rendered
        self.sky.reparentTo(render)
        # Scale the size of the sky.
        self.sky.setScale(40)

        # These are the same steps we used to load the sun in the last step.
        # Again, we use loader.loadModel since we're using planet_sphere more
        # than once.
        self.sun = loader.loadModel("models/planet_sphere")
        self.sun_tex = loader.loadTexture("models/sun_1k_tex.jpg")
        self.sun.setTexture(self.sun_tex, 1)
        self.sun.reparentTo(render)
        self.sun.setScale(2 * self.sizescale)

        # Now we load the planets, which we load using the same steps we used to
        # load the sun. The only difference is that the models are not parented
        # directly to render for the reasons described above.
        # The values used for scale are the ratio of the planet's radius to Earth's
        # radius, multiplied by our global scale variable. In the same way, the
        # values used for orbit are the ratio of the planet's orbit to Earth's
        # orbit, multiplied by our global orbit scale variable

        # Load mercury
        self.mercury = loader.loadModel("models/planet_sphere")
        self.mercury_tex = loader.loadTexture("models/mercury_1k_tex.jpg")
        self.mercury.setTexture(self.mercury_tex, 1)
        self.mercury.reparentTo(self.orbit_root_mercury)
        # Set the position of mercury. By default, all nodes are pre assigned the
        # position (0, 0, 0) when they are first loaded. We didn't reposition the
        # sun and sky because they are centered in the solar system. Mercury,
        # however, needs to be offset so we use .setPos to offset the
        # position of mercury in the X direction with respect to its orbit radius.
        # We will do this for the rest of the planets.
        self.mercury.setPos(0.38 * self.orbitscale, 0, 0)
        self.mercury.setScale(0.385 * self.sizescale)

        # Load Venus
        self.venus = loader.loadModel("models/planet_sphere")
        self.venus_tex = loader.loadTexture("models/venus_1k_tex.jpg")
        self.venus.setTexture(self.venus_tex, 1)
        self.venus.reparentTo(self.orbit_root_venus)
        self.venus.setPos(0.72 * self.orbitscale, 0, 0)
        self.venus.setScale(0.923 * self.sizescale)

        # Load Mars
        self.mars = loader.loadModel("models/planet_sphere")
        self.mars_tex = loader.loadTexture("models/mars_1k_tex.jpg")
        self.mars.setTexture(self.mars_tex, 1)
        self.mars.reparentTo(self.orbit_root_mars)
        self.mars.setPos(1.52 * self.orbitscale, 0, 0)
        self.mars.setScale(0.515 * self.sizescale)

        # Load Earth
        self.earth = loader.loadModel("models/planet_sphere")
        self.earth_tex = loader.loadTexture("models/earth_1k_tex.jpg")
        self.earth.setTexture(self.earth_tex, 1)
        self.earth.reparentTo(self.orbit_root_earth)
        self.earth.setScale(self.sizescale)
        self.earth.setPos(self.orbitscale, 0, 0)

        # The center of the moon's orbit is exactly the same distance away from
        # The sun as the Earth's distance from the sun
        self.orbit_root_moon.setPos(self.orbitscale, 0, 0)

        # Load the moon
        self.moon = loader.loadModel("models/planet_sphere")
        self.moon_tex = loader.loadTexture("models/moon_1k_tex.jpg")
        self.moon.setTexture(self.moon_tex, 1)
        self.moon.reparentTo(self.orbit_root_moon)
        self.moon.setScale(0.1 * self.sizescale)
        self.moon.setPos(0.1 * self.orbitscale, 0, 0)
    # end loadPlanets()

# end class world

w = World()
base.run()
