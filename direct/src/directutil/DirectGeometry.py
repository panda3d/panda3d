from PandaModules import *
from PandaObject import *

X_AXIS = Vec3(1,0,0)
Y_AXIS = Vec3(0,1,0)
Z_AXIS = Vec3(0,0,1)
NEG_X_AXIS = Vec3(-1,0,0)
NEG_Y_AXIS = Vec3(0,-1,0)
NEG_Z_AXIS = Vec3(0,0,-1)
ZERO_VEC = ORIGIN = Vec3(0)
UNIT_VEC = Vec3(1)
ZERO_POINT = Point3(0)

class LineNodePath(NodePath):
    def __init__(self, parent = None, **kw):

        # Initialize the superclass
        NodePath.__init__(self)

        if parent is None:
            parent = hidden

        # Attach a geomNode to the parent and set self to be
        # the resulting node path
        self.lineNode = GeomNode()
        self.assign(parent.attachNewNode( self.lineNode ))

        # Create a lineSegs object to hold the line
        ls = self.lineSegs = LineSegs()
        # Initialize the lineSegs parameters
        ls.setThickness( kw.get('thickness', 1.0) )
        ls.setColor( kw.get('colorVec', VBase4(1.0)) )

    def moveTo( self, *_args ):
        apply( self.lineSegs.moveTo, _args )

    def drawTo( self, *_args ):
        apply( self.lineSegs.drawTo, _args )

    def create( self, frameAccurate = 0 ):
        self.lineSegs.create( self.lineNode, frameAccurate )

    def reset( self ):
        self.lineSegs.reset()
        self.lineNode.clear()

    def isEmpty( self ):
        return self.lineSegs.isEmpty()

    def setThickness( self, thickness ):
        self.lineSegs.setThickness( thickness )

    def setColor( self, *_args ):
        apply( self.lineSegs.setColor, _args )

    def setVertex( self, *_args):
        apply( self.lineSegs.setVertex, _args )

    def setVertexColor( self, vertex, *_args ):
        apply( self.lineSegs.setVertexColor, (vertex,) + _args )

    def getCurrentPosition( self ):
        return self.lineSegs.getCurrentPosition()

    def getNumVertices( self ):
        return self.lineSegs.getNumVertices()

    def getVertex( self, index ):
        return self.lineSegs.getVertex(index)

    def getVertexColor( self ):
        return self.lineSegs.getVertexColor()


##
## Given a point in space, and a direction, find the point of intersection
## of that ray with a plane at the specified origin, with the specified normal
def planeIntersect (lineOrigin, lineDir, planeOrigin, normal):
    t = 0
    offset = planeOrigin - lineOrigin
    t = offset.dot(normal) / lineDir.dot(normal)
    hitPt = lineDir * t
    return hitPt + lineOrigin

def ROUND_TO(value, divisor):
    return round(value/float(divisor)) * divisor
def ROUND_INT(val):
    return int(round(val))

# Set direct drawing style for an object
# Never light object or draw in wireframe
def useDirectRenderStyle(nodePath):
    nodePath.getBottomArc().setTransition(LightTransition.allOff())
    nodePath.setRenderModeFilled()
