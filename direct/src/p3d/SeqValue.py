__all__ = ["SeqValue"]

import types

class SeqValue:

    """ This represents a sequence value read from a contents.xml
    file, either from the <contents> or the <package> section.  It's
    represented as series of dotted integers in the xml file, and
    stored internally as a tuple of integers.

    It may be incremented, which increments only the last integer in
    the series; or it may be compared with another SeqValue, which
    compares all of the integers componentwise. """

    def __init__(self, value = None):
        self.value = ()
        if value is not None:
            self.set(value)

    def set(self, value):
        """ Sets the seq from the indicated value of unspecified
        type. """
        if isinstance(value, types.TupleType):
            self.setFromTuple(value)
        elif isinstance(value, types.StringTypes):
            self.setFromString(value)
        else:
            raise TypeError, 'Invalid sequence type: %s' % (value,)

    def setFromTuple(self, value):
        """ Sets the seq from the indicated tuple of integers. """
        assert isinstance(value, types.TupleType)
        self.value = value

    def setFromString(self, value):
        """ Sets the seq from the indicated string of dot-separated
        integers.  Raises ValueError on error. """
        assert isinstance(value, types.StringTypes)
        
        self.value = ()
        if value:
            value = value.split('.')
            value = map(int, value)
            self.value = tuple(value)

    def loadXml(self, xelement, attribute = 'seq'):
        """ Reads the seq from the indicated XML element.  Returns
        true if loaded, false if not given or if there was an
        error. """

        self.value = ()
        value = xelement.Attribute(attribute)
        if value:
            try:
                self.setFromString(value)
            except ValueError:
                return False
            return True

        return False


    def storeXml(self, xelement, attribute = 'seq'):
        """ Adds the seq to the indicated XML element. """
        if self.value:
            value = '.'.join(map(str, self.value))
            xelement.SetAttribute(attribute, value)

    def __add__(self, inc):
        """ Increments the seq value, returning the new value. """
        if not self.value:
            value = (1,)
        else:
            value = self.value[:-1] + (self.value[-1] + inc,)
        return SeqValue(value)

    def __cmp__(self, other):
        """ Compares to another seq value. """
        return cmp(self.value, other.value)

    def __bool__(self):
        return bool(self.value)

    def __str__(self):
        return 'SeqValue%s' % (repr(self.value))
    
