# Author: Shao Zhang and Phil Saltzman
# Last Updated: 4/18/2005
#
# textureMovie.py shows how to set up a texture movie effect. This tutorial
# shows how to use that effect in a different context. Instead of setting the
# texture based on time, this tutorial has an elevator that sets a texture on
# its floor based on its height so that shadows cast by nearby lights line up
# correctly.
#
# The elevator, shaft, and shadow movie were created by T.J. Jackson
# for the Entertainment Technology Center class Building Virtual Worlds

import direct.directbase.DirectStart
from panda3d.core import Texture
from direct.interval.LerpInterval import LerpFunc
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
from direct.task.Task import Task
import sys

#Our specialized function to load texture movies and output them as lists
#From textureMovie.py. Look there for a full explanation on how it works
def loadTextureMovie(frames, name, suffix, padding = 1):
  return [loader.loadTexture((name+"%0"+str(padding)+"d."+suffix) % i)
          for i in range(frames)]

class World(DirectObject):
  def __init__(self):
    #Standard initialization stuff
    #Standard title that's on screen in every tutorial
    self.title = OnscreenText(text='Panda3D: Tutorial - Texture "Movies" (Elevator)',
      style=1, fg=(1,1,1,1), pos=(0.6,-0.95), scale = .07)
    
    #Load the elevator and attach it to render
    self.elevator = loader.loadModel('models/elevator')
    self.elevator.reparentTo(render)        
    
    #Load the plane that will be animated and attach it to the elevator iteslf
    self.shadowPlane = loader.loadModel('models/shadowPlane')
    self.shadowPlane.reparentTo(self.elevator)
    self.shadowPlane.setPos(0,0,.01)
    #Load the textures that will be applied to the polygon
    self.shadowTexs = loadTextureMovie(60, 'shadow/barShadows.', 'jpg')

    #Add the task that will animate the plane
    taskMgr.add(self.elevatorShadows, 'elevatorTask')

    #Builds the shaft, which is a 30ft repeatable segment
    self.shaft = []
    for i in range(-1,2):
      sh = loader.loadModel('models/shaft')
      sh.reparentTo(render)
      sh.setPos(-6.977, 0, 30*i)
      self.shaft.append(sh)

    #Linearly move the elevator's height using an interval.
    #If you replaced this with some other way of moving the elevator, the
    #texture would compensate since it's based on height and not time
    LerpFunc(self.elevator.setZ, fromData = 30, toData = -30,
         duration = 5).loop()
         
    #Puts the camera relative to the elevator in a position that
    #shows off the texture movie
    base.disableMouse()
    camera.reparentTo(self.elevator)
    camera.setPosHpr(-9, 0, 20, -90, -60, 0)

  #The task that runs the elevator. This is nearly the same as the one in
  #textureMovie.py with a few differences:
  #1) This task is only used once so the parameters were placed with hard-coded
  #   Values
  #2) Intead of basing the current frame on task.time,
  #   it's based on elevator.getZ()
  def elevatorShadows(self, task):
    texFrame = (int((self.elevator.getZ()%30)/.5)+26)%60
    self.shadowPlane.setTexture(self.shadowTexs[texFrame], 1)
    return Task.cont

w = World()
run()

