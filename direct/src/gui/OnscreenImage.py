"""OnscreenImage module: contains the OnscreenImage class.

See the :ref:`onscreenimage` page in the programming manual for explanation of
this class.
"""

__all__ = ['OnscreenImage']

from panda3d.core import *
from direct.showbase.DirectObject import DirectObject
import sys

if sys.version_info >= (3, 0):
    stringType = str
else:
    stringType = basestring


class OnscreenImage(DirectObject, NodePath):
    def __init__(self, image = None,
                 pos = None,
                 hpr = None,
                 scale = None,
                 color = None,
                 parent = None,
                 sort = 0):
        """
        Make a image node from string or a `~panda3d.core.NodePath`, put
        it into the 2-D scene graph and set it up with all the indicated
        parameters.

        Parameters:

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

        if parent is None:
            from direct.showbase import ShowBaseGlobal
            parent = ShowBaseGlobal.aspect2d
        self.setImage(image, parent = parent, sort = sort)

        # Adjust pose
        # Set pos
        if (isinstance(pos, tuple) or
            isinstance(pos, list)):
            self.setPos(*pos)
        elif isinstance(pos, VBase3):
            self.setPos(pos)
        # Hpr
        if (isinstance(hpr, tuple) or
            isinstance(hpr, list)):
            self.setHpr(*hpr)
        elif isinstance(hpr, VBase3):
            self.setHpr(hpr)
        # Scale
        if (isinstance(scale, tuple) or
            isinstance(scale, list)):
            self.setScale(*scale)
        elif isinstance(scale, VBase3):
            self.setScale(scale)
        elif (isinstance(scale, float) or
              isinstance(scale, int)):
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
        elif isinstance(image, stringType) or \
             isinstance(image, Texture):
            if isinstance(image, Texture):
                # It's a Texture
                tex = image
            else:
                # It's a Texture file name
                tex = TexturePool.loadTexture(image)
                if not tex:
                    raise IOError('Could not load texture: %s' % (image))
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
                    print('OnscreenImage: node %s not found' % image[1])
            else:
                print('OnscreenImage: model %s not found' % image[0])

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
                    (isinstance(value, tuple) or
                     isinstance(value, list))):
                    setter(*value)
                else:
                    setter(value)
            except AttributeError:
                print('OnscreenImage.configure: invalid option: %s' % option)

    # Allow index style references
    def __setitem__(self, key, value):
        self.configure(*(), **{key: value})

    def cget(self, option):
        # Get current configuration setting.
        # This is for compatibility with DirectGui functions
        getter = getattr(self, 'get' + option[0].upper() + option[1:])
        return getter()

    # Allow index style refererences
    __getitem__ = cget

    def destroy(self):
        self.removeNode()
