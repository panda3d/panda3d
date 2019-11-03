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

    #overloading completely unnecessary?
    #def destroy(self):
    #    self.destroy()
        #DirectGuiWidget.destroy(self)

    def clearText(self):
        self['text'] = None
        self.setText()

    def setText(self, text=None):
        self.new_set("text",text)
        
    def clearGeom(self):
        self['geom'] = None
        self.setGeom()
    
    def unset(self,name):
        """what set[Geom,Text,Image] would have done, if no image, text
        or geom are provided takes 
        name must be in ["geom","text","image"]"""
        
        if name not in ["geom","text","image"]:
            raise TypeError("name must be in ['geom','text','image']")
            
        #max number of states is 4 I think.
        c=0
        m=4
        while c < m:
            comp_name=name+str(c)
            if self.hascomponent(comp_name):
                self.destroycomponent()
            c+=1
    
    def long_type_check(self,name):
        """basically an elaborate type check."""
        object_ = self[name]
        if name=="image":
            if (isinstance(object_, NodePath) or 
                isinstance(object_, Texture) or 
                isinstance(object_, stringType)):
                return True
        elif name=="text":
            if isinstance(object_, stringType):
                return True
        elif name=="geom":
            if (isinstance(object_, NodePath) or 
                isinstance(object_, stringType)):
                return True
        else:
            raise TypeError("how did you get here?")
            
        return False
    
    def new_set(self,name,object_=None):
        """my new code golf par is 1"""
        assert name in ["geom","image","text"]
        if object_ is not None:
            self[name]=object_
        else:
            self.unset(name)
            
        object_=self[name]
        
        mul_bool = self.long_type_check(name)
        
        if mul_bool  == True:
            object_list = (object_,) * self["numStates"]
        elif mul_bool == False:
            object_list = object_
        else:
            raise ValueError
                
        name_based_classes={
        "geom":OnscreenGeom,
        "image":OnscreenImage,
        "text":OnscreenText,}
        
        name_based_constants={
            "geom":{
                    "geom":object_,
                    "sort":DGG.GEOM_SORT_INDEX,
                    },
            "imgage":{
                    "image":object_,
                    "sort":DGG.IMAGE_SORT_INDEX
                    },
            "text":{
                    "text":object_,
                    "mayChange":self['textMayChange'],
                    "sort":DGG.TEXT_SORT_INDEX
                    },
                }
        #Index is probably constant and only used at creation.
        #should be a local variable to the constructor.
        
        c=0
        m=self["numStates"]
        while c < m:
            component_name = name+str(c)
            comp_input = object_list[c]
            
            if self.hascomponent(component_name):
                self[component_name]=object_
            else:
                self.createcomponent(
                        component_name, 
                        (), 
                        name,
                        name_based_classes[name],#class type, positional
                        (),
                        parent = self.stateNodePath[c],
                        scale = 1,
                        **name_based_constants[name], #keywords, order irrelevant
                        )
            c+=1
    
    def setGeom(self, geom=None):
        self.new_set("geom",geom)
        
    def clearImage(self):
        self['image'] = None
        self.setImage()

    def setImage(self, image=None):
        self.new_set("image",image)
