import string
import OnscreenText
import Button
import StateData

from DirectObject import *
from ShowBaseGlobal import *

class ForceAcknowledge(StateData.StateData):

    def __init__(self, doneEvent, message):
	"""___init___(self, Event)"""
	StateData.StateData.__init__(self, doneEvent)
	return None

    def enter(self, message):
	"""enter(self, string)
	"""
	if self.isLoaded == 0:
	    self.load()

	if self.text:
	    self.text.setText(message)
	    self.text.reparentTo(aspect2d)

	if self.okButton:
	    self.okButton.manage()
	    self.accept("ForceAcknowledge-ok", self.__handleOk)
	return None

    def exit(self):
	"""exit(self)
	"""
	if self.isLoaded == 0:
	    return None

	self.ignore("ForceAcknowledge-ok")

	self.text.reparentTo(hidden)
	self.okButton.unmanage()
	return None
	
    def load(self):
	"""load(self)
	"""
	if self.isLoaded == 1:
	    return None

	# create a message
	self.text = OnscreenText.OnscreenText(parent = hidden,
                                              scale = 0.08,
                                              pos = (0.0, 0.25))
	# create a button
	self.okButton = Button.Button("ForceAcknowledge", "OK",
                                      event = "ForceAcknowledge-ok")
	self.okButton.setPos(0.0, -0.5)
	# set rollover event
	self.okButton.button.setPriority(self.okButton.button, GuiItem.PHighest)

	self.isLoaded = 1
	return None
	
    def unload(self):
	"""unload(self)
	"""
	if self.isLoaded == 0:
	    return None
	
	self.exit()

	# GUI
	self.okButton.cleanup()
	self.okButton = None
	self.isLoaded = 0
	return None

    def __handleOk(self, item):
        if (item == self.okButton.button):
            self.doneStatus = "ok"	
            messenger.send(self.doneEvent)
