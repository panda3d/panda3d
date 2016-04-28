
from panda3d.core import *
from .DirectGlobals import *
from .DirectUtil import *
import math

class LineNodePath(NodePath):
    def __init__(self, parent = None, name = None,
                 thickness = 1.0, colorVec = VBase4(1)):

        # Initialize the superclass
        NodePath.__init__(self)

        if parent is None:
            parent = hidden

        # Attach a geomNode to the parent and set self to be
        # the resulting node path
        self.lineNode = GeomNode("lineNode")
        self.assign(parent.attachNewNode(self.lineNode))
        if name:
            self.setName(name)

        # Create a lineSegs object to hold the line
        ls = self.lineSegs = LineSegs()
        # Initialize the lineSegs parameters
        ls.setThickness(thickness)
        ls.setColor(colorVec)

    def moveTo(self, *_args):
        self.lineSegs.moveTo(*_args)

    def drawTo(self, *_args):
        self.lineSegs.drawTo(*_args)

    def create(self, frameAccurate = 0):
        self.lineSegs.create(self.lineNode, frameAccurate)

    def reset(self):
        self.lineSegs.reset()
        self.lineNode.removeAllGeoms()

    def isEmpty(self):
        return self.lineSegs.isEmpty()

    def setThickness(self, thickness):
        self.lineSegs.setThickness(thickness)

    def setColor(self, *_args):
        self.lineSegs.setColor(*_args)

    def setVertex(self, *_args):
        self.lineSegs.setVertex(*_args)

    def setVertexColor(self, vertex, *_args):
        self.lineSegs.setVertexColor(*(vertex,) + _args)

    def getCurrentPosition(self):
        return self.lineSegs.getCurrentPosition()

    def getNumVertices(self):
        return self.lineSegs.getNumVertices()

    def getVertex(self, index):
        return self.lineSegs.getVertex(index)

    def getVertexColor(self):
        return self.lineSegs.getVertexColor()

    def drawArrow(self, sv, ev, arrowAngle, arrowLength):
        """
        Do the work of moving the cursor around to draw an arrow from
        sv to ev. Hack: the arrows take the z value of the end point
        """
        self.moveTo(sv)
        self.drawTo(ev)
        v = sv - ev
        # Find the angle of the line
        angle = math.atan2(v[1], v[0])
        # Get the arrow angles
        a1 = angle + deg2Rad(arrowAngle)
        a2 = angle - deg2Rad(arrowAngle)
        # Get the arrow points
        a1x = arrowLength * math.cos(a1)
        a1y = arrowLength * math.sin(a1)
        a2x = arrowLength * math.cos(a2)
        a2y = arrowLength * math.sin(a2)
        z = ev[2]
        self.moveTo(ev)
        self.drawTo(Point3(ev + Point3(a1x, a1y, z)))
        self.moveTo(ev)
        self.drawTo(Point3(ev + Point3(a2x, a2y, z)))

    def drawArrow2d(self, sv, ev, arrowAngle, arrowLength):
        """
        Do the work of moving the cursor around to draw an arrow from
        sv to ev. Hack: the arrows take the z value of the end point
        """
        self.moveTo(sv)
        self.drawTo(ev)
        v = sv - ev
        # Find the angle of the line
        angle = math.atan2(v[2], v[0])
        # Get the arrow angles
        a1 = angle + deg2Rad(arrowAngle)
        a2 = angle - deg2Rad(arrowAngle)
        # Get the arrow points
        a1x = arrowLength * math.cos(a1)
        a1y = arrowLength * math.sin(a1)
        a2x = arrowLength * math.cos(a2)
        a2y = arrowLength * math.sin(a2)
        self.moveTo(ev)
        self.drawTo(Point3(ev + Point3(a1x, 0.0, a1y)))
        self.moveTo(ev)
        self.drawTo(Point3(ev + Point3(a2x, 0.0, a2y)))

    def drawLines(self, lineList):
        """
        Given a list of lists of points, draw a separate line for each list
        """
        for pointList in lineList:
            self.moveTo(*pointList[0])
            for point in pointList[1:]:
                self.drawTo(*point)

##
## Given a point in space, and a direction, find the point of intersection
## of that ray with a plane at the specified origin, with the specified normal
def planeIntersect (lineOrigin, lineDir, planeOrigin, normal):
    t = 0
    offset = planeOrigin - lineOrigin
    t = offset.dot(normal) / lineDir.dot(normal)
    hitPt = lineDir * t
    return hitPt + lineOrigin

def getNearProjectionPoint(nodePath):
    # Find the position of the projection of the specified node path
    # on the near plane
    origin = nodePath.getPos(base.direct.camera)
    # project this onto near plane
    if origin[1] != 0.0:
        return origin * (base.direct.dr.near / origin[1])
    else:
        # Object is coplaner with camera, just return something reasonable
        return Point3(0, base.direct.dr.near, 0)

def getScreenXY(nodePath):
    # Where does the node path's projection fall on the near plane
    nearVec = getNearProjectionPoint(nodePath)
    # Clamp these coordinates to visible screen
    nearX = CLAMP(nearVec[0], base.direct.dr.left, base.direct.dr.right)
    nearY = CLAMP(nearVec[2], base.direct.dr.bottom, base.direct.dr.top)
    # What percentage of the distance across the screen is this?
    percentX = (nearX - base.direct.dr.left)/base.direct.dr.nearWidth
    percentY = (nearY - base.direct.dr.bottom)/base.direct.dr.nearHeight
    # Map this percentage to the same -1 to 1 space as the mouse
    screenXY = Vec3((2 * percentX) - 1.0, nearVec[1], (2 * percentY) - 1.0)
    # Return the resulting value
    return screenXY

def getCrankAngle(center):
    # Used to compute current angle of mouse (relative to the coa's
    # origin) in screen space
    x = base.direct.dr.mouseX - center[0]
    y = base.direct.dr.mouseY - center[2]
    return (180 + rad2Deg(math.atan2(y, x)))

def relHpr(nodePath, base, h, p, r):
    # Compute nodePath2newNodePath relative to base coordinate system
    # nodePath2base
    mNodePath2Base = nodePath.getMat(base)
    # delta scale, orientation, and position matrix
    mBase2NewBase = Mat4(Mat4.identMat()) # [gjeon] fixed to give required argument
    composeMatrix(mBase2NewBase, UNIT_VEC, VBase3(h, p, r), ZERO_VEC,
                  CSDefault)
    # base2nodePath
    mBase2NodePath = base.getMat(nodePath)
    # nodePath2 Parent
    mNodePath2Parent = nodePath.getMat()
    # Compose the result
    resultMat = mNodePath2Base * mBase2NewBase
    resultMat = resultMat * mBase2NodePath
    resultMat = resultMat * mNodePath2Parent
    # Extract and apply the hpr
    hpr = Vec3(0)
    decomposeMatrix(resultMat, VBase3(), hpr, VBase3(),
                    CSDefault)
    nodePath.setHpr(hpr)

# Quaternion interpolation
def qSlerp(startQuat, endQuat, t):
    startQ = Quat(startQuat)
    destQuat = Quat(Quat.identQuat())
    # Calc dot product
    cosOmega = (startQ.getI() * endQuat.getI() +
                startQ.getJ() * endQuat.getJ() +
                startQ.getK() * endQuat.getK() +
                startQ.getR() * endQuat.getR())
    # If the above dot product is negative, it would be better to
    # go between the negative of the initial and the final, so that
    # we take the shorter path.
    if cosOmega < 0.0:
        cosOmega *= -1
        startQ.setI(-1 * startQ.getI())
        startQ.setJ(-1 * startQ.getJ())
        startQ.setK(-1 * startQ.getK())
        startQ.setR(-1 * startQ.getR())
    if ((1.0 + cosOmega) > Q_EPSILON):
        # usual case
        if ((1.0 - cosOmega) > Q_EPSILON):
            # usual case
            omega = math.acos(cosOmega)
            sinOmega = math.sin(omega)
            startScale = math.sin((1.0 - t) * omega)/sinOmega
            endScale = math.sin(t * omega)/sinOmega
        else:
            # ends very close
            startScale = 1.0 - t
            endScale = t
        destQuat.setI(startScale * startQ.getI() +
                      endScale * endQuat.getI())
        destQuat.setJ(startScale * startQ.getJ() +
                      endScale * endQuat.getJ())
        destQuat.setK(startScale * startQ.getK() +
                      endScale * endQuat.getK())
        destQuat.setR(startScale * startQ.getR() +
                      endScale * endQuat.getR())
    else:
        # ends nearly opposite
        destQuat.setI(-startQ.getJ())
        destQuat.setJ(startQ.getI())
        destQuat.setK(-startQ.getR())
        destQuat.setR(startQ.getK())
        startScale = math.sin((0.5 - t) * math.pi)
        endScale = math.sin(t * math.pi)
        destQuat.setI(startScale * startQ.getI() +
                      endScale * endQuat.getI())
        destQuat.setJ(startScale * startQ.getJ() +
                      endScale * endQuat.getJ())
        destQuat.setK(startScale * startQ.getK() +
                      endScale * endQuat.getK())
    return destQuat

