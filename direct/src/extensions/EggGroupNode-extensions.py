
    # For iterating over children
    def getChildren(self):
        """Returns a Python list of the egg node's children."""
        result = []
        child = self.getFirstChild()
        while (child != None):
            result.append(child)
            child = self.getNextChild()
        return result
    
