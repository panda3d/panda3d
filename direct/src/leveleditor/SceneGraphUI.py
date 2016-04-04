"""
Defines Scene Graph tree UI
"""
from .SceneGraphUIBase import *

class SceneGraphUI(SceneGraphUIBase):
    def __init__(self, parent, editor):
        SceneGraphUIBase.__init__(self, parent, editor)

    def populateExtraMenu(self):
        pass
