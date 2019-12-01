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
        if text is not None:
            self["text"] = text

        text = self["text"]
        component_class = OnscreenText
        component_kwargs ={
                "scale": 1,
                "mayChange": self['textMayChange'],
                "sort": DGG.TEXT_SORT_INDEX, # this is "30"
        }
        
        if text is None:
            text_list = (None,) * self['numStates']
        else:
            if self.inputTypeValidation("text", text):#used by button and directmenu
                text_list = (text,) * self['numStates']
            else:
                text_list = text
        
        self.reinitComponent("text", text_list, component_kwargs, component_class)

    def clearGeom(self):
        self['geom'] = None
        self.setGeom()

    def inputTypeValidation(self, name, object_):
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
        return False

    def reinitComponent(self, name, object_list, component_kwargs,component_class):
        """set function common code"""
        assert name in ("geom", "image", "text")
        
        if len(object_list) != self['numStates']:
            raise ValueError
        
        # constants should be local to or default arguments of constructors
        for c in range(self['numStates']):
            component_name = name + str(c)
            comp_input = object_list[c]
                        
            if self.hascomponent(component_name):
                if comp_input is None:
                    self.destroycomponent(component_name)
                else:
                    self[component_name+"_"+name] = comp_input
            else:
                if comp_input is None:
                    return
                    
                self.createcomponent(
                    component_name,
                    (),
                    name,
                    component_class,
                    (),
                    parent=self.stateNodePath[c],
                    **component_kwargs
                )

    def setGeom(self, geom=None):
        if geom is not None:
            self["geom"] = geom

        geom = self["geom"]
        component_class = OnscreenGeom
        component_kwargs = {
                "scale": 1,
                "sort": DGG.GEOM_SORT_INDEX, # this is "20"
            }
        
        if geom is None:
            geom_list = (None,) * self['numStates']
        else:
            if self.inputTypeValidation(name, geom):
                geom_list = (geom,) * self['numStates']
            else:
                geom_list = geom
        self.reinitComponent("geom", geom_list, component_kwargs, component_class)

    def clearImage(self):
        self['image'] = None
        self.setImage()

    def setImage(self, image=None):
        
        if image is not None:
            self["image"] = image

        image = self["image"]
        component_class=OnscreenImage,
        component_kwargs = {
                "scale": 1,
                "sort": DGG.IMAGE_SORT_INDEX, # this is "10"
            }
            
        if image is None:
            image_list = (None,) * self['numStates']
        else:
            if self.inputTypeValidation("image", image):#used by button and directmenu
                image_list = (image,) * self['numStates']
            else:
                image_list = image
                
        self.reinitComponent("image", image_list, component_kwargs, component_class)
