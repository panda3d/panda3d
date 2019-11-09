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

See the :ref:`directframe` page in the programming manual for a more in-depth
explanation and an example of how to use this class.
"""

__all__ = ['DirectFrame']

from panda3d.core import *
from . import DirectGuiGlobals as DGG
from .DirectGuiBase import *
from .OnscreenImage import OnscreenImage
from .OnscreenGeom import OnscreenGeom
from .OnscreenText import OnscreenText
import sys

if sys.version_info >= (3, 0):
    stringType = str
else:
    stringType = basestring


class DirectFrame(DirectGuiWidget):
    DefDynGroups = ('text', 'geom', 'image')

    def __init__(self, parent=None, **kw):
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
                           dynamicGroups=DirectFrame.DefDynGroups)

        # Initialize superclasses
        DirectGuiWidget.__init__(self, parent)

        # Call option initialization functions
        self.initialiseoptions(DirectFrame)

    def clearText(self):
        self['text'] = None
        self.setText()

    def setText(self, text=None):
        self.new_set("text", text)

    def clearGeom(self):
        self['geom'] = None
        self.setGeom()

    def long_type_check(self, name, object_):
        if isinstance(object_, stringType):
            return True
        if name == "image":
            if ((isinstance(object_, NodePath) or
                isinstance(object_, Texture)) 
                or
                ((len(object_) == 2) and# Its a model/node pair of strings
                isinstance(object_[0], stringType) and
                isinstance(object_[1], stringType))):
                return True
        elif name == "geom":
            if (isinstance(object_, NodePath)):
                return True

    def new_set(self, name, object_=None):
        """my new code golf par is 1"""
        assert name in ["geom", "image", "text"]
        if object_ is not None:
            self[name] = object_

        object_ = self[name]

        if object_ == None:
            object_list = (None,) * self['numStates']
        else:
            if self.long_type_check(name, object_):
                object_list = (object_,) * self['numStates']
            else:
                object_list = object_

        name_based_classes = {
            "geom": OnscreenGeom,
            "image": OnscreenImage,
            "text": OnscreenText, }

        name_based_constants = {
            "geom": {
                "scale": 1,
                "sort": DGG.GEOM_SORT_INDEX, # this is "20"
            },
            "image": {
                "scale": 1,
                "sort": DGG.IMAGE_SORT_INDEX, # this is "10"
            },
            "text": {
                "scale": 1,
                "mayChange": self['textMayChange'],
                "sort": DGG.TEXT_SORT_INDEX, # this is "30"
            },
        }
        # constants should be local to or default arguments of constructors

        c = 0
        m = self["numStates"]
        for c in range(self['numStates']):
            component_name = name+str(c)
            comp_input = object_list[c]
            name_based_constants[name][name] = comp_input

            if self.hascomponent(component_name):
                if comp_input == None:
                    self.destroycomponent(component_name)
                else:
                    self[component_name+"_"+name] = comp_input
            else:
                if comp_input == None:
                    return
                self.createcomponent(
                    component_name,
                    (),
                    name,
                    name_based_classes[name],  # class type, positional
                    (),
                    parent=self.stateNodePath[c],
                    **name_based_constants[name]
                )  # keywords, order irrelevant
            c += 1

    def setGeom(self, geom=None):
        self.new_set("geom", geom)

    def clearImage(self):
        self['image'] = None
        self.setImage()

    def setImage(self, image=None):
        self.new_set("image", image)
