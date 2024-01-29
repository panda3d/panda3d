title = 'Pmw.PanedWidget demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):

        # Create a main PanedWidget with a few panes.
        self.pw = Pmw.PanedWidget(parent,
                orient='vertical',
                hull_borderwidth = 1,
                hull_relief = 'sunken',
                hull_width=300,
                hull_height=400)
        for self.numPanes in range(4):
            if self.numPanes == 1:
                name = 'Fixed size'
                pane = self.pw.add(name, min = .1, max = .1)
            else:
                name = 'Pane ' + str(self.numPanes)
                pane = self.pw.add(name, min = .1, size = .25)
            label = tkinter.Label(pane, text = name)
            label.pack(side = 'left', expand = 1)
            button = tkinter.Button(pane, text = 'Delete',
                    command = lambda s=self, n=name: s.deletePane(n))
            button.pack(side = 'left', expand = 1)
            # TODO: add buttons to invoke self.moveOneUp and self.moveOneUp.

        self.pw.pack(expand = 1, fill='both')

        buttonBox = Pmw.ButtonBox(parent)
        buttonBox.pack(fill = 'x')
        buttonBox.add('Add pane', command = self.addPane)
        buttonBox.add('Move pane', command = self.move)
        self.moveSrc = 0
        self.moveNewPos = 1
        self.moveBack = 0

    def move(self):
        numPanes = len(self.pw.panes())
        if numPanes == 0:
            print('No panes to move!')
            return

        if self.moveSrc >= numPanes:
            self.moveSrc = numPanes - 1
        if self.moveNewPos >= numPanes:
            self.moveNewPos = numPanes - 1
        print(('Moving pane', self.moveSrc, 'to new position', self.moveNewPos))
        self.pw.move(self.moveSrc, self.moveNewPos)

        self.moveSrc, self.moveNewPos = self.moveNewPos, self.moveSrc
        if self.moveBack:
            if self.moveNewPos == numPanes - 1:
                self.moveNewPos = 0
                if self.moveSrc == numPanes - 1:
                    self.moveSrc = 0
                else:
                    self.moveSrc = self.moveSrc + 1
            else:
                self.moveNewPos = self.moveNewPos + 1
        self.moveBack = not self.moveBack

    def addPane(self):
        self.numPanes = self.numPanes + 1
        name = 'Pane ' + str(self.numPanes)
        print(('Adding', name))
        pane = self.pw.add(name, min = .1, size = .25)
        label = tkinter.Label(pane, text = name)
        label.pack(side = 'left', expand = 1)
        button = tkinter.Button(pane, text = 'Delete',
                command = lambda s=self, n=name: s.deletePane(n))
        button.pack(side = 'left', expand = 1)
        self.pw.updatelayout()

    def deletePane(self, name):
        print(('Deleting', name))
        self.pw.delete(name)
        self.pw.updatelayout()

    def moveOneUp(self, name):
        self.pw.move(name, name, -1)

    def moveOneDown(self, name):
        self.pw.move(name, name, 1)

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
