"""AttribDesc.py module: contains the AttribDesc class"""

class AttribDesc:
    """
    Entity attribute descriptor
    name == name of attribute
    """
    def __init__(self, name):
        self.name = name
    def getName(self):
        return self.name
    def __str__(self):
        return self.name
    def __repr__(self):
        return "AttribDesc('%s')" % self.name
