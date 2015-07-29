#!/usr/bin/env python

# Author: Tree Form starplant@gmail.com

from direct.showbase.ShowBase import ShowBase
from panda3d.core import FrameBufferProperties, TextNode, BitMask32, LPoint3
from panda3d.core import WindowProperties, GraphicsOutput, Texture, GraphicsPipe
from direct.showbase.DirectObject import DirectObject
from direct.gui.OnscreenText import OnscreenText
from sys import exit

# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(1, 1, 1, 1),
                        pos=(-1.25, pos), align=TextNode.ALeft, scale=.05)

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, fg=(1, 1, 1, 1), shadow=(0, 0, 0, 1),
                        pos=(1.25, -0.95), align=TextNode.ARight, scale=.07)


class DistortionDemo(ShowBase):
    def __init__(self):
        # Initialize the ShowBase class from which we inherit, which will
        # create a window and set up everything we need for rendering into it.
        ShowBase.__init__(self)

        if not base.win.getGsg().getSupportsBasicShaders():
            t = addTitle("Distortion Demo: Video driver says Cg shaders not supported.")
            return

        self.disableMouse()
        self.setBackgroundColor(0, 0, 0)

        # Show the instructions
        self.title = addTitle("Panda3D: Tutorial - Distortion Effect")
        self.inst1 = addInstructions(0.92, "ESC: Quit")
        self.inst2 = addInstructions(0.86, "Space: Toggle distortion filter On/Off")
        self.inst4 = addInstructions(0.80, "V: View the render-to-texture results")

        # Load background
        self.seascape = loader.loadModel("models/plane")
        self.seascape.reparentTo(render)
        self.seascape.setPosHpr(0, 145, 0, 0, 0, 0)
        self.seascape.setScale(100)
        self.seascape.setTexture(loader.loadTexture("models/ocean.jpg"))

        # Create the distortion buffer. This buffer renders like a normal
        # scene,
        self.distortionBuffer = self.makeFBO("model buffer")
        self.distortionBuffer.setSort(-3)
        self.distortionBuffer.setClearColor((0, 0, 0, 0))

        # We have to attach a camera to the distortion buffer. The distortion camera
        # must have the same frustum as the main camera. As long as the aspect
        # ratios match, the rest will take care of itself.
        distortionCamera = self.makeCamera(self.distortionBuffer, scene=render,
                                           lens=self.cam.node().getLens(), mask=BitMask32.bit(4))

        # load the object with the distortion
        self.distortionObject = loader.loadModel("models/boat")
        self.distortionObject.setScale(1)
        self.distortionObject.setPos(0, 20, -3)
        self.distortionObject.hprInterval(10, LPoint3(360, 0, 0)).loop()
        self.distortionObject.reparentTo(render)

        # Create the shader that will determime what parts of the scene will
        # distortion
        distortionShader = loader.loadShader("distortion.sha")
        self.distortionObject.setShader(distortionShader)
        self.distortionObject.hide(BitMask32.bit(4))

        # Textures
        tex1 = loader.loadTexture("models/water.png")
        self.distortionObject.setShaderInput("waves", tex1)

        self.texDistortion = Texture()
        self.distortionBuffer.addRenderTexture(
            self.texDistortion, GraphicsOutput.RTMBindOrCopy, GraphicsOutput.RTPColor)
        self.distortionObject.setShaderInput("screen", self.texDistortion)

        # Panda contains a built-in viewer that lets you view the results of
        # your render-to-texture operations.  This code configures the viewer.
        self.accept("v", self.bufferViewer.toggleEnable)
        self.accept("V", self.bufferViewer.toggleEnable)
        self.bufferViewer.setPosition("llcorner")
        self.bufferViewer.setLayout("hline")
        self.bufferViewer.setCardSize(0.652, 0)

        # event handling
        self.accept("space", self.toggleDistortion)
        self.accept("escape", exit, [0])
        self.distortionOn = True

    def makeFBO(self, name):
        # This routine creates an offscreen buffer.  All the complicated
        # parameters are basically demanding capabilities from the offscreen
        # buffer - we demand that it be able to render to texture on every
        # bitplane, that it can support aux bitplanes, that it track
        # the size of the host window, that it can render to texture
        # cumulatively, and so forth.
        winprops = WindowProperties()
        props = FrameBufferProperties()
        props.setRgbColor(1)
        return self.graphicsEngine.makeOutput(
            self.pipe, "model buffer", -2, props, winprops,
            GraphicsPipe.BFSizeTrackHost | GraphicsPipe.BFRefuseWindow,
            self.win.getGsg(), self.win)

    def toggleDistortion(self):
        # Toggles the distortion on/off.
        if self.distortionOn:
            self.distortionObject.hide()
        else:
            self.distortionObject.show()
        self.distortionOn = not(self.distortionOn)

demo = DistortionDemo()
demo.run()
