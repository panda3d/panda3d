# Author: Shao Zhang, Phil Saltzman, and Elan Ruskin
# Last Updated: 4/20/2005
#
# This tutorial shows how to load, play, and manipulate sounds
# and sound intervals in a panda project.

import direct.directbase.DirectStart
from panda3d.core import NodePath,TextNode
from panda3d.core import Vec3,Vec4
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
from direct.interval.SoundInterval import SoundInterval
from direct.gui.DirectSlider import DirectSlider
from direct.gui.DirectButton import DirectButton
from direct.interval.MetaInterval import Sequence,Parallel
from direct.interval.LerpInterval import LerpFunc
import sys

class World(DirectObject):
  def __init__(self):
  
    #Our standard title and instructions text
    self.title = OnscreenText(text="Panda3D: Tutorial - Musicbox(sounds)",
                              style=1, fg=(1,1,1,1), pos=(0.7,-0.95), scale = .07)
    self.escapeEventText = OnscreenText(text="ESC: Quit",
                                    style=1, fg=(1,1,1,1), pos=(-1.3, 0.95),
                                    align=TextNode.ALeft, scale = .05)
  
    #Set up the key input 
    self.accept('escape', sys.exit)

    #Fix the camera position
    base.disableMouse()

    #Loading sounds is done in a similar way to loading other things
    #Loading the main music box song
    self.musicBoxSound = base.loadMusic('music/musicbox.ogg')
    self.musicBoxSound.setVolume(.5)   #Volume is a percentage from 0 to 1
    self.musicBoxSound.setLoopCount(0) #0 means loop forever, 1 (default) means
                                       #play once. 2 or higher means play that
                                       #many times

    #Sound objects do not have a pause function, just play and stop. So we will
    #Use this variable to keep track of where the sound is at when it was stoped
    #to impliment pausing
    self.musicTime = 0

    #Loading the open/close effect
    #loadSFX and loadMusic are identical. They are often used for organization
    #(loadMusic is used for background music, loadSfx is used for other effects)
    self.lidSfx = base.loadSfx('music/openclose.ogg')
    #The open/close file has both effects in it. Fortunatly we can use intervals
    #to easily define parts of a sound file to play
    self.lidOpenSfx = SoundInterval(self.lidSfx, duration = 2, startTime = 0)
    self.lidCloseSfx = SoundInterval(self.lidSfx, startTime = 5)

    #For this tutorial, it seemed appropriate to have on screen controls. The
    #following code creates them
    #This is a label for a slider
    self.sliderText = OnscreenText("Volume",
                    style=1, fg=(1,1,1,1), pos=(0,0.8), scale = .07)
    #The slider itself. It calls self.setMusicBoxVolume when changed
    self.slider = DirectSlider(pos = Vec3(0,0,.7), value = .50,
                    command = self.setMusicBoxVolume)
    #A button that calls self.toggleMusicBox when pressed
    self.button = DirectButton(pos = Vec3(.7,0,.7), text = "Open Box",
                   scale = .1, pad = (.5, .5),
                   rolloverSound = None, clickSound = None,
                   command = self.toggleMusicBox)

    #A variable to represent the state of the simulation. It starts closed
    self.boxOpen = False

    #Here we load and set up the music box. It was modeled in a complex way, so
    #setting it up will be complicated
    self.musicBox=loader.loadModel('models/MusicBox')
    self.musicBox.setPos(0, 60, -10)
    self.musicBox.reparentTo(render)
    #Just like the scene graph contains hierarchies of nodes, so can
    #models. You can get the NodePath for the node using the find
    #function, and then you can animate the model by moving its parts
    #To see the hierarchy of a model, use, the ls function
    #self.musicBox.ls() prints out the entire hierarchy of the model

    #Finding pieces of the model
    self.Lid   = self.musicBox.find('**/lid')
    self.Panda = self.musicBox.find('**/turningthing')  

    #This model was made with the hinge in the wrong place
    #this is here so we have something to turn
    self.HingeNode = self.musicBox.find('**/box').attachNewNode('nHingeNode')
    self.HingeNode.setPos(.8659,6.5,5.4)
    #WRT - ie with respect to. Reparents the object without changing
    #its position, size, or orientation
    self.Lid.wrtReparentTo(self.HingeNode)
    self.HingeNode.setHpr(0,90,0)

    #This sets up an interval to play the close sound and actually close the box
    #at the same time.
    self.lidClose = Parallel(
      self.lidCloseSfx,
      LerpFunc(self.HingeNode.setP, duration = 2,
           fromData = 0, toData = 90,
           blendType ='easeInOut'))

    #Same thing for opening the box
    self.lidOpen = Parallel(
      self.lidOpenSfx,
      LerpFunc(self.HingeNode.setP, duration = 2,
           fromData = 90, toData = 0,
           blendType ='easeInOut'))

    #The interval for turning the panda
    self.PandaTurn=self.Panda.hprInterval( 7, Vec3(360,0,0) )
    #Do a quick loop and pause to set it as a looping interval so it can be
    #started with resume and loop properly
    self.PandaTurn.loop()
    self.PandaTurn.pause()

  def setMusicBoxVolume(self):
    #Simply reads the current value from the slider and sets it in the sound
    newVol = self.slider.guiItem.getValue()
    self.musicBoxSound.setVolume(newVol)

  def toggleMusicBox(self):
    if self.boxOpen:
      #close the box
      self.lidClose.start()               #Start the close box interval
      self.PandaTurn.pause()              #Pause the figurine turning
      #Save the current time of the music box song
      self.musicTime = self.musicBoxSound.getTime() 
      self.musicBoxSound.stop()           #Stop the music box song
      self.button['text'] = "Open Box"    #Prepare to change button label
    else:
      #open the box
      self.lidOpen.start()                #Start the open box interval
      self.PandaTurn.resume()             #Resume the figuring turning
      #Reset the time of the music box song so it starts where it left off
      self.musicBoxSound.setTime(self.musicTime)
      self.musicBoxSound.play()           #Play the music box song
      self.button['text'] = "Close Box"   #Prepare to change button label

    self.button.setText()                 #Actually change the button label
    self.boxOpen = not self.boxOpen       #Set our state to opposite what it was
                                          #(closed to open or open to closed)

# and we can run!
w = World()
run()

