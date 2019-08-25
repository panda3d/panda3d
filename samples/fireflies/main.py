#!/usr/bin/env python

# Author: Josh Yelon
# Date: 7/11/2005
#
# See the associated manual page for an explanation.
#
from direct.showbase.ShowBase import ShowBase
from panda3d.core import FrameBufferProperties, WindowProperties
from panda3d.core import GraphicsPipe, GraphicsOutput
from panda3d.core import Filename, Texture, Shader
from panda3d.core import RenderState, CardMaker
from panda3d.core import PandaNode, TextNode, NodePath
from panda3d.core import RenderAttrib, AlphaTestAttrib, ColorBlendAttrib
from panda3d.core import CullFaceAttrib, DepthTestAttrib, DepthWriteAttrib
from panda3d.core import LPoint3, LVector3, BitMask32
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
from direct.interval.MetaInterval import Sequence
from direct.task.Task import Task
from direct.actor.Actor import Actor
import sys
import os
import random

# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(1, 1, 1, 1), shadow=(0, 0, 0, 1),
                        parent=base.a2dTopLeft, align=TextNode.ALeft,
                        pos=(0.08, -pos - 0.04), scale=.05)

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, pos=(-0.1, 0.09), scale=.08,
                        parent=base.a2dBottomRight, align=TextNode.ARight,
                        fg=(1, 1, 1, 1), shadow=(0, 0, 0, 1))


class FireflyDemo(ShowBase):
    def __init__(self):
        # Initialize the ShowBase class from which we inherit, which will
        # create a window and set up everything we need for rendering into it.
        ShowBase.__init__(self)
        self.setBackgroundColor((0, 0, 0, 0))

        # Preliminary capabilities check.

        if not self.win.getGsg().getSupportsBasicShaders():
            self.t = addTitle("Firefly Demo: Video driver reports that Cg "
                              "shaders are not supported.")
            return
        if not self.win.getGsg().getSupportsDepthTexture():
            self.t = addTitle("Firefly Demo: Video driver reports that depth "
                              "textures are not supported.")
            return

        # This algorithm uses two offscreen buffers, one of which has
        # an auxiliary bitplane, and the offscreen buffers share a single
        # depth buffer.  This is a heck of a complicated buffer setup.

        self.modelbuffer = self.makeFBO("model buffer", 1)
        self.lightbuffer = self.makeFBO("light buffer", 0)

        # Creation of a high-powered buffer can fail, if the graphics card
        # doesn't support the necessary OpenGL extensions.

        if self.modelbuffer is None or self.lightbuffer is None:
            self.t = addTitle("Firefly Demo: Video driver does not support "
                              "multiple render targets")
            return

        # Create four render textures: depth, normal, albedo, and final.
        # attach them to the various bitplanes of the offscreen buffers.

        self.texDepth = Texture()
        self.texDepth.setFormat(Texture.FDepthStencil)
        self.texAlbedo = Texture()
        self.texNormal = Texture()
        self.texFinal = Texture()

        self.modelbuffer.addRenderTexture(self.texDepth,
            GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPDepthStencil)
        self.modelbuffer.addRenderTexture(self.texAlbedo,
            GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPColor)
        self.modelbuffer.addRenderTexture(self.texNormal,
            GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPAuxRgba0)

        self.lightbuffer.addRenderTexture(self.texFinal,
            GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPColor)

        # Set the near and far clipping planes.

        self.cam.node().getLens().setNear(50.0)
        self.cam.node().getLens().setFar(500.0)
        lens = self.cam.node().getLens()

        # This algorithm uses three cameras: one to render the models into the
        # model buffer, one to render the lights into the light buffer, and
        # one to render "plain" stuff (non-deferred shaded) stuff into the
        # light buffer.  Each camera has a bitmask to identify it.

        self.modelMask = 1
        self.lightMask = 2
        self.plainMask = 4

        self.modelcam = self.makeCamera(self.modelbuffer,
            lens=lens, scene=render, mask=self.modelMask)
        self.lightcam = self.makeCamera(self.lightbuffer,
            lens=lens, scene=render, mask=self.lightMask)
        self.plaincam = self.makeCamera(self.lightbuffer,
            lens=lens, scene=render, mask=self.plainMask)

        # Panda's main camera is not used.

        self.cam.node().setActive(0)

        # Take explicit control over the order in which the three
        # buffers are rendered.

        self.modelbuffer.setSort(1)
        self.lightbuffer.setSort(2)
        self.win.setSort(3)

        # Within the light buffer, control the order of the two cams.

        self.lightcam.node().getDisplayRegion(0).setSort(1)
        self.plaincam.node().getDisplayRegion(0).setSort(2)

        # By default, panda usually clears the screen before every
        # camera and before every window.  Tell it not to do that.
        # Then, tell it specifically when to clear and what to clear.

        self.modelcam.node().getDisplayRegion(0).disableClears()
        self.lightcam.node().getDisplayRegion(0).disableClears()
        self.plaincam.node().getDisplayRegion(0).disableClears()
        self.cam.node().getDisplayRegion(0).disableClears()
        self.cam2d.node().getDisplayRegion(0).disableClears()
        self.modelbuffer.disableClears()
        self.win.disableClears()

        self.modelbuffer.setClearColorActive(1)
        self.modelbuffer.setClearDepthActive(1)
        self.lightbuffer.setClearColorActive(1)
        self.lightbuffer.setClearColor((0, 0, 0, 1))

        # Miscellaneous stuff.

        self.disableMouse()
        self.camera.setPos(-9.112, -211.077, 46.951)
        self.camera.setHpr(0, -7.5, 2.4)
        random.seed()

        # Calculate the projection parameters for the final shader.
        # The math here is too complex to explain in an inline comment,
        # I've put in a full explanation into the HTML intro.

        proj = self.cam.node().getLens().getProjectionMat()
        proj_x = 0.5 * proj.getCell(3, 2) / proj.getCell(0, 0)
        proj_y = 0.5 * proj.getCell(3, 2)
        proj_z = 0.5 * proj.getCell(3, 2) / proj.getCell(2, 1)
        proj_w = -0.5 - 0.5 * proj.getCell(1, 2)

        # Configure the render state of the model camera.

        tempnode = NodePath(PandaNode("temp node"))
        tempnode.setAttrib(
            AlphaTestAttrib.make(RenderAttrib.MGreaterEqual, 0.5))
        tempnode.setShader(loader.loadShader("model.sha"))
        tempnode.setAttrib(DepthTestAttrib.make(RenderAttrib.MLessEqual))
        self.modelcam.node().setInitialState(tempnode.getState())

        # Configure the render state of the light camera.

        tempnode = NodePath(PandaNode("temp node"))
        tempnode.setShader(loader.loadShader("light.sha"))
        tempnode.setShaderInput("texnormal", self.texNormal)
        tempnode.setShaderInput("texalbedo", self.texAlbedo)
        tempnode.setShaderInput("texdepth", self.texDepth)
        tempnode.setShaderInput("proj", (proj_x, proj_y, proj_z, proj_w))
        tempnode.setAttrib(ColorBlendAttrib.make(ColorBlendAttrib.MAdd,
            ColorBlendAttrib.OOne, ColorBlendAttrib.OOne))
        tempnode.setAttrib(
            CullFaceAttrib.make(CullFaceAttrib.MCullCounterClockwise))
        # The next line causes problems on Linux.
        # tempnode.setAttrib(DepthTestAttrib.make(RenderAttrib.MGreaterEqual))
        tempnode.setAttrib(DepthWriteAttrib.make(DepthWriteAttrib.MOff))
        self.lightcam.node().setInitialState(tempnode.getState())

        # Configure the render state of the plain camera.

        rs = RenderState.makeEmpty()
        self.plaincam.node().setInitialState(rs)

        # Clear any render attribs on the root node. This is necessary
        # because by default, panda assigns some attribs to the root
        # node.  These default attribs will override the
        # carefully-configured render attribs that we just attached
        # to the cameras.  The simplest solution is to just clear
        # them all out.

        render.setState(RenderState.makeEmpty())

        # My artist created a model in which some of the polygons
        # don't have textures.  This confuses the shader I wrote.
        # This little hack guarantees that everything has a texture.

        white = loader.loadTexture("models/white.jpg")
        render.setTexture(white, 0)

        # Create two subroots, to help speed cull traversal.

        self.lightroot = NodePath(PandaNode("lightroot"))
        self.lightroot.reparentTo(render)
        self.modelroot = NodePath(PandaNode("modelroot"))
        self.modelroot.reparentTo(render)
        self.lightroot.hide(BitMask32(self.modelMask))
        self.modelroot.hide(BitMask32(self.lightMask))
        self.modelroot.hide(BitMask32(self.plainMask))

        # Load the model of a forest.  Make it visible to the model camera.
        # This is a big model, so we load it asynchronously while showing a
        # load text.  We do this by passing in a callback function.
        self.loading = addTitle("Loading models...")

        self.forest = NodePath(PandaNode("Forest Root"))
        self.forest.reparentTo(render)
        self.forest.hide(BitMask32(self.lightMask | self.plainMask))
        loader.loadModel([
            "models/background",
            "models/foliage01",
            "models/foliage02",
            "models/foliage03",
            "models/foliage04",
            "models/foliage05",
            "models/foliage06",
            "models/foliage07",
            "models/foliage08",
            "models/foliage09"],
            callback=self.finishLoading)

        # Cause the final results to be rendered into the main window on a
        # card.

        self.card = self.lightbuffer.getTextureCard()
        self.card.setTexture(self.texFinal)
        self.card.reparentTo(render2d)

        # Panda contains a built-in viewer that lets you view the results of
        # your render-to-texture operations.  This code configures the viewer.

        self.bufferViewer.setPosition("llcorner")
        self.bufferViewer.setCardSize(0, 0.40)
        self.bufferViewer.setLayout("vline")
        self.toggleCards()
        self.toggleCards()

        # Firefly parameters

        self.fireflies = []
        self.sequences = []
        self.scaleseqs = []
        self.glowspheres = []
        self.fireflysize = 1.0
        self.spheremodel = loader.loadModel("misc/sphere")

        # Create the firefly model, a fuzzy dot
        dotSize = 1.0
        cm = CardMaker("firefly")
        cm.setFrame(-dotSize, dotSize, -dotSize, dotSize)
        self.firefly = NodePath(cm.generate())
        self.firefly.setTexture(loader.loadTexture("models/firefly.png"))
        self.firefly.setAttrib(ColorBlendAttrib.make(ColorBlendAttrib.M_add,
            ColorBlendAttrib.O_incoming_alpha, ColorBlendAttrib.O_one))

        # these allow you to change parameters in realtime

        self.accept("escape", sys.exit, [0])
        self.accept("arrow_up",   self.incFireflyCount, [1.1111111])
        self.accept("arrow_down", self.decFireflyCount, [0.9000000])
        self.accept("arrow_right", self.setFireflySize, [1.1111111])
        self.accept("arrow_left",  self.setFireflySize, [0.9000000])
        self.accept("v", self.toggleCards)
        self.accept("V", self.toggleCards)

    def finishLoading(self, models):
        # This function is used as callback to loader.loadModel, and called
        # when all of the models have finished loading.

        # Attach the models to the scene graph.
        for model in models:
            model.reparentTo(self.forest)

        # Show the instructions.
        self.loading.destroy()
        self.title = addTitle("Panda3D: Tutorial - Fireflies using Deferred Shading")
        self.inst1 = addInstructions(0.06, "ESC: Quit")
        self.inst2 = addInstructions(0.12, "Up/Down: More / Fewer Fireflies (Count: unknown)")
        self.inst3 = addInstructions(0.18, "Right/Left: Bigger / Smaller Fireflies (Radius: unknown)")
        self.inst4 = addInstructions(0.24, "V: View the render-to-texture results")

        self.setFireflySize(25.0)
        while len(self.fireflies) < 5:
            self.addFirefly()
        self.updateReadout()

        self.nextadd = 0
        taskMgr.add(self.spawnTask, "spawner")

    def makeFBO(self, name, auxrgba):
        # This routine creates an offscreen buffer.  All the complicated
        # parameters are basically demanding capabilities from the offscreen
        # buffer - we demand that it be able to render to texture on every
        # bitplane, that it can support aux bitplanes, that it track
        # the size of the host window, that it can render to texture
        # cumulatively, and so forth.
        winprops = WindowProperties()
        props = FrameBufferProperties()
        props.setRgbColor(True)
        props.setRgbaBits(8, 8, 8, 8)
        props.setDepthBits(1)
        props.setAuxRgba(auxrgba)
        return self.graphicsEngine.makeOutput(
            self.pipe, "model buffer", -2,
            props, winprops,
            GraphicsPipe.BFSizeTrackHost | GraphicsPipe.BFCanBindEvery |
            GraphicsPipe.BFRttCumulative | GraphicsPipe.BFRefuseWindow,
            self.win.getGsg(), self.win)

    def addFirefly(self):
        pos1 = LPoint3(random.uniform(-50, 50), random.uniform(-100, 150), random.uniform(-10, 80))
        dir = LVector3(random.uniform(-1, 1), random.uniform(-1, 1), random.uniform(-1, 1))
        dir.normalize()
        pos2 = pos1 + (dir * 20)
        fly = self.lightroot.attachNewNode(PandaNode("fly"))
        glow = fly.attachNewNode(PandaNode("glow"))
        dot = fly.attachNewNode(PandaNode("dot"))
        color_r = 1.0
        color_g = random.uniform(0.8, 1.0)
        color_b = min(color_g, random.uniform(0.5, 1.0))
        fly.setColor(color_r, color_g, color_b, 1.0)
        fly.setShaderInput("lightcolor", (color_r, color_g, color_b, 1.0))
        int1 = fly.posInterval(random.uniform(7, 12), pos1, pos2)
        int2 = fly.posInterval(random.uniform(7, 12), pos2, pos1)
        si1 = fly.scaleInterval(random.uniform(0.8, 1.5),
            LPoint3(0.2, 0.2, 0.2), LPoint3(0.2, 0.2, 0.2))
        si2 = fly.scaleInterval(random.uniform(1.5, 0.8),
            LPoint3(1.0, 1.0, 1.0), LPoint3(0.2, 0.2, 0.2))
        si3 = fly.scaleInterval(random.uniform(1.0, 2.0),
            LPoint3(0.2, 0.2, 0.2), LPoint3(1.0, 1.0, 1.0))
        siseq = Sequence(si1, si2, si3)
        siseq.loop()
        siseq.setT(random.uniform(0, 1000))
        seq = Sequence(int1, int2)
        seq.loop()
        self.spheremodel.instanceTo(glow)
        self.firefly.instanceTo(dot)
        glow.setScale(self.fireflysize * 1.1)
        glow.hide(BitMask32(self.modelMask | self.plainMask))
        dot.hide(BitMask32(self.modelMask | self.lightMask))
        dot.setColor(color_r, color_g, color_b, 1.0)
        self.fireflies.append(fly)
        self.sequences.append(seq)
        self.glowspheres.append(glow)
        self.scaleseqs.append(siseq)

    def updateReadout(self):
        self.inst2.destroy()
        self.inst2 = addInstructions(0.12,
            "Up/Down: More / Fewer Fireflies (Currently: %d)" % len(self.fireflies))
        self.inst3.destroy()
        self.inst3 = addInstructions(0.18,
            "Right/Left: Bigger / Smaller Fireflies (Radius: %d ft)" % self.fireflysize)

    def toggleCards(self):
        self.bufferViewer.toggleEnable()
        # When the cards are not visible, I also disable the color clear.
        # This color-clear is actually not necessary, the depth-clear is
        # sufficient for the purposes of the algorithm.
        if (self.bufferViewer.isEnabled()):
            self.modelbuffer.setClearColorActive(True)
        else:
            self.modelbuffer.setClearColorActive(False)

    def incFireflyCount(self, scale):
        n = int((len(self.fireflies) * scale) + 1)
        while (n > len(self.fireflies)):
            self.addFirefly()
        self.updateReadout()

    def decFireflyCount(self, scale):
        n = int(len(self.fireflies) * scale)
        if (n < 1):
            n = 1
        while (len(self.fireflies) > n):
            self.glowspheres.pop()
            self.sequences.pop().finish()
            self.scaleseqs.pop().finish()
            self.fireflies.pop().removeNode()
        self.updateReadout()

    def setFireflySize(self, n):
        n = n * self.fireflysize
        self.fireflysize = n
        for x in self.glowspheres:
            x.setScale(self.fireflysize * 1.1)
        self.updateReadout()

    def spawnTask(self, task):
        if task.time > self.nextadd:
            self.nextadd = task.time + 1.0
            if (len(self.fireflies) < 300):
                self.incFireflyCount(1.03)
        return Task.cont

demo = FireflyDemo()
demo.run()
