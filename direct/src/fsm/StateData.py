
"""StateData module: contains StateData class"""

from DirectObject import *

import DirectNotifyGlobal

class StateData(DirectObject):

    """StateData class: """

    notify = DirectNotifyGlobal.directNotify.newCategory('StateData')

    def __init__(self, doneEvent):
        """__init__(self, Event)
        """
        self.doneEvent = doneEvent
        self.doneStatus = None
        self.isLoaded = 0
        self.isEntered = 0
        return None
    
    def cleanup(self):
        """cleanup(self)
        """
        print "state data cleanup!!!"
        self.unload()
        return None

    def enter(self):
        """enter(self)
        """
        # Use isEntered to avoid redundant entry work
        if self.isEntered == 1:
            return None
        self.isEntered = 1
        # Use isLoaded to avoid redundant loading
        if self.isLoaded == 0:
            self.load()
        self.notify.debug('enter()')
        return None

    def exit(self):
        """exit(self)
        """
        if self.isEntered == 0:
            return None
        self.isEntered = 0
        self.notify.debug('exit()')
        return None

    def load(self):
        """load(self)
        """
        if self.isLoaded == 1:
            return None
        self.isLoaded = 1
        self.notify.debug('load()')
        return None

    def unload(self):
        """unload(self)
        """
        if self.isLoaded == 0:
            return None
        self.isLoaded = 0
        self.exit()
        self.notify.debug('unload()')
        return None

    def getDoneStatus(self):
        """getDoneStatus(self)
        """
        return self.doneStatus


            
        

    







