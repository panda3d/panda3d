
"""StateData module: contains StateData class"""

from DirectObject import *

class StateData(DirectObject):

    """StateData class: """

    def __init__(self, doneEvent):
        """__init__(self, Event)
        """
	self.doneEvent = doneEvent
	self.isLoaded = 0

    def enter(self):
        """enter(self)"""
	print "Called abstract enter function"

    def exit(self):
	"""exit(self)"""
	print "Called abstract exit function"

    def load(self):
	"""load(self)"""
	print "Called abstract load function"
	
    def unload(self):
	"""unload(self)"""
	print "Called abstract unload function"


            
        

    







