
    """
    NurbsCurveEvaluator-extensions module: contains methods to extend
    functionality of the NurbsCurveEvaluator class
    """

    def getKnots(self):
        """Returns the knot vector as a Python list of floats"""
        knots = []
        for i in range(self.getNumKnots()):
            knots.append(self.getKnot(i))
        return knots

    def getVertices(self, relTo = None):
        """Returns the vertices as a Python list of Vec4's, relative
        to the indicated space if given."""
        
        verts = []
        if relTo:
            for i in range(self.getNumVertices()):
                verts.append(self.getVertex(i, relTo))
        else:
            for i in range(self.getNumVertices()):
                verts.append(self.getVertex(i))
        return verts

