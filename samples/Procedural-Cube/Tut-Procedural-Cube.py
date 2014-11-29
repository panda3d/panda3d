# Author: Kwasi Mensah (kmensah@andrew.cmu.edu)
# Date: 8/02/2005
#
# This is meant to be a simple example of how to draw a cube
# using Panda's new Geom Interface. Quads arent directly supported 
# since they get broken down to trianlges anyway.
#

from direct.directbase import DirectStart
from direct.showbase.DirectObject import DirectObject
from direct.gui.DirectGui import *
from direct.interval.IntervalGlobal import *
from panda3d.core import lookAt
from panda3d.core import GeomVertexFormat, GeomVertexData
from panda3d.core import Geom, GeomTriangles, GeomVertexWriter
from panda3d.core import Texture, GeomNode
from panda3d.core import PerspectiveLens
from panda3d.core import CardMaker
from panda3d.core import Light, Spotlight
from panda3d.core import TextNode
from panda3d.core import Vec3, Vec4, Point3
import sys, os

base.disableMouse()
base.camera.setPos(0, -10, 0)

title = OnscreenText(text="Panda3D: Tutorial - Making a Cube Procedurally",
                       style=1, fg=(1,1,1,1),
                       pos=(0.5,-0.95), scale = .07)
escapeEvent = OnscreenText( 
 			 text="1: Set a Texture onto the Cube",
     			 style=1, fg=(1,1,1,1), pos=(-1.3, 0.95),
			 align=TextNode.ALeft, scale = .05)
spaceEvent = OnscreenText( 
 			 text="2: Toggle Light from the front On/Off",
     			 style=1, fg=(1,1,1,1), pos=(-1.3, 0.90),
			 align=TextNode.ALeft, scale = .05)
upDownEvent = OnscreenText( 
 			 text="3: Toggle Light from on top On/Off",
     			 style=1, fg=(1,1,1,1), pos=(-1.3, 0.85),
			 align=TextNode.ALeft, scale = .05)


#you cant normalize in-place so this is a helper function
def myNormalize(myVec):
	myVec.normalize()
	return myVec

#helper function to make a square given the Lower-Left-Hand and Upper-Right-Hand corners 
def makeSquare(x1,y1,z1, x2,y2,z2):
	format=GeomVertexFormat.getV3n3cpt2()
	vdata=GeomVertexData('square', format, Geom.UHDynamic)

	vertex=GeomVertexWriter(vdata, 'vertex')
	normal=GeomVertexWriter(vdata, 'normal')
	color=GeomVertexWriter(vdata, 'color')
	texcoord=GeomVertexWriter(vdata, 'texcoord')
	
	#make sure we draw the sqaure in the right plane
	if x1!=x2:
		vertex.addData3f(x1, y1, z1)
		vertex.addData3f(x2, y1, z1)
		vertex.addData3f(x2, y2, z2)
		vertex.addData3f(x1, y2, z2)

		normal.addData3f(myNormalize(Vec3(2*x1-1, 2*y1-1, 2*z1-1)))
		normal.addData3f(myNormalize(Vec3(2*x2-1, 2*y1-1, 2*z1-1)))
		normal.addData3f(myNormalize(Vec3(2*x2-1, 2*y2-1, 2*z2-1)))
		normal.addData3f(myNormalize(Vec3(2*x1-1, 2*y2-1, 2*z2-1)))
		
	else:
		vertex.addData3f(x1, y1, z1)
		vertex.addData3f(x2, y2, z1)
		vertex.addData3f(x2, y2, z2)
		vertex.addData3f(x1, y1, z2)

		normal.addData3f(myNormalize(Vec3(2*x1-1, 2*y1-1, 2*z1-1)))
		normal.addData3f(myNormalize(Vec3(2*x2-1, 2*y2-1, 2*z1-1)))
		normal.addData3f(myNormalize(Vec3(2*x2-1, 2*y2-1, 2*z2-1)))
		normal.addData3f(myNormalize(Vec3(2*x1-1, 2*y1-1, 2*z2-1)))

	#adding different colors to the vertex for visibility
	color.addData4f(1.0,0.0,0.0,1.0)
	color.addData4f(0.0,1.0,0.0,1.0)
	color.addData4f(0.0,0.0,1.0,1.0)
	color.addData4f(1.0,0.0,1.0,1.0)

	texcoord.addData2f(0.0, 1.0)
	texcoord.addData2f(0.0, 0.0)
	texcoord.addData2f(1.0, 0.0)
	texcoord.addData2f(1.0, 1.0)

	#quads arent directly supported by the Geom interface
	#you might be interested in the CardMaker class if you are
	#interested in rectangle though
	tri1=GeomTriangles(Geom.UHDynamic)
	tri2=GeomTriangles(Geom.UHDynamic)

	tri1.addVertex(0)
	tri1.addVertex(1)
	tri1.addVertex(3)

	tri2.addConsecutiveVertices(1,3)

	tri1.closePrimitive()
	tri2.closePrimitive()


	square=Geom(vdata)
	square.addPrimitive(tri1)
	square.addPrimitive(tri2)
	
	return square

square0=makeSquare(-1,-1,-1, 1,-1, 1)
square1=makeSquare(-1, 1,-1, 1, 1, 1)
square2=makeSquare(-1, 1, 1, 1,-1, 1)
square3=makeSquare(-1, 1,-1, 1,-1,-1)
square4=makeSquare(-1,-1,-1,-1, 1, 1)
square5=makeSquare( 1,-1,-1, 1, 1, 1)
snode=GeomNode('square')
snode.addGeom(square0)
snode.addGeom(square1)
snode.addGeom(square2)
snode.addGeom(square3)
snode.addGeom(square4)
snode.addGeom(square5)

cube=render.attachNewNode(snode)
cube.hprInterval(1.5,Point3(360,360,360)).loop()

#OpenGl by default only draws "front faces" (polygons whose vertices are specified CCW).
cube.setTwoSided(True)

class MyTapper(DirectObject):
	def __init__(self):
		self.testTexture=loader.loadTexture("maps/envir-reeds.png")
		self.accept("1", self.toggleTex)
		self.accept("2", self.toggleLightsSide)
		self.accept("3", self.toggleLightsUp)
		
		self.LightsOn=False
		self.LightsOn1=False
		slight = Spotlight('slight')
		slight.setColor(Vec4(1, 1, 1, 1))
		lens = PerspectiveLens()
		slight.setLens(lens)
		self.slnp = render.attachNewNode(slight)
		self.slnp1= render.attachNewNode(slight)
		
	def toggleTex(self):
		global cube
		if cube.hasTexture():
			cube.setTextureOff(1)
		else:
			cube.setTexture(self.testTexture)
		
	def toggleLightsSide(self):
		global cube
		self.LightsOn=not(self.LightsOn)
		
		if self.LightsOn:
			render.setLight(self.slnp)
			self.slnp.setPos(cube, 10,-400,0)
			self.slnp.lookAt(Point3(10, 0, 0))
		else:
			render.setLightOff(self.slnp)

	def toggleLightsUp(self):
		global cube
		self.LightsOn1=not(self.LightsOn1)
		
		if self.LightsOn1:
			render.setLight(self.slnp1)
			self.slnp1.setPos(cube, 10,0,400)
			self.slnp1.lookAt(Point3(10, 0, 0))
		else:
			render.setLightOff(self.slnp1)
		
		
			
			

t=MyTapper()

run()





