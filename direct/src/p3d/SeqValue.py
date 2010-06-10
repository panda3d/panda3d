class SeqValue:

    """ This represents a sequence value read from a contents.xml
    file, either from the <contents> or the <package> section.  It's
    represented as series of dotted integers in the xml file, and
    stored internally as a tuple of integers.

    It may be incremented, which increments only the last integer in
    the series; or it may be compared with another SeqValue, which
    compares all of the integers componentwise. """

    def __init__(self, *value):
        self.value = value
        if not self.value:
            self.value = (0,)

    def loadXml(self, xelement):
        """ Reads the seq from the indicated XML element.  Returns
        true if loaded, false if not given or if there was an
        error. """

        self.value = (0,)
        value = xelement.Attribute('seq')
        if value:
            value = value.split('.')
            try:
                value = map(int, value)
            except ValueError:
                value = None
        if value:
            self.value = tuple(value)
            return True
        return False


    def storeXml(self, xelement):
        """ Adds the seq to the indicated XML element. """
        value = '.'.join(map(str, self.value))
        xelement.SetAttribute('seq', value)

    def __add__(self, inc):
        """ Increments the seq value, returning the new value. """
        value = self.value[:-1] + (self.value[-1] + inc,)
        return SeqValue(*value)

    def __cmp__(self, other):
        """ Compares to another seq value. """
        return cmp(self.value, other.value)

    def __str__(self):
        return 'SeqValue%s' % (repr(self.value))
    
