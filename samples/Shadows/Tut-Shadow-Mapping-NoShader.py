from panda3d.core import *
import sys,os
loadPrcFileData("", "prefer-parasite-buffer #f")

import direct.directbase.DirectStart
from direct.interval.IntervalGlobal import *
from direct.gui.DirectGui import OnscreenText
from direct.showbase.DirectObject import DirectObject
from direct.actor import Actor
from random import *

# Function to put instructions on the screen.
def addInstructions(pos, msg):
  return OnscreenText(text=msg,style=1,fg=(1,1,1,1),mayChange=1,
  pos=(-1.3, pos), align=TextNode.ALeft, scale = .05,
  shadow=(0,0,0,1), shadowOffset=(0.1,0.1))

# Function to put title on the screen.
def addTitle(text):
  return OnscreenText(text=text,style=1,fg=(1,1,1,1),
  pos=(1.3,-0.95), align=TextNode.ARight, scale = .07)

class World(DirectObject):
  def __init__(self):
    # Preliminary capabilities check.
    
    if (base.win.getGsg().getSupportsDepthTexture()==0):
      self.t=addTitle("No Depth Textures")
      return
    if (base.win.getGsg().getSupportsShadowFilter()==0):
      self.t=addTitle("No ARB_shadow")
      return
    
    # creating the offscreen buffer.
    
    winprops = WindowProperties.size(512,512)
    props = FrameBufferProperties()
    props.setRgbColor(1)
    props.setAlphaBits(1)
    props.setDepthBits(1)
    LBuffer = base.graphicsEngine.makeOutput(
         base.pipe, "offscreen buffer", -2,
         props, winprops,
         GraphicsPipe.BFRefuseWindow,
         base.win.getGsg(), base.win)
  
    if (LBuffer == None):
       self.t=addTitle("Buffer failed to setup")
       return

    Ldepthmap = Texture()
    LBuffer.addRenderTexture(Ldepthmap,
      GraphicsOutput.RTMBindOrCopy,
      GraphicsOutput.RTPDepthStencil)
    Ldepthmap.setWrapU(Texture.WMClamp)
    Ldepthmap.setWrapV(Texture.WMClamp)
    Ldepthmap.setMinfilter(Texture.FTShadow)
    Ldepthmap.setMagfilter(Texture.FTShadow)
    
    self.inst_t=addInstructions(.94,'T: stop/start Teapot')
    self.inst_x=addInstructions(.88,'Left/Right: cam angle')
  
    base.setBackgroundColor(0,0,0.2,1)
  
    base.camLens.setNearFar(1.0,150)
    base.camLens.setFov(75)
    base.disableMouse()

    # Load the scene.
  
    floorTex=loader.loadTexture('maps/envir-ground.jpg')
    cm=CardMaker('')
    cm.setFrame(-2,2,-2,2)
    floor = render.attachNewNode(PandaNode("floor"))
    for y in range(12):
      for x in range(12):
        nn = floor.attachNewNode(cm.generate())
        nn.setP(-90)
        nn.setPos((x-6)*4, (y-6)*4, 0)
    floor.setTexture(floorTex)
    floor.flattenStrong()
    
    self.teapot=loader.loadModel('teapot')
    self.teapot.reparentTo(render)
    self.teapot.setPos(0,-20,10)
    self.teapot.setTwoSided(True)
    self.tMovement=self.teapot.hprInterval(50,Point3(0,360,360))
    self.tMovement.loop()
  
    self.accept('escape',sys.exit)
  
    self.accept("arrow_left", self.incCamPos, [-1])
    self.accept("arrow_right", self.incCamPos, [1])
    self.accept("t", self.toggleInterval, [self.tMovement])
    self.accept("T", self.toggleInterval, [self.tMovement])
    self.accept("v", base.bufferViewer.toggleEnable)
    self.accept("V", base.bufferViewer.toggleEnable)
    self.accept("o", base.oobe)
  
    self.LCam=base.makeCamera(LBuffer)
    self.LCam.node().setScene(render)
    self.LCam.node().getLens().setFov(40)
    self.LCam.node().getLens().setNearFar(10,100)
    self.LCam.node().showFrustum()
    self.LCam.setPos(0,-40,25)
    self.LCam.lookAt(0,-10,0)
    # Reparent a normal spotlight to it, sharing the same lens
    self.Light=self.LCam.attachNewNode(Spotlight("Light"))
    self.Light.node().setLens(self.LCam.node().getLens())
    render.setLight(self.Light)

    # default values
    self.cameraSelection = 0
  
    # Enable texture projection of the depthmap onto the scene
    ts = TextureStage("shadow")
    render.projectTexture(ts, Ldepthmap, self.Light)
    
    self.incCamPos(0)

  def toggleInterval(self, ival):
    if (ival.isPlaying()):
      ival.pause()
    else:
      ival.resume()

  def incCamPos(self,n):
    self.cameraSelection = (self.cameraSelection + n) % 3
    if (self.cameraSelection == 0):
      base.cam.reparentTo(render)
      base.cam.setPos(30,-45,26)
      base.cam.lookAt(0,0,0)
    if (self.cameraSelection == 1):
      base.cam.reparentTo(render)
      base.cam.setPos(7,-23,12)
      base.cam.lookAt(self.teapot)
      self.LCam.node().hideFrustum()
    if (self.cameraSelection == 2):
      base.cam.reparentTo(render)
      base.cam.setPos(-7,-23,12)
      base.cam.lookAt(self.teapot)
      self.LCam.node().hideFrustum()
    if (self.cameraSelection == 3):
      base.cam.reparentTo(render)
      base.cam.setPos(1000,0,195)
      base.cam.lookAt(0,0,0)
      self.LCam.node().showFrustum()

World()
run()

