
    """
    NodePathCollection-extensions module: contains methods to extend
    functionality of the NodePathCollection class
    """

    # For iterating over children
    def asList(self):
        """Converts a NodePathCollection into a list"""
        if self.isEmpty():
            return []
        else:
            npList = []
            for nodePathIndex in range(self.getNumPaths()):
                npList.append(self.getPath(nodePathIndex))
            return npList
