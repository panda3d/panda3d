
"""StateData module: contains StateData class"""

from DirectObject import *

class StateData(DirectObject):

    """StateData class: """

    def __init__(self, doneEvent):
        """__init__(self, Event)
        """
	self.doneEvent = doneEvent
	self.isLoaded = 0
	self.isEntered = 0
	return None
    
    def cleanup(self):
	"""cleanup(self)
	"""
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
	print "Called StateData enter function"
	return None

    def exit(self):
	"""exit(self)
	"""
	if self.isEntered == 0:
	    return None
	self.isEntered = 0
	print "Called StateData exit function"
	return None

    def load(self):
	"""load(self)
	"""
	if self.isLoaded == 1:
	    return None
	self.isLoaded = 1
	print "Called StateData load function"
	return None
	
    def unload(self):
	"""unload(self)
	"""
	if self.isLoaded == 0:
	    return None
	self.isLoaded = 0
	self.exit()
	print "Called StateData unload function"
	return None


            
        

    







