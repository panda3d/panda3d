#!/usr/bin/env python

# Author: Shao Zhang and Phil Saltzman
# Last Updated: 2015-03-13
#
# This tutorial will cover events and how they can be used in Panda
# Specifically, this lesson will use events to capture keyboard presses and
# mouse clicks to trigger actions in the world. It will also use events
# to count the number of orbits the Earth makes around the sun. This
# tutorial uses the same base code from the solar system tutorial.

from direct.showbase.ShowBase import ShowBase
base = ShowBase()

from panda3d.core import TextNode
from direct.interval.IntervalGlobal import *
from direct.gui.DirectGui import *
from direct.showbase.DirectObject import DirectObject
import sys

# We start this tutorial with the standard class. However, the class is a
# subclass of an object called DirectObject. This gives the class the ability
# to listen for and respond to events. From now on the main class in every
# tutorial will be a subclass of DirectObject


class World(DirectObject):
    # Macro-like function used to reduce the amount to code needed to create the
    # on screen instructions

    def genLabelText(self, text, i):
        return OnscreenText(text=text, pos=(0.06, -.06 * (i + 0.5)), fg=(1, 1, 1, 1),
                            parent=base.a2dTopLeft,align=TextNode.ALeft, scale=.05)

    def __init__(self):

        # The standard camera position and background initialization
        base.setBackgroundColor(0, 0, 0)
        base.disableMouse()
        camera.setPos(0, 0, 45)
        camera.setHpr(0, -90, 0)

        # The global variables we used to control the speed and size of objects
        self.yearscale = 60
        self.dayscale = self.yearscale / 365.0 * 5
        self.orbitscale = 10
        self.sizescale = 0.6

        self.loadPlanets()  # Load, texture, and position the planets
        self.rotatePlanets()  # Set up the motion to start them moving

        # The standard title text that's in every tutorial
        # Things to note:
        #-fg represents the forground color of the text in (r,g,b,a) format
        #-pos  represents the position of the text on the screen.
        #      The coordinate system is a x-y based wih 0,0 as the center of the
        #      screen
        #-align sets the alingment of the text relative to the pos argument.
        #      Default is center align.
        #-scale set the scale of the text
        #-mayChange argument lets us change the text later in the program.
        #       By default mayChange is set to 0. Trying to change text when
        #       mayChange is set to 0 will cause the program to crash.
        self.title = OnscreenText(
            text="Panda3D: Tutorial 3 - Events",
            parent=base.a2dBottomRight, align=TextNode.A_right,
            style=1, fg=(1, 1, 1, 1), pos=(-0.1, 0.1), scale=.07)

        self.mouse1EventText = self.genLabelText(
            "Mouse Button 1: Toggle entire Solar System [RUNNING]", 1)
        self.skeyEventText = self.genLabelText("[S]: Toggle Sun [RUNNING]", 2)
        self.ykeyEventText = self.genLabelText("[Y]: Toggle Mercury [RUNNING]", 3)
        self.vkeyEventText = self.genLabelText("[V]: Toggle Venus [RUNNING]", 4)
        self.ekeyEventText = self.genLabelText("[E]: Toggle Earth [RUNNING]", 5)
        self.mkeyEventText = self.genLabelText("[M]: Toggle Mars [RUNNING]", 6)
        self.yearCounterText = self.genLabelText("0 Earth years completed", 7)

        self.yearCounter = 0  # year counter for earth years
        self.simRunning = True  # boolean to keep track of the
        # state of the global simulation

        # Events
        # Each self.accept statement creates an event handler object that will call
        # the specified function when that event occurs.
        # Certain events like "mouse1", "a", "b", "c" ... "z", "1", "2", "3"..."0"
        # are references to keyboard keys and mouse buttons. You can also define
        # your own events to be used within your program. In this tutorial, the
        # event "newYear" is not tied to a physical input device, but rather
        # is sent by the function that rotates the Earth whenever a revolution
        # completes to tell the counter to update
        # Exit the program when escape is pressed
        self.accept("escape", sys.exit)
        self.accept("mouse1", self.handleMouseClick)
        self.accept("e", self.handleEarth)
        self.accept("s",  # message name
                    self.togglePlanet,  # function to call
                    ["Sun",  # arguments to be passed to togglePlanet
                     # See togglePlanet's definition below for
                     # an explanation of what they are
                     self.day_period_sun,
                     None,
                     self.skeyEventText])
        # Repeat the structure above for the other planets
        self.accept("y", self.togglePlanet,
                    ["Mercury", self.day_period_mercury,
                     self.orbit_period_mercury, self.ykeyEventText])
        self.accept("v", self.togglePlanet,
                    ["Venus", self.day_period_venus,
                     self.orbit_period_venus, self.vkeyEventText])
        self.accept("m", self.togglePlanet,
                    ["Mars", self.day_period_mars,
                     self.orbit_period_mars, self.mkeyEventText])
        self.accept("newYear", self.incYear)
    # end __init__

    def handleMouseClick(self):
        # When the mouse is clicked, if the simulation is running pause all the
        # planets and sun, otherwise resume it
        if self.simRunning:
            print("Pausing Simulation")
            # changing the text to reflect the change from "RUNNING" to
            # "PAUSED"
            self.mouse1EventText.setText(
                "Mouse Button 1: Toggle entire Solar System [PAUSED]")
            # For each planet, check if it is moving and if so, pause it
            # Sun
            if self.day_period_sun.isPlaying():
                self.togglePlanet("Sun", self.day_period_sun, None,
                                  self.skeyEventText)
            if self.day_period_mercury.isPlaying():
                self.togglePlanet("Mercury", self.day_period_mercury,
                                  self.orbit_period_mercury, self.ykeyEventText)
            # Venus
            if self.day_period_venus.isPlaying():
                self.togglePlanet("Venus", self.day_period_venus,
                                  self.orbit_period_venus, self.vkeyEventText)
            #Earth and moon
            if self.day_period_earth.isPlaying():
                self.togglePlanet("Earth", self.day_period_earth,
                                  self.orbit_period_earth, self.ekeyEventText)
                self.togglePlanet("Moon", self.day_period_moon,
                                  self.orbit_period_moon)
            # Mars
            if self.day_period_mars.isPlaying():
                self.togglePlanet("Mars", self.day_period_mars,
                                  self.orbit_period_mars, self.mkeyEventText)
        else:
            #"The simulation is paused, so resume it
            print("Resuming Simulation")
            self.mouse1EventText.setText(
                "Mouse Button 1: Toggle entire Solar System [RUNNING]")
            # the not operator does the reverse of the previous code
            if not self.day_period_sun.isPlaying():
                self.togglePlanet("Sun", self.day_period_sun, None,
                                  self.skeyEventText)
            if not self.day_period_mercury.isPlaying():
                self.togglePlanet("Mercury", self.day_period_mercury,
                                  self.orbit_period_mercury, self.ykeyEventText)
            if not self.day_period_venus.isPlaying():
                self.togglePlanet("Venus", self.day_period_venus,
                                  self.orbit_period_venus, self.vkeyEventText)
            if not self.day_period_earth.isPlaying():
                self.togglePlanet("Earth", self.day_period_earth,
                                  self.orbit_period_earth, self.ekeyEventText)
                self.togglePlanet("Moon", self.day_period_moon,
                                  self.orbit_period_moon)
            if not self.day_period_mars.isPlaying():
                self.togglePlanet("Mars", self.day_period_mars,
                                  self.orbit_period_mars, self.mkeyEventText)
        # toggle self.simRunning
        self.simRunning = not self.simRunning
    # end handleMouseClick

    # The togglePlanet function will toggle the intervals that are given to it
    # between paused and playing.
    # Planet is the name to print
    # Day is the interval that spins the planet
    # Orbit is the interval that moves around the orbit
    # Text is the OnscreenText object that needs to be updated
    def togglePlanet(self, planet, day, orbit=None, text=None):
        if day.isPlaying():
            print("Pausing " + planet)
            state = " [PAUSED]"
        else:
            print("Resuming " + planet)
            state = " [RUNNING]"

        # Update the onscreen text if it is given as an argument
        if text:
            old = text.getText()
            # strip out the last segment of text after the last white space
            # and append the string stored in 'state'
            text.setText(old[0:old.rfind(' ')] + state)

        # toggle the day interval
        self.toggleInterval(day)
        # if there is an orbit interval, toggle it
        if orbit:
            self.toggleInterval(orbit)
    # end togglePlanet

    # toggleInterval does exactly as its name implies
    # It takes an interval as an argument. Then it checks to see if it is playing.
    # If it is, it pauses it, otherwise it resumes it.
    def toggleInterval(self, interval):
        if interval.isPlaying():
            interval.pause()
        else:
            interval.resume()
    # end toggleInterval

    # Earth needs a special buffer function because the moon is tied to it
    # When the "e" key is pressed, togglePlanet is called on both the earth and
    # the moon.
    def handleEarth(self):
        self.togglePlanet("Earth", self.day_period_earth,
                          self.orbit_period_earth, self.ekeyEventText)
        self.togglePlanet("Moon", self.day_period_moon,
                          self.orbit_period_moon)
    # end handleEarth

    # the function incYear increments the variable yearCounter and then updates
    # the OnscreenText 'yearCounterText' every time the message "newYear" is
    # sent
    def incYear(self):
        self.yearCounter += 1
        self.yearCounterText.setText(
            str(self.yearCounter) + " Earth years completed")
    # end incYear


#########################################################################
# Except for the one commented line below, this is all as it was before #
# Scroll down to the next comment to see an example of sending messages #
#########################################################################

    def loadPlanets(self):
        self.orbit_root_mercury = render.attachNewNode('orbit_root_mercury')
        self.orbit_root_venus = render.attachNewNode('orbit_root_venus')
        self.orbit_root_mars = render.attachNewNode('orbit_root_mars')
        self.orbit_root_earth = render.attachNewNode('orbit_root_earth')

        self.orbit_root_moon = (
            self.orbit_root_earth.attachNewNode('orbit_root_moon'))

        self.sky = loader.loadModel("models/solar_sky_sphere")

        self.sky_tex = loader.loadTexture("models/stars_1k_tex.jpg")
        self.sky.setTexture(self.sky_tex, 1)
        self.sky.reparentTo(render)
        self.sky.setScale(40)

        self.sun = loader.loadModel("models/planet_sphere")
        self.sun_tex = loader.loadTexture("models/sun_1k_tex.jpg")
        self.sun.setTexture(self.sun_tex, 1)
        self.sun.reparentTo(render)
        self.sun.setScale(2 * self.sizescale)

        self.mercury = loader.loadModel("models/planet_sphere")
        self.mercury_tex = loader.loadTexture("models/mercury_1k_tex.jpg")
        self.mercury.setTexture(self.mercury_tex, 1)
        self.mercury.reparentTo(self.orbit_root_mercury)
        self.mercury.setPos(0.38 * self.orbitscale, 0, 0)
        self.mercury.setScale(0.385 * self.sizescale)

        self.venus = loader.loadModel("models/planet_sphere")
        self.venus_tex = loader.loadTexture("models/venus_1k_tex.jpg")
        self.venus.setTexture(self.venus_tex, 1)
        self.venus.reparentTo(self.orbit_root_venus)
        self.venus.setPos(0.72 * self.orbitscale, 0, 0)
        self.venus.setScale(0.923 * self.sizescale)

        self.mars = loader.loadModel("models/planet_sphere")
        self.mars_tex = loader.loadTexture("models/mars_1k_tex.jpg")
        self.mars.setTexture(self.mars_tex, 1)
        self.mars.reparentTo(self.orbit_root_mars)
        self.mars.setPos(1.52 * self.orbitscale, 0, 0)
        self.mars.setScale(0.515 * self.sizescale)

        self.earth = loader.loadModel("models/planet_sphere")
        self.earth_tex = loader.loadTexture("models/earth_1k_tex.jpg")
        self.earth.setTexture(self.earth_tex, 1)
        self.earth.reparentTo(self.orbit_root_earth)
        self.earth.setScale(self.sizescale)
        self.earth.setPos(self.orbitscale, 0, 0)

        self.orbit_root_moon.setPos(self.orbitscale, 0, 0)

        self.moon = loader.loadModel("models/planet_sphere")
        self.moon_tex = loader.loadTexture("models/moon_1k_tex.jpg")
        self.moon.setTexture(self.moon_tex, 1)
        self.moon.reparentTo(self.orbit_root_moon)
        self.moon.setScale(0.1 * self.sizescale)
        self.moon.setPos(0.1 * self.orbitscale, 0, 0)

    def rotatePlanets(self):
        self.day_period_sun = self.sun.hprInterval(20, (360, 0, 0))

        self.orbit_period_mercury = self.orbit_root_mercury.hprInterval(
            (0.241 * self.yearscale), (360, 0, 0))
        self.day_period_mercury = self.mercury.hprInterval(
            (59 * self.dayscale), (360, 0, 0))

        self.orbit_period_venus = self.orbit_root_venus.hprInterval(
            (0.615 * self.yearscale), (360, 0, 0))
        self.day_period_venus = self.venus.hprInterval(
            (243 * self.dayscale), (360, 0, 0))

        # Here the earth interval has been changed to rotate like the rest of the
        # planets and send a message before it starts turning again. To send a
        # message, the call is simply messenger.send("message"). The "newYear"
        # message is picked up by the accept("newYear"...) statement earlier, and
        # calls the incYear function as a result
        self.orbit_period_earth = Sequence(
            self.orbit_root_earth.hprInterval(
                self.yearscale, (360, 0, 0)),
            Func(messenger.send, "newYear"))
        self.day_period_earth = self.earth.hprInterval(
            self.dayscale, (360, 0, 0))

        self.orbit_period_moon = self.orbit_root_moon.hprInterval(
            (.0749 * self.yearscale), (360, 0, 0))
        self.day_period_moon = self.moon.hprInterval(
            (.0749 * self.yearscale), (360, 0, 0))

        self.orbit_period_mars = self.orbit_root_mars.hprInterval(
            (1.881 * self.yearscale), (360, 0, 0))
        self.day_period_mars = self.mars.hprInterval(
            (1.03 * self.dayscale), (360, 0, 0))

        self.day_period_sun.loop()
        self.orbit_period_mercury.loop()
        self.day_period_mercury.loop()
        self.orbit_period_venus.loop()
        self.day_period_venus.loop()
        self.orbit_period_earth.loop()
        self.day_period_earth.loop()
        self.orbit_period_moon.loop()
        self.day_period_moon.loop()
        self.orbit_period_mars.loop()
        self.day_period_mars.loop()
    # end RotatePlanets()

# end class world

w = World()
base.run()
