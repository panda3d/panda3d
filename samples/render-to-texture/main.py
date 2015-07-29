#!/usr/bin/env python

from direct.showbase.ShowBase import ShowBase
from panda3d.core import Filename, Texture
from panda3d.core import AmbientLight, DirectionalLight, PointLight
from panda3d.core import NodePath, TextNode
from direct.task.Task import Task
from direct.actor.Actor import Actor
from direct.gui.OnscreenText import OnscreenText
import sys
import os
import random

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

random.seed()

class TeapotOnTVDemo(ShowBase):

    def __init__(self):
        ShowBase.__init__(self)
        self.disableMouse()
        self.setBackgroundColor((0, 0, 0, 1))

        # Post the instructions.
        self.title = addTitle("Panda3D: Tutorial - Using Render-to-Texture")
        self.inst1 = addInstructions(0.06, "ESC: Quit")
        self.inst2 = addInstructions(0.12, "Up/Down: Zoom in/out on the Teapot")
        self.inst3 = addInstructions(0.18, "Left/Right: Move teapot left/right")
        self.inst4 = addInstructions(0.24, "V: View the render-to-texture results")

        # we now get buffer thats going to hold the texture of our new scene
        altBuffer = self.win.makeTextureBuffer("hello", 256, 256)

        # now we have to setup a new scene graph to make this scene
        altRender = NodePath("new render")

        # this takes care of setting up ther camera properly
        self.altCam = self.makeCamera(altBuffer)
        self.altCam.reparentTo(altRender)
        self.altCam.setPos(0, -10, 0)

        # get the teapot and rotates it for a simple animation
        self.teapot = loader.loadModel('teapot')
        self.teapot.reparentTo(altRender)
        self.teapot.setPos(0, 0, -1)
        self.teapot.hprInterval(1.5, (360, 360, 360)).loop()

        # put some lighting on the teapot
        dlight = DirectionalLight('dlight')
        alight = AmbientLight('alight')
        dlnp = altRender.attachNewNode(dlight)
        alnp = altRender.attachNewNode(alight)
        dlight.setColor((0.8, 0.8, 0.5, 1))
        alight.setColor((0.2, 0.2, 0.2, 1))
        dlnp.setHpr(0, -60, 0)
        altRender.setLight(dlnp)
        altRender.setLight(alnp)

        # Put lighting on the main scene
        plight = PointLight('plight')
        plnp = render.attachNewNode(plight)
        plnp.setPos(0, 0, 10)
        render.setLight(plnp)
        render.setLight(alnp)

        # Panda contains a built-in viewer that lets you view the results of
        # your render-to-texture operations.  This code configures the viewer.

        self.accept("v", self.bufferViewer.toggleEnable)
        self.accept("V", self.bufferViewer.toggleEnable)
        self.bufferViewer.setPosition("llcorner")
        self.bufferViewer.setCardSize(1.0, 0.0)

        # Create the tv-men. Each TV-man will display the
        # offscreen-texture on his TV screen.
        self.tvMen = []
        self.makeTvMan(-5, 30, 1, altBuffer.getTexture(), 0.9)
        self.makeTvMan(5, 30, 1, altBuffer.getTexture(), 1.4)
        self.makeTvMan(0, 23, -3, altBuffer.getTexture(), 2.0)
        self.makeTvMan(-5, 20, -6, altBuffer.getTexture(), 1.1)
        self.makeTvMan(5, 18, -5, altBuffer.getTexture(), 1.7)

        self.accept("escape", sys.exit, [0])
        self.accept("arrow_up", self.zoomIn)
        self.accept("arrow_down", self.zoomOut)
        self.accept("arrow_left", self.moveLeft)
        self.accept("arrow_right", self.moveRight)

    def makeTvMan(self, x, y, z, tex, playrate):
        man = Actor()
        man.loadModel('models/mechman_idle')
        man.setPos(x, y, z)
        man.reparentTo(render)
        faceplate = man.find("**/faceplate")
        faceplate.setTexture(tex, 1)
        man.setPlayRate(playrate, "mechman_anim")
        man.loop("mechman_anim")
        self.tvMen.append(man)

    def zoomIn(self):
        self.altCam.setY(self.altCam.getY() * 0.9)

    def zoomOut(self):
        self.altCam.setY(self.altCam.getY() * 1.2)

    def moveLeft(self):
        self.altCam.setX(self.altCam.getX() + 1)

    def moveRight(self):
        self.altCam.setX(self.altCam.getX() - 1)

demo = TeapotOnTVDemo()
demo.run()
