"""MopathInterval module: contains the MopathInterval class"""

__all__ = ['MopathInterval']

import LerpInterval
from pandac.PandaModules import *
from direct.directnotify.DirectNotifyGlobal import *

# import Mopath

class MopathInterval(LerpInterval.LerpFunctionInterval):
    # Name counter
    mopathNum = 1
    # create MopathInterval DirectNotify category
    notify = directNotify.newCategory('MopathInterval')
    # Class methods
    def __init__(self, mopath, node, fromT = 0, toT = None,
                 duration = None, blendType = 'noBlend', name = None):
        if toT == None:
            toT = mopath.getMaxT()

        if duration == None:
            duration = abs(toT - fromT)
        
        # Generate unique name if necessary
        if (name == None):
            name = 'Mopath-%d' % MopathInterval.mopathNum
            MopathInterval.mopathNum += 1        

        LerpInterval.LerpFunctionInterval.__init__(
            self, self.__doMopath, fromData = fromT, toData = toT,
            duration = duration, blendType = blendType,
            name = name)
        
        self.mopath = mopath 
        self.node = node

    def destroy(self):
        """Cleanup to avoid a garbage cycle."""
        self.function = None

    def __doMopath(self, t):
        """
        Go to time t
        """
        self.mopath.goTo(self.node, t)

