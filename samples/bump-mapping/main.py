#!/usr/bin/env python
#
# Bump mapping is a way of making polygonal surfaces look
# less flat.  This sample uses normal mapping for all
# surfaces, and also parallax mapping for the column.
#
# This is a tutorial to show how to do normal mapping
# in panda3d using the Shader Generator.

from direct.showbase.ShowBase import ShowBase
from panda3d.core import loadPrcFileData
from panda3d.core import WindowProperties
from panda3d.core import Filename, Shader
from panda3d.core import AmbientLight, PointLight
from panda3d.core import TextNode
from panda3d.core import LPoint3, LVector3
from direct.task.Task import Task
from direct.actor.Actor import Actor
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
from direct.filter.CommonFilters import *
import sys
import os


# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(1, 1, 1, 1), scale=.05,
                        shadow=(0, 0, 0, 1), parent=base.a2dTopLeft,
                        pos=(0.08, -pos - 0.04), align=TextNode.ALeft)

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, fg=(1, 1, 1, 1), scale=.08,
                        parent=base.a2dBottomRight, align=TextNode.ARight,
                        pos=(-0.1, 0.09), shadow=(0, 0, 0, 1))


class BumpMapDemo(ShowBase):

    def __init__(self):
        # Configure the parallax mapping settings (these are just the defaults)
        loadPrcFileData("", "parallax-mapping-samples 3\n"
                            "parallax-mapping-scale 0.1")

        # Initialize the ShowBase class from which we inherit, which will
        # create a window and set up everything we need for rendering into it.
        ShowBase.__init__(self)

        # Check video card capabilities.
        if not self.win.getGsg().getSupportsBasicShaders():
            addTitle("Bump Mapping: "
                "Video driver reports that Cg shaders are not supported.")
            return

        # Post the instructions
        self.title = addTitle("Panda3D: Tutorial - Bump Mapping")
        self.inst1 = addInstructions(0.06, "Press ESC to exit")
        self.inst2 = addInstructions(0.12, "Move mouse to rotate camera")
        self.inst3 = addInstructions(0.18, "Left mouse button: Move forwards")
        self.inst4 = addInstructions(0.24, "Right mouse button: Move backwards")
        self.inst5 = addInstructions(0.30, "Enter: Turn bump maps Off")

        # Load the 'abstract room' model.  This is a model of an
        # empty room containing a pillar, a pyramid, and a bunch
        # of exaggeratedly bumpy textures.

        self.room = loader.loadModel("models/abstractroom")
        self.room.reparentTo(render)

        # Make the mouse invisible, turn off normal mouse controls
        self.disableMouse()
        props = WindowProperties()
        props.setCursorHidden(True)
        self.win.requestProperties(props)
        self.camLens.setFov(60)

        # Set the current viewing target
        self.focus = LVector3(55, -55, 20)
        self.heading = 180
        self.pitch = 0
        self.mousex = 0
        self.mousey = 0
        self.last = 0
        self.mousebtn = [0, 0, 0]

        # Start the camera control task:
        taskMgr.add(self.controlCamera, "camera-task")
        self.accept("escape", sys.exit, [0])
        self.accept("mouse1", self.setMouseBtn, [0, 1])
        self.accept("mouse1-up", self.setMouseBtn, [0, 0])
        self.accept("mouse2", self.setMouseBtn, [1, 1])
        self.accept("mouse2-up", self.setMouseBtn, [1, 0])
        self.accept("mouse3", self.setMouseBtn, [2, 1])
        self.accept("mouse3-up", self.setMouseBtn, [2, 0])
        self.accept("enter", self.toggleShader)
        self.accept("j", self.rotateLight, [-1])
        self.accept("k", self.rotateLight, [1])
        self.accept("arrow_left", self.rotateCam, [-1])
        self.accept("arrow_right", self.rotateCam, [1])

        # Add a light to the scene.
        self.lightpivot = render.attachNewNode("lightpivot")
        self.lightpivot.setPos(0, 0, 25)
        self.lightpivot.hprInterval(10, LPoint3(360, 0, 0)).loop()
        plight = PointLight('plight')
        plight.setColor((1, 1, 1, 1))
        plight.setAttenuation(LVector3(0.7, 0.05, 0))
        plnp = self.lightpivot.attachNewNode(plight)
        plnp.setPos(45, 0, 0)
        self.room.setLight(plnp)

        # Add an ambient light
        alight = AmbientLight('alight')
        alight.setColor((0.2, 0.2, 0.2, 1))
        alnp = render.attachNewNode(alight)
        self.room.setLight(alnp)

        # Create a sphere to denote the light
        sphere = loader.loadModel("models/icosphere")
        sphere.reparentTo(plnp)

        # Tell Panda that it should generate shaders performing per-pixel
        # lighting for the room.
        self.room.setShaderAuto()

        self.shaderenable = 1

    def setMouseBtn(self, btn, value):
        self.mousebtn[btn] = value

    def rotateLight(self, offset):
        self.lightpivot.setH(self.lightpivot.getH() + offset * 20)

    def rotateCam(self, offset):
        self.heading = self.heading - offset * 10

    def toggleShader(self):
        self.inst5.destroy()
        if (self.shaderenable):
            self.inst5 = addInstructions(0.30, "Enter: Turn bump maps On")
            self.shaderenable = 0
            self.room.setShaderOff()
        else:
            self.inst5 = addInstructions(0.30, "Enter: Turn bump maps Off")
            self.shaderenable = 1
            self.room.setShaderAuto()

    def controlCamera(self, task):
        # figure out how much the mouse has moved (in pixels)
        md = self.win.getPointer(0)
        x = md.getX()
        y = md.getY()
        if self.win.movePointer(0, 100, 100):
            self.heading = self.heading - (x - 100) * 0.2
            self.pitch = self.pitch - (y - 100) * 0.2
        if self.pitch < -45:
            self.pitch = -45
        if self.pitch > 45:
            self.pitch = 45
        self.camera.setHpr(self.heading, self.pitch, 0)
        dir = self.camera.getMat().getRow3(1)
        elapsed = task.time - self.last
        if self.last == 0:
            elapsed = 0
        if self.mousebtn[0]:
            self.focus = self.focus + dir * elapsed * 30
        if self.mousebtn[1] or self.mousebtn[2]:
            self.focus = self.focus - dir * elapsed * 30
        self.camera.setPos(self.focus - (dir * 5))
        if self.camera.getX() < -59.0:
            self.camera.setX(-59)
        if self.camera.getX() > 59.0:
            self.camera.setX(59)
        if self.camera.getY() < -59.0:
            self.camera.setY(-59)
        if self.camera.getY() > 59.0:
            self.camera.setY(59)
        if self.camera.getZ() < 5.0:
            self.camera.setZ(5)
        if self.camera.getZ() > 45.0:
            self.camera.setZ(45)
        self.focus = self.camera.getPos() + (dir * 5)
        self.last = task.time
        return Task.cont

demo = BumpMapDemo()
demo.run()
