from PandaObject import *
import math

class LineNodePath(NodePath):
    def __init__(self, parent = hidden, **kw):

        # Initialize the superclass
        NodePath.__init__(self)

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

    def setVertexColor( self, vertex, *_args ):
        apply( self.lineSegs.setVertexColor, (vertex,) + _args )

    def getCurrentPosition( self ):
        return self.lineSegs.getCurrentPosition()

    def getNumVertices( self ):
        return self.lineSegs.getNumVertices()

    def getVertex( self ):
        return self.lineSegs.getVertex()

    def getVertexColor( self ):
        return self.lineSegs.getVertexColor()
