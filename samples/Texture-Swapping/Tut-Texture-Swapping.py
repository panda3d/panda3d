# Author: Shao Zhang and Phil Saltzman
# Last Updated: 4/18/2005
#
# This tutorial shows how to use a sequence of textures on an object to
# achieve a specific effect. Popular uses of this technique are:
# -Animated sprites
# -Moving shadows
#
# This tutorial also demonstrates the billboard function which orients
# an object to alsways face to the camera. This is useful for 2D sprites
# in a 3D world.

# The duck animation was created by Shane Liesegang and William Houng
# for the Entertainment Technology Center class Building Virtual Worlds

import direct.directbase.DirectStart
from panda3d.core import Texture
from panda3d.core import BillboardEffect
from panda3d.core import Camera
from panda3d.core import TextNode
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
from direct.task.Task import Task
import sys

class World(DirectObject):
  def __init__(self):
    #Standard initialization stuff
    #Standard title that's on screen in every tutorial
    self.title = OnscreenText(text='Panda3D: Tutorial - Texture "Movies"',
                              style=1, fg=(1,1,1,1), pos=(0.7,-0.95),
                              scale = .07)
    
    #Text to show the keyboard keys and their functions on screen
    self.escapeEventText = OnscreenText(
      text="ESC: Quit", style=1, fg=(1,1,1,1), pos=(-1.3, 0.95),
      align=TextNode.ALeft, scale = .05)
    self.onekeyEventText = OnscreenText(
      text="[1]: Freeview camera", style=1, fg=(1,1,1,1), pos=(-1.3, 0.90),
      align=TextNode.ALeft, scale = .05)
    self.twokeyEventText = OnscreenText(
      text="[2]: Preset Camera Angle 2 (Verify billboard effect)",
      style=1, fg=(1,1,1,1), pos=(-1.3, 0.85),
      align=TextNode.ALeft, scale = .05, mayChange = 1)
    
    base.setBackgroundColor(0, 0, 0)        #Set the background color
    
    #Set up the key input
    self.accept('escape', sys.exit)         #Escape quits
    self.accept('1', self.setViewMain)      #Free view
    self.accept('2', self.setViewBillboard) #Billboard effect view

    #Initialization specific to this world
    #Load a polygon plane (4 sided square) to put an animated duck sprite on
    self.duckPlane = loader.loadModel('models/plane')
    self.duckPlane.setPos(-2, 8, 0)         #set its position
    self.duckPlane.reparentTo(render)       #reparent to render

    #Enable tranparency: this attribute needs to be set for Panda to render the
    #transparency in the duck's texture as transparent rather than opaque
    self.duckPlane.setTransparency(1)

    #Now we call our special 'loadTextureMovie' function that returns a list
    #containing all of the textures for the duck sprite.
    #Check the function definition later in this file for its parameters
    self.duckTexs = self.loadTextureMovie(24, 'duck/duck_fly_left',
                                          'png', padding = 2)

    #Next we add a task to our task list that will animate the texture on the
    #duck plane according to the time elapsed.
    self.duckTask = taskMgr.add(self.textureMovie, "duckTask")
    #The function self.textureMovie is set to run any texture movie that
    #animates and loops based on time (rather that some other value like
    #position). To do that, it is set up to expect a number of parameters set
    #in the task object. The following lines set those parameters

    #Framerate: The texture will be changed 36 times per second
    self.duckTask.fps = 36
    #self.duckPlane is the object whose texture should be changed
    self.duckTask.obj = self.duckPlane  
    #self.duckTexs (which we created earlier with self.oadTextureMovie)
    #contains the list of textures to animate from
    self.duckTask.textures = self.duckTexs

    #Now, instead of a duck, we will put an animated explosion onto a polygon
    #This is the same as loading the duck animation, with the expection that
    #we will "billboard" the explosion so that it always faces the camera
    self.expPlane = loader.loadModel('models/plane')  #load the object
    self.expPlane.setPos(2, 8, 0)                         #set the position
    self.expPlane.reparentTo(render)                      #reparent to render
    self.expPlane.setTransparency(1)                      #enable transparency
    #load the texture movie
    self.expTexs = self.loadTextureMovie(51, 'explosion/explosion',
                                         'png', padding = 4)

    #create the animation task
    self.expTask = taskMgr.add(self.textureMovie, "explosionTask")
    self.expTask.fps = 30                                 #set framerate
    self.expTask.obj = self.expPlane                      #set object
    self.expTask.textures = self.expTexs                  #set texture list

    #This create the "billboard" effect that will rotate the object so that it
    #is always rendered as facing the eye (camera)
    self.expPlane.node().setEffect(BillboardEffect.makePointEye())
    
    #The code below generates the plane you see with the numbers and arrows.
    #This is just to give a sense of orientation as the camera is moved around.
    self.orientPlane = loader.loadModel('models/plane') #Load the object
    #load the texture
    self.orientTex = loader.loadTexture("models/textures/orientation.png")
    self.orientPlane.setTexture(self.orientTex, 1)        #Set the texture
    self.orientPlane.reparentTo(render)                   #Parent to render
    #Set the position, orientation, and scale
    self.orientPlane.setPosHprScale(0, 8, -1, 0, -90, 0, 10, 10, 10)

  #This function is run every frame by our tasks to animate the textures
  def textureMovie(self, task):
    #Here we calculate the current frame number by multiplying the current time
    #(in seconds) by the frames per second variable we set earlier
    currentFrame = int(task.time * task.fps)

    #Now we need to set the current texture on task.obj, which is the object
    #we specified earlier when we loaded the duck and explosion.
    #We will set its texture to one of the textures in the list we defined
    #earlier in task.textures.

    #Since we want the movie to loop, we need to reset the image index when it
    #reaches the end. We do this by performing a remainder operation (modulo,
    #"%" in python) on currentFrame with the number of frames, which is the
    #length of our frame list (len(task.textures)).
    #This is a common programming technique to achieve looping as it garuntees
    #a value in range of the list
    task.obj.setTexture(task.textures[currentFrame % len(task.textures)], 1)
    return Task.cont          #Continue the task indefinitely
    
  #Our custom load function to load the textures needed for a movie into a
  #list. It assumes the the files are named
  #"path/name<serial number>.extention"
  #It takes the following arguments
  #Frames: The number of frames to load
  #name: The "path/name" part of the filename path
  #suffix: The "extention" part of the path
  #padding: The number of digit the serial number contians:
  #         e.g. if the serial number is 0001 then padding is 4
  def loadTextureMovie(self, frames, name, suffix, padding = 1):
    #The following line is very complicated but does a lot in one line
    #Here's the explanation from the inside out:
    #first, a string representing the filename is built an example is:
    #"path/name%04d.extention"
    #The % after the string is an operator that works like C's sprintf function
    #It tells python to put the next argument (i) in place of the %04d
    #For more string formatting information look in the python manual
    #That string is then passed to the loader.loadTexture function
    #The loader.loadTexture command gets done in a loop once for each frame,
    #And the result is returned as a list.
    #For more information on "list comprehensions" see the python manual
    return [loader.loadTexture((name+"%0"+str(padding)+"d."+suffix) % i) 
        for i in range(frames)]
        
  #Enables freelook
  def setViewMain(self):
    camera.reparentTo(render)
    base.enableMouse()

  #Disables freelook and places the camera in a good position to
  #demonstrate the billboard effect
  def setViewBillboard(self):
    base.disableMouse()
    camera.reparentTo(render)
    camera.setPosHpr(-7, 7, 0, -90, 0, 0)

w = World()          #Create an instance of our class
run()                #Run the world

