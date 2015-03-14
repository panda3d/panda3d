#!/usr/bin/env python

# Author: Kwasi Mensah (kmensah@andrew.cmu.edu)
# Date: 7/25/2005

from direct.showbase.ShowBase import ShowBase
from panda3d.core import Filename, Shader
from panda3d.core import PandaNode, NodePath
from panda3d.core import AmbientLight, DirectionalLight
from panda3d.core import TextNode, LPoint3
from direct.showbase.DirectObject import DirectObject
from direct.filter.CommonFilters import CommonFilters
from direct.gui.OnscreenText import OnscreenText
from direct.actor.Actor import Actor
import sys
import os

# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(1, 1, 1, 1),
                        parent=base.a2dTopLeft, align=TextNode.ALeft,
                        pos=(0.08, -pos - 0.04), scale=.05)

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, fg=(1, 1, 1, 1),
                        parent=base.a2dBottomRight, align=TextNode.ARight,
                        pos=(-0.1, 0.09), scale=.08)


class GlowDemo(ShowBase):
    def __init__(self):
        # Initialize the ShowBase class from which we inherit, which will
        # create a window and set up everything we need for rendering into it.
        ShowBase.__init__(self)

        base.disableMouse()
        base.setBackgroundColor(0, 0, 0)
        camera.setPos(0, -50, 0)

        # Check video card capabilities.
        if not base.win.getGsg().getSupportsBasicShaders():
            addTitle(
                "Glow Filter: Video driver reports that Cg shaders are not supported.")
            return

        # Use class 'CommonFilters' to enable a bloom filter.
        # The brightness of a pixel is measured using a weighted average
        # of R,G,B,A.  We put all the weight on Alpha, meaning that for
        # us, the framebuffer's alpha channel alpha controls bloom.

        self.filters = CommonFilters(base.win, base.cam)
        filterok = self.filters.setBloom(
            blend=(0, 0, 0, 1), desat=-0.5, intensity=3.0, size="small")
        if (filterok == False):
            addTitle(
                "Toon Shader: Video card not powerful enough to do image postprocessing")
            return
        self.glowSize = 1

        # Post the instructions
        self.title = addTitle("Panda3D: Tutorial - Glow Filter")
        self.inst1 = addInstructions(0.06, "ESC: Quit")
        self.inst2 = addInstructions(0.12, "Space: Toggle Glow Filter Small/Med/Large/Off")
        self.inst3 = addInstructions(0.18, "Enter: Toggle Running/Spinning")
        self.inst4 = addInstructions(0.24, "V: View the render-to-texture results")

        # load our model

        self.tron = Actor()
        self.tron.loadModel("models/tron")
        self.tron.loadAnims({"running": "models/tron_anim"})
        self.tron.reparentTo(render)
        self.interval = self.tron.hprInterval(60, LPoint3(360, 0, 0))
        self.interval.loop()
        self.isRunning = False

        # put some lighting on the model

        dlight = DirectionalLight('dlight')
        alight = AmbientLight('alight')
        dlnp = render.attachNewNode(dlight)
        alnp = render.attachNewNode(alight)
        dlight.setColor((1.0, 0.7, 0.2, 1))
        alight.setColor((0.2, 0.2, 0.2, 1))
        dlnp.setHpr(0, -60, 0)
        render.setLight(dlnp)
        render.setLight(alnp)

        # Panda contains a built-in viewer that lets you view the results of
        # your render-to-texture operations.  This code configures the viewer.
        self.accept("v", base.bufferViewer.toggleEnable)
        self.accept("V", base.bufferViewer.toggleEnable)
        base.bufferViewer.setPosition("llcorner")
        base.bufferViewer.setLayout("hline")
        # base.camLens.setFov(100)
        # event handling
        self.accept("space", self.toggleGlow)
        self.accept("enter", self.toggleDisplay)
        self.accept("escape", sys.exit, [0])

    def toggleGlow(self):
        self.glowSize = self.glowSize + 1
        if self.glowSize == 4:
            self.glowSize = 0
        self.filters.setBloom(blend=(0, 0, 0, 1), desat=-0.5, intensity=3.0,
                              size=self.glowSize)

    def toggleDisplay(self):
        self.isRunning = not self.isRunning
        if not self.isRunning:
            camera.setPos(0, -50, 0)
            self.tron.stop("running")
            self.tron.pose("running", 0)
            self.interval.loop()
        else:
            camera.setPos(0, -170, 3)
            self.interval.finish()
            self.tron.setHpr(0, 0, 0)
            self.tron.loop("running")

demo = GlowDemo()
demo.run()
