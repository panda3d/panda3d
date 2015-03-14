#!/usr/bin/env python

# Author: Josh Yelon
#
# This is a tutorial to show one of the simplest applications
# of copy-to-texture: motion trails.
#

from direct.showbase.ShowBase import ShowBase
from panda3d.core import GraphicsOutput
from panda3d.core import Filename, Texture
from panda3d.core import CardMaker
from panda3d.core import NodePath, TextNode
from panda3d.core import AmbientLight, DirectionalLight
from direct.showbase.DirectObject import DirectObject
from direct.gui.OnscreenText import OnscreenText
from direct.task.Task import Task
from direct.actor.Actor import Actor
from random import uniform
import sys
import os

def addInstructions(pos, msg):
    return OnscreenText(text=msg, parent=base.a2dTopLeft,
                        style=1, fg=(1, 1, 1, 1), pos=(0.06, -pos - 0.03),
                        align=TextNode.ALeft, scale=.05)


class MotionTrails(ShowBase):
    def __init__(self):
        # Initialize the ShowBase class from which we inherit, which will
        # create a window and set up everything we need for rendering into it.
        ShowBase.__init__(self)

        self.disableMouse()
        self.camera.setPos(0, -26, 4)
        self.setBackgroundColor(0, 0, 0)

        # Create a texture into which we can copy the main window.
        # We set it to RTMTriggeredCopyTexture mode, which tells it that we
        # want it to copy the window contents into a texture every time we
        # call self.win.triggerCopy().
        self.tex = Texture()
        self.tex.setMinfilter(Texture.FTLinear)
        self.win.addRenderTexture(self.tex,
            GraphicsOutput.RTMTriggeredCopyTexture)

        # Set the initial color to clear the texture to, before rendering it.
        # This is necessary because we don't clear the texture while rendering,
        # and otherwise the user might see garbled random data from GPU memory.
        self.tex.setClearColor((0, 0, 0, 1))
        self.tex.clearImage()

        # Create another 2D camera. Tell it to render before the main camera.
        self.backcam = self.makeCamera2d(self.win, sort=-10)
        self.background = NodePath("background")
        self.backcam.reparentTo(self.background)
        self.background.setDepthTest(0)
        self.background.setDepthWrite(0)
        self.backcam.node().getDisplayRegion(0).setClearDepthActive(0)

        # Obtain two texture cards. One renders before the dragon, the other
        # after.
        self.bcard = self.win.getTextureCard()
        self.bcard.reparentTo(self.background)
        self.bcard.setTransparency(1)
        self.fcard = self.win.getTextureCard()
        self.fcard.reparentTo(self.render2d)
        self.fcard.setTransparency(1)

        # Initialize one of the nice effects.
        self.chooseEffectGhost()

        # Add the task that initiates the screenshots.
        taskMgr.add(self.takeSnapShot, "takeSnapShot")

        # Create some black squares on top of which we will
        # place the instructions.
        blackmaker = CardMaker("blackmaker")
        blackmaker.setColor(0, 0, 0, 1)
        blackmaker.setFrame(-1.00, -0.50, 0.65, 1.00)
        instcard = NodePath(blackmaker.generate())
        instcard.reparentTo(self.render2d)
        blackmaker.setFrame(-0.5, 0.5, -1.00, -0.85)
        titlecard = NodePath(blackmaker.generate())
        titlecard.reparentTo(self.render2d)

        # Panda does its best to hide the differences between DirectX and
        # OpenGL.  But there are a few differences that it cannot hide.
        # One such difference is that when OpenGL copies from a
        # visible window to a texture, it gets it right-side-up.  When
        # DirectX does it, it gets it upside-down.  There is nothing panda
        # can do to compensate except to expose a flag and let the
        # application programmer deal with it.  You should only do this
        # in the rare event that you're copying from a visible window
        # to a texture.
        if self.win.getGsg().getCopyTextureInverted():
            print("Copy texture is inverted.")
            self.bcard.setScale(1, 1, -1)
            self.fcard.setScale(1, 1, -1)

        # Put up the instructions
        title = OnscreenText(text="Panda3D: Tutorial - Motion Trails",
                             fg=(1, 1, 1, 1), parent=base.a2dBottomCenter,
                             pos=(0, 0.1), scale=.08)

        instr0 = addInstructions(0.06, "Press ESC to exit")
        instr1 = addInstructions(0.12, "Press 1: Ghost effect")
        instr2 = addInstructions(0.18, "Press 2: PaintBrush effect")
        instr3 = addInstructions(0.24, "Press 3: Double Vision effect")
        instr4 = addInstructions(0.30, "Press 4: Wings of Blue effect")
        instr5 = addInstructions(0.36, "Press 5: Whirlpool effect")

        # Enable the key events
        self.accept("escape", sys.exit, [0])
        self.accept("1", self.chooseEffectGhost)
        self.accept("2", self.chooseEffectPaintBrush)
        self.accept("3", self.chooseEffectDoubleVision)
        self.accept("4", self.chooseEffectWingsOfBlue)
        self.accept("5", self.chooseEffectWhirlpool)

    def takeSnapShot(self, task):
        if task.time > self.nextclick:
            self.nextclick += 1.0 / self.clickrate
            if self.nextclick < task.time:
                self.nextclick = task.time
            self.win.triggerCopy()
        return Task.cont

    def chooseEffectGhost(self):
        self.setBackgroundColor(0, 0, 0, 1)
        self.bcard.hide()
        self.fcard.show()
        self.fcard.setColor(1.0, 1.0, 1.0, 0.99)
        self.fcard.setScale(1.00)
        self.fcard.setPos(0, 0, 0)
        self.fcard.setR(0)
        self.clickrate = 30
        self.nextclick = 0

    def chooseEffectPaintBrush(self):
        self.setBackgroundColor(0, 0, 0, 1)
        self.bcard.show()
        self.fcard.hide()
        self.bcard.setColor(1, 1, 1, 1)
        self.bcard.setScale(1.0)
        self.bcard.setPos(0, 0, 0)
        self.bcard.setR(0)
        self.clickrate = 10000
        self.nextclick = 0

    def chooseEffectDoubleVision(self):
        self.setBackgroundColor(0, 0, 0, 1)
        self.bcard.show()
        self.bcard.setColor(1, 1, 1, 1)
        self.bcard.setScale(1.0)
        self.bcard.setPos(-0.05, 0, 0)
        self.bcard.setR(0)
        self.fcard.show()
        self.fcard.setColor(1, 1, 1, 0.60)
        self.fcard.setScale(1.0)
        self.fcard.setPos(0.05, 0, 0)
        self.fcard.setR(0)
        self.clickrate = 10000
        self.nextclick = 0

    def chooseEffectWingsOfBlue(self):
        self.setBackgroundColor(0, 0, 0, 1)
        self.fcard.hide()
        self.bcard.show()
        self.bcard.setColor(1.0, 0.90, 1.0, 254.0 / 255.0)
        self.bcard.setScale(1.1, 1, 0.95)
        self.bcard.setPos(0, 0, 0.05)
        self.bcard.setR(0)
        self.clickrate = 30
        self.nextclick = 0

    def chooseEffectWhirlpool(self):
        self.setBackgroundColor(0, 0, 0, 1)
        self.bcard.show()
        self.fcard.hide()
        self.bcard.setColor(1, 1, 1, 1)
        self.bcard.setScale(0.999)
        self.bcard.setPos(0, 0, 0)
        self.bcard.setR(1)
        self.clickrate = 10000
        self.nextclick = 0


t = MotionTrails()

character = Actor()
character.loadModel('models/dancer')
character.reparentTo(t.render)
character.loadAnims({'win': 'models/dancer'})
character.loop('win')
# character.hprInterval(15, LPoint3(360, 0,0)).loop()

# put some lighting on the model
dlight = DirectionalLight('dlight')
alight = AmbientLight('alight')
dlnp = t.render.attachNewNode(dlight)
alnp = t.render.attachNewNode(alight)
dlight.setColor((1.0, 0.9, 0.8, 1))
alight.setColor((0.2, 0.3, 0.4, 1))
dlnp.setHpr(0, -60, 0)
t.render.setLight(dlnp)
t.render.setLight(alnp)

t.run()
