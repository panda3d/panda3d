from LevelEditorUIBase import *

class LevelEditorUI(LevelEditorUIBase):
    """ Class for Panda3D LevelEditor """ 
    frameWidth = 800
    frameHeight = 600
    appversion      = '0.1'
    appname         = 'Panda3D Level Editor'
    
    def __init__(self, editor, *args, **kw):
        if not kw.get('size'):
            kw['size'] = wx.Size(self.frameWidth, self.frameHeight)
        LevelEditorUIBase.__init__(self, editor)
