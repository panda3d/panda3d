"""A DirectFrame is a basic DirectGUI component that acts as the base
class for various other components, and can also serve as a basic
container to hold other DirectGUI components.

A DirectFrame can have:

* A background texture (pass in path to image, or Texture Card)
* A midground geometry item (pass in geometry)
* A foreground text Node (pass in text string or OnscreenText)

Each of these has 1 or more states.  The same object can be used for
all states or each state can have a different text/geom/image (for
radio button and check button indicators, for example).
"""

__all__ = ['DirectFrame']

from panda3d.core import *
from . import DirectGuiGlobals as DGG
from .DirectGuiBase import *
from .OnscreenImage import OnscreenImage
from .OnscreenGeom import OnscreenGeom
import sys

if sys.version_info >= (3, 0):
    stringType = str
else:
    stringType = basestring


class DirectFrame(DirectGuiWidget):
    DefDynGroups = ('text', 'geom', 'image')
    def __init__(self, parent = None, **kw):
        # Inherits from DirectGuiWidget
        optiondefs = (
            # Define type of DirectGuiWidget
            ('pgFunc',          PGItem,     None),
            ('numStates',       1,          None),
            ('state',           self.inactiveInitState, None),
            # Frame can have:
            # A background texture
            ('image',           None,       self.setImage),
            # A midground geometry item
            ('geom',            None,       self.setGeom),
            # A foreground text node
            ('text',            None,       self.setText),
            # Change default value of text mayChange flag from 0
            # (OnscreenText.py) to 1
            ('textMayChange',  1,          None),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs,
                           dynamicGroups = DirectFrame.DefDynGroups)

        # Initialize superclasses
        DirectGuiWidget.__init__(self, parent)

        # Call option initialization functions
        self.initialiseoptions(DirectFrame)

    def destroy(self):
        DirectGuiWidget.destroy(self)

    def clearText(self):
        self['text'] = None
        self.setText()

    def setText(self, text=None):
        if text is not None:
            self['text'] = text

        # Determine if user passed in single string or a sequence
        if self['text'] == None:
            textList = (None,) * self['numStates']
        elif isinstance(self['text'], stringType):
            # If just passing in a single string, make a tuple out of it
            textList = (self['text'],) * self['numStates']
        else:
            # Otherwise, hope that the user has passed in a tuple/list
            textList = self['text']
        # Create/destroy components
        for i in range(self['numStates']):
            component = 'text' + repr(i)
            # If fewer items specified than numStates,
            # just repeat last item
            try:
                text = textList[i]
            except IndexError:
                text = textList[-1]

            if self.hascomponent(component):
                if text == None:
                    # Destroy component
                    self.destroycomponent(component)
                else:
                    self[component + '_text'] = text
            else:
                if text == None:
                    return
                else:
                    from .OnscreenText import OnscreenText
                    self.createcomponent(
                        component, (), 'text',
                        OnscreenText,
                        (), parent = self.stateNodePath[i],
                        text = text, scale = 1, mayChange = self['textMayChange'],
                        sort = DGG.TEXT_SORT_INDEX,
                        )

    def clearGeom(self):
        self['geom'] = None
        self.setGeom()

    def setGeom(self, geom=None):
        if geom is not None:
            self['geom'] = geom

        # Determine argument type
        geom = self['geom']

        if geom == None:
            # Passed in None
            geomList = (None,) * self['numStates']
        elif isinstance(geom, NodePath) or \
             isinstance(geom, stringType):
            # Passed in a single node path, make a tuple out of it
            geomList = (geom,) * self['numStates']
        else:
            # Otherwise, hope that the user has passed in a tuple/list
            geomList = geom

        # Create/destroy components
        for i in range(self['numStates']):
            component = 'geom' + repr(i)
            # If fewer items specified than numStates,
            # just repeat last item
            try:
                geom = geomList[i]
            except IndexError:
                geom = geomList[-1]

            if self.hascomponent(component):
                if geom == None:
                    # Destroy component
                    self.destroycomponent(component)
                else:
                    self[component + '_geom'] = geom
            else:
                if geom == None:
                    return
                else:
                    self.createcomponent(
                        component, (), 'geom',
                        OnscreenGeom,
                        (), parent = self.stateNodePath[i],
                        geom = geom, scale = 1,
                        sort = DGG.GEOM_SORT_INDEX)

    def clearImage(self):
        self['image'] = None
        self.setImage()

    def setImage(self, image=None):
        if image is not None:
            self['image'] = image

        # Determine argument type
        arg = self['image']
        if arg == None:
            # Passed in None
            imageList = (None,) * self['numStates']
        elif isinstance(arg, NodePath) or \
             isinstance(arg, Texture) or \
             isinstance(arg, stringType):
            # Passed in a single node path, make a tuple out of it
            imageList = (arg,) * self['numStates']
        else:
            # Otherwise, hope that the user has passed in a tuple/list
            if ((len(arg) == 2) and
                isinstance(arg[0], stringType) and
                isinstance(arg[1], stringType)):
                # Its a model/node pair of strings
                imageList = (arg,) * self['numStates']
            else:
                # Assume its a list of node paths
                imageList = arg
        # Create/destroy components
        for i in range(self['numStates']):
            component = 'image' + repr(i)
            # If fewer items specified than numStates,
            # just repeat last item
            try:
                image = imageList[i]
            except IndexError:
                image = imageList[-1]

            if self.hascomponent(component):
                if image == None:
                    # Destroy component
                    self.destroycomponent(component)
                else:
                    self[component + '_image'] = image
            else:
                if image == None:
                    return
                else:
                    self.createcomponent(
                        component, (), 'image',
                        OnscreenImage,
                        (), parent = self.stateNodePath[i],
                        image = image, scale = 1,
                        sort = DGG.IMAGE_SORT_INDEX)
