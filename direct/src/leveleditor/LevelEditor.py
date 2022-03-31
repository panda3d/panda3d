"""
This is just a sample code.

LevelEditor, ObjectHandler, ObjectPalette should be rewritten
to be game specific.
"""

from .LevelEditorUI import *
from .LevelEditorBase import *
from .ObjectMgr import *
from .AnimMgr import *
from .ObjectHandler import *
from .ObjectPalette import *
from .ProtoPalette import *

class LevelEditor(LevelEditorBase):
    """ Class for Panda3D LevelEditor """
    def __init__(self):
        LevelEditorBase.__init__(self)

        # define your own config file similar to this
        self.settingsFile = os.path.dirname(__file__) + '/LevelEditor.cfg'

        # If you have your own ObjectPalette and ObjectHandler
        # connect them in your own LevelEditor class
        self.objectMgr = ObjectMgr(self)
        self.animMgr = AnimMgr(self)
        self.objectPalette = ObjectPalette()
        self.objectHandler = ObjectHandler(self)
        self.protoPalette = ProtoPalette()

        # Populating uderlined data-structures
        self.ui = LevelEditorUI(self)
        self.ui.SetCursor(wx.Cursor(wx.CURSOR_WAIT))
        self.objectPalette.populate()
        self.protoPalette.populate()

        # Updating UI-panels based on the above data
        self.ui.objectPaletteUI.populate()
        self.ui.protoPaletteUI.populate()

        # When you define your own LevelEditor class inheriting LevelEditorBase
        # you should call self.initialize() at the end of __init__() function
        self.initialize()
        self.ui.SetCursor(wx.Cursor(wx.CURSOR_ARROW))
