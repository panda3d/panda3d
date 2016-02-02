from panda3d.core import *
from math import *

GEO_ID = 0

def circleX(angle, radius, centerX, centerY):
    x = radius * cos(angle) + centerX
    return x

def circleY(angle, radius, centerX, centerY):
    y = radius * sin(angle) + centerY
    return y

def getCirclePoints(segCount, centerX, centerY, radius, wideX= 1.0, wideY = 1.0):
    returnShape = []
    for seg in range(0, segCount):
        coordX = wideX * (circleX(((pi * 2.0) * float(float(seg) / float(segCount))), radius, centerX, centerY))
        coordY = wideY * (circleY(((pi * 2.0) * float(float(seg) / float(segCount))), radius, centerX, centerY))
        returnShape.append((coordX, coordY, 1))

    coordX =  wideX * (circleX(((pi * 2.0) * float(0 / segCount)), radius, centerX, centerY))
    coordY =  wideY * (circleY(((pi * 2.0) * float(0 / segCount)), radius, centerX, centerY))
    returnShape.append((coordX, coordY, 1))
    return returnShape


def addCircle(attachNode, vertexCount, radius, color = Vec4(1.0, 1.0, 1.0, 1.0), centerColor = None, layer = 0):
    targetGN=GeomNode("target Circle")
    if centerColor == None:
        centerColor = color
    zFloat = 0.025
    targetCircleShape = getCirclePoints(5 + (vertexCount), 0.0, 0.0, radius)
    gFormat = GeomVertexFormat.getV3cp()
    targetCircleVertexData = GeomVertexData("holds my vertices", gFormat, Geom.UHDynamic)
    targetCircleVertexWriter = GeomVertexWriter(targetCircleVertexData, "vertex")
    targetCircleColorWriter = GeomVertexWriter(targetCircleVertexData, "color")
    targetCircleVertexWriter.addData3f(0.0, 0.0, zFloat) #center
    targetCircleColorWriter.addData4f(centerColor[0], centerColor[1], centerColor[2], centerColor[3])

    for vertex in targetCircleShape:
        targetCircleVertexWriter.addData3f(0.0 + vertex[0] , 0.0 + vertex[1] , zFloat)
        targetCircleColorWriter.addData4f(color[0], color[1], color[2], color[3])
        #targetCircleColorWriter.addData4f(1.0, 1.0, 1.0, 1.0)

    targetTris=GeomTrifans(Geom.UHStatic) # triangle object

    sizeTarget = len(targetCircleShape)
    targetTris.addVertex(0)
    for countVertex in range(1, sizeTarget + 1):
        targetTris.addVertex(countVertex)
    targetTris.addVertex(1)
    targetTris.closePrimitive()

    targetGeom=Geom(targetCircleVertexData)
    targetGeom.addPrimitive(targetTris)
    attachNode.addGeom(targetGeom)
    return targetGeom

def addCircleGeom(rootNode,  vertexCount, radius, color = Vec4(1.0, 1.0, 1.0, 1.0), centerColor = None, layer = 0):
     global GEO_ID
     GN=GeomNode("Circle %s" % (GEO_ID))
     GEO_ID += 1
     NodePathGeom = rootNode.attachNewNode(GN)
     geo = addCircle(GN, vertexCount, radius, color, centerColor,layer)
     return NodePathGeom, GN, geo

def addSquare(attachNode, sizeX, sizeY, color = Vec4(1.0, 1.0, 1.0, 1.0), layer = 0):
    targetGN=GeomNode("Square Geom")
    sX = sizeX / 2.0
    sY = sizeY / 2.0

    color1 = color
    color2 = color
    color3 = color

    gFormat = GeomVertexFormat.getV3n3cpt2()
    boxVertexData = GeomVertexData("vertices", gFormat, Geom.UHDynamic)

    boxVertexWriter = GeomVertexWriter(boxVertexData, "vertex")
    boxNormalWriter = GeomVertexWriter(boxVertexData, "normal")
    boxColorWriter = GeomVertexWriter(boxVertexData, "color")
    boxTextureWriter = GeomVertexWriter(boxVertexData, "texcoord")


    boxVertexWriter.addData3f(-sX, sY, 0.0)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])
    boxTextureWriter.addData2f(0.0, 1.0)

    boxVertexWriter.addData3f(-sX, -sY, 0.0)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])
    boxTextureWriter.addData2f(0.0, 0.0)

    boxVertexWriter.addData3f(sX, -sY, 0.0)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])
    boxTextureWriter.addData2f(1.0, 0.0)

    boxVertexWriter.addData3f(sX, sY, 0.0)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])
    boxTextureWriter.addData2f(1.0, 1.0)

    boxTris=GeomTristrips(Geom.UHStatic) # trianglestrip obejcet

    boxTris.addVertex(1)
    boxTris.addVertex(2)
    boxTris.addVertex(0)
    boxTris.addVertex(3)
    boxTris.closePrimitive()

    boxGeom=Geom(boxVertexData)
    boxGeom.addPrimitive(boxTris)


    attachNode.addGeom(boxGeom)
    return boxGeom

def addSquareGeom(rootNode, sizeX, sizeY, color = Vec4(1.0, 1.0, 1.0, 1.0), layer = 0):
     global GEO_ID
     GN=GeomNode("Square %s" % (GEO_ID))
     GEO_ID += 1
     NodePathGeom = rootNode.attachNewNode(GN)
     geo = addSquare(GN, sizeX, sizeY, color, layer)
     return NodePathGeom, GN, geo

def addBox(attachNode, sizeX, sizeY, sizeZ, color = Vec4(1.0, 1.0, 1.0, 1.0), darken = 0):
    targetGN=GeomNode("Box Geom")
    sX = sizeX / 2.0
    sY = sizeY / 2.0
    sZ = sizeZ / 2.0

    color1 = color
    color2 = color
    color3 = color
    if darken:
        color1 = color * 0.75 #Vec4(0.0, 0.0, 0.0, 1.0)
        color2 = color * 0.50 #Vec4(0.0, 0.0, 0.0, 1.0)
        color3 = color * 0.25 #Vec4(0.0, 0.0, 0.0, 1.0)

    gFormat = GeomVertexFormat.getV3n3cp()
    boxVertexData = GeomVertexData("vertices", gFormat, Geom.UHDynamic)

    boxVertexWriter = GeomVertexWriter(boxVertexData, "vertex")
    boxNormalWriter = GeomVertexWriter(boxVertexData, "normal")
    boxColorWriter = GeomVertexWriter(boxVertexData, "color")


    #Front

    boxVertexWriter.addData3f(sX, sY, sZ)
    boxNormalWriter.addData3f(0, 1, 0)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])

    boxVertexWriter.addData3f(sX, sY, -sZ)
    boxNormalWriter.addData3f(0, 1, 0)
    boxColorWriter.addData4f(color1[0], color1[1], color1[2], color1[3])

    boxVertexWriter.addData3f(-sX, sY, -sZ)
    boxNormalWriter.addData3f(0, 1, 0)
    boxColorWriter.addData4f(color1[0], color1[1], color1[2], color1[3])

    boxVertexWriter.addData3f(-sX, sY, sZ)
    boxNormalWriter.addData3f(0, 1, 0)
    boxColorWriter.addData4f(color1[0], color1[1], color1[2], color1[3])

    #Back

    boxVertexWriter.addData3f(-sX, -sY, sZ)
    boxNormalWriter.addData3f(0, -1, 0)
    boxColorWriter.addData4f(color2[0], color2[1], color2[2], color2[3])

    boxVertexWriter.addData3f(-sX, -sY, -sZ)
    boxNormalWriter.addData3f(0, -1, 0)
    boxColorWriter.addData4f(color3[0], color3[1], color3[2], color3[3])

    boxVertexWriter.addData3f(sX, -sY, -sZ)
    boxNormalWriter.addData3f(0, -1, 0)
    boxColorWriter.addData4f(color2[0], color2[1], color2[2], color2[3])

    boxVertexWriter.addData3f(sX, -sY, sZ)
    boxNormalWriter.addData3f(0, -1, 0)
    boxColorWriter.addData4f(color2[0], color2[1], color2[2], color2[3])

    #Top

    boxVertexWriter.addData3f(-sX, sY, sZ)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])

    boxVertexWriter.addData3f(-sX, -sY, sZ)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])

    boxVertexWriter.addData3f(sX, -sY, sZ)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])

    boxVertexWriter.addData3f(sX, sY, sZ)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])

    #Bottom

    boxVertexWriter.addData3f(sX, sY, -sZ)
    boxNormalWriter.addData3f(0, 0, -1)
    boxColorWriter.addData4f(color3[0], color3[1], color3[2], color3[3])

    boxVertexWriter.addData3f(sX, -sY, -sZ)
    boxNormalWriter.addData3f(0, 0, -1)
    boxColorWriter.addData4f(color3[0], color3[1], color3[2], color3[3])

    boxVertexWriter.addData3f(-sX, -sY, -sZ)
    boxNormalWriter.addData3f(0, 0, -1)
    boxColorWriter.addData4f(color3[0], color3[1], color3[2], color3[3])

    boxVertexWriter.addData3f(-sX, sY, -sZ)
    boxNormalWriter.addData3f(0, 0, -1)
    boxColorWriter.addData4f(color3[0], color3[1], color3[2], color3[3])

    #Right

    boxVertexWriter.addData3f(sX, sY, sZ)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])

    boxVertexWriter.addData3f(sX, -sY, sZ)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color1[0], color1[1], color1[2], color1[3])

    boxVertexWriter.addData3f(sX, -sY, -sZ)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color1[0], color1[1], color1[2], color1[3])

    boxVertexWriter.addData3f(sX, sY, -sZ)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color1[0], color1[1], color1[2], color1[3])


    #Left

    boxVertexWriter.addData3f(-sX, sY, -sZ)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color2[0], color2[1], color2[2], color2[3])

    boxVertexWriter.addData3f(-sX, -sY, -sZ)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color3[0], color3[1], color3[2], color3[3])

    boxVertexWriter.addData3f(-sX, -sY, sZ)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color2[0], color2[1], color2[2], color2[3])

    boxVertexWriter.addData3f(-sX, sY, sZ)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color2[0], color2[1], color2[2], color2[3])


    boxTris=GeomTristrips(Geom.UHStatic) # trianglestrip obejcet
    boxTris.addVertex(0)#(1)
    boxTris.addVertex(1)#(2)
    boxTris.addVertex(3)#(0)
    boxTris.addVertex(2)#(3)
    boxTris.closePrimitive()

    boxTris.addVertex(5)
    boxTris.addVertex(6)
    boxTris.addVertex(4)
    boxTris.addVertex(7)
    boxTris.closePrimitive()

    boxTris.addVertex(9)
    boxTris.addVertex(10)
    boxTris.addVertex(8)
    boxTris.addVertex(11)
    boxTris.closePrimitive()

    boxTris.addVertex(13)
    boxTris.addVertex(14)
    boxTris.addVertex(12)
    boxTris.addVertex(15)
    boxTris.closePrimitive()

    boxTris.addVertex(16) #(17)
    boxTris.addVertex(17) #(18)
    boxTris.addVertex(19) #(16)
    boxTris.addVertex(18) #(19)
    boxTris.closePrimitive()

    boxTris.addVertex(21)
    boxTris.addVertex(22)
    boxTris.addVertex(20)
    boxTris.addVertex(23)
    boxTris.closePrimitive()

    boxGeom=Geom(boxVertexData)
    boxGeom.addPrimitive(boxTris)


    attachNode.addGeom(boxGeom)
    return boxGeom

def addBoxGeom(rootNode, sizeX, sizeY, sizeZ, color = Vec4(1.0, 1.0, 1.0, 1.0), darken = 0):
     global GEO_ID
     GN=GeomNode("Box %s" % (GEO_ID))
     GEO_ID += 1
     nodePathGeom = rootNode.attachNewNode(GN)
     geo = addBox(GN, sizeX, sizeY, sizeZ, color, darken)
     return nodePathGeom, GN, geo


def addArrow(attachNode, sizeX, sizeY, color = Vec4(1.0, 1.0, 1.0, 1.0), layer = 0):
    targetGN=GeomNode("Arrow Geom")
    sX = sizeX / 2.0
    sY = sizeY / 2.0

    color1 = color
    color2 = color
    color3 = color

    gFormat = GeomVertexFormat.getV3n3cp()
    boxVertexData = GeomVertexData("vertices", gFormat, Geom.UHDynamic)

    boxVertexWriter = GeomVertexWriter(boxVertexData, "vertex")
    boxNormalWriter = GeomVertexWriter(boxVertexData, "normal")
    boxColorWriter = GeomVertexWriter(boxVertexData, "color")


    boxVertexWriter.addData3f(-sX, sY, 0.0)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])

    boxVertexWriter.addData3f(-sX, -sY, 0.0)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])

    boxVertexWriter.addData3f(sX, -sY, 0.0)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])

    boxVertexWriter.addData3f(sX, sY, 0.0)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])

    boxTris=GeomTristrips(Geom.UHStatic) # trianglestrip obejcet

    boxTris.addVertex(1)
    boxTris.addVertex(2)
    boxTris.addVertex(0)
    boxTris.addVertex(3)
    boxTris.closePrimitive()

    boxVertexWriter.addData3f(-sX * 2.0, sY, 0.0)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])

    boxVertexWriter.addData3f(sX * 2.0, sY, 0.0)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])

    boxVertexWriter.addData3f(0.0, sY * 2.0, 0.0)
    boxNormalWriter.addData3f(0, 0, 1)
    boxColorWriter.addData4f(color[0], color[1], color[2], color[3])

    boxTris.addVertex(4)
    boxTris.addVertex(5)
    boxTris.addVertex(6)
    boxTris.closePrimitive()


    boxGeom=Geom(boxVertexData)
    boxGeom.addPrimitive(boxTris)


    attachNode.addGeom(boxGeom)
    return boxGeom

def addArrowGeom(rootNode, sizeX, sizeY, color = Vec4(1.0, 1.0, 1.0, 1.0), layer = 0):
     global GEO_ID
     GN=GeomNode("Arrow %s" % (GEO_ID))
     GEO_ID += 1
     NodePathGeom = rootNode.attachNewNode(GN)
     geo = addArrow(GN, sizeX, sizeY, color, layer)
     return NodePathGeom, GN, geo
