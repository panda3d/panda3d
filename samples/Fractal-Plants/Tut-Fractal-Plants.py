#Author: Kwasi Mensah (kmensah@andrew.cmu.edu)
#Date: 8/05/2005
#
#This demo shows how to make quasi-fractal trees in Panda.
#Its primarily meant to be a more complex example on how to use
#Panda's Geom interface.
#
#


from direct.directbase import DirectStart
from panda3d.core import Filename,InternalName
from panda3d.core import GeomVertexArrayFormat, GeomVertexFormat
from panda3d.core import Geom, GeomNode, GeomTrifans, GeomTristrips
from panda3d.core import GeomVertexReader, GeomVertexWriter
from panda3d.core import GeomVertexRewriter, GeomVertexData
from panda3d.core import PerspectiveLens, TextNode
from panda3d.core import TransformState,CullFaceAttrib
from panda3d.core import Light,AmbientLight,Spotlight
from panda3d.core import NodePath
from panda3d.core import Vec3,Vec4,Mat4
from direct.task.Task import Task
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
import math, random, time, sys, os


random.seed()
base.disableMouse()
base.camera.setPos(0,-180,30)
numPrimitives=0

title = OnscreenText(text="Panda3D: Tutorial - Procdurally Making a Tree",
                       style=1, fg=(1,1,1,1),
                       pos=(0.6,-0.95), scale = .07)
qEvent = OnscreenText( 
 			 text="Q: Start Scene Over",
     			 style=1, fg=(1,1,1,1), pos=(-1.3, 0.95),
			 align=TextNode.ALeft, scale = .05)
wEvent = OnscreenText( 
 			 text="W: Add Another Tree",
     			 style=1, fg=(1,1,1,1), pos=(-1.3, 0.90),
			 align=TextNode.ALeft, scale = .05)


#this is a helper function you can use to make a circle in the x-y plane
#i didnt end up needing it but this comes up fairly often so I thought
#I should keep this in the code. Feel free to use.

def makeCircle(vdata, numVertices=40,offset=Vec3(0,0,0), direction=1):
	circleGeom=Geom(vdata)

	vertWriter=GeomVertexWriter(vdata, "vertex")
	normalWriter=GeomVertexWriter(vdata, "normal")
	colorWriter=GeomVertexWriter(vdata, "color")
	uvWriter=GeomVertexWriter(vdata, "texcoord")
	drawWriter=GeomVertexWriter(vdata, "drawFlag")

	#make sure we start at the end of the GeomVertexData so we dont overwrite anything
	#that might be there already
	startRow=vdata.getNumRows()

	vertWriter.setRow(startRow)
	colorWriter.setRow(startRow)
	uvWriter.setRow(startRow)
	normalWriter.setRow(startRow)
	drawWriter.setRow(startRow)

	angle=2*math.pi/numVertices
	currAngle=angle

	for i in range(numVertices):
		position=Vec3(math.cos(currAngle)+offset.getX(), math.sin(currAngle)+offset.getY(),offset.getZ())
		vertWriter.addData3f(position)
		uvWriter.addData2f(position.getX()/2.0+0.5,position.getY()/2.0+0.5)
		colorWriter.addData4f(1.0, 1.0, 1.0, 1.0)
		position.setZ(position.getZ()*direction)
		position.normalize()
		normalWriter.addData3f(position)
		
		#at default Opengl only draws "front faces" (all shapes whose vertices are arranged CCW). We
		#need direction so we can specify which side we want to be the front face
		currAngle+=angle*direction

	circle=GeomTrifans(Geom.UHStatic)
	circle.addConsecutiveVertices(startRow, numVertices)
	circle.closePrimitive()

	circleGeom.addPrimitive(circle)
	
	return circleGeom

#Another helper function that I thought was to useful too throw away. Enjoy.
def makeCylinder(vdata,numVertices=40):
	topCircleGeom=makeCircle(vdata, numVertices,Vec3(0,0, 1))
	bottomCircleGeom=makeCircle(vdata, numVertices,Vec3(0,0,0),-1)
	
	
	body=GeomTristrips(Geom.UHStatic)
	
	j=40
	i=0
	while i < numVertices+1:
		body.addVertex(i)
		body.addVertex(j)
		i+=1
		if j==40:
			j=2*numVertices-1
		else:
			j-=1
		body.addVertex(i)
		body.addVertex(j)
		j-=1
		i+=1
	
	body.addVertex(numVertices-1)
	body.addVertex(0)
	body.addVertex(numVertices)
	body.closePrimitive()
	#print body
		
	

	cylinderGeom=Geom(vdata)
	
	cylinderGeom.addPrimitive(body)
	cylinderGeom.copyPrimitivesFrom(topCircleGeom)
	cylinderGeom.copyPrimitivesFrom(bottomCircleGeom)

	
	cylinderGeom.decomposeInPlace()
	cylinderGeom.unifyInPlace()
	return cylinderGeom


#this computes the new Axis which we'll make a branch grow alowng when we split
def randomAxis(vecList):
	fwd=vecList[0]
	perp1=vecList[1]	
	perp2=vecList[2]

	nfwd=fwd+perp1*(2*random.random()-1) + perp2*(2*random.random()-1)
	nfwd.normalize()
	
	nperp2=nfwd.cross(perp1)
	nperp2.normalize()
	
	nperp1=nfwd.cross(nperp2)
	nperp1.normalize()

	return [nfwd, nperp1, nperp2]
	

#this makes smalle variations in direction when we are growing a branch but not splitting
def smallRandomAxis(vecList):		
	fwd=vecList[0]
	perp1=vecList[1]
	perp2=vecList[2]
	
	nfwd=fwd+perp1*(1*random.random()-0.5) + perp2*(1*random.random()-0.5)
	nfwd.normalize()
	
	nperp2=nfwd.cross(perp1)
	nperp2.normalize()
	
	nperp1=nfwd.cross(nperp2)
	nperp1.normalize()

	return [nfwd, nperp1, nperp2]



#this draws the body of the tree. This draws a ring of vertices and connects the rings with
#triangles to form the body.
#this keepDrawing paramter tells the function wheter or not we're at an end
#if the vertices before you were an end, dont draw branches to it
def drawBody(nodePath, vdata, pos, vecList, radius=1, keepDrawing=True,numVertices=8):

	circleGeom=Geom(vdata)

	vertWriter=GeomVertexWriter(vdata, "vertex")
	colorWriter=GeomVertexWriter(vdata, "color")
	normalWriter=GeomVertexWriter(vdata, "normal")
	drawReWriter=GeomVertexRewriter(vdata, "drawFlag")
	texReWriter=GeomVertexRewriter(vdata, "texcoord")
	
	
	startRow=vdata.getNumRows()
	vertWriter.setRow(startRow)
	colorWriter.setRow(startRow)
	normalWriter.setRow(startRow)
	
	sCoord=0

	if (startRow!=0):
		texReWriter.setRow(startRow-numVertices)
		sCoord=texReWriter.getData2f().getX()+1
		
		drawReWriter.setRow(startRow-numVertices)
		if(drawReWriter.getData1f()==False):
			sCoord-=1
	
	drawReWriter.setRow(startRow)
	texReWriter.setRow(startRow)	
	
	angleSlice=2*math.pi/numVertices
	currAngle=0
			
	#axisAdj=Mat4.rotateMat(45, axis)*Mat4.scaleMat(radius)*Mat4.translateMat(pos)

	perp1=vecList[1]
	perp2=vecList[2]	

	#vertex information is written here
	for i in range(numVertices):
		adjCircle=pos+(perp1*math.cos(currAngle)+perp2*math.sin(currAngle))*radius
		normal=perp1*math.cos(currAngle)+perp2*math.sin(currAngle)		
		normalWriter.addData3f(normal)
		vertWriter.addData3f(adjCircle)
		texReWriter.addData2f(sCoord,(i+0.001)/(numVertices-1))
		colorWriter.addData4f(0.5,0.5,0.5,1)
		drawReWriter.addData1f(keepDrawing)
		currAngle+=angleSlice

	
	drawReader=GeomVertexReader(vdata, "drawFlag")
	drawReader.setRow(startRow-numVertices)

	#we cant draw quads directly so we use Tristrips
	if (startRow!=0) & (drawReader.getData1f()!=False):
		lines=GeomTristrips(Geom.UHStatic)
		half=int(numVertices*0.5)
		for i in range(numVertices):
			lines.addVertex(i+startRow)
			if i< half:
				lines.addVertex(i+startRow-half)
			else:
				lines.addVertex(i+startRow-half-numVertices)

		lines.addVertex(startRow)
		lines.addVertex(startRow-half)
		lines.closePrimitive()
		lines.decompose()
		circleGeom.addPrimitive(lines)
		

		circleGeomNode=GeomNode("Debug")
		circleGeomNode.addGeom(circleGeom)

		#I accidentally made the front-face face inwards. Make reverse makes the tree render properly and
			#should cause any surprises to any poor programmer that tries to use this code
		circleGeomNode.setAttrib(CullFaceAttrib.makeReverse(),1)
		global numPrimitives
		numPrimitives+=numVertices*2
	
		nodePath.attachNewNode(circleGeomNode)
	
#this draws leafs when we reach an end		
def drawLeaf(nodePath,vdata,pos=Vec3(0,0,0),vecList=[Vec3(0,0,1), Vec3(1,0,0),Vec3(0,-1,0)], scale=0.125):
	
	#use the vectors that describe the direction the branch grows to make the right 
		#rotation matrix
	newCs=Mat4(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)
	newCs.setRow(0, vecList[2]) #right
	newCs.setRow(1, vecList[1]) #up
	newCs.setRow(2, vecList[0]) #forward
	newCs.setRow(3, Vec3(0,0,0))
	newCs.setCol(3,Vec4(0,0,0,1))

	axisAdj=Mat4.scaleMat(scale)*newCs*Mat4.translateMat(pos)	
	
	#orginlly made the leaf out of geometry but that didnt look good
	#I also think there should be a better way to handle the leaf texture other than
	#hardcoding the filename
	leafModel=loader.loadModel('models/shrubbery')
	leafTexture=loader.loadTexture('models/material-10-cl.png')


	leafModel.reparentTo(nodePath)
	leafModel.setTexture(leafTexture,1)
	leafModel.setTransform(TransformState.makeMat(axisAdj))

#recursive algorthim to make the tree
def makeFractalTree(bodydata, nodePath,length, pos=Vec3(0,0,0), numIterations=11, numCopies=4,vecList=[Vec3(0,0,1),Vec3(1,0,0), Vec3(0,-1,0)]):
	if numIterations>0:

		drawBody(nodePath, bodydata, pos, vecList, length.getX())

		
		#move foward along the right axis
		newPos=pos+vecList[0]*length.length()

	
		#only branch every third level (sorta)
		if numIterations%3==0:
			#decrease dimensions when we branch			
			length=Vec3(length.getX()/2, length.getY()/2, length.getZ()/1.1)
			for i in range(numCopies):
				makeFractalTree(bodydata, nodePath,length,newPos, numIterations-1, numCopies,randomAxis(vecList))
		else:
			#just make another branch connected to this one with a small variation in direction
			makeFractalTree(bodydata, nodePath,length,newPos, numIterations-1,numCopies,smallRandomAxis(vecList))

	else:
		drawBody(nodePath,bodydata, pos, vecList, length.getX(),False)
		drawLeaf(nodePath,bodydata, pos,vecList)



alight = AmbientLight('alight')
alight.setColor(Vec4(0.5, 0.5, 0.5, 1))
alnp = render.attachNewNode(alight)
render.setLight(alnp)

slight = Spotlight('slight')
slight.setColor(Vec4(1, 1, 1, 1))
lens = PerspectiveLens()
slight.setLens(lens)
slnp = render.attachNewNode(slight)
render.setLight(slnp)

slnp.setPos(0, 0,40)

#rotating light to show that normals are calculated correctly
def updateLight(task):
	global slnp
	currPos=slnp.getPos()
	currPos.setX(100*math.cos(task.time)/2)
	currPos.setY(100*math.sin(task.time)/2)
	slnp.setPos(currPos)

	
		
	slnp.lookAt(render)
	return Task.cont	

taskMgr.add(updateLight, "rotating Light")

#add some interactivity to the program
class MyTapper(DirectObject):
	def __init__(self):
		formatArray=GeomVertexArrayFormat()
		formatArray.addColumn(InternalName.make("drawFlag"), 1, Geom.NTUint8, Geom.COther)

		format=GeomVertexFormat(GeomVertexFormat.getV3n3cpt2())
		format.addArray(formatArray)
		self.format=GeomVertexFormat.registerFormat(format)

		bodydata=GeomVertexData("body vertices", format, Geom.UHStatic)

		self.barkTexture=loader.loadTexture("barkTexture.jpg")
		treeNodePath=NodePath("Tree Holder")
		makeFractalTree(bodydata,treeNodePath,Vec3(4,4,7))

		treeNodePath.setTexture(self.barkTexture,1)
		treeNodePath.reparentTo(render)

		self.accept("q", self.regenTree)
		self.accept("w", self.addTree)
		self.accept("arrow_up", self.upIterations)
		self.accept("arrow_down", self.downIterations)
		self.accept("arrow_right", self.upCopies)
		self.accept("arrow_left", self.downCopies)

		self.numIterations=11
		self.numCopies=4
		
		
		self.upDownEvent = OnscreenText( 
 			 text="Up/Down: Increase/Decrease the number of iterations ("+str(self.numIterations)+")",
     			 style=1, fg=(1,1,1,1), pos=(-1.3, 0.85),
			 align=TextNode.ALeft, scale = .05, mayChange=True)
		
		self.leftRightEvent = OnscreenText( 
 			 text="Left/Right: Increase/Decrease branching ("+str(self.numCopies)+")",
     			 style=1, fg=(1,1,1,1), pos=(-1.3, 0.80),
			 align=TextNode.ALeft, scale = .05, mayChange=True)
		
		
	def upIterations(self):
		self.numIterations+=1
		self.upDownEvent.setText("Up/Down: Increase/Decrease the number of iterations ("+str(self.numIterations)+")")

	def downIterations(self):
		self.numIterations-=1	
		self.upDownEvent.setText("Up/Down: Increase/Decrease the number of Iteratations("+str(self.numIterations)+")")

	def upCopies(self):
		self.numCopies+=1
		self.leftRightEvent.setText("Left/Right: Increase/Decrease branching("+str(self.numCopies)+")")

	def downCopies(self):
		self.numCopies-=1
		self.leftRightEvent.setText("Left/Right: Increase/Decrease branching("+str(self.numCopies)+")")

	def regenTree(self):
		forest=	render.findAllMatches("Tree Holder")
		forest.detach()


		bodydata=GeomVertexData("body vertices", self.format, Geom.UHStatic)
		
		treeNodePath=NodePath("Tree Holder")
		makeFractalTree(bodydata, treeNodePath,Vec3(4,4,7), Vec3(0,0,0),self.numIterations, self.numCopies)

		treeNodePath.setTexture(self.barkTexture,1)
		treeNodePath.reparentTo(render)

	def addTree(self):	

		bodydata=GeomVertexData("body vertices", self.format, Geom.UHStatic)
		
		randomPlace=Vec3(200*random.random()-100, 200*random.random()-100, 0)
		#randomPlace.normalize()
	

		treeNodePath=NodePath("Tree Holder")
		makeFractalTree(bodydata, treeNodePath,Vec3(4,4,7), randomPlace, self.numIterations, self.numCopies)

		treeNodePath.setTexture(self.barkTexture,1)
		treeNodePath.reparentTo(render)

t=MyTapper()
print numPrimitives


run()

