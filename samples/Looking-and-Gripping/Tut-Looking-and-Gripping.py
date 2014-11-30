# Author: Shao Zhang and Phil Saltzman
# Models and Textures by: Shaun Budhram, Will Houng, and David Tucker
# Last Updated: 5/1/2005
#
# This tutorial will cover exposing joints and manipulating them. Specifically,
# we will take control of the neck joint of a humanoid character and rotate that
# joint to always face the mouse cursor.  This will in turn make the head of the
# character "look" at the mouse cursor.  We will also expose the hand joint and
# use it as a point to "attach" objects that the character can hold.  By
# parenting an object to a hand joint, the object will stay in the character's
# hand even if the hand is moving through an animation.

import direct.directbase.DirectStart
from panda3d.core import AmbientLight,DirectionalLight
from panda3d.core import TextNode,NodePath,LightAttrib
from panda3d.core import Vec3,Vec4
from direct.actor.Actor import Actor
from direct.task.Task import Task
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
import sys

#A simple function to make sure a value is in a given range, -1 to 1 by default
def restrain(i, mn = -1, mx = 1): return min(max(i, mn), mx)

#Macro-like function used to reduce the amount to code needed to create the
#on screen instructions
def genLabelText(text, i):
  return OnscreenText(text = text, pos = (-1.3, .95-.06*i), fg=(1,1,1,1),
                      align = TextNode.ALeft, scale = .05)

class World(DirectObject):
  def __init__(self):
    #This code puts the standard title and instruction text on screen
    self.title = OnscreenText(text="Panda3D: Tutorial - Joint Manipulation",
                              style=1, fg=(1,1,1,1),
                              pos=(0.7,-0.95), scale = .07)
    self.onekeyText   = genLabelText("ESC: Quit", 0)
    self.onekeyText   = genLabelText("[1]: Teapot", 1)
    self.twokeyText   = genLabelText("[2]: Candy cane", 2)
    self.threekeyText = genLabelText("[3]: Banana", 3)
    self.fourkeyText  = genLabelText("[4]: Sword", 4)
      
    #setup key input
    self.accept('escape', sys.exit)
    self.accept('1', self.setObject, [0])
    self.accept('2', self.setObject, [1])
    self.accept('3', self.setObject, [2])
    self.accept('4', self.setObject, [3])

    base.disableMouse()                  #Disable mouse-based camera-control
    camera.setPos(0,-15, 2)              #Position the camera

    self.eve = Actor("models/eve", #Load our animated charachter
                           {'walk' : "models/eve_walk"})
    self.eve.reparentTo(render)          #Put it in the scene

    #Now we use controlJoint to get a NodePath that's in control of her neck
    #This must be done before any animations are played
    self.eveNeck = self.eve.controlJoint(None, 'modelRoot', 'Neck')

    #We now play an animation. An animation must be played, or at least posed
    #for the nodepath we just got from controlJoint to actually effect the model
    self.eve.actorInterval("walk", playRate = 2).loop()

    #Now we add a task that will take care of turning the head
    taskMgr.add(self.turnHead, "turnHead")

    #Now we will expose the joint the hand joint. ExposeJoint allows us to
    #get the position of a joint while it is animating. This is different than
    #controlJonit which stops that joint from animating but lets us move it.
    #This is particularly usefull for putting an object (like a weapon) in an
    #actor's hand
    self.rightHand = self.eve.exposeJoint(None, 'modelRoot', 'RightHand')

    #This is a table with models, positions, rotations, and scales of objects to
    #be attached to our exposed joint. These are stock models and so they needed
    #to be repositioned to look right.
    positions = [("models/teapot", (0,-.66,-.95), (90,0,90), .4),
                 ("models/candycane", (.15,-.99,-.22), (90,0,90), 1),
                 ("models/banana", (.08,-.1,.09), (0,-90,0), 1.75),
                 ("models/sword", (.11,.19,.06), (0,0,90), 1)]
    self.models = []                 #A list that will store our models objects
    for row in positions:
      np = loader.loadModel(row[0])              #Load the model
      np.setPos(row[1][0], row[1][1], row[1][2]) #Position it
      np.setHpr(row[2][0], row[2][1], row[2][2]) #Rotate it
      np.setScale(row[3])                        #Scale it
      #Reparent the model to the exposed joint. That way when the joint moves,
      #the model we just loaded will move with it.
      np.reparentTo(self.rightHand)
      self.models.append(np)                     #Add it to our models list

    self.setObject(0)                            #Make object 0 the first shown
    self.setupLights()                           #Put in some default lighting

  #This is what we use to change which object it being held. It just hides all of
  #the objects and then unhides the one that was selected
  def setObject(self, i):
    for np in self.models: np.hide()
    self.models[i].show()

  #This task gets the position of mouse each frame, and rotates the neck based
  #on it.
  def turnHead(self, task):
    #Check to make sure the mouse is readable
    if base.mouseWatcherNode.hasMouse():
      #get the mouse position as a Vec2. The values for each axis are from -1 to
      #1. The top-left is (-1,-1), the bottom right is (1,1)
      mpos = base.mouseWatcherNode.getMouse()
      #Here we multiply the values to get the amount of degrees to turn
      #Restrain is used to make sure the values returned by getMouse are in the
      #valid range. If this particular model were to turn more than this,
      #significant tearing would be visable
      self.eveNeck.setP(restrain(mpos.getX()) * 50)
      self.eveNeck.setH(restrain(mpos.getY()) * 20)

    return Task.cont                        #Task continues infinitely

  def setupLights(self):                    #Sets up some default lighting
    ambientLight = AmbientLight( "ambientLight" )
    ambientLight.setColor( Vec4(.4, .4, .35, 1) )
    directionalLight = DirectionalLight( "directionalLight" )
    directionalLight.setDirection( Vec3( 0, 8, -2.5 ) )
    directionalLight.setColor( Vec4( 0.9, 0.8, 0.9, 1 ) )
    render.setLight(render.attachNewNode( directionalLight ) )
    render.setLight(render.attachNewNode( ambientLight ) )

w = World()        #Create an instance of our class
run()              #Run the simulation

