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
	self.soundRollover = None
	self.soundOk = None
	return None

    def enter(self, message):
	"""enter(self, string)
	"""
	if self.isLoaded == 0:
	    self.load()

	if self.text:
	    self.text.setText(message)
	    self.text.reparentTo(render2d)

	if self.okButton:
	    self.okButton.manage()
	    self.accept("ForceAcknowledge-rollover", self.__handleRollover)
	    self.accept("ForceAcknowledge-ok", self.__handleOk)
	return None

    def exit(self):
	"""exit(self)
	"""
	if self.isLoaded == 0:
	    return None

	self.ignore("ForceAcknowledge-rollover")
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
	self.text = OnscreenText.OnscreenText("", 0.0, 0.25)
        self.text.freeze()
	self.text.node().setAlign(0)
	self.text.node().setTextColor(0.0, 0.0, 0.0, 1.0)
	self.text.node().setFrameColor(1.0, 1.0, 1.0, 1.0)
	self.text.setScale(0.08)
        self.text.thaw()

	# create a button
	self.okButton = Button.Button("ForceAcknowledge", "OK")
	self.okButton.setPos(0.0, -0.5)
	# set rollover event
	self.okButton.button.setUpRolloverEvent("ForceAcknowledge-rollover")
	self.okButton.button.setDownRolloverEvent("ForceAcknowledge-ok")
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
	self.text.removeNode()
	self.text.cleanup()
	self.text = None
	self.okButton.cleanup()
	self.okButton = None
	self.isLoaded = 0
	return None

    def __handleRollover(self):
	return None

    def __handleOk(self):
	self.doneStatus = "ok"	
	messenger.send(self.doneEvent)
