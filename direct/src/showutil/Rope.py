from pandac.PandaModules import *
import types

class Rope(NodePath):
    """
    This class defines a Nurbs curve whose control vertices are
    defined based on points relative to one or more nodes in space, so
    that the "rope" will animate as the nodes move around.  It uses
    the C++ RopeNode class to achieve fancy rendering effects like
    thick lines built from triangle strips.
    """

    showRope = base.config.GetBool('show-rope', 1)
    
    def __init__(self, name = 'Rope'):
        self.ropeNode = RopeNode(name)
        self.curve = NurbsCurveEvaluator()
        self.ropeNode.setCurve(self.curve)
        NodePath.__init__(self, self.ropeNode)
        self.name = name
        
    def setup(self, order, verts, knots = None):
        # This must be called to define the shape of the curve
        # initially, and may be called again as needed to adjust the
        # curve's properties.

        # order must be either 1, 2, 3, or 4, and is one more than the
        # degree of the curve; most NURBS curves are order 4.

        # verts is a list of (NodePath, point) tuples, defining the
        # control vertices of the curve.  For each control vertex, the
        # NodePath may refer to an arbitrary node in the scene graph,
        # indicating the point should be interpreted in the coordinate
        # space of that node (and it will automatically move when the
        # node is moved), or it may be the empty NodePath or None to
        # indicate the point should be interpreted in the coordinate
        # space of the Rope itself.  Each point value may be either a
        # 3-tuple or a 4-tuple (or a VBase3 or VBase4).  If it is a
        # 3-component vector, it represents a 3-d point in space; a
        # 4-component vector represents a point in 4-d homogeneous
        # space; that is to say, a 3-d point and an additional weight
        # factor (which should have been multiplied into the x y z
        # components).

        # knots is optional.  If specified, it should be a list of
        # floats, and should be of length len(verts) + order.  If it
        # is omitted, a default knot string is generated that consists
        # of the first (order - 1) and last (order - 1) values the
        # same, and the intermediate values incrementing by 1.
        
        self.order = order
        self.verts = verts
        self.knots = knots

        self.recompute()

    def recompute(self):
        # Recomputes the curve after its properties have changed.
        # Normally it is not necessary for the user to call this
        # directly.
        
        if not self.showRope:
            return
        numVerts = len(self.verts)
        self.curve.reset(numVerts)
        self.curve.setOrder(self.order)
        for i in range(numVerts):
            nodePath, point = self.verts[i]
            if isinstance(point, types.TupleType):
                if (len(point) >= 4):
                    self.curve.setVertex(i, VBase4(point[0], point[1], point[2], point[3]))
                else:
                    self.curve.setVertex(i, VBase3(point[0], point[1], point[2]))
            else:
                self.curve.setVertex(i, point)
            if nodePath:
                self.curve.setVertexSpace(i, nodePath)

        if self.knots != None:
            for i in range(len(self.knots)):
                self.curve.setKnot(i, self.knots[i])

        self.ropeNode.resetBound(self)

    def getPoints(self, len):
        # Returns a list of len points, evenly distributed in
        # parametric space on the rope, in the coordinate space of the
        # Rope itself.
        
        result = self.curve.evaluate(self)
        numPts = len
        ropePts = []
        for i in range(numPts):
            pt = Point3()
            result.evalPoint(i / float(numPts - 1), pt)
            ropePts.append(pt)
        return ropePts
