from PandaObject import *

class DirectGrid(NodePath):
    def __init__(self, parent = hidden):
        NodePath.__init__(self)
        self.assign(parent.attachNewNode( NamedNode('grid') ))

