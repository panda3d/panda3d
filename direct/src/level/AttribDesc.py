"""AttribDesc.py module: contains the AttribDesc class"""

class AttribDesc:
    """
    Entity attribute descriptor
    name == name of attribute
    default == default value for attrib
    """
    def __init__(self, name, default, datatype='string'):
        self.name = name
        self.default = default
        self.datatype = datatype

    def getName(self):
        return self.name
    def getDefaultValue(self):
        return self.default
    def getDatatype(self):
        return self.datatype

    def __str__(self):
        return self.name
    def __repr__(self):
        return "AttribDesc(%s, %s, %s)" % (
            repr(self.name),
            repr(self.default),
            repr(self.datatype)
            )
