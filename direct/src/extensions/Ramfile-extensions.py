
    """
    Ramfile-extensions module: contains methods to extend functionality
    of the Ramfile class
    """

    def readlines(self):
        """Reads all the lines at once and returns a list."""
        lines = []
        line = self.readline()
        while line:
            lines.append(line)
            line = self.readline()
        return lines
    
