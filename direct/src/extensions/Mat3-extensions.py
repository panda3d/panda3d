
    """
    Mat3-extensions module: contains methods to extend functionality
    of the LMatrix3f class.
    """

    def __repr__(self):
        return '%s(\n%s,\n%s,\n%s)' % (
            self.__class__.__name__, self.getRow(0).pPrintValues(), self.getRow(1).pPrintValues(), self.getRow(2).pPrintValues())

    def pPrintValues(self):
        """
        Pretty print
        """
        return "\n%s\n%s\n%s" % (
            self.getRow(0).pPrintValues(), self.getRow(1).pPrintValues(), self.getRow(2).pPrintValues())
