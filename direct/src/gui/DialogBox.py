from DirectObject import *
from ShowBaseGlobal import *
from GuiGlobals import *

import string
import OnscreenText
import Button
import StateData
import OnscreenPanel

# just an OK button
Acknowledge = 1

# OK and CANCEL buttons
TwoChoice = 2

class DialogBox(OnscreenPanel.OnscreenPanel):

    def __init__(self, doneEvent, message = "", style = Acknowledge,
                 font = getDefaultFont(), wordwrap = 12):
	"""___init___(self, Event, string="", int, model, int=12)"""

        self.doneEvent = doneEvent
        self.message = message
        self.style = style
        self.font = font
        self.wordwrap = wordwrap
	self.soundRollover = None
	self.soundOk = None
        self.isLoaded = 0

        # initialize our OnscreenPanel essence
        # NOTE: all db's have the same name so we can kill tem easily
        OnscreenPanel.OnscreenPanel.__init__(self, "globalDialog")
         
	return None

    def show(self):
	"""show(self)
	"""
	if self.isLoaded == 0:
	    self.load()

        OnscreenPanel.OnscreenPanel.show(self)
        
	return None

    def hide(self):
	"""hide(self)
	"""
	if self.isLoaded == 0:
	    return None
        
        OnscreenPanel.OnscreenPanel.hide(self)

	return None
	
    def load(self):
	"""load(self)
	"""
	if self.isLoaded == 1:
	    return None

        # make the panel
        self.makePanel(rect = (-0.5, 0.5, -0.4, 0.4),
                       drawOrder = 32000, font = self.font,
                       bg = (0.8, 0.8, 0.8, 1.0))
        
	# create a message
	self.makeText(self.message, wordwrap = self.wordwrap, scale = 0.08,
                      pos = (0.0, 0.25))

        if (self.style == TwoChoice):
            # create OK and CANCEL buttons
            self.makeButton("OK", pos = (-0.325, -0.25),
                            func = self.__handleOk)
            self.makeButton("CANCEL", pos = (0.2, -0.25),
                            func = self.__handleCancel)
        else:
            # create a centered OK  button
            self.makeButton("OK", pos = (0.0, -0.25), func = self.__handleOk)
            
	self.isLoaded = 1
	return None
	
    def unload(self):
	"""unload(self)
	"""
	if self.isLoaded == 0:
	    return None
	
	self.hide()
        self.cleanup()

	self.isLoaded = 0
	return None

    def __handleRollover(self):
	return None

    def __handleOk(self):
	self.doneStatus = "ok"	
	messenger.send(self.doneEvent)

    def __handleCancel(self):
        self.doneStatus = "cancel"
	messenger.send(self.doneEvent)

    def setMessage(self, message):
        """setMessage(self, string)
        """
        if self.isLoaded == 1:
            self.panelText[0].setText(message)
        
        





