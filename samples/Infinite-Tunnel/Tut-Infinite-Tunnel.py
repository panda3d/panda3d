# Author: Shao Zhang and Phil Saltzman
# Last Updated: 4/18/2005
#
# This tutorial will cover fog and how it can be used to make a finite length
# tunnel seem endless by hiding its endpoint in darkness. Fog in panda works by
# coloring objects based on their distance from the camera. Fog is not a 3D
# volume object like real world fog.
# With the right settings, Fog in panda can mimic the appearence of real world # fog.
#
# The tunnel and texture was originally created by Vamsi Bandaru and Victoria
# Webb for the Entertainment Technology Center class Building Virtual Worlds

import direct.directbase.DirectStart 
from panda3d.core import Fog
from panda3d.core import TextNode
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
from direct.interval.MetaInterval import Sequence
from direct.interval.LerpInterval import LerpFunc
from direct.interval.FunctionInterval import Func
import sys

#Global variables for the tunnel dimensions and speed of travel
TUNNEL_SEGMENT_LENGTH = 50           
TUNNEL_TIME = 2   #Amount of time for one segment to travel the
                  #distance of TUNNEL_SEGMENT_LENGTH

class World(DirectObject):
  #Macro-like function used to reduce the amount to code needed to create the
  #on screen instructions
  def genLabelText(self, text, i):
    return OnscreenText(text = text, pos = (-1.3, .95-.06*i), fg=(1,1,1,1),
                       align = TextNode.ALeft, scale = .05)
  
  def __init__(self):
    ###Standard initialization stuff
    #Standard title that's on screen in every tutorial
    self.title = OnscreenText(
      text="Panda3D: Tutorial - Fog", style=1, fg=(1,1,1,1),
      pos=(0.9,-0.95), scale = .07)

    #Code to generate the on screen instructions
    self.escapeEventText = self.genLabelText("ESC: Quit", 0)
    self.pkeyEventText = self.genLabelText("[P]: Pause", 1)
    self.tkeyEventText = self.genLabelText("[T]: Toggle Fog", 2)
    self.dkeyEventText = self.genLabelText("[D]: Make fog color black", 3)
    self.sdkeyEventText = self.genLabelText(
      "[SHIFT+D]: Make background color black", 4)
    self.rkeyEventText = self.genLabelText("[R]: Make fog color red", 5)
    self.srkeyEventText = self.genLabelText(
      "[SHIFT+R]: Make background color red", 6)
    self.bkeyEventText = self.genLabelText("[B]: Make fog color blue", 7)
    self.sbkeyEventText = self.genLabelText(
      "[SHIFT+B]: Make background color blue", 8)
    self.gkeyEventText = self.genLabelText("[G]: Make fog color green", 9)
    self.sgkeyEventText = self.genLabelText(
      "[SHIFT+G]: Make background color green", 10)
    self.lkeyEventText = self.genLabelText(
      "[L]: Make fog color light grey", 11)
    self.slkeyEventText = self.genLabelText(
      "[SHIFT+L]: Make background color light grey", 12)
    self.pluskeyEventText = self.genLabelText("[+]: Increase fog density", 13)
    self.minuskeyEventText = self.genLabelText("[-]: Decrease fog density", 14)

    base.disableMouse() #disable mouse control so that we can place the camera
    camera.setPosHpr(0,0,10, 0, -90, 0)
    base.setBackgroundColor(0,0,0) #set the background color to black
    
    ###World specific-code
    
    #Create an instance of fog called 'distanceFog'.
    #'distanceFog' is just a name for our fog, not a specific type of fog.
    self.fog = Fog('distanceFog')
    #Set the initial color of our fog to black.
    self.fog.setColor(0, 0, 0)
    #Set the density/falloff of the fog.  The range is 0-1.
    #The higher the numer, the "bigger" the fog effect.
    self.fog.setExpDensity(.08)
    #We will set fog on render which means that everything in our scene will
    #be affected by fog. Alternatively, you could only set fog on a specific
    #object/node and only it and the nodes below it would be affected by
    #the fog.
    render.setFog(self.fog)

    #Define the keyboard input
    #Escape closes the demo
    self.accept('escape', sys.exit)         
    #Handle pausing the tunnel
    self.accept('p', self.handlePause)
    #Handle turning the fog on and off
    self.accept('t', ToggleFog, [render, self.fog])
    #Sets keys to set the fog to various colors
    self.accept('r', self.fog.setColor, [1,0,0])
    self.accept('g', self.fog.setColor, [0,1,0])
    self.accept('b', self.fog.setColor, [0,0,1])
    self.accept('l', self.fog.setColor, [.7,.7,.7])
    self.accept('d', self.fog.setColor, [0,0,0])
    #Sets keys to change the background colors
    self.accept('shift-r', base.setBackgroundColor, [1,0,0])
    self.accept('shift-g', base.setBackgroundColor, [0,1,0])
    self.accept('shift-b', base.setBackgroundColor, [0,0,1])
    self.accept('shift-l', base.setBackgroundColor, [.7,.7,.7])
    self.accept('shift-d', base.setBackgroundColor, [0,0,0])
    #Increases the fog density when "+" key is pressed
    self.accept('+', self.addFogDensity, [.01])
    #This is to handle the other "+" key (it's over = on the keyboard)
    self.accept('=', self.addFogDensity, [.01])
    self.accept('shift-=', self.addFogDensity, [.01])
    #Decreases the fog density when the "-" key is pressed
    self.accept('-', self.addFogDensity, [-.01])
    
    #Load the tunel and start the tunnel
    self.initTunnel()
    self.contTunnel()

  #This function will change the fog density by the amount passed into it
  #This function is needed so that it can look up the current value and
  #change it when the key is pressed. If you wanted to bind a key to set it
  #at a given value you could call self.fog.setExpDensity directly
  def addFogDensity(self, change):
    #The min() statement makes sure the density is never over 1
    #The max() statement makes sure the density is never below 0
    self.fog.setExpDensity(
      min(1, max(0, self.fog.getExpDensity() + change)))
    
  #Code to initialize the tunnel
  def initTunnel(self):
    #Creates the list [None, None, None, None]
    self.tunnel = [None for i in range(4)]
    
    for x in range(4):
      #Load a copy of the tunnel
      self.tunnel[x] = loader.loadModel('models/tunnel')
      #The front segment needs to be attached to render
      if x == 0: self.tunnel[x].reparentTo(render)
      #The rest of the segments parent to the previous one, so that by moving
      #the front segement, the entire tunnel is moved
      else:      self.tunnel[x].reparentTo(self.tunnel[x-1])
      #We have to offset each segment by its length so that they stack onto
      #each other. Otherwise, they would all occupy the same space.
      self.tunnel[x].setPos(0, 0, -TUNNEL_SEGMENT_LENGTH)
      #Now we have a tunnel consisting of 4 repeating segments with a
      #hierarchy like this:
      #render<-tunnel[0]<-tunnel[1]<-tunnel[2]<-tunnel[3]
      
  #This function is called to snap the front of the tunnel to the back
  #to simulate traveling through it
  def contTunnel(self):
    #This line uses slices to take the front of the list and put it on the
    #back. For more information on slices check the Python manual
    self.tunnel = self.tunnel[1:]+ self.tunnel[0:1]
    #Set the front segment (which was at TUNNEL_SEGMENT_LENGTH) to 0, which
    #is where the previous segment started
    self.tunnel[0].setZ(0)
    #Reparent the front to render to preserve the hierarchy outlined above
    self.tunnel[0].reparentTo(render)
    #Set the scale to be apropriate (since attributes like scale are
    #inherited, the rest of the segments have a scale of 1)
    self.tunnel[0].setScale(.155, .155, .305)
    #Set the new back to the values that the rest of teh segments have
    self.tunnel[3].reparentTo(self.tunnel[2])
    self.tunnel[3].setZ(-TUNNEL_SEGMENT_LENGTH)
    self.tunnel[3].setScale(1)
    
    #Set up the tunnel to move one segment and then call contTunnel again
    #to make the tunnel move infinitely
    self.tunnelMove = Sequence(
      LerpFunc(self.tunnel[0].setZ,
               duration = TUNNEL_TIME,
               fromData = 0,
               toData = TUNNEL_SEGMENT_LENGTH*.305),
      Func(self.contTunnel)
      )
    self.tunnelMove.start()
    
  #This function calls toggle interval to pause or unpause the tunnel.
  #Like addFogDensity, ToggleInterval could not be passed directly in the
  #accept command since the value of self.tunnelMove changes whenever
  #self.contTunnel is called
  def handlePause(self):
    ToggleInterval(self.tunnelMove)
#End Class World

#This function will toggle any interval passed to it between playing and paused
def ToggleInterval(interval):
  if interval.isPlaying(): interval.pause()
  else: interval.resume()
  
#This function will toggle fog on a node
def ToggleFog(node, fog):
  #If the fog attached to the node is equal to the one we passed in, then
  #fog is on and we should clear it
  if node.getFog() == fog: node.clearFog()
  #Otherwise fog is not set so we should set it
  else: node.setFog(fog)
  
w = World()
run()

