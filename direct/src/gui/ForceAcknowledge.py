import string
import OnscreenText
import Button
import StateData

from DirectObject import *

class ForceAcknowledge(StateData.StateData):

    def __init__(self, doneEvent, message):
	"""___init___(self, Event)"""
	StateData.__init__(doneEvent)
	self.soundRollover = None
	self.soundOk = None

    def enter(self, message):
	"""enter(self, string)"""
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
	"""exit(self)"""
	self.ignore("ForceAcknowledge-rollover")
	self.ignore("ForceAcknowledge-ok")

	self.text.reparentTo(hidden)
	self.okButton.unmanage()
	
    def load(self):
	"""load(self)"""
	# create a message
	self.text = OnscreenText.OnscreenText("", 0.0, 0.5)
	self.text.node().setAlign(0)
	self.text.node().setTextColor(0.0, 0.0, 0.0, 1.0)
	self.text.node().setFrameColor(1.0, 1.0, 1.0, 1.0)
	self.text.setScale(0.08)

	# create a button
	self.okButton = Button.Button("ForceAcknowledge", "OK")
	self.okButton.setPos(0.0, -0.5)
	# set rollover event
	self.okButton.setUpRolloverEvent("ForceAcknowledge-rollover")
	self.okButton.setDownRolloverEvent("ForceAcknowledge-ok")

	self.exit()
	self.isLoaded = 1
	
    def unload(self):
	"""unload(self)"""
	# GUI
	self.title.removeNode()
	del(self.okButton)
	self.isLoaded = 0

    def __handleRollover(self):
	return None

    def __handleOk(self):
	messenger.send(self.doneEvent, [0])
