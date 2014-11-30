#Author: Josh Yelon
#Date: 7/11/2005
#
# See the associated HTML file for an explanation.

import direct.directbase.DirectStart
from panda3d.core import FrameBufferProperties, WindowProperties
from panda3d.core import GraphicsPipe,GraphicsOutput
from panda3d.core import Filename,Texture,Shader
from panda3d.core import RenderState, CardMaker
from panda3d.core import PandaNode,TextNode,NodePath
from panda3d.core import RenderAttrib,AlphaTestAttrib,ColorBlendAttrib
from panda3d.core import CullFaceAttrib,DepthTestAttrib,DepthWriteAttrib
from panda3d.core import Point3,Vec3,Vec4,BitMask32
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
from direct.interval.MetaInterval import Sequence
from direct.task.Task import Task
from direct.actor.Actor import Actor
import sys,os,random

# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(1,1,1,1),
                        pos=(-1.3, pos), align=TextNode.ALeft, scale = .05)

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, fg=(1,1,1,1),
                        pos=(1.3,-0.95), align=TextNode.ARight, scale = .07)


class FireflyDemo(DirectObject):
    def __init__(self):

        # Preliminary capabilities check.

        if (base.win.getGsg().getSupportsBasicShaders()==0):
            self.t=addTitle("Firefly Demo: Video driver reports that shaders are not supported.")
            return
        if (base.win.getGsg().getSupportsDepthTexture()==0):
            self.t=addTitle("Firefly Demo: Video driver reports that depth textures are not supported.")
            return
        
        # This algorithm uses two offscreen buffers, one of which has
        # an auxiliary bitplane, and the offscreen buffers share a single
        # depth buffer.  This is a heck of a complicated buffer setup.

        self.modelbuffer = self.makeFBO("model buffer",1)
        self.lightbuffer = self.makeFBO("light buffer",0)
        
        # Creation of a high-powered buffer can fail, if the graphics card
        # doesn't support the necessary OpenGL extensions.

        if (self.modelbuffer == None) or (self.lightbuffer == None):
            self.t=addTitle("Toon Shader: Video driver does not support multiple render targets")
            return

        # Create four render textures: depth, normal, albedo, and final.
        # attach them to the various bitplanes of the offscreen buffers.

        self.texDepth = Texture()
        self.texDepth.setFormat(Texture.FDepthStencil)
        self.texAlbedo = Texture()
        self.texNormal = Texture()
        self.texFinal = Texture()

        self.modelbuffer.addRenderTexture(self.texDepth,  GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPDepthStencil)
        self.modelbuffer.addRenderTexture(self.texAlbedo, GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPColor)
        self.modelbuffer.addRenderTexture(self.texNormal, GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPAuxRgba0)

        self.lightbuffer.addRenderTexture(self.texFinal,  GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPColor)

        # Set the near and far clipping planes.
        
        base.cam.node().getLens().setNear(50.0)
        base.cam.node().getLens().setFar(500.0)
        lens = base.cam.node().getLens()

        # This algorithm uses three cameras: one to render the models into the
        # model buffer, one to render the lights into the light buffer, and
        # one to render "plain" stuff (non-deferred shaded) stuff into the light
        # buffer.  Each camera has a bitmask to identify it.

        self.modelMask = 1
        self.lightMask = 2
        self.plainMask = 4
        
        self.modelcam=base.makeCamera(self.modelbuffer, lens=lens, scene=render, mask=self.modelMask)
        self.lightcam=base.makeCamera(self.lightbuffer, lens=lens, scene=render, mask=self.lightMask)
        self.plaincam=base.makeCamera(self.lightbuffer, lens=lens, scene=render, mask=self.plainMask)

        # Panda's main camera is not used.
        
        base.cam.node().setActive(0)
        
        # Take explicit control over the order in which the three
        # buffers are rendered.

        self.modelbuffer.setSort(1)
        self.lightbuffer.setSort(2)
        base.win.setSort(3)

        # Within the light buffer, control the order of the two cams.

        self.lightcam.node().getDisplayRegion(0).setSort(1)
        self.plaincam.node().getDisplayRegion(0).setSort(2)

        # By default, panda usually clears the screen before every
        # camera and before every window.  Tell it not to do that.
        # Then, tell it specifically when to clear and what to clear.

        self.modelcam.node().getDisplayRegion(0).disableClears()
        self.lightcam.node().getDisplayRegion(0).disableClears()
        self.plaincam.node().getDisplayRegion(0).disableClears()
        base.cam.node().getDisplayRegion(0).disableClears()
        base.cam2d.node().getDisplayRegion(0).disableClears()
        self.modelbuffer.disableClears()
        base.win.disableClears()

        self.modelbuffer.setClearColorActive(1)
        self.modelbuffer.setClearDepthActive(1)
        self.lightbuffer.setClearColorActive(1)
        self.lightbuffer.setClearColor(Vec4(0,0,0,1))

        # Miscellaneous stuff.
        
        base.disableMouse()
        base.camera.setPos(-9.112,-211.077,46.951)
        base.camera.setHpr(0, -7.5, 2.4)
        base.setBackgroundColor(Vec4(0,0,0,0))
        random.seed()

        # Calculate the projection parameters for the final shader.
        # The math here is too complex to explain in an inline comment,
        # I've put in a full explanation into the HTML intro.

        proj = base.cam.node().getLens().getProjectionMat()
        proj_x = 0.5 * proj.getCell(3,2) / proj.getCell(0,0)
        proj_y = 0.5 * proj.getCell(3,2)
        proj_z = 0.5 * proj.getCell(3,2) / proj.getCell(2,1)
        proj_w = -0.5 - 0.5*proj.getCell(1,2)
        
        # Configure the render state of the model camera.
        
        tempnode = NodePath(PandaNode("temp node"))
        tempnode.setAttrib(AlphaTestAttrib.make(RenderAttrib.MGreaterEqual, 0.5))
        tempnode.setShader(loader.loadShader("model.sha"))
        tempnode.setAttrib(DepthTestAttrib.make(RenderAttrib.MLessEqual))
        self.modelcam.node().setInitialState(tempnode.getState())
        
        # Configure the render state of the light camera.

        tempnode = NodePath(PandaNode("temp node"))
        tempnode.setShader(loader.loadShader("light.sha"))
        tempnode.setShaderInput("texnormal",self.texNormal)
        tempnode.setShaderInput("texalbedo",self.texAlbedo)
        tempnode.setShaderInput("texdepth",self.texDepth)
        tempnode.setShaderInput("proj",Vec4(proj_x,proj_y,proj_z,proj_w))
        tempnode.setAttrib(ColorBlendAttrib.make(ColorBlendAttrib.MAdd, ColorBlendAttrib.OOne, ColorBlendAttrib.OOne))
        tempnode.setAttrib(CullFaceAttrib.make(CullFaceAttrib.MCullCounterClockwise))
        # The next line causes problems on Linux.
        #tempnode.setAttrib(DepthTestAttrib.make(RenderAttrib.MGreaterEqual))
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
        render.setTexture(white,0)

        # Create two subroots, to help speed cull traversal.
        
        self.lightroot = NodePath(PandaNode("lightroot"))
        self.lightroot.reparentTo(render)
        self.modelroot = NodePath(PandaNode("modelroot"))
        self.modelroot.reparentTo(render) 
        self.lightroot.hide(BitMask32(self.modelMask))
        self.modelroot.hide(BitMask32(self.lightMask))
        self.modelroot.hide(BitMask32(self.plainMask))

        # Load the model of a forest. Make it visible to the model camera.

        self.forest=NodePath(PandaNode("Forest Root"))
        self.forest.reparentTo(render)
        loader.loadModel("models/background").reparentTo(self.forest)
        loader.loadModel("models/foliage01").reparentTo(self.forest)
        loader.loadModel("models/foliage02").reparentTo(self.forest)
        loader.loadModel("models/foliage03").reparentTo(self.forest)
        loader.loadModel("models/foliage04").reparentTo(self.forest)
        loader.loadModel("models/foliage05").reparentTo(self.forest)
        loader.loadModel("models/foliage06").reparentTo(self.forest)
        loader.loadModel("models/foliage07").reparentTo(self.forest)
        loader.loadModel("models/foliage08").reparentTo(self.forest)
        loader.loadModel("models/foliage09").reparentTo(self.forest)
        self.forest.hide(BitMask32(self.lightMask | self.plainMask))

        # Cause the final results to be rendered into the main window on a card.
        
        self.card = self.lightbuffer.getTextureCard()
        self.card.setTexture(self.texFinal)
        self.card.reparentTo(render2d)
        
        # Post the instructions.

        self.title = addTitle("Panda3D: Tutorial - Fireflies using Deferred Shading")
        self.inst1 = addInstructions(0.95,"ESC: Quit")
        self.inst2 = addInstructions(0.90,"Up/Down: More / Fewer Fireflies (Count: unknown)")
        self.inst3 = addInstructions(0.85,"Right/Left: Bigger / Smaller Fireflies (Radius: unknown)")
        self.inst4 = addInstructions(0.80,"V: View the render-to-texture results")

        # Panda contains a built-in viewer that lets you view the results of
        # your render-to-texture operations.  This code configures the viewer.

        base.bufferViewer.setPosition("llcorner")
        base.bufferViewer.setCardSize(0,0.40)
        base.bufferViewer.setLayout("vline")
        self.toggleCards()
        self.toggleCards()

        # Firefly parameters

        self.fireflies = []
        self.sequences = []
        self.scaleseqs = []
        self.glowspheres = []
        self.fireflysize = 1.0
        self.spheremodel = loader.loadModel("misc/sphere")
        self.setFireflySize(25.0)
        while (len(self.fireflies)<5): self.addFirefly()
        self.updateReadout()

        # these allow you to change parameters in realtime

        self.accept("escape", sys.exit, [0])
        self.accept("arrow_up",   self.incFireflyCount, [1.1111111])
        self.accept("arrow_down", self.decFireflyCount, [0.9000000])
        self.accept("arrow_right", self.setFireflySize, [1.1111111])
        self.accept("arrow_left",  self.setFireflySize, [0.9000000])
        self.accept("v", self.toggleCards)
        self.accept("V", self.toggleCards)

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
        props.setRgbColor(1)
        props.setAlphaBits(1)
        props.setDepthBits(1)
        props.setAuxRgba(auxrgba)
        return base.graphicsEngine.makeOutput(
             base.pipe, "model buffer", -2,
             props, winprops,
             GraphicsPipe.BFSizeTrackHost | GraphicsPipe.BFCanBindEvery | 
             GraphicsPipe.BFRttCumulative | GraphicsPipe.BFRefuseWindow,
             base.win.getGsg(), base.win)

    def addFirefly(self):
        pos1 = Point3(random.uniform(-50, 50), random.uniform(-100, 150), random.uniform(-10,80))
        dir = Vec3(random.uniform(-1, 1), random.uniform(-1, 1), random.uniform(-1, 1))
        dir.normalize()
        pos2 = pos1 + (dir*20)
        fly = self.lightroot.attachNewNode(PandaNode("fly"))
        glow = fly.attachNewNode(PandaNode("glow"))
        dot  = fly.attachNewNode(PandaNode("dot"))
        color_r = random.uniform(0.7,1.0)
        color_g = 1.0
        color_b = 0.8
        fly.setShaderInput("lightcolor", color_r, color_g, color_b, 1.0)
        int1 = fly.posInterval(random.uniform(7,12), pos1, pos2)
        int2 = fly.posInterval(random.uniform(7,12), pos2, pos1)
        si1 = fly.scaleInterval(random.uniform(0.8,1.5), Point3(0.2,0.2,0.2), Point3(0.2,0.2,0.2))
        si2 = fly.scaleInterval(random.uniform(1.5,0.8), Point3(1.0,1.0,1.0), Point3(0.2,0.2,0.2))
        si3 = fly.scaleInterval(random.uniform(1.0,2.0), Point3(0.2,0.2,0.2), Point3(1.0,1.0,1.0))
        siseq = Sequence(si1, si2, si3)
        siseq.loop()
        siseq.setT(random.uniform(0,1000))
        seq = Sequence(int1, int2)
        seq.loop()
        self.spheremodel.instanceTo(glow)
        self.spheremodel.instanceTo(dot)
        glow.setScale(self.fireflysize*1.1)
        glow.hide(BitMask32(self.modelMask | self.plainMask))
        dot.setScale(0.6)
        dot.hide(BitMask32(self.modelMask | self.lightMask))
        dot.setColor(color_r, color_g, color_b, 1.0)
        self.fireflies.append(fly)
        self.sequences.append(seq)
        self.glowspheres.append(glow)
        self.scaleseqs.append(siseq)

    def updateReadout(self):
        self.inst2.destroy()
        self.inst2 = addInstructions(0.90,"Up/Down: More / Fewer Fireflies (Currently: "+str(len(self.fireflies))+")")
        self.inst3.destroy()
        self.inst3 = addInstructions(0.85,"Right/Left: Bigger / Smaller Fireflies (Radius: "+str(self.fireflysize)+" ft)")

    def toggleCards(self):
        base.bufferViewer.toggleEnable()
        # When the cards are not visible, I also disable the color clear.
        # This color-clear is actually not necessary, the depth-clear is
        # sufficient for the purposes of the algorithm.
        if (base.bufferViewer.isEnabled()):
            self.modelbuffer.setClearColorActive(1)
        else:
            self.modelbuffer.setClearColorActive(0)

    def incFireflyCount(self, scale):
        n = int((len(self.fireflies) * scale) + 1)
        while (n > len(self.fireflies)):
            self.addFirefly()
        self.updateReadout()

    def decFireflyCount(self, scale):
        n = int(len(self.fireflies) * scale)
        if (n < 1): n=1
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

t=FireflyDemo()

run()


