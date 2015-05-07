"""
Simple console widget for rocket
"""
import sys, os.path

# workaround: https://www.panda3d.org/forums/viewtopic.php?t=10062&p=99697#p99054
#from panda3d import rocket
import _rocketcore as rocket

from panda3d.rocket import RocketRegion, RocketInputHandler

class Console(object):
    def __init__(self, base, context, cols, rows, commandHandler):
        self.base = base

        self.context = context
        self.loadFonts()
        self.cols = cols
        self.rows = rows
        self.commandHandler = commandHandler

        self.setupConsole()
        self.allowEditing(True)

    def getTextContainer(self):
        return self.textEl

    def setPrompt(self, prompt):
        self.consolePrompt = prompt

    def allowEditing(self, editMode):
        self.editMode = editMode
        if editMode:
            self.input = ""
            if not self.lastLine:
                self.addLine("")
            self.newEditLine()

    def loadFonts(self):
        rocket.LoadFontFace("Perfect DOS VGA 437.ttf")

    def setupConsole(self):
        self.document = self.context.LoadDocument("console.rml")
        if not self.document:
            raise AssertionError("did not find console.rml")

        el = self.document.GetElementById('content')

        self.textEl = el

        # roundabout way of accessing the current object through rocket event...

        # add attribute to let Rocket know about the receiver
        self.context.console = self

        # then reference through the string format (dunno how else to get the event...)
        self.document.AddEventListener(
            'keydown', 'document.context.console.handleKeyDown(event)', True)
        self.document.AddEventListener(
            'textinput', 'document.context.console.handleTextInput(event)', True)

        self.consolePrompt = "C:\\>"

        self.input = ""
        self.lastLine = None

        self.blinkState = False
        self.queueBlinkCursor()

        self.document.Show()

    def queueBlinkCursor(self):
        self.base.taskMgr.doMethodLater(0.2, self.blinkCursor, 'blinkCursor')

    def blinkCursor(self, task):
        self.blinkState = not self.blinkState
        if self.editMode:
            self.updateEditLine(self.input)
        self.queueBlinkCursor()

    def escape(self, text):
        return text. \
                replace('<', '&lt;'). \
                replace('>', '&gt;'). \
                replace('"', '&quot;')

    def addLine(self, text):
        curKids = list(self.textEl.child_nodes)
        while len(curKids) >= self.rows:
            self.textEl.RemoveChild(curKids[0])
            curKids = curKids[1:]

        line = self.document.CreateTextNode(self.escape(text) + '\n')
        self.textEl.AppendChild(line)
        self.lastLine = line

    def addLines(self, lines):
        for line in lines:
            self.addLine(line)

    def updateEditLine(self, newInput=''):
        newText = self.consolePrompt + newInput
        self.lastLine.text = self.escape(newText) + (self.blinkState and '_' or '')
        self.input = newInput

    def scroll(self):
        self.blinkState = False
        self.updateEditLine(self.input + '\n')

    def handleKeyDown(self, event):
        """
        Handle control keys
        """
        keyId = event.parameters['key_identifier']
        if not self.editMode:
            if keyId == rocket.key_identifier.PAUSE:
                if event.parameters['ctrl_key']:
                    self.commandHandler(None)

            return

        if keyId == rocket.key_identifier.RETURN:
            # emit line without cursor
            self.scroll()

            # handle command
            self.commandHandler(self.input)

            if self.editMode:
                # start with new "command"
                self.addLine(self.consolePrompt)
                self.updateEditLine("")

        elif keyId == rocket.key_identifier.BACK:
            self.updateEditLine(self.input[0:-1])

    def handleTextInput(self, event):
        if not self.editMode:
            return

        # handle normal text character
        data = event.parameters['data']
        if 32 <= data < 128:
            self.updateEditLine(self.input + chr(data))

    def newEditLine(self):
        self.addLine("")
        self.updateEditLine()

    def cls(self):
        curKids = list(self.textEl.child_nodes)
        for kid in curKids:
            self.textEl.RemoveChild(kid)