from DirectObject import *
from ShowBaseGlobal import *
from GuiGlobals import *

import DirectNotifyGlobal
import string
import OnscreenText
import Button
import StateData
import OnscreenPanel

# No buttons at all
NoButtons = 0

# just an OK button
Acknowledge = 1

# OK and CANCEL buttons
TwoChoice = 2

class DialogBox(OnscreenPanel.OnscreenPanel):

    notify = DirectNotifyGlobal.directNotify.newCategory("DialogBox")

    def __init__(self, message = "", doneEvent = None, style = NoButtons,
                 font = getDefaultFont(), wordwrap = 12, okButtonText = "OK",
                 cancelButtonText = "Cancel"):
        """___init___(self, Event, string="", int, model, int=12)"""

        # Sanity check
        if (doneEvent == None) and (style != NoButtons):
            self.notify.error("Boxes with buttons must specify a doneEvent.")
            
        self.__doneEvent = doneEvent
        self.message = message
        self.style = style
        self.font = font
        self.wordwrap = wordwrap
        self.okButtonText = okButtonText
        self.cancelButtonText = cancelButtonText

        # initialize our OnscreenPanel essence
        # NOTE: all db's have the same name so we can kill them easily
        OnscreenPanel.OnscreenPanel.__init__(self, "globalDialog")
         
        # make the panel
        self.makePanel(rect = (-0.5, 0.5, -0.4, 0.4),
                       drawOrder = 32000, font = self.font,
                       bg = (0.8, 0.8, 0.8, 1.0))
        
        # create a message
        self.makeText(self.message, wordwrap = self.wordwrap, scale = 0.08,
                      pos = (0.0, 0.25))

        if (self.style == TwoChoice):
            # create OK and CANCEL buttons
            self.makeButton(self.okButtonText,
                            pos = (-0.325, -0.25),
                            func = self.handleOk,
                            event = "ok")
            self.makeButton(self.cancelButtonText,
                            pos = (0.2, -0.25),
                            func = self.handleCancel,
                            event = "cancel")
        elif (self.style == Acknowledge):
            # create a centered OK  button
            self.makeButton(self.okButtonText,
                            pos = (0.0, -0.25),
                            func = self.handleOk,
                            event = "ok")
        elif (self.style == NoButtons):
            # No buttons at all
            pass
        else:
            "Sanity check"
            self.notify.error("No such style as: " + str(self.style))
            
        return None

    def show(self):
        """show(self)
        """
        base.transitions.fadeScreen()
        OnscreenPanel.OnscreenPanel.show(self)
        
        return None

    def hide(self):
        """hide(self)
        """
        base.transitions.noTransitions()        
        OnscreenPanel.OnscreenPanel.hide(self)

        return None

    def cleanup(self):
        """unload(self)
        """
        self.hide()
        OnscreenPanel.OnscreenPanel.cleanup(self)

        return None

    # def handleRollover(self):
        # return None

    def handleOk(self, okButton, item):
        assert(self.style != NoButtons)
        if (okButton == item):
            self.doneStatus = "ok"
            messenger.send(self.__doneEvent)

    def handleCancel(self, cancelButton, item):
        assert(self.style == TwoChoice)
        if (cancelButton == item):
            self.doneStatus = "cancel"
            messenger.send(self.__doneEvent)

    def setMessage(self, message):
        """setMessage(self, string)
        """
        self.panelText[0].setText(message)
