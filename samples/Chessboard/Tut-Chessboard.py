# Author: Shao Zhang and Phil Saltzman
# Models: Eddie Canaan
# Last Updated: 5/1/2005
#
# This tutorial shows how to determine what objects the mouse is pointing to
# We do this using a collision ray that extends from the mouse position
# and points straight into the scene, and see what it collides with. We pick
# the object with the closest collision

import direct.directbase.DirectStart
from panda3d.core import CollisionTraverser,CollisionNode
from panda3d.core import CollisionHandlerQueue,CollisionRay
from panda3d.core import AmbientLight,DirectionalLight,LightAttrib
from panda3d.core import TextNode
from panda3d.core import Point3,Vec3,Vec4,BitMask32
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
from direct.task.Task import Task
import sys

#First we define some contants for the colors
BLACK = Vec4(0,0,0,1)
WHITE = Vec4(1,1,1,1)
HIGHLIGHT = Vec4(0,1,1,1)
PIECEBLACK = Vec4(.15, .15, .15, 1)

#Now we define some helper functions that we will need later

#This function, given a line (vector plus origin point) and a desired z value,
#will give us the point on the line where the desired z value is what we want.
#This is how we know where to position an object in 3D space based on a 2D mouse
#position. It also assumes that we are dragging in the XY plane.
#
#This is derived from the mathmatical of a plane, solved for a given point
def PointAtZ(z, point, vec):
  return point + vec * ((z-point.getZ()) / vec.getZ())

#A handy little function for getting the proper position for a given square
def SquarePos(i):
  return Point3((i%8) - 3.5, int(i/8) - 3.5, 0)

#Helper function for determining wheter a square should be white or black
#The modulo operations (%) generate the every-other pattern of a chess-board
def SquareColor(i):
  if (i + ((i/8)%2))%2: return BLACK
  else: return WHITE

class World(DirectObject):
  def __init__(self):
    #This code puts the standard title and instruction text on screen
    self.title = OnscreenText(text="Panda3D: Tutorial - Mouse Picking",
                              style=1, fg=(1,1,1,1),
                              pos=(0.8,-0.95), scale = .07)
    self.escapeEvent = OnscreenText( 
      text="ESC: Quit",
      style=1, fg=(1,1,1,1), pos=(-1.3, 0.95),
      align=TextNode.ALeft, scale = .05)
    self.mouse1Event = OnscreenText(
      text="Left-click and drag: Pick up and drag piece",
      style=1, fg=(1,1,1,1), pos=(-1.3, 0.90),
      align=TextNode.ALeft, scale = .05)

    self.accept('escape', sys.exit)              #Escape quits
    base.disableMouse()                          #Disble mouse camera control
    camera.setPosHpr(0, -13.75, 6, 0, -25, 0)    #Set the camera
    self.setupLights()                           #Setup default lighting
    
    #Since we are using collision detection to do picking, we set it up like
    #any other collision detection system with a traverser and a handler
    self.picker = CollisionTraverser()            #Make a traverser
    self.pq     = CollisionHandlerQueue()         #Make a handler
    #Make a collision node for our picker ray
    self.pickerNode = CollisionNode('mouseRay')
    #Attach that node to the camera since the ray will need to be positioned
    #relative to it
    self.pickerNP = camera.attachNewNode(self.pickerNode)
    #Everything to be picked will use bit 1. This way if we were doing other
    #collision we could seperate it
    self.pickerNode.setFromCollideMask(BitMask32.bit(1))
    self.pickerRay = CollisionRay()               #Make our ray
    self.pickerNode.addSolid(self.pickerRay)      #Add it to the collision node
    #Register the ray as something that can cause collisions
    self.picker.addCollider(self.pickerNP, self.pq)
    #self.picker.showCollisions(render)

    #Now we create the chess board and its pieces

    #We will attach all of the squares to their own root. This way we can do the
    #collision pass just on the sqaures and save the time of checking the rest
    #of the scene
    self.squareRoot = render.attachNewNode("squareRoot")
    
    #For each square
    self.squares = [None for i in range(64)]
    self.pieces = [None for i in range(64)]
    for i in range(64):
      #Load, parent, color, and position the model (a single square polygon)
      self.squares[i] = loader.loadModel("models/square")
      self.squares[i].reparentTo(self.squareRoot)
      self.squares[i].setPos(SquarePos(i))
      self.squares[i].setColor(SquareColor(i))
      #Set the model itself to be collideable with the ray. If this model was
      #any more complex than a single polygon, you should set up a collision
      #sphere around it instead. But for single polygons this works fine.
      self.squares[i].find("**/polygon").node().setIntoCollideMask(
        BitMask32.bit(1))
      #Set a tag on the square's node so we can look up what square this is
      #later during the collision pass
      self.squares[i].find("**/polygon").node().setTag('square', str(i))

      #We will use this variable as a pointer to whatever piece is currently
      #in this square

    #The order of pieces on a chessboard from white's perspective. This list
    #contains the constructor functions for the piece classes defined below
    pieceOrder = (Rook, Knight, Bishop, Queen, King, Bishop, Knight, Rook)

    for i in range (8,16):
      #Load the white pawns
      self.pieces[i] = Pawn(i, WHITE)
    for i in range (48,56):
      #load the black pawns
      self.pieces[i] = Pawn(i, PIECEBLACK)
    for i in range(8):
      #Load the special pieces for the front row and color them white
      self.pieces[i] = pieceOrder[i](i, WHITE)
      #Load the special pieces for the back row and color them black
      self.pieces[i+56] = pieceOrder[i](i+56, PIECEBLACK)

    #This will represent the index of the currently highlited square
    self.hiSq = False
    #This wil represent the index of the square where currently dragged piece
    #was grabbed from
    self.dragging = False

    #Start the task that handles the picking
    self.mouseTask = taskMgr.add(self.mouseTask, 'mouseTask')
    self.accept("mouse1", self.grabPiece)       #left-click grabs a piece
    self.accept("mouse1-up", self.releasePiece) #releasing places it

  #This function swaps the positions of two pieces
  def swapPieces(self, fr, to):
    temp = self.pieces[fr]
    self.pieces[fr] = self.pieces[to]
    self.pieces[to] = temp
    if self.pieces[fr]:
      self.pieces[fr].square = fr
      self.pieces[fr].obj.setPos(SquarePos(fr))
    if self.pieces[to]:
      self.pieces[to].square = to
      self.pieces[to].obj.setPos(SquarePos(to))

  def mouseTask(self, task):
    #This task deals with the highlighting and dragging based on the mouse
    
    #First, clear the current highlight
    if self.hiSq is not False:
      self.squares[self.hiSq].setColor(SquareColor(self.hiSq))
      self.hiSq = False
      
    #Check to see if we can access the mouse. We need it to do anything else
    if base.mouseWatcherNode.hasMouse():
      #get the mouse position
      mpos = base.mouseWatcherNode.getMouse()
      
      #Set the position of the ray based on the mouse position
      self.pickerRay.setFromLens(base.camNode, mpos.getX(), mpos.getY())
      
      #If we are dragging something, set the position of the object
      #to be at the appropriate point over the plane of the board
      if self.dragging is not False:
        #Gets the point described by pickerRay.getOrigin(), which is relative to
        #camera, relative instead to render
        nearPoint = render.getRelativePoint(camera, self.pickerRay.getOrigin())
        #Same thing with the direction of the ray
        nearVec = render.getRelativeVector(camera, self.pickerRay.getDirection())
        self.pieces[self.dragging].obj.setPos(
          PointAtZ(.5, nearPoint, nearVec))

      #Do the actual collision pass (Do it only on the squares for
      #efficiency purposes)
      self.picker.traverse(self.squareRoot)
      if self.pq.getNumEntries() > 0:
        #if we have hit something, sort the hits so that the closest
        #is first, and highlight that node
        self.pq.sortEntries()
        i = int(self.pq.getEntry(0).getIntoNode().getTag('square'))
        #Set the highlight on the picked square
        self.squares[i].setColor(HIGHLIGHT)
        self.hiSq = i
          
    return Task.cont

  def grabPiece(self):
    #If a square is highlighted and it has a piece, set it to dragging mode
    if (self.hiSq is not False and
      self.pieces[self.hiSq]):
      self.dragging = self.hiSq
      self.hiSq = False
    
  def releasePiece(self):
    #Letting go of a piece. If we are not on a square, return it to its original
    #position. Otherwise, swap it with the piece in the new square
    if self.dragging is not False:   #Make sure we really are dragging something
      #We have let go of the piece, but we are not on a square
      if self.hiSq is False:
        self.pieces[self.dragging].obj.setPos(
          SquarePos(self.dragging))
      else:
        #Otherwise, swap the pieces
        self.swapPieces(self.dragging, self.hiSq)
        
    #We are no longer dragging anything
    self.dragging = False

  def setupLights(self):    #This function sets up some default lighting
    ambientLight = AmbientLight( "ambientLight" )
    ambientLight.setColor( Vec4(.8, .8, .8, 1) )
    directionalLight = DirectionalLight( "directionalLight" )
    directionalLight.setDirection( Vec3( 0, 45, -45 ) )
    directionalLight.setColor( Vec4( 0.2, 0.2, 0.2, 1 ) )
    render.setLight(render.attachNewNode( directionalLight ) )
    render.setLight(render.attachNewNode( ambientLight ) )

#Class for a piece. This just handels loading the model and setting initial
#position and color
class Piece:
  def __init__(self, square, color):
    self.obj = loader.loadModel(self.model)
    self.obj.reparentTo(render)
    self.obj.setColor(color)
    self.obj.setPos(SquarePos(square))

#Classes for each type of chess piece
#Obviously, we could have done this by just passing a string to Piece's init.
#But if you watned to make rules for how the pieces move, a good place to start
#would be to make an isValidMove(toSquare) method for each piece type
#and then check if the destination square is acceptible during ReleasePiece
class Pawn(Piece):
  model = "models/pawn"
class King(Piece):
  model = "models/king"
class Queen(Piece):
  model = "models/queen"
class Bishop(Piece):
  model = "models/bishop"
class Knight(Piece):
  model = "models/knight"
class Rook(Piece):
  model = "models/rook"

#Do the main initialization and start 3D rendering
w = World()
run()

