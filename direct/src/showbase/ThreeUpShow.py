"""Undocumented Module"""

__all__ = ['ThreeUpShow']


import ShowBase

class ThreeUpShow(ShowBase.ShowBase):
    def __init__(self):
        ShowBase.ShowBase.__init__(self)

    def makeCamera(self, win, chan = None, layer = None, layerSort = 0,
                   scene = None,
                   displayRegion = (0, 1, 0, 1), aspectRatio = None):
        self.camRS=ShowBase.ShowBase.makeCamera(
                self, win, displayRegion = (.5, 1, 0, 1), aspectRatio=.67, camName='camRS')
        self.camLL=ShowBase.ShowBase.makeCamera(
                self, win, displayRegion = (0, .5, 0, .5), camName='camLL')
        self.camUR=ShowBase.ShowBase.makeCamera(
                self, win, displayRegion = (0, .5, .5, 1), camName='camUR')
        return self.camUR

