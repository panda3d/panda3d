from PandaObject import *

class DirectGrid(NodePath):
    def __init__(self, parent = None):
        NodePath.__init__(self)
        if parent is None:
            parent = hidden
        self.assign(parent.attachNewNode( NamedNode('grid') ))

