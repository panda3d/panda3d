"""
Simple console widget for RmlUi
"""
import html


class Console(object):
    def __init__(self, base, doc, cols, rows, commandHandler):
        self.base = base

        self.document = doc
        self.cols = cols
        self.rows = rows
        self.commandHandler = commandHandler

        self.consolePrompt = "C:\\>"
        self.input = ""
        self.lastLine = None
        self.blinkState = False

        self._lines = []

        el = self.document.get_element_by_id('content')
        self.textEl = el

        self.allowEditing(True)

    def getTextContainer(self):
        return self.textEl

    def setPrompt(self, prompt):
        self.consolePrompt = prompt

    def allowEditing(self, editMode):
        self.editMode = editMode
        if editMode:
            self.input = ""
            if not self._lines:
                self.addLine("")
            self.newEditLine()

    def addLine(self, text):
        self._lines.append(text)
        while len(self._lines) > self.rows:
            self._lines.pop(0)
        self.lastLine = text
        self._render()

    def addLines(self, lines):
        for line in lines:
            self.addLine(line)

    def updateEditLine(self, newInput=''):
        self.input = newInput
        self._render()

    def _render(self):
        parts = []
        for line in self._lines:
            parts.append(html.escape(line))
        if self.editMode:
            newText = self.consolePrompt + self.input
            parts.append(html.escape(newText) + (self.blinkState and '_' or ''))
        self.textEl.set_inner_rml("\n".join(parts))

    def scroll(self):
        self.blinkState = False
        self.addLine(self.consolePrompt + self.input)

    def queueBlinkCursor(self):
        self.base.taskMgr.doMethodLater(0.2, self.blinkCursor, 'blinkCursor')

    def blinkCursor(self, task):
        self.blinkState = not self.blinkState
        if self.editMode:
            self._render()
        self.queueBlinkCursor()
        return task.done

    def newEditLine(self):
        self.updateEditLine()
        self.queueBlinkCursor()

    def cls(self):
        self._lines = []
        self.lastLine = None
        self._render()

    # ------------------------------------------------------------------
    # Keyboard input — called by main.py via Panda3D key events
    # (replaces the keydown/textinput RmlUi JS event listeners from libRocket)

    def handleChar(self, keyname):
        if not self.editMode:
            return
        if len(keyname) == 1 and 32 <= ord(keyname) < 127:
            self.updateEditLine(self.input + keyname)

    def handleBackspace(self):
        if not self.editMode:
            return
        self.updateEditLine(self.input[:-1])

    def handleEnter(self):
        if not self.editMode:
            return
        # emit line without cursor
        self.scroll()
        # handle command
        self.commandHandler(self.input)
        if self.editMode:
            # start with new "command"
            self.updateEditLine("")

    def handleBreak(self):
        self.commandHandler(None)
