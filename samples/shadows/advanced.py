#!/usr/bin/env python

from panda3d.core import *
import sys
import os

from direct.showbase.ShowBase import ShowBase
from direct.interval.IntervalGlobal import *
from direct.gui.DirectGui import OnscreenText
from direct.showbase.DirectObject import DirectObject
from direct.actor import Actor
from random import *

# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(1, 1, 1, 1), scale=.05,
                        shadow=(0, 0, 0, 1), parent=base.a2dTopLeft,
                        pos=(0.08, -pos - 0.04), align=TextNode.ALeft)

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, fg=(1, 1, 1, 1), scale=.07,
                        parent=base.a2dBottomRight, align=TextNode.ARight,
                        pos=(-0.1, 0.09), shadow=(0, 0, 0, 1))


class World(DirectObject):

    def __init__(self):
        # Preliminary capabilities check.

        if not base.win.getGsg().getSupportsBasicShaders():
            self.t = addTitle(
                "Shadow Demo: Video driver reports that shaders are not supported.")
            return
        if not base.win.getGsg().getSupportsDepthTexture():
            self.t = addTitle(
                "Shadow Demo: Video driver reports that depth textures are not supported.")
            return

        # creating the offscreen buffer.

        winprops = WindowProperties(size=(512, 512))
        props = FrameBufferProperties()
        props.setRgbColor(1)
        props.setAlphaBits(1)
        props.setDepthBits(1)
        LBuffer = base.graphicsEngine.makeOutput(
            base.pipe, "offscreen buffer", -2,
            props, winprops,
            GraphicsPipe.BFRefuseWindow,
            base.win.getGsg(), base.win)
        self.buffer = LBuffer

        if not LBuffer:
            self.t = addTitle(
                "Shadow Demo: Video driver cannot create an offscreen buffer.")
            return

        Ldepthmap = Texture()
        LBuffer.addRenderTexture(Ldepthmap, GraphicsOutput.RTMBindOrCopy,
                                 GraphicsOutput.RTPDepthStencil)
        if base.win.getGsg().getSupportsShadowFilter():
            Ldepthmap.setMinfilter(Texture.FTShadow)
            Ldepthmap.setMagfilter(Texture.FTShadow)

        # Adding a color texture is totally unnecessary, but it helps with
        # debugging.
        Lcolormap = Texture()
        LBuffer.addRenderTexture(Lcolormap, GraphicsOutput.RTMBindOrCopy,
                                 GraphicsOutput.RTPColor)

        self.inst_p = addInstructions(0.06, 'P : stop/start the Panda Rotation')
        self.inst_w = addInstructions(0.12, 'W : stop/start the Walk Cycle')
        self.inst_t = addInstructions(0.18, 'T : stop/start the Teapot')
        self.inst_l = addInstructions(0.24, 'L : move light source far or close')
        self.inst_v = addInstructions(0.30, 'V : View the Depth-Texture results')
        self.inst_u = addInstructions(0.36, 'U : toggle updating the shadow map')
        self.inst_x = addInstructions(0.42, 'Left/Right Arrow : switch camera angles')
        self.inst_a = addInstructions(0.48, 'Something about A/Z and push bias')

        base.setBackgroundColor(0, 0, 0.2, 1)

        base.camLens.setNearFar(1.0, 10000)
        base.camLens.setFov(75)
        base.disableMouse()

        # Load the scene.

        floorTex = loader.loadTexture('maps/envir-ground.jpg')
        cm = CardMaker('')
        cm.setFrame(-2, 2, -2, 2)
        floor = render.attachNewNode(PandaNode("floor"))
        for y in range(12):
            for x in range(12):
                nn = floor.attachNewNode(cm.generate())
                nn.setP(-90)
                nn.setPos((x - 6) * 4, (y - 6) * 4, 0)
        floor.setTexture(floorTex)
        floor.flattenStrong()

        self.pandaAxis = render.attachNewNode('panda axis')
        self.pandaModel = Actor.Actor('panda-model', {'walk': 'panda-walk4'})
        self.pandaModel.reparentTo(self.pandaAxis)
        self.pandaModel.setPos(9, 0, 0)
        self.pandaModel.setShaderInput("scale", (0.01, 0.01, 0.01, 1.0))
        self.pandaWalk = self.pandaModel.actorInterval('walk', playRate=1.8)
        self.pandaWalk.loop()
        self.pandaMovement = self.pandaAxis.hprInterval(
            20.0, LPoint3(-360, 0, 0), startHpr=LPoint3(0, 0, 0))
        self.pandaMovement.loop()

        self.teapot = loader.loadModel('teapot')
        self.teapot.reparentTo(render)
        self.teapot.setPos(0, -20, 10)
        self.teapot.setShaderInput("texDisable", (1, 1, 1, 1))
        self.teapotMovement = self.teapot.hprInterval(50, LPoint3(0, 360, 360))
        self.teapotMovement.loop()

        self.accept('escape', sys.exit)

        self.accept("arrow_left", self.incrementCameraPosition, [-1])
        self.accept("arrow_right", self.incrementCameraPosition, [1])
        self.accept("p", self.toggleInterval, [self.pandaMovement])
        self.accept("t", self.toggleInterval, [self.teapotMovement])
        self.accept("w", self.toggleInterval, [self.pandaWalk])
        self.accept("v", base.bufferViewer.toggleEnable)
        self.accept("u", self.toggleUpdateShadowMap)
        self.accept("l", self.incrementLightPosition, [1])
        self.accept("o", base.oobe)
        self.accept('a', self.adjustPushBias, [1.1])
        self.accept('z', self.adjustPushBias, [0.9])

        self.LCam = base.makeCamera(LBuffer)
        self.LCam.node().setScene(render)
        self.LCam.node().getLens().setFov(40)
        self.LCam.node().getLens().setNearFar(10, 100)

        # default values
        self.pushBias = 0.04
        self.ambient = 0.2
        self.cameraSelection = 0
        self.lightSelection = 0

        # setting up shader
        render.setShaderInput('light', self.LCam)
        render.setShaderInput('Ldepthmap', Ldepthmap)
        render.setShaderInput('ambient', (self.ambient, 0, 0, 1.0))
        render.setShaderInput('texDisable', (0, 0, 0, 0))
        render.setShaderInput('scale', (1, 1, 1, 1))

        # Put a shader on the Light camera.
        lci = NodePath(PandaNode("Light Camera Initializer"))
        lci.setShader(loader.loadShader('caster.sha'))
        self.LCam.node().setInitialState(lci.getState())

        # Put a shader on the Main camera.
        # Some video cards have special hardware for shadow maps.
        # If the card has that, use it.  If not, use a different
        # shader that does not require hardware support.

        mci = NodePath(PandaNode("Main Camera Initializer"))
        if base.win.getGsg().getSupportsShadowFilter():
            mci.setShader(loader.loadShader('shadow.sha'))
        else:
            mci.setShader(loader.loadShader('shadow-nosupport.sha'))
        base.cam.node().setInitialState(mci.getState())

        self.incrementCameraPosition(0)
        self.incrementLightPosition(0)
        self.adjustPushBias(1.0)

    def toggleInterval(self, ival):
        if ival.isPlaying():
            ival.pause()
        else:
            ival.resume()

    def toggleUpdateShadowMap(self):
        self.buffer.active = not self.buffer.active

    def incrementCameraPosition(self, n):
        self.cameraSelection = (self.cameraSelection + n) % 6
        if (self.cameraSelection == 0):
            base.cam.reparentTo(render)
            base.cam.setPos(30, -45, 26)
            base.cam.lookAt(0, 0, 0)
            self.LCam.node().hideFrustum()
        if (self.cameraSelection == 1):
            base.cam.reparentTo(self.pandaModel)
            base.cam.setPos(7, -3, 9)
            base.cam.lookAt(0, 0, 0)
            self.LCam.node().hideFrustum()
        if (self.cameraSelection == 2):
            base.cam.reparentTo(self.pandaModel)
            base.cam.setPos(-7, -3, 9)
            base.cam.lookAt(0, 0, 0)
            self.LCam.node().hideFrustum()
        if (self.cameraSelection == 3):
            base.cam.reparentTo(render)
            base.cam.setPos(7, -23, 12)
            base.cam.lookAt(self.teapot)
            self.LCam.node().hideFrustum()
        if (self.cameraSelection == 4):
            base.cam.reparentTo(render)
            base.cam.setPos(-7, -23, 12)
            base.cam.lookAt(self.teapot)
            self.LCam.node().hideFrustum()
        if (self.cameraSelection == 5):
            base.cam.reparentTo(render)
            base.cam.setPos(1000, 0, 195)
            base.cam.lookAt(0, 0, 0)
            self.LCam.node().showFrustum()

    def incrementLightPosition(self, n):
        self.lightSelection = (self.lightSelection + n) % 2
        if (self.lightSelection == 0):
            self.LCam.setPos(0, -40, 25)
            self.LCam.lookAt(0, -10, 0)
            self.LCam.node().getLens().setNearFar(10, 100)
        if (self.lightSelection == 1):
            self.LCam.setPos(0, -600, 200)
            self.LCam.lookAt(0, -10, 0)
            self.LCam.node().getLens().setNearFar(10, 1000)

    def shaderSupported(self):
        return base.win.getGsg().getSupportsBasicShaders() and \
               base.win.getGsg().getSupportsDepthTexture() and \
               base.win.getGsg().getSupportsShadowFilter()

    def adjustPushBias(self, inc):
        self.pushBias *= inc
        self.inst_a.setText(
            'A/Z: Increase/Decrease the Push-Bias [%F]' % self.pushBias)
        render.setShaderInput('push', self.pushBias)

if __name__ == '__main__':
    base = ShowBase()
    w = World()
    base.run()
