"""OnscreenGeom module: contains the OnscreenGeom class"""

__all__ = ['OnscreenGeom']

from pandac.PandaModules import *
import DirectGuiGlobals as DGG
from direct.showbase.DirectObject import DirectObject
import string,types

class OnscreenGeom(DirectObject, NodePath):
    def __init__(self, geom = None,
                 pos = None,
                 hpr = None,
                 scale = None,
                 color = None,
                 parent = None,
                 sort = 0):
        """
        Make a geom node from string or a node path,
        put it into the 2d sg and set it up with all the indicated parameters.

        The parameters are as follows:

          geom: the actual geometry to display or a file name.
                This may be omitted and specified later via setGeom()
                if you don't have it available.

          pos: the x, y, z position of the geometry on the screen.
               This maybe a 3-tuple of floats or a vector.
               y should be zero

          hpr: the h, p, r of the geometry on the screen.
               This maybe a 3-tuple of floats or a vector.

          scale: the size of the geometry.  This may either be a single
                 float, a 3-tuple of floats, or a vector, specifying a
                 different x, y, z scale.  y should be 1

          color: the (r, g, b, a) color of the geometry.  This is
                 normally a 4-tuple of floats or ints.

          parent: the NodePath to parent the geometry to initially.
        """
        # We ARE a node path.  Initially, we're an empty node path.
        NodePath.__init__(self)
        if parent == None:
            parent = aspect2d

        self.setGeom(geom, parent = parent, sort = sort, color = color)

        # Adjust pose
        # Set pos
        if (isinstance(pos, types.TupleType) or
            isinstance(pos, types.ListType)):
            apply(self.setPos, pos)
        elif isinstance(pos, VBase3):
            self.setPos(pos)
        # Hpr
        if (isinstance(hpr, types.TupleType) or
            isinstance(hpr, types.ListType)):
            apply(self.setHpr, hpr)
        elif isinstance(hpr, VBase3):
            self.setPos(hpr)
        # Scale
        if (isinstance(scale, types.TupleType) or
            isinstance(scale, types.ListType)):
            apply(self.setScale, scale)
        elif isinstance(scale, VBase3):
            self.setPos(scale)
        elif (isinstance(scale, types.FloatType) or
              isinstance(scale, types.IntType)):
            self.setScale(scale)

    def setGeom(self, geom,
                parent = NodePath(),
                transform = None,
                sort = 0,
                color = None):
        # Get the original parent, transform, and sort, if any, so we can
        # preserve them across this call.
        if not self.isEmpty():
            parent = self.getParent()
            if transform == None:
                # If we're replacing a previous image, we throw away
                # the new image's transform in favor of the original
                # image's transform.
                transform = self.getTransform()
            sort = self.getSort()
            if color == None and self.hasColor():
                color = self.getColor()

        self.removeNode()

        # Assign geometry
        if isinstance(geom, NodePath):
            self.assign(geom.copyTo(parent, sort))
        elif isinstance(geom, types.StringTypes):
            self.assign(loader.loadModel(geom))
            self.reparentTo(parent, sort)

        if not self.isEmpty():
            if transform:
                self.setTransform(transform.compose(self.getTransform()))

            # Set color, if specified
            if color:
                self.setColor(color[0], color[1], color[2], color[3])

    def getGeom(self):
        return self

    def configure(self, option=None, **kw):
        for option, value in kw.items():
            # Use option string to access setter function
            try:
                setter = getattr(self, 'set' + option[0].upper() + option[1:])
                if (((setter == self.setPos) or
                     (setter == self.setHpr) or
                     (setter == self.setScale)) and
                    (isinstance(value, types.TupleType) or
                     isinstance(value, types.ListType))):
                    apply(setter, value)
                else:
                    setter(value)
            except AttributeError:
                print 'OnscreenText.configure: invalid option:', option

    # Allow index style references
    def __setitem__(self, key, value):
        apply(self.configure, (), {key: value})

    def cget(self, option):
        # Get current configuration setting.
        # This is for compatibility with DirectGui functions
        getter = getattr(self, 'get' + option[0].upper() + option[1:])
        return getter()

    # Allow index style refererences
    __getitem__ = cget

    def destroy(self):
        self.removeNode()
