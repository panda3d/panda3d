title = 'Pmw.HistoryText demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create and pack the PanedWidget to hold the query and result
        # windows.
        # !! panedwidget should automatically size to requested size
        panedWidget = Pmw.PanedWidget(parent,
                orient = 'vertical',
                hull_height = 400,
                hull_width = 550)
        panedWidget.add('query', min = 0.05, size = 0.2)
        panedWidget.add('buttons', min = 0.1, max = 0.1)
        panedWidget.add('results', min = 0.05)
        panedWidget.pack(fill = 'both', expand = 1)

        # Create and pack the HistoryText.
        self.historyText = Pmw.HistoryText(panedWidget.pane('query'),
                text_wrap = 'none',
                text_width = 60,
                text_height = 10,
                historycommand = self.statechange,
        )
        self.historyText.pack(fill = 'both', expand = 1)
        self.historyText.component('text').focus()

        buttonList = (
            [20, None],
            ['Clear', self.clear],
            ['Undo', self.historyText.undo],
            ['Redo', self.historyText.redo],
            [20, None],
            ['Prev', self.historyText.prev],
            ['Next', self.historyText.next],
            [30, None],
            ['Execute', Pmw.busycallback(self.executeQuery)],
        )
        self.buttonDict = {}

        buttonFrame = panedWidget.pane('buttons')
        for text, cmd in buttonList:
            if type(text) == type(69):
                frame = tkinter.Frame(buttonFrame, width = text)
                frame.pack(side = 'left')
            else:
                button = tkinter.Button(buttonFrame, text = text, command = cmd)
                button.pack(side = 'left')
                self.buttonDict[text] = button

        for text in ('Prev', 'Next'):
            self.buttonDict[text].configure(state = 'disabled')

        self.results = Pmw.ScrolledText(panedWidget.pane('results'), text_wrap = 'none')
        self.results.pack(fill = 'both', expand = 1)

    def statechange(self, prevstate, nextstate):
        self.buttonDict['Prev'].configure(state = prevstate)
        self.buttonDict['Next'].configure(state = nextstate)

    def clear(self):
        self.historyText.delete('1.0', 'end')

    def addnewlines(self, text):
        if len(text) == 1:
            text = text + '\n'
        if text[-1] != '\n':
            text = text + '\n'
        if text[-2] != '\n':
            text = text + '\n'
        return text

    def executeQuery(self):
        sql = self.historyText.get()
        self.results.insert('end', 'Query:\n' + self.addnewlines(sql))
        self.results.see('end')
        self.results.update_idletasks()
        self.historyText.addhistory()
        results = 'Results:\nfoo'
        if len(results) > 0:
            self.results.insert('end', self.addnewlines(results))
        self.results.see('end')


######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = tkinter.Tk()
    Pmw.initialise(root)
    root.title(title)

    exitButton = tkinter.Button(root, text = 'Exit', command = root.destroy)
    exitButton.pack(side = 'bottom')
    widget = Demo(root)
    root.mainloop()
