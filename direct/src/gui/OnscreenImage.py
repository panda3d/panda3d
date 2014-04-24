"""OnscreenImage module: contains the OnscreenImage class"""

__all__ = ['OnscreenImage']

from pandac.PandaModules import *
import DirectGuiGlobals as DGG
from direct.showbase.DirectObject import DirectObject
import string,types

class OnscreenImage(DirectObject, NodePath):
    def __init__(self, image = None,
                 pos = None,
                 hpr = None,
                 scale = None,
                 color = None,
                 parent = None,
                 sort = 0):
        """
        Make a image node from string or a node path,
        put it into the 2d sg and set it up with all the indicated parameters.

        The parameters are as follows:

          image: the actual geometry to display or a file name.
                This may be omitted and specified later via setImage()
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
        self.setImage(image, parent = parent, sort = sort)

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
            self.setHpr(hpr)
        # Scale
        if (isinstance(scale, types.TupleType) or
            isinstance(scale, types.ListType)):
            apply(self.setScale, scale)
        elif isinstance(scale, VBase3):
            self.setScale(scale)
        elif (isinstance(scale, types.FloatType) or
              isinstance(scale, types.IntType)):
            self.setScale(scale)

        # Set color
        if color:
            # Set color, if specified
            self.setColor(color[0], color[1], color[2], color[3])

    def setImage(self, image,
                 parent = NodePath(),
                 transform = None,
                 sort = 0):
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

        self.removeNode()

        # Assign geometry
        if isinstance(image, NodePath):
            self.assign(image.copyTo(parent, sort))
        elif isinstance(image, types.StringTypes) or \
             isinstance(image, Texture):
            if isinstance(image, Texture):
                # It's a Texture
                tex = image
            else:
                # It's a Texture file name
                tex = loader.loadTexture(image)
            cm = CardMaker('OnscreenImage')
            cm.setFrame(-1, 1, -1, 1)
            self.assign(parent.attachNewNode(cm.generate(), sort))
            self.setTexture(tex)
        elif type(image) == type(()):
            # Assume its a file+node name, extract texture from node
            model = loader.loadModel(image[0])
            if model:
                node = model.find(image[1])
                if node:
                    self.assign(node.copyTo(parent, sort))
                else:
                    print 'OnscreenImage: node %s not found' % image[1]
            else:
                print 'OnscreenImage: model %s not found' % image[0]

        if transform and not self.isEmpty():
            self.setTransform(transform)

    def getImage(self):
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
                print 'OnscreenImage.configure: invalid option:', option

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
