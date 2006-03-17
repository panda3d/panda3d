
    """
    VBase4-extensions module: contains methods to extend functionality
    of the VBase4 class
    """

    def __repr__(self):
        return '%s(%s, %s, %s, %s)' % (
            self.__class__.__name__, self[0], self[1], self[2], self[3])

    def pPrintValues(self):
        """
        Pretty print
        """
        return '% 10.4f, % 10.4f, % 10.4f, % 10.4f' % (self[0], self[1], self[2], self[3])
