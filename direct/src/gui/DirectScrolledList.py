from DirectFrame import *
from DirectButton import *
import GuiGlobals


"""
def choseAvatar(name, avId):
    print name, avId
    
model = loader.loadModel("phase_4/models/gui/friendslist_gui")

s = DirectScrolledList(
    image = model.find("**/FriendsBox_Open"),
    relief = None,
    # inc and dec are DirectButtons
    incButton_text = "inc",
    incButton_text_scale = 0.1,
    incButton_pos = (0,0,0.1),
    decButton_text = "dec",
    decButton_text_scale = 0.1,
    decButton_pos = (0,0,-0.1),
    # itemFrame is a DirectFrame
    itemFrame_pos = (0,0,0),
    itemFrame_relief = FLAT,
    itemFrame_frameColor = (1,1,1,1),
    # each item is a button with text on it
    numItemsVisible = 3,
    items = ('Top', 'Flippy', 'Joe', 'Flippy', 'Ashy', 'Bottom'),
    items_text_scale = 0.1,
    items_relief = FLAT,
    command = choseAvatar,
    extraArgs = ([1], [2], [3]),
    )

s.addItem(string, extraArg)
s.removeItem(index)
s.setItems(stringList, extraArgList)
"""
    

class DirectScrolledList(DirectFrame):
    def __init__(self, parent = guiTop, **kw):
        # Inherits from DirectFrame
        optiondefs = (
            # Define type of DirectGuiWidget
            ('items',           [],        None),
            ('command',         None,      None),
            ('extraArgs',       [],        None),
            ('numItemsVisible', 1,         None),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs, dynamicGroups = ("items",))

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        self.createcomponent("incButton", (), "incButton",
                             DirectButton, (),
                             parent = self,
                             )
        self.createcomponent("decButton", (), "decButton",
                             DirectButton, (),
                             parent = self,
                             )
        self.createcomponent("itemFrame", (), "itemFrame",
                             DirectFrame, (),
                             parent = self,
                             )

        for i in range(len(self["items"])):
            item = self["items"][i]
            self.createcomponent("item"+str(i), (), "items",
                                 DirectButton, (),
                                 parent = self.component("itemFrame"),
                                 text = item,
                                 command = self["command"],
                                 extraArgs = [item] + self["extraArgs"][i],
                                 )
            
        self.initialiseoptions(DirectScrolledList)

