"""
This is just a sample code.

LevelEditor, ObjectHandler, ObjectPalette should be rewritten
to be game specific.
"""

from LevelEditorBase import *
from ObjectHandler import *
from ObjectPalette import *
from LevelEditorUI import *
from ProtoPalette import *

class LevelEditor(LevelEditorBase):
    """ Class for Panda3D LevelEditor """ 
    def __init__(self):
        LevelEditorBase.__init__(self)

        # define your own config file similar to this
        self.settingsFile = os.path.dirname(__file__) + '/LevelEditor.cfg'

        # If you have your own ObjectPalette and ObjectHandler
        # connect them in your own LevelEditor class
        self.objectPalette = ObjectPalette()
        self.objectHandler = ObjectHandler(self)
        self.protoPalette = ProtoPalette()

        # LevelEditorUI class must declared after ObjectPalette
        self.ui = LevelEditorUI(self)
        
        # When you define your own LevelEditor class inheriting LevelEditorBase
        # you should call self.initialize() at the end of __init__() function
        self.initialize()
