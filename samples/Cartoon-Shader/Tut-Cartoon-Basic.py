import direct.directbase.DirectStart
from panda3d.core import PandaNode,LightNode,TextNode
from panda3d.core import Filename, NodePath
from panda3d.core import PointLight, AmbientLight
from panda3d.core import LightRampAttrib, AuxBitplaneAttrib
from panda3d.core import CardMaker
from panda3d.core import Shader, Texture
from panda3d.core import Point3,Vec4,Vec3
from direct.task.Task import Task
from direct.actor.Actor import Actor
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
from direct.showbase.BufferViewer import BufferViewer
from direct.filter.CommonFilters import CommonFilters
import sys,os

# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(1,1,1,1),
                        pos=(-1.3, pos), align=TextNode.ALeft, scale = .05)

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, fg=(1,1,1,1),
                        pos=(1.3,-0.95), align=TextNode.ARight, scale = .07)

class ToonMaker(DirectObject):
    def __init__(self):
        base.disableMouse()
        base.cam.node().getLens().setNear(10.0)
        base.cam.node().getLens().setFar(200.0)
        camera.setPos(0, -50, 0)
        
        # Check video card capabilities.
        
        if (base.win.getGsg().getSupportsBasicShaders() == 0):
            addTitle("Toon Shader: Video driver reports that shaders are not supported.")
            return
        
        # Enable a 'light ramp' - this discretizes the lighting,
        # which is half of what makes a model look like a cartoon.
        # Light ramps only work if shader generation is enabled,
        # so we call 'setShaderAuto'.

        tempnode = NodePath(PandaNode("temp node"))
        tempnode.setAttrib(LightRampAttrib.makeSingleThreshold(0.5, 0.4))
        tempnode.setShaderAuto()
        base.cam.node().setInitialState(tempnode.getState())
        
        # Use class 'CommonFilters' to enable a cartoon inking filter.
        # This can fail if the video card is not powerful enough, if so,
        # display an error and exit.
        
        self.separation = 1 # Pixels
        self.filters = CommonFilters(base.win, base.cam)
        filterok = self.filters.setCartoonInk(separation=self.separation)
        if (filterok == False):
            addTitle("Toon Shader: Video card not powerful enough to do image postprocessing")
            return
        
        # Post the instructions.
        
        self.title = addTitle("Panda3D: Tutorial - Toon Shading with Normals-Based Inking")
        self.inst1 = addInstructions(0.95,"ESC: Quit")
        self.inst2 = addInstructions(0.90,"Up/Down: Increase/Decrease Line Thickness")
        self.inst3 = addInstructions(0.85,"V: View the render-to-texture results")
        
        # Load a dragon model and animate it.
        
        self.character = Actor()
        self.character.loadModel('models/nik-dragon')
        self.character.reparentTo(render)
        self.character.loadAnims({'win': 'models/nik-dragon'})
        self.character.loop('win')
        self.character.hprInterval(15, Point3(360, 0,0)).loop()
        
        # Create a non-attenuating point light and an ambient light.
        
        plightnode = PointLight("point light")
        plightnode.setAttenuation(Vec3(1,0,0))
        plight = render.attachNewNode(plightnode)
        plight.setPos(30,-50,0)
        alightnode = AmbientLight("ambient light")
        alightnode.setColor(Vec4(0.8,0.8,0.8,1))
        alight = render.attachNewNode(alightnode)
        render.setLight(alight)
        render.setLight(plight)
        
        # Panda contains a built-in viewer that lets you view the 
        # results of all render-to-texture operations.  This lets you
        # see what class CommonFilters is doing behind the scenes.
        
        self.accept("v", base.bufferViewer.toggleEnable)
        self.accept("V", base.bufferViewer.toggleEnable)
        base.bufferViewer.setPosition("llcorner")
        self.accept("s", self.filters.manager.resizeBuffers)
        
        # These allow you to change cartooning parameters in realtime
        
        self.accept("escape", sys.exit, [0])
        self.accept("arrow_up", self.increaseSeparation)
        self.accept("arrow_down", self.decreaseSeparation)
    
    def increaseSeparation(self):
        self.separation = self.separation * 1.11111111;
        print self.separation
        self.filters.setCartoonInk(separation = self.separation)
    
    def decreaseSeparation(self):
        self.separation = self.separation * 0.90000000;
        print self.separation
        self.filters.setCartoonInk(separation = self.separation)

t = ToonMaker()
run()
