
    # For iterating over vertices
    def getVertices(self):
        """Returns a Python list of the egg primitive's vertices."""
        result = []
        for i in range(self.getNumVertices()):
            result.append(self.getVertex(i))
        return result
