#!/usr/bin/env python

from direct.showbase.ShowBase import ShowBase
from panda3d.core import PandaNode, LightNode, TextNode
from panda3d.core import Filename, NodePath
from panda3d.core import PointLight, AmbientLight
from panda3d.core import LightRampAttrib, AuxBitplaneAttrib
from panda3d.core import CardMaker
from panda3d.core import Shader, Texture
from direct.task.Task import Task
from direct.actor.Actor import Actor
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
from direct.showbase.BufferViewer import BufferViewer
from direct.filter.CommonFilters import CommonFilters
import sys
import os


# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(1, 1, 1, 1),
                        parent=base.a2dTopLeft, align=TextNode.ALeft,
                        pos=(0.08, -pos - 0.04), scale=.05)

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, pos=(-0.1, 0.09), scale=.08,
                        parent=base.a2dBottomRight, align=TextNode.ARight,
                        fg=(1, 1, 1, 1), shadow=(0, 0, 0, 1))


class ToonMaker(ShowBase):

    def __init__(self):
        # Initialize the ShowBase class from which we inherit, which will
        # create a window and set up everything we need for rendering into it.
        ShowBase.__init__(self)

        self.disableMouse()
        self.cam.node().getLens().setNear(10.0)
        self.cam.node().getLens().setFar(200.0)
        camera.setPos(0, -50, 0)

        # Check video card capabilities.
        if not self.win.getGsg().getSupportsBasicShaders():
            addTitle("Toon Shader: Video driver reports that Cg shaders are not supported.")
            return

        # Enable a 'light ramp' - this discretizes the lighting,
        # which is half of what makes a model look like a cartoon.
        # Light ramps only work if shader generation is enabled,
        # so we call 'setShaderAuto'.

        tempnode = NodePath(PandaNode("temp node"))
        tempnode.setAttrib(LightRampAttrib.makeSingleThreshold(0.5, 0.4))
        tempnode.setShaderAuto()
        self.cam.node().setInitialState(tempnode.getState())

        # Use class 'CommonFilters' to enable a cartoon inking filter.
        # This can fail if the video card is not powerful enough, if so,
        # display an error and exit.

        self.separation = 1  # Pixels
        self.filters = CommonFilters(self.win, self.cam)
        filterok = self.filters.setCartoonInk(separation=self.separation)
        if (filterok == False):
            addTitle(
                "Toon Shader: Video card not powerful enough to do image postprocessing")
            return

        # Show instructions in the corner of the window.
        self.title = addTitle(
            "Panda3D: Tutorial - Toon Shading with Normals-Based Inking")
        self.inst1 = addInstructions(0.06, "ESC: Quit")
        self.inst2 = addInstructions(0.12, "Up/Down: Increase/Decrease Line Thickness")
        self.inst3 = addInstructions(0.18, "V: View the render-to-texture results")

        # Load a dragon model and animate it.
        self.character = Actor()
        self.character.loadModel('models/nik-dragon')
        self.character.reparentTo(render)
        self.character.loadAnims({'win': 'models/nik-dragon'})
        self.character.loop('win')
        self.character.hprInterval(15, (360, 0, 0)).loop()

        # Create a non-attenuating point light and an ambient light.
        plightnode = PointLight("point light")
        plightnode.setAttenuation((1, 0, 0))
        plight = render.attachNewNode(plightnode)
        plight.setPos(30, -50, 0)
        alightnode = AmbientLight("ambient light")
        alightnode.setColor((0.8, 0.8, 0.8, 1))
        alight = render.attachNewNode(alightnode)
        render.setLight(alight)
        render.setLight(plight)

        # Panda contains a built-in viewer that lets you view the
        # results of all render-to-texture operations.  This lets you
        # see what class CommonFilters is doing behind the scenes.
        self.accept("v", self.bufferViewer.toggleEnable)
        self.accept("V", self.bufferViewer.toggleEnable)
        self.bufferViewer.setPosition("llcorner")
        self.accept("s", self.filters.manager.resizeBuffers)

        # These allow you to change cartooning parameters in realtime
        self.accept("escape", sys.exit, [0])
        self.accept("arrow_up", self.increaseSeparation)
        self.accept("arrow_down", self.decreaseSeparation)

    def increaseSeparation(self):
        self.separation = self.separation * 1.11111111
        print("separation: %f" % (self.separation))
        self.filters.setCartoonInk(separation=self.separation)

    def decreaseSeparation(self):
        self.separation = self.separation * 0.90000000
        print("separation: %f" % (self.separation))
        self.filters.setCartoonInk(separation=self.separation)

t = ToonMaker()
t.run()
