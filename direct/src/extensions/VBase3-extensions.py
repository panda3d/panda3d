
    """
    VBase3-extensions module: contains methods to extend functionality
    of the VBase3 class
    """

    def __repr__(self):
        return '%s(%s,%s,%s)' % (
            self.__class__.__name__,self[0],self[1],self[2])
