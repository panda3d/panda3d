#!/usr/bin/env python

# Author: Kwasi Mensah (kmensah@andrew.cmu.edu)
# Date: 7/25/2005

from direct.showbase.ShowBase import ShowBase
from panda3d.core import Filename, Shader
from panda3d.core import PandaNode, NodePath
from panda3d.core import ColorBlendAttrib
from panda3d.core import AmbientLight, DirectionalLight
from panda3d.core import TextNode, LPoint3, LVector4
from direct.showbase.DirectObject import DirectObject
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


# This function is responsible for setting up the two blur filters.
# It just makes a temp Buffer, puts a screen aligned card, and then sets
# the appropiate shader to do all the work. Gaussian blurs are decomposable
# into a two-pass algorithm which is faster than the equivalent one-pass
# algorithm, so we do it in two passes: one pass that blurs in the horizontal
# direction, and one in the vertical direction.
def makeFilterBuffer(srcbuffer, name, sort, prog):
    blurBuffer = base.win.makeTextureBuffer(name, 512, 512)
    blurBuffer.setSort(sort)
    blurBuffer.setClearColor(LVector4(1, 0, 0, 1))
    blurCamera = base.makeCamera2d(blurBuffer)
    blurScene = NodePath("new Scene")
    blurCamera.node().setScene(blurScene)
    shader = loader.loadShader(prog)
    card = srcbuffer.getTextureCard()
    card.reparentTo(blurScene)
    card.setShader(shader)
    return blurBuffer


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

        # Post the instructions
        self.title = addTitle("Panda3D: Tutorial - Glow Filter")
        self.inst1 = addInstructions(0.06, "ESC: Quit")
        self.inst2 = addInstructions(0.12, "Space: Toggle Glow Filter On/Off")
        self.inst3 = addInstructions(0.18, "Enter: Toggle Running/Spinning")
        self.inst4 = addInstructions(0.24, "V: View the render-to-texture results")

        # Create the shader that will determime what parts of the scene will
        # glow
        glowShader = loader.loadShader("shaders/glowShader.sha")

        # load our model
        self.tron = Actor()
        self.tron.loadModel("models/tron")
        self.tron.loadAnims({"running": "models/tron_anim"})
        self.tron.reparentTo(render)
        self.interval = self.tron.hprInterval(60, LPoint3(360, 0, 0))
        self.interval.loop()
        self.isRunning = False

        # put some lighting on the tron model
        dlight = DirectionalLight('dlight')
        alight = AmbientLight('alight')
        dlnp = render.attachNewNode(dlight)
        alnp = render.attachNewNode(alight)
        dlight.setColor(LVector4(1.0, 0.7, 0.2, 1))
        alight.setColor(LVector4(0.2, 0.2, 0.2, 1))
        dlnp.setHpr(0, -60, 0)
        render.setLight(dlnp)
        render.setLight(alnp)

        # create the glow buffer. This buffer renders like a normal scene,
        # except that only the glowing materials should show up nonblack.
        glowBuffer = base.win.makeTextureBuffer("Glow scene", 512, 512)
        glowBuffer.setSort(-3)
        glowBuffer.setClearColor(LVector4(0, 0, 0, 1))

        # We have to attach a camera to the glow buffer. The glow camera
        # must have the same frustum as the main camera. As long as the aspect
        # ratios match, the rest will take care of itself.
        glowCamera = base.makeCamera(
            glowBuffer, lens=base.cam.node().getLens())

        # Tell the glow camera to use the glow shader
        tempnode = NodePath(PandaNode("temp node"))
        tempnode.setShader(glowShader)
        glowCamera.node().setInitialState(tempnode.getState())

        # set up the pipeline: from glow scene to blur x to blur y to main
        # window.
        blurXBuffer = makeFilterBuffer(
            glowBuffer,  "Blur X", -2, "shaders/XBlurShader.sha")
        blurYBuffer = makeFilterBuffer(
            blurXBuffer, "Blur Y", -1, "shaders/YBlurShader.sha")
        self.finalcard = blurYBuffer.getTextureCard()
        self.finalcard.reparentTo(render2d)

        # This attribute is used to add the results of the post-processing
        # effects to the existing framebuffer image, rather than replace it.
        # This is mainly useful for glow effects like ours.
        self.finalcard.setAttrib(ColorBlendAttrib.make(ColorBlendAttrib.MAdd))

        # Panda contains a built-in viewer that lets you view the results of
        # your render-to-texture operations.  This code configures the viewer.
        self.accept("v", base.bufferViewer.toggleEnable)
        self.accept("V", base.bufferViewer.toggleEnable)
        base.bufferViewer.setPosition("llcorner")
        base.bufferViewer.setLayout("hline")
        base.bufferViewer.setCardSize(0.652, 0)

        # event handling
        self.accept("space", self.toggleGlow)
        self.accept("enter", self.toggleDisplay)
        self.accept("escape", sys.exit, [0])

        self.glowOn = True

    def toggleGlow(self):
        if self.glowOn:
            self.finalcard.reparentTo(hidden)
        else:
            self.finalcard.reparentTo(render2d)
        self.glowOn = not(self.glowOn)

    def toggleDisplay(self):
        self.isRunning = not(self.isRunning)
        if not(self.isRunning):
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
