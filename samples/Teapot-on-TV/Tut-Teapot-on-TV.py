import direct.directbase.DirectStart
from panda3d.core import Filename,Texture
from panda3d.core import AmbientLight,DirectionalLight
from panda3d.core import NodePath,TextNode
from panda3d.core import Point3,Vec3,Vec4
from direct.task.Task import Task
from direct.actor.Actor import Actor
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
import sys, os, random

# Figure out what directory contains this program
MYDIR=os.path.abspath(sys.path[0])
MYDIR=Filename.fromOsSpecific(MYDIR).getFullpath()

# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(1,1,1,1),
                        pos=(-1.3, pos), align=TextNode.ALeft, scale = .05)

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, fg=(1,1,1,1),
                        pos=(1.3,-0.95), align=TextNode.ARight, scale = .07)

random.seed()
base.setBackgroundColor(Vec4(0.0,0.0,0.0,1.0))

class World(DirectObject):
    def __init__(self):
        
        # Post the instructions.
        self.title = addTitle("Panda3D: Tutorial - Using Render-to-Texture")
        self.inst1 = addInstructions(0.95,"ESC: Quit")
        self.inst2 = addInstructions(0.90,"Up/Down: Zoom in/out on the Teapot")
        self.inst3 = addInstructions(0.85,"Left/Right: Move teapot left/right")
        self.inst4 = addInstructions(0.80,"V: View the render-to-texture results")

        #we get a handle to the default window
        mainWindow=base.win

        #we now get buffer thats going to hold the texture of our new scene   
        altBuffer=mainWindow.makeTextureBuffer("hello", 256, 256)
        
        #now we have to setup a new scene graph to make this scene
        altRender=NodePath("new render")

        #this takes care of setting up ther camera properly
        self.altCam=base.makeCamera(altBuffer)
        self.altCam.reparentTo(altRender)        
        self.altCam.setPos(0,-10,0)

        #get the teapot and rotates it for a simple animation
        self.teapot=loader.loadModel('models/teapot')
        self.teapot.reparentTo(altRender)
        self.teapot.setPos(0,0,-1)
        self.teapot.hprInterval(1.5,Point3(360,360,360)).loop()
        
        #put some lighting on the teapot
        dlight = DirectionalLight('dlight')
        alight = AmbientLight('alight')
        dlnp = altRender.attachNewNode(dlight) 
        alnp = altRender.attachNewNode(alight)
        dlight.setColor(Vec4(0.8, 0.8, 0.5, 1))
        alight.setColor(Vec4(0.2, 0.2, 0.2, 1))
        dlnp.setHpr(0, -60, 0) 
        altRender.setLight(dlnp)
        altRender.setLight(alnp)
        
        # Panda contains a built-in viewer that lets you view the results of
        # your render-to-texture operations.  This code configures the viewer.

        self.accept("v", base.bufferViewer.toggleEnable)
        self.accept("V", base.bufferViewer.toggleEnable)
        base.bufferViewer.setPosition("llcorner")
        base.bufferViewer.setCardSize(1.0, 0.0)

        # Create the tv-men. Each TV-man will display the
        # offscreen-texture on his TV screen.
        self.tvMen = []
        self.makeTvMan(-5,30, 1,altBuffer.getTexture(),0.9)
        self.makeTvMan( 5,30, 1,altBuffer.getTexture(),1.4)
        self.makeTvMan( 0,23,-3,altBuffer.getTexture(),2.0)
        self.makeTvMan(-5,20,-6,altBuffer.getTexture(),1.1)
        self.makeTvMan( 5,18,-5,altBuffer.getTexture(),1.7)

        self.accept("escape", sys.exit, [0])
        self.accept("arrow_up", self.zoomIn)
        self.accept("arrow_down", self.zoomOut)
        self.accept("arrow_left", self.moveLeft)
        self.accept("arrow_right", self.moveRight)

    def makeTvMan(self,x,y,z,tex,playrate):
        man = Actor()
        man.loadModel('models/mechman_idle')
        man.setPos(x,y,z)
        man.reparentTo(render)
        fp = man.find("**/faceplate")
        fp.setTexture(tex,1)
        man.setPlayRate(playrate, "mechman_anim")
        man.loop("mechman_anim")
        self.tvMen.append(man)

    def zoomIn(self):
        self.altCam.setY(self.altCam.getY()*0.9)
    
    def zoomOut(self):
        self.altCam.setY(self.altCam.getY()*1.2)
    
    def moveLeft(self):
        self.altCam.setX(self.altCam.getX()+1)
    
    def moveRight(self):
        self.altCam.setX(self.altCam.getX()-1)


w=World()
run()

